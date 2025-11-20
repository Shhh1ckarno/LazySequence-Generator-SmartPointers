

#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include <stdexcept>
#include <string>
#include <iostream>
template <typename T>
struct Node{
    T key;
    Node<T>* next;
    Node<T>* previous;
    explicit Node(const T& k);
};
template <typename T>
class LinkedList {
public:
    LinkedList();
    LinkedList(T* items, int count);
    LinkedList(const LinkedList<T>& list);
    ~LinkedList();
    void Append(const T& value);
    void Prepend(const T& value);
    void Insert(const T& value,int index);
    void Remove(int index);
    T Get(int index) const;
    int GetLength() const;
    T GetFirst() const;
    T GetLast() const;
    LinkedList<T>* Concat(const LinkedList<T>& list);
    LinkedList<T>* GetSublist(int startIndex, int endIndex);
private:
    Node<T>* root;
    int size;
};
template <typename T>
Node <T>::Node(const T& k):key(k),next(nullptr),previous(nullptr){};
template <typename T>
LinkedList<T>::LinkedList():root(nullptr),size(0) {}
template <typename T>
LinkedList<T>::LinkedList(T* items, int count) : root(nullptr), size(0) {
    if (count<0){
        throw std::invalid_argument("Count must be positive");
    }
    for (int i = 0; i < count; ++i) {
        Append(items[i]);
    }
}
template <typename T>
LinkedList<T>::LinkedList(const LinkedList<T> &list) {
    root= nullptr;
    size=0;
    if (list.root==nullptr){
        return;
    }
    root=new Node<T>(list.root->key);
    size=1;
    Node<T>* now=root;
    Node<T>* n=list.root->next;
    while (n!= nullptr){
        now->next=new Node<T>(n->key);
        now->next->previous=now;
        now=now->next;
        n=n->next;
        size+=1;
    }
}
template <typename T>
LinkedList<T>* LinkedList<T>::Concat(const LinkedList<T>& list) {
    auto* result = new LinkedList<T>(*this);
    Node<T>* current = list.root;
    while (current) {
        result->Append(current->key);
        current = current->next;
    }
    return result;
}
template <typename T>
LinkedList<T>* LinkedList<T>::GetSublist(int startIndex, int endIndex) {
    if (startIndex >= size || startIndex < 0 || endIndex < 0 || endIndex >= size || startIndex > endIndex) {
        throw std::out_of_range("Index out of range");
    }

    auto* result = new LinkedList<T>();
    Node<T>* now = root;
    for (int i = 0; i < startIndex; ++i) {
        now = now->next;
    }
    for (int i = startIndex; i <= endIndex; ++i) {
        result->Append(now->key);
        now = now->next;
    }
    return result;
}
template <typename T>
LinkedList<T>::~LinkedList(){
    Node<T>* now=root;
    while (now!= nullptr){
        Node<T>* n=now->next;
        delete now;
        now=n;
    }
}
template <typename T>
void LinkedList<T>::Append(const T& value){
    auto* NewNode=new Node<T>(value);
    if (root==nullptr){
        root=NewNode;
    }
    else{
        Node<T>* now=root;
        while(now->next!= nullptr){
            now=now->next;
        }
        now->next=NewNode;
        NewNode->previous=now;
    }
    size+=1;
};
template <typename T>
void LinkedList<T>::Prepend(const T& value){
    auto* NewNode=new Node<T>(value);
    if (root!=nullptr) {
        NewNode->next = root;
        root->previous = NewNode;
    }
    root=NewNode;
    size+=1;
}
template <typename T>
void LinkedList<T>::Insert(const T &value, int index) {
    if (index>size or index<0){
        throw std::out_of_range("Index out of range");
    }
    if (index==0){
        Prepend(value);
        return;
    }
    if (index==size){
        Append(value);
        return;
    }
    auto* NewNode=new Node<T>(value);
    int index_now=0;
    Node<T>* now=root;
    while(index_now!=(index-1)){
        now=now->next;
        index_now+=1;
    }
    Node<T>* n=now->next;
    now->next=NewNode;
    NewNode->previous=now;
    NewNode->next=n;
    n->previous=NewNode;
    size+=1;
}
template <typename T>
T LinkedList<T>::Get(int index) const{
    if (index<0 or index>=size){
        throw std::out_of_range("Index out of range");
    }
    else{
        int index_now=0;
        Node<T>* n=root;
        while (index_now!= index){
            n=n->next;
            index_now+=1;
        }
        return n->key;
    }
}
template <typename T>
void LinkedList<T>::Remove(int index){
    if (index<0 or index>=size){
        throw std::out_of_range("Index out of range");
    }
    else{
        if (size == 0) {
            throw std::underflow_error("List is empty");
        }
        int index_now=0;
        Node<T>* n=root;
        if (index==0){
            root=n->next;
            if (root!=nullptr){
                root->previous=nullptr;
            }
            delete n;
            size-=1;
            return;
        }
        while (index_now!=index){
            n=n->next;
            index_now+=1;
        }
        if (n->next != nullptr) {
            n->next->previous = n->previous;
        }
        if (n->previous != nullptr) {
            n->previous->next = n->next;
        }

        delete n;
        size--;
    }
}
template <typename T>
int LinkedList<T>::GetLength() const {
    return size;
}
template <typename T>
T LinkedList<T>::GetFirst() const{
    return Get(0);
}
template <typename T>
T LinkedList<T>::GetLast() const{
    return Get(size-1);
}
#endif
