Problem Statement: Build a Simplified Electronic Exchange

Implement a simplified limit order book and matching engine similar to those used in electronic exchanges.

The system receives a stream of events and maintains an order book for a single instrument.

Your implementation should support:

Adding new limit orders
Cancelling existing orders
Modifying existing orders
Matching buy and sell orders using price-time priority
Generating trades
Maintaining the remaining resting orders
Computing trading P&L from executed trades
Part 1 — Order Book

Each order contains

OrderID
Timestamp
Side (BUY / SELL)
Price
Quantity

Assume

OrderID is unique.
Timestamps are strictly increasing.
Price is an integer.
Quantity is positive.
Supported operations
Add
ADD OrderID Side Price Qty

Example

ADD 101 BUY 100 50

If the order cannot be completely matched, the remaining quantity rests in the book.

Cancel
CANCEL OrderID

Remove the order if it exists.

Cancelling a non-existent order should do nothing.

Modify
MODIFY OrderID NewPrice NewQty

Modification loses time priority.

Treat it exactly as

Cancel

↓

Insert new order

with the same OrderID.

Part 2 — Matching Engine

Whenever a new order arrives:

For BUY

Incoming Buy Price >= Best Ask

means a trade occurs.

For SELL

Incoming Sell Price <= Best Bid

means a trade occurs.

Matching Rules

Orders match using

Best price
FIFO within same price

Price priority

BUY

Highest price first

SELL

Lowest price first

Time priority

Earlier order wins.

Trade Price

Trade occurs at

Resting order price

Example

Book

SELL 100 @101

Incoming

BUY 105

Trade price

101
Partial fills

Support

BUY 100

SELL 30

Remaining

BUY 70

Example

Book

BUY 100 x 40
BUY 100 x 60

Incoming

SELL 100 x 70

Trades

40

30

Remaining

BUY 100 x30
Trade Generation

Each execution generates

BuyOrderID

SellOrderID

Price

Quantity

Timestamp

Store every trade.

Part 3 — Queries

Support

Best Bid

Return

Price

Total Quantity

Example

101

250
Best Ask

Return

102

175
Print Order Book

Example

ASK

103 100

102 250

--------------

101 300

100 75

BID
Part 4 — Position & PnL

Given the generated trade stream and a current market price,

compute

Net Position

Example

Bought

150

Sold

80

Position

+70
Average Entry Price

Weighted average of open position.

Example

Bought

100 @100

100 @110

Average

105
Realized PnL

Profit from closed positions.

Example

Buy

100 @100

Sell

100 @120

Realized

2000
Unrealized PnL

Using current market price

Example

Position

100 @100

Current

110

Unrealized

1000
Constraints
Number of events ≤ 10^6

Order IDs unique

Price ≤ 10^6

Qty ≤ 10^6

Your implementation should aim for approximately:

Add: O(log P) plus the work required to match trades
Cancel: O(1) lookup + O(1) removal from a price level (using appropriate iterators or links)
Modify: Cancel + Add
Best Bid/Ask: O(1) or O(log P)

where P is the number of active price levels.

Bonus (HFT-Level Extensions)

If you finish the core engine, add these one by one:

Market orders.
Iceberg orders.
Immediate-Or-Cancel (IOC).
Fill-Or-Kill (FOK).
Good-Till-Cancel (GTC).
Self-trade prevention.
Trade IDs and sequence numbers.
Replace operations that preserve priority when only quantity decreases.
Multi-symbol support.
A lock-free inbound queue feeding a single-threaded matching engine.
Market data snapshots and incremental updates.
Persistence via an append-only event log and replay.
