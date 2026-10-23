# Market Data Replay and Recovery Simulator

A C++23 learning project that models a small part of an exchange market-data
feed and a client consuming it. The exchange-side demo publishes binary market
events over UDP, retains the encoded events in a bounded history, and serves
missing events over TCP. The client detects gaps in sequence numbers, buffers
out-of-order events, recovers gaps, and applies ordered events to a local
order book.

This is a local simulation, not an exchange implementation. The client order
book is updated from market-data events using `addRestingOrder`; it does not
match the received orders.

## Architecture

```text
                           Exchange-side process

FeedProducer -> encode FeedPacket -> FeedHistory -> UDP publisher
                                      |
                                      +-> TCP recovery server

                            Client-side process

UDP receiver -> decode -> pending packet buffer -> sequencer
                                              -> MarketDataApplier -> Orderbook
                                  ^
                                  |
                    TCP recovery client requests missing sequence range
```

The demo intentionally omits UDP sequences 5 and 6. When the client receives
sequence 7, it requests sequences 5--6 from the TCP recovery server. Those
events are then sequenced and applied before the buffered sequence 7.

## Implemented components

- A binary `FeedPacket` protocol with magic, version, sequence number,
  message type, payload length, and network-byte-order encoding.
- `AddOrder` and `CancelOrder` event payload codecs.
- UDP feed publishing to the local loopback interface (`127.0.0.1:5000`).
- TCP recovery on port `5001`, using length-prefixed encoded packets.
- A mutex-protected, power-of-two rolling `FeedHistory` for recovery lookup.
- Fixed-size pending packet buffer (`2^20` slots) and ordered draining.
- Validation for malformed packets, invalid payloads, old packets, and packets
  outside the receiver buffer window.
- A local market-data applier that updates the existing `OrderBook_v2` book.
- Unit tests for the protocol, feed history, packet sequencer, applier, and
  receiver behavior.

## Build and run

From this directory:

```bash
make
```

Start the client first, then the exchange-side server in another terminal:

```bash
# Terminal 1
./udp_receiver

# Terminal 2
./market_data_server
```

The example feed contains ten events and the receiver exits after applying
sequence 10. `market_data_server` keeps its TCP recovery thread alive, so stop
it with `Ctrl-C` after the demo completes.

Run all unit tests with:

```bash
make test
```

## Benchmark

Run the benchmark with:

```bash
make bench
```

The benchmark compiles with `-O3 -DNDEBUG -march=native` and records one
million individual latency samples for each workload. Packet generation and
allocation happen before the timed region. Each timed sample covers:

```text
encoded packet bytes -> decode -> receiver buffer/sequencer
                     -> MarketDataApplier -> local Orderbook update
```

It **does not** include UDP/TCP socket operations, kernel scheduling, TCP
connection setup, or NIC latency. The reordered workload models out-of-order
arrival and pending-buffer draining in-process; it does not perform a real TCP
recovery request per sample.

### Results

Measured on 2026-10-13 on macOS/Darwin arm64 using Apple Clang 21.0.0
(`g++` driver). Results naturally vary with machine load and should be
re-measured on the target machine.

| Workload | Samples | Average | Median | p99 | p99.9 |
| --- | ---: | ---: | ---: | ---: | ---: |
| Add-only client processing | 1,000,000 | 144.135 ns | 125 ns | 917 ns | 1,542 ns |
| Cancel-only client processing | 1,000,000 | 114.216 ns | 125 ns | 209 ns | 1,291 ns |
| Mixed add/cancel client processing | 1,000,000 | 347.723 ns | 208 ns | 1,375 ns | 2,209 ns |
| Reordered recovery client processing | 1,000,000 | 142.380 ns | 125 ns | 250 ns | 1,750 ns |

The mixed workload is deterministic (seed `42`) and produces roughly 70%
adds and 30% cancels while ensuring each cancellation targets an earlier
active order. The reordered workload delivers each ten-event block in the
order `1, 2, 3, 4, 7, 5, 6, 8, 9, 10` relative to the block start, exercising
the pending buffer and ordered drain.

The benchmark also reported rare multi-millisecond maximum samples. Those
outliers are expected in a normal desktop environment because of OS scheduling
and interrupts; median and tail percentiles are more useful than a single
maximum sample for comparing changes.

## Current scope and limitations

- The runnable feed is a small, deterministic local demonstration.
- UDP and TCP handling in the receiver are synchronous; a TCP recovery request
  temporarily stops UDP reads.
- The TCP recovery client uses one short-lived connection per missing range.
- This project does not yet measure end-to-end socket/NIC latency or implement
  asynchronous ingress, multicast, authentication, or production-grade
  resilience.

