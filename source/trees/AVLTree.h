// AVLTree.h - self-balancing Binary Search Tree (key = int, value = V)
// insert / search in O(log n). Rotations: LL, RR, LR, RL.
#pragma once
#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdlib>

template <typename V>
class AVLTree {
public:
    struct RotationStats {
        int ll = 0, rr = 0, lr = 0, rl = 0;
        int total() const { return ll + rr + lr + rl; }
    };

    AVLTree() = default;
    ~AVLTree() { destroy(root_); }
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    // Returns false (and changes nothing) if the key already exists.
    bool insert(int key, const V& value) {
        bool inserted = false;
        root_ = insertRec(root_, key, value, inserted);
        if (inserted) ++count_;
        return inserted;
    }

    // Returns pointer to stored value, or nullptr if not found. O(log n).
    const V* search(int key) const {
        Node* cur = root_;
        while (cur) {
            if (key == cur->key) return &cur->value;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return nullptr;
    }

    bool contains(int key) const { return search(key) != nullptr; }
    std::size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }
    int height() const { return heightOf(root_); }
    const RotationStats& rotations() const { return rot_; }

    // Verification helpers (used by tests)
    bool isBST() const { return bstCheck(root_, LLONG_MIN, LLONG_MAX); }
    bool isBalanced() const { return balancedHeight(root_) != -1; }

    // Traversals: visitor signature f(int key, const V& value)
    template <typename F> void inorder(F f) const   { inorderRec(root_, f); }
    template <typename F> void preorder(F f) const  { preorderRec(root_, f); }
    template <typename F> void postorder(F f) const { postorderRec(root_, f); }

private:
    struct Node {
        int key;
        V value;
        Node* left = nullptr;
        Node* right = nullptr;
        int height = 1;
        Node(int k, const V& v) : key(k), value(v) {}
    };

    Node* root_ = nullptr;
    std::size_t count_ = 0;
    RotationStats rot_;

    static int heightOf(const Node* n) { return n ? n->height : 0; }
    static void update(Node* n) { n->height = 1 + std::max(heightOf(n->left), heightOf(n->right)); }
    static int balanceOf(const Node* n) { return n ? heightOf(n->left) - heightOf(n->right) : 0; }

    static Node* rotateRight(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        x->right = y;
        update(y);
        update(x);
        return x;
    }
    static Node* rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        update(x);
        update(y);
        return y;
    }

    Node* insertRec(Node* node, int key, const V& value, bool& inserted) {
        if (!node) { inserted = true; return new Node(key, value); }
        if (key < node->key)      node->left  = insertRec(node->left, key, value, inserted);
        else if (key > node->key) node->right = insertRec(node->right, key, value, inserted);
        else { inserted = false; return node; }  // duplicate key

        update(node);
        int b = balanceOf(node);

        if (b > 1 && key < node->left->key) { ++rot_.ll; return rotateRight(node); }          // LL
        if (b < -1 && key > node->right->key) { ++rot_.rr; return rotateLeft(node); }         // RR
        if (b > 1 && key > node->left->key) {                                                  // LR
            ++rot_.lr;
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (b < -1 && key < node->right->key) {                                                // RL
            ++rot_.rl;
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        return node;
    }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    static bool bstCheck(const Node* n, long long lo, long long hi) {
        if (!n) return true;
        if (n->key <= lo || n->key >= hi) return false;
        return bstCheck(n->left, lo, n->key) && bstCheck(n->right, n->key, hi);
    }

    // returns height, or -1 if the subtree is not balanced
    static int balancedHeight(const Node* n) {
        if (!n) return 0;
        int l = balancedHeight(n->left);
        if (l == -1) return -1;
        int r = balancedHeight(n->right);
        if (r == -1) return -1;
        if (std::abs(l - r) > 1) return -1;
        return 1 + std::max(l, r);
    }

    template <typename F> static void inorderRec(const Node* n, F& f) {
        if (!n) return;
        inorderRec(n->left, f);
        f(n->key, n->value);
        inorderRec(n->right, f);
    }
    template <typename F> static void preorderRec(const Node* n, F& f) {
        if (!n) return;
        f(n->key, n->value);
        preorderRec(n->left, f);
        preorderRec(n->right, f);
    }
    template <typename F> static void postorderRec(const Node* n, F& f) {
        if (!n) return;
        postorderRec(n->left, f);
        postorderRec(n->right, f);
        f(n->key, n->value);
    }
};
