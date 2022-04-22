#pragma once

/**
 * @brief Defines a template class for a node in a singly linked list
 * 
 * This Node simply contains data and the pointer to the next Node
 * 
 * @tparam T Any structure or type (e.g. TimingMetadata)
 */
template <class T>
class Node {
    template<class U> friend class LinkedList; // declare a (template) friend class that can access the private members of Node
    public:
        T data; //**< The data stored in the node */
    private:
        Node<T>* next; //**< The pointer to the next node */
};

/**
 * @brief Implements the singly linked list
 * 
 * This simplifies some code down the line, especially printing out the sampling metadata
 * as LinkedList can store pointers to the metadata and then a single print function can
 * iterate through the entire list to print at once.
 * 
 * @tparam T Any structure or type (e.g. TimingMetadata)
 */
template <class T>
class LinkedList {
    public:
        /**
         * @brief Construct a new Linked List object
         * 
         * By default, the list is empty, with a nullptr for the head, and a length of 0
         */
        LinkedList() : head(nullptr), length(0) {}; // this is a one-liner declaration

        /**
         * @brief Insert a new node (at the head of the list)
         * 
         * That is, given a linked list A -> B -> C, inserting D will yield
         * D -> A -> B -> C
         * 
         * @param data The new data to add to the list
         */
        void insert(T data);
        T get(size_t ind);
        size_t get_length() { return length; };

    private:
        Node<T>* head; //**< The start of the linked list
        size_t length; //**< The number of Nodes in the linked list
};

template <class T>
void LinkedList<T>::insert(T data) {
    Node<T>* new_node = new Node<T>;
    new_node->data = data;
    new_node->next = head;
    ++length;
    head = new_node;
}

template <class T>
T LinkedList<T>::get(size_t ind) {
    if (ind > length) {
        return head->data;
    }
    
    size_t i = 0;
    Node<T>* temp = head;
    while (i++ != ind) {
        temp = temp->next;
    }
    return temp->data;
}