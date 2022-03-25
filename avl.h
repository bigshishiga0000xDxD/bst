#pragma once

#include <algorithm>

template<class T>
class Set {
public:
    Set<T>() {}

    template<typename iterator>
    Set<T>(iterator begin, iterator end) {
        for (; begin != end; ++begin) {
            insert(*begin);
        }
    }

    Set<T>(std::initializer_list<T> list) {
        for (T x : list) {
            insert(x);
        }
    }

    Set<T>(const Set<T>& other) {
        root_ = copy(other.root_); 
        begin_ = root_ ? iterator(find_min(root_), *this) : iterator(*this);
        size_ = other.size_;
    }

    Set<T>& operator=(const Set<T>& other) {
        if (this != &other) {
            free(root_);
            root_ = copy(other.root_);
            begin_ = root_ ? iterator(find_min(root_), *this) : iterator(*this);
            size_ = other.size_;
        }
        return *this;
    }

    ~Set<T>() {
        free(root_);
    }

    void insert(const T& key) {
        root_ = insert(root_, key);
        root_->parent = nullptr;
    }

    void erase(const T& key) {
        root_ = erase(root_, key);
        if (root_ != nullptr) {
            root_->parent = nullptr;
        }
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }
private:
    struct Node {
        Node *left = nullptr;
        Node *right = nullptr;
        Node *parent = nullptr;
        int height = 0;
        T value = T{};

        explicit Node(T _value) : value(_value) {}
        Node(const Node& other) : height(other.height), value(other.value) {}
    };

public:
    class iterator {
    private:
        Node *ptr = nullptr;
        const Set<T>* parent = nullptr;

    public:        
        iterator() {}
        iterator(const Set<T>& set) : parent(&set) {}
        iterator(Node *u, const Set<T>& set) : ptr(u), parent(&set) {}
    
        const T operator*() const {
            return ptr->value;
        }

        const T* operator->() const {
            return &ptr->value;
        }

        bool operator==(const iterator& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const iterator& other) const {
            return ptr != other.ptr;
        }

        iterator& operator++() {
            if (ptr->right != nullptr) {
                ptr = ptr->right;
                while (ptr->left != nullptr) {
                    ptr = ptr->left; 
                }
            } else {
                while (ptr->parent != nullptr && ptr->parent->right == ptr) {
                    ptr = ptr->parent; 
                }
                ptr = ptr->parent;
            }
            return *this;
        }

        iterator operator++(int) {
            auto temp = *this;
            ++(*this);
            return temp;
        }

        iterator& operator--() {
            if (ptr == nullptr) {
                ptr = parent->root_;
                while (ptr->right != nullptr) {
                    ptr = ptr->right;
                }
            } else if (ptr->left != nullptr) {
                ptr = ptr->left;
                while (ptr->right != nullptr) {
                    ptr = ptr->right;
                }
            } else {
                while (ptr->parent != nullptr && ptr->parent->left == ptr) {
                    ptr = ptr->parent;
                }
                ptr = ptr->parent;
            }
            return *this;
        }

        iterator operator--(int) {
            auto temp = *this;
            --(*this);
            return temp;
        }
    };

    iterator begin() const {
        return begin_;
    }

    iterator end() const {
        return iterator(*this);
    }

    iterator find(const T& key) const {
        return iterator(find(root_, key), *this); 
    }

    iterator lower_bound(const T& key) const {
        if (size_ == 0) return iterator(*this);
        Node *ptr = lower_bound(root_, key);
        if (ptr->value < key) {
            return ++iterator(ptr, *this);
      } else {
            return iterator(ptr, *this);
        }
    }

private:
    Node *root_= nullptr;
    size_t size_ = 0;

    iterator begin_ = iterator(*this);

    inline int get_height(Node *u) const {
        return u ? u->height : 0;
    }

    inline int get_diff(Node *u) const {
        return get_height(u->left) - get_height(u->right);
    }

    inline void update(Node *u) {
        u->height = std::max(get_height(u->left), get_height(u->right)) + 1;
    }

    inline void link_left(Node *u, Node *v) {
        u->left = v;
        if (v) v->parent = u;
    }

    inline void link_right(Node *u, Node *v) {
        u->right = v;
        if (v) v->parent = u;
    }

    Node* rotate_right(Node *u) {
        Node *res = u->left;
        link_left(u, res->right);
        link_right(res, u);

        update(u);
        update(res);
        return res;
    }

    Node* rotate_left(Node *u) {
        Node *res = u->right;
        link_right(u, res->left);
        link_left(res, u);

        update(u);
        update(res);
        return res;
    }

    Node* rotate_left_big(Node *u) {
        link_right(u, rotate_right(u->right));
        return rotate_left(u);
    }

    Node* rotate_right_big(Node *u) {
        link_left(u, rotate_left(u->left));
        return rotate_right(u);
    }

    Node* rebalance(Node *u) {
        if (abs(get_diff(u)) < 2) return u;

        if (get_diff(u) == -2) {
            if (get_diff(u->right) <= 0) {
                return rotate_left(u);
            } else {
                return rotate_left_big(u);
            }
        } else {
            if (get_diff(u->left) >= 0) {
                return rotate_right(u);
            } else {
                return rotate_right_big(u);
            }
        }
    }

    Node* insert(Node *u, const T& key) {
        if (!u) {
            ++size_;
            Node *v = new Node(key);
            if (begin_ == iterator(*this) || key < *begin_) {
                begin_ = iterator(v, *this);
            }
            return v;
        } else if (key < u->value) {
            link_left(u, insert(u->left, key));
        } else if (u->value < key) {
            link_right(u, insert(u->right, key));
        } else {
            return u;
        }
        update(u);
        return rebalance(u);
    }

    Node* erase(Node *u, const T& key) {
        if (!u) {
            return u;
        } else if (key < u->value) {
            link_left(u, erase(u->left, key));
        } else if (u->value < key) {
            link_right(u, erase(u->right, key));
        } else {
            --size_;
            if (iterator(u, *this) == begin_) {
                ++begin_;
            }

            if (u->right == nullptr) {
                Node *v = u->left;
                delete u;
                return v;
            } else {
                Node *v = find_min(u->right);
                link_right(v, remove_min(u->right));
                link_left(v, u->left);
                update(v);
                delete u;
                return rebalance(v);
            }
        }
        update(u);
        return rebalance(u);
    }

    Node* find(Node *u, const T& key) const {
        if (!u) {
            return nullptr;
        } else if (key < u->value) {
            return find(u->left, key);
        } else if (u->value < key) {
            return find(u->right, key);
        } else {
            return u;
        }
    }

    Node* lower_bound(Node *u, const T& key) const {
        if (key < u->value) {
            if (u->left == nullptr) {
                return u;
            } else {
                return lower_bound(u->left, key);
            }
        } else if (u->value < key) {
            if (u->right == nullptr) {
                return u;
            } else {
                return lower_bound(u->right, key);
            }
        } else {
            return u;
        }
    }

    Node* find_min(Node *u) const {
        return u->left ? find_min(u->left) : u;
    }

    Node* remove_min(Node *u) {
        if (u->left == nullptr) {
            return u->right;
        } else {
            link_left(u, remove_min(u->left));
            update(u);
            return rebalance(u);
        }
    }

    Node* copy(Node *u) {
        if (!u) return nullptr;
        Node *v = new Node(*u); 
        link_left(v, copy(u->left));
        link_right(v, copy(u->right));
        return v;
    }

    void free(Node *u) {
        if (!u) return;
        free(u->left);
        free(u->right);
        delete u;
    }
};
 
