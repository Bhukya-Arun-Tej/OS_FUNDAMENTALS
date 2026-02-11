#include<iostream>
#include<utility>
#include<queue>
#include<map>
#include<vector>

using Data = int;
struct Node{
    Data data;
    Node* left;
    Node* right;

    Node(Data data): data(data),left(nullptr), right(nullptr){};
};

class BST{
    private:
    Node* root;
    void print(Node* root);
    Node* deleteUtil_recursive(Node* root, Data data);
    void deleteUtil_iterative(Node* &root, Data data);
    void printLevelOrder(Node* root) const;
    void prune(Node* root);

    
    public:
    BST(): root(nullptr){};
    ~BST(){
        prune(root);
        root = nullptr;
    }
    void insertNode(Data data);
    void deleteNode(Data data);
    void print();
    void printLevelOrder();
    Node* find(Data data);
    
};

void BST:: prune(Node* root){
    if(!root)return;
    prune(root->left);
    prune(root->right);
    delete(root);
}

void BST:: printLevelOrder(){
    printLevelOrder(root);
}
void BST:: printLevelOrder(Node* root) const {
    if(!root)return;
    std::queue<Node*> q;
    q.push(root);
    while(!q.empty() ){
        int size = q.size();
        for(int i=0;i<size;i++){
            Node* temp = q.front();
            q.pop();
            std::cout<<temp->data<<" ";
            if(temp->left){
                q.push(temp->left);
            }
            if(temp->right){
                q.push(temp->right);
            }
        }
        std::cout<<"\n";
    } 
}

Node* BST:: find(Data data){
    Node* travel = root;
    while(travel){
        if(travel->data == data){
            return travel;
        }
        if(data<travel->data){
            travel = travel->left;
        }
        else{
            travel = travel->right;
        }
    }
    return nullptr;
}

void BST:: insertNode(Data data){
    if(root == nullptr){
        Node* temp = new Node(data);
        root = temp;
        return;
    }

    Node* parent = nullptr;
    Node* travel=root;
    while(travel){
        if(travel->data == data){
            throw std::runtime_error("Node already exists");
            // return;
        }
        parent = travel;
        if(data<travel->data){
            travel = travel->left;
        }
        else{
            travel = travel->right;
        }
    }

    Node* temp = new Node(data);
    if(data<parent->data){
        parent->left =temp;
    }
    else{
        parent->right = temp;
    }
}

void BST:: print(){
    print(root);
    std::cout<<"\n";
}

void BST:: print(Node* root){
    if(!root)return;
    print(root->left);
    std::cout<<root->data<< " ";
    print(root->right);
}

void BST:: deleteNode(Data data){
    // deleteUtil_recursive(root, data);
    deleteUtil_iterative(root,data);
}

Node* BST:: deleteUtil_recursive(Node* root, Data data){
    // std::cout<<"In deletion - root is "<<root->data<<"\n";
    //     print();
    if(!root){
        return nullptr;
    }
    if(data < root->data){
        root->left = deleteUtil_recursive(root->left,data);
    }
    else if(data>root->data){
        root->right = deleteUtil_recursive(root->right,data);
    }
    else if(data == root->data){
        if(root->left and root->right){
            Node* succ = root->right;
            while(succ->left){
                succ=succ->left;
            }
            root->data = succ->data;
            root->right = deleteUtil_recursive(root->right,succ->data);
            return root;
        }
        if(!root->left){
            Node* temp = root->right;
            delete(root);
            return temp;
        }
        if(!root->right){
            Node* temp = root->left;
            delete(root);
            return temp;
        }

        delete(root);
        return nullptr;
    }
    return root;
}

void BST :: deleteUtil_iterative(Node* & root, Data data){
    Node* parent = nullptr;
    Node* temp = root;
    while(temp){
        if(temp->data == data){
            break;
        }
        parent = temp;
        if(data<temp->data){
            temp = temp->left;
        }
        else{
            temp=temp->right;
        }
    }
    if(temp==nullptr){
        std::cout<<"Nothin to delete\n";return;
    }

    if(temp->left and temp->right){
        Node* succ = temp->right;
        Node* succParent = temp;
        while(succ and succ->left){
            succParent = succ;
            succ = succ->left;
        }

        temp->data = succ->data;
        if(succParent == temp){
            succParent->right = succ->right;
            delete(succ);
        }
        else{
            succParent->left = succ->right;
        }
        return;
    }

    if(!temp->left){
        if(parent == nullptr){
            Node* t = root->right;
            delete(root);
            root = t;return;
        }
        if(parent->left == temp){
            parent->left = temp->right;
        }
        else{
            parent->right = temp->right;
        }
        delete(temp);
        return;
    }

    if(!temp->right){
        if(parent==nullptr){
            // std::cout<<"left only for root\n";
            Node* t = root->left;
            delete(root);
            root =t;
            return;
        }
        if(parent->left == temp){
            parent->left = temp->left;
        }
        else{
            parent->right = temp->left;
        }
        delete(temp);
        return;
    }


}

int main(){
    BST tree;
    tree.insertNode(100);
    tree.insertNode(110);
    tree.insertNode(70);
    tree.insertNode(90);
    tree.insertNode(80);
    tree.insertNode(86);
    tree.insertNode(95);
    tree.print();
    tree.printLevelOrder();

    if(tree.find(80)){
        std::cout<<"found 80 \n";
    }

    if(!tree.find(81)){
        std::cout<<"81 not found \n";
    }

    tree.deleteNode(90);
    tree.printLevelOrder();
}