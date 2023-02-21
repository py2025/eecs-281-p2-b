// Project identifier: 43DE0E0C4C76BFAA6D8C2F5AEAE0518A9C42CF4E

#ifndef PAIRINGPQ_H
#define PAIRINGPQ_H

#include "Eecs281PQ.h"
#include <deque>
#include <utility>

// A specialized version of the priority queue ADT implemented as a pairing
// heap.
template<typename TYPE, typename COMP_FUNCTOR = std::less<TYPE>>
class PairingPQ : public Eecs281PQ<TYPE, COMP_FUNCTOR> {
    // This is a way to refer to the base class object.
    using BaseClass = Eecs281PQ<TYPE, COMP_FUNCTOR>;

public:
    // Each node within the pairing heap
    class Node {
        public:
            // TODO: After you add add one extra pointer (see below), be sure
            // to initialize it here.
            explicit Node(const TYPE &val)
                : elt{ val }, child{ nullptr }, sibling{ nullptr }, prev{ nullptr }
            {}

            // Description: Allows access to the element at that Node's
            // position.  There are two versions, getElt() and a dereference
            // operator, use whichever one seems more natural to you.
            // Runtime: O(1) - this has been provided for you.
            const TYPE &getElt() const { return elt; }
            const TYPE &operator*() const { return elt; }

            // The following line allows you to access any private data
            // members of this Node class from within the PairingPQ class.
            // (ie: myNode.elt is a legal statement in PairingPQ's add_node()
            // function).
            friend PairingPQ;

        private:
            TYPE elt;
            Node *child;
            Node *sibling;
            Node *prev;
    }; // Node


    // Description: Construct an empty pairing heap with an optional
    //              comparison functor.
    // Runtime: O(1)
    explicit PairingPQ(COMP_FUNCTOR comp = COMP_FUNCTOR()) :
        BaseClass{ comp } {
    } // PairingPQ()


    // Description: Construct a pairing heap out of an iterator range with an
    //              optional comparison functor.
    // Runtime: O(n) where n is number of elements in range.
    template<typename InputIterator>
    PairingPQ(InputIterator start, InputIterator end, COMP_FUNCTOR comp = COMP_FUNCTOR()) :
        BaseClass{ comp } {
        for (auto i = start; i < end; ++i) {
            push(*i);
        }
    } // PairingPQ()


    // Description: Copy constructor.
    // Runtime: O(n)
    PairingPQ(const PairingPQ &other) :
        BaseClass{ other.compare } {
        
        std::deque<Node*> dq;
        dq.push_back(other.root);
        while (!dq.empty()) {
            Node * node = dq.front();
            if (node->sibling)
                dq.push_back(node->sibling);
            if (node->child)
                dq.push_back(node->child);
            push(node->elt);
            dq.pop_front();
        }
    } // PairingPQ()


    // Description: Copy assignment operator.
    // Runtime: O(n)
    PairingPQ &operator=(const PairingPQ &rhs) {
        PairingPQ cp(rhs);
        std::swap(cp.num, this->num);
        std::swap(cp.root, this->root);

        return *this;
    } // operator=()


    // Description: Destructor
    // Runtime: O(n)
    ~PairingPQ() {
        if (!root) return;
        
        std::deque<Node*> dq;
        dq.push_back(root);
        while (!dq.empty()) {
            Node* node = dq.front();
            if (node->sibling)
                dq.push_back(node->sibling);
            if(node->child)
                dq.push_back(node->child);
            dq.pop_front();
            delete node;
        }
    } // ~PairingPQ()


    // Description: Assumes that all elements inside the pairing heap are out
    //              of order and 'rebuilds' the pairing heap by fixing the
    //              pairing heap invariant.  You CANNOT delete 'old' nodes
    //              and create new ones!
    // Runtime: O(n)
    virtual void updatePriorities() {
        if (num <= 1) return;

        std::deque<Node*> dq;
        Node* node = root;
        dq.push_back(node);
        root = nullptr;

        while(!dq.empty()) {
            node = dq.front();
            if(node->sibling)
                dq.push_back(node->sibling);
            if(node->child)
                dq.push_back(node->child);

            node->prev = nullptr;
            node->sibling = nullptr;
            node->child = nullptr;
            if(root == nullptr) root = node;
            else root = meld(node, root);
            dq.pop_front();
        }
    } // updatePriorities()

    // Description: Add a new element to the pairing heap. This is already
    //              done. You should implement push functionality entirely
    //              in the addNode() function, and this function calls
    //              addNode().
    // Runtime: O(1)
    virtual void push(const TYPE &val) {
        addNode(val);
    } // push()


    // Description: Remove the most extreme (defined by 'compare') element
    //              from the pairing heap.
    // Note: We will not run tests on your code that would require it to pop
    // an element when the pairing heap is empty. Though you are welcome to
    // if you are familiar with them, you do not need to use exceptions in
    // this project.
    // Runtime: Amortized O(log(n))
    virtual void pop() {
        --num;
        Node* child = root->child;
        delete root;
        root = nullptr;
        if (!child) return;
        else if (!child->sibling) {
            child->prev = nullptr;
            root = child;
            return;
        }

        std::deque<Node*> dq;
        while (child) {
            Node* node = child;
            child = child->sibling;
            node->sibling = nullptr;
            node->prev = nullptr;
            dq.push_back(node);
        }

        while (dq.size() != 1) {
            Node* a = dq.front();
            dq.pop_front();
            
            Node* b = dq.front();
            dq.pop_front();

            dq.push_back(meld(a, b));
        }

        root = dq.front();
    } // pop()


    // Description: Return the most extreme (defined by 'compare') element of
    //              the pairing heap. This should be a reference for speed.
    //              It MUST be const because we cannot allow it to be
    //              modified, as that might make it no longer be the most
    //              extreme element.
    // Runtime: O(1)
    virtual const TYPE &top() const {
        return root->elt;
    } // top()


    // Description: Get the number of elements in the pairing heap.
    // Runtime: O(1)
    virtual std::size_t size() const {
        return num;
    } // size()

    // Description: Return true if the pairing heap is empty.
    // Runtime: O(1)
    virtual bool empty() const {
        return num == 0;
    } // empty()


    // Description: Updates the priority of an element already in the pairing
    //              heap by replacing the element refered to by the Node with
    //              new_value.  Must maintain pairing heap invariants.
    //
    // PRECONDITION: The new priority, given by 'new_value' must be more
    //              extreme (as defined by comp) than the old priority.
    //
    // Runtime: As discussed in reading material.
    void updateElt(Node* node, const TYPE &new_value) {
        node->elt = new_value;
        if (node == root) return;

        if (!node->prev && !node->sibling) return;

        if (!node->sibling)
            node->prev = nullptr;
        else {
            if (node->prev->child == node)
                node->prev->child = node->sibling;
            else {
                Node* n = node->prev->child;
                while (n->sibling != node)
                    n = n->sibling;
                n->sibling = node->sibling;
            }
            node->sibling = nullptr;
            node->prev = nullptr;
        }
        root = meld(root, node);
    } // updateElt()


    // Description: Add a new element to the pairing heap. Returns a Node*
    //              corresponding to the newly added element.
    // Runtime: O(1)
    // NOTE: Whenever you create a node, and thus return a Node *, you must
    //       be sure to never move or copy/delete that node in the future,
    //       until it is eliminated by the user calling pop(). Remember this
    //       when you implement updateElt() and updatePriorities().
    Node* addNode(const TYPE &val) {
        ++num;
        Node* node = new Node(val);
        if (!root) root = node;
        else root = meld(node, root);
        return node;
    } // addNode()


private:
    // TODO: Add any additional member variables or member functions you
    // require here.
    // TODO: We recommend creating a 'meld' function (see the Pairing Heap
    // papers).
    Node* root = nullptr;
    uint32_t num = 0;

    Node* meld(Node* r1, Node* r2) {
        if (this->compare(r1->elt, r2->elt)) {
            r1->prev = r2;
            r1->sibling = r2->child;
            r2->child = r1;
            return r2;
        }
        else {
            r2->prev = r1;
            r2->sibling = r1->child;
            r1->child = r2;
            return r1;
        }
    }
    // NOTE: For member variables, you are only allowed to add a "root
    //       pointer" and a "count" of the number of nodes. Anything else
    //       (such as a deque) should be declared inside of member functions
    //       as needed.
};


#endif // PAIRINGPQ_H
