#pragma once

#include "utils.hpp"
#include <memory>
#include <stack>
#include <cassert>

//-----------------------------------------------------------------------------------------

namespace avl {

template <typename T, typename key_type = int>
class node_t {
    using unique_ptr_node_t = typename std::unique_ptr<node_t<T, key_type>>;

    unique_ptr_node_t left_      = nullptr;
    unique_ptr_node_t right_     = nullptr;
    node_t<T, key_type>* parent_ = nullptr;
    size_t size_   = 1;
    size_t height_ = 1;
    key_type key_;

    public:
        T data_;

        node_t(key_type key, T data) : key_(key), data_(data) {};
        node_t(const node_t<T, key_type>& node) : key_(node.key_), data_(node.data_),
                                                  height_(node.height_) {

            unique_ptr_node_t ret_node = safe_copy(node);
            left_  = std::move(ret_node->left_);
            right_ = std::move(ret_node->right_);
        }
        node_t(key_type key, T data, size_t size, size_t height) :
            key_(key), data_(data),
            size_(size), height_(height)
            {};

        unique_ptr_node_t safe_copy (const node_t<T, key_type>& node);
        node_t<T, key_type>& operator= (const node_t<T, key_type>& node);
        node_t(node_t<T, key_type>&& node) = default;
        node_t<T, key_type>& operator= (node_t<T, key_type>&& node) = default;


        int find_balance_fact(const unique_ptr_node_t& node) const {
            if (node)
                return (get_height(node->right_) - get_height(node->left_));
            return 0;
        }
        unique_ptr_node_t& get_left()  {return left_;};
        unique_ptr_node_t& get_right() {return right_;};
        node_t<T, key_type>* get_parent() {return parent_;};
        void set_parent(node_t<T, key_type>* node) {parent_ = node;};
        void set_left(unique_ptr_node_t& node) {left_ = node;};
        void set_right(unique_ptr_node_t& node) {right_ = node;};

        size_t get_height(const unique_ptr_node_t& node) const {
            if (node) return node->height_; return 0;
        }
        size_t get_size(const unique_ptr_node_t& node) const {
            if (node) return node->size_; return 0;
        }
        key_type get_key() const {
            return key_;
        }
        void change_height(unique_ptr_node_t& node) {
            if (node) {
                node->height_ = 1 + std::max(get_height(node->left_),
                                             get_height(node->right_));
            }
            else
                node->height_ = 1;
        }
        void change_size(unique_ptr_node_t& node) {
            if (node) {
                node->size_ = 1 + get_size(node->left_) +
                                  get_size(node->right_);
            }
        }

        unique_ptr_node_t balance_subtree(unique_ptr_node_t& cur_node, T key);
        unique_ptr_node_t rotate_to_left(unique_ptr_node_t& cur_node);
        unique_ptr_node_t rotate_to_right(unique_ptr_node_t& cur_node);
        unique_ptr_node_t insert(unique_ptr_node_t& cur_node,
                                           const T& data, const key_type key);


        std::vector<T> store_inorder_walk() const;
        void graphviz_dump(graphviz::dump_graph_t& tree_dump) const ;
        node_t<T, key_type>* upper_bound(avl::node_t<T, key_type>* node, key_type key) const;
        node_t<T, key_type>* lower_bound(avl::node_t<T, key_type>* node, key_type key) const;

        size_t define_node_rank(const node_t<T, key_type>* root, const node_t<T, key_type>* cur_node) const;
};
}

//-----------------------------------------------------------------------------------------

namespace avl {

template<typename T, typename key_type>
node_t<T, key_type>& node_t<T, key_type>::operator= (const node_t<T, key_type>& node) {
    if (this == &node)
        return *this;

    std::unique_ptr<node_t<T, key_type>> tmp_left_  =
                                std::make_unique<node_t<T, key_type>> (*(node.left_));
    std::unique_ptr<node_t<T, key_type>> tmp_right_ =
                                std::make_unique<node_t<T, key_type>> (*(node.right_));
    assert(tmp_left_ != nullptr && tmp_right_ != nullptr);

    key_   = node.key_;
    height_= node.height_;
    data_  = node.data_;

    left_  = std::move(tmp_left_);
    right_ = std::move(tmp_right_);

    return *this;
}

template<typename T, typename key_type>
typename node_t<T, key_type>::unique_ptr_node_t
node_t<T, key_type>::safe_copy(const node_t<T, key_type>& origine_node) {

    auto origine_node_ptr = &origine_node;
    std::unique_ptr<node_t<T, key_type>> new_node =
                        std::make_unique<node_t<T, key_type>>(origine_node_ptr->key_,
                        origine_node_ptr->data_, origine_node_ptr->size_, origine_node_ptr->height_);

    node_t<T, key_type>* iter_node = new_node.get();
    while (origine_node_ptr != nullptr) {
        if (iter_node->left_ == nullptr && origine_node_ptr->left_ != nullptr) {
            iter_node->left_ = std::make_unique<node_t<T, key_type>>(
                            origine_node_ptr->left_->key_, origine_node_ptr->left_->data_,
                            origine_node_ptr->left_->size_, origine_node_ptr->left_->height_);
            iter_node->left_->parent_ = iter_node;

            iter_node    = iter_node->left_.get();
            origine_node_ptr = origine_node_ptr->left_.get();
        }
        else if (iter_node->right_ == nullptr && origine_node_ptr->right_ != nullptr) {
            iter_node->right_ = std::make_unique<node_t<T, key_type>>(
                            origine_node_ptr->right_->key_, origine_node_ptr->right_ ->data_,
                            origine_node_ptr->right_->size_, origine_node_ptr->right_->height_);
            iter_node->right_->parent_ = iter_node;

            iter_node    = iter_node->right_.get();
            origine_node_ptr = origine_node_ptr->right_.get();
        }
        else {
            iter_node = iter_node->parent_;
            origine_node_ptr = origine_node_ptr->parent_;
        }
    }
    return new_node;
}

//-----------------------------------------------------------------------------------------

template<typename T, typename key_type>
typename node_t<T, key_type>::unique_ptr_node_t
node_t<T, key_type>::insert(unique_ptr_node_t& cur_node, const T& data, const key_type key) {
    if(!cur_node)
        throw("Invalid ptr");

    if (cur_node->key_ < key) {
        if (cur_node->right_ != nullptr) {
            auto new_node = insert(cur_node->right_, key, data);
            std::swap(cur_node->right_, new_node); //just for interest
        }
        else {
            cur_node->right_ = std::make_unique<node_t<T, key_type>>(key, data);
            assert(cur_node->right_ != nullptr);
        }
        cur_node->right_->parent_ = cur_node.get();
    }
    else if (cur_node->key_ > key) {
        if (cur_node->left_ != nullptr) {
            cur_node->left_ = insert(cur_node->left_, key, data);
        }
        else {
            cur_node->left_ = std::make_unique<node_t<T, key_type>>(key, data);
            assert(cur_node->left_ != nullptr);
        }
        cur_node->left_->parent_ = cur_node.get();
    }

    change_height(cur_node);
    change_size(cur_node);

    return balance_subtree(cur_node, key);
}

//----------------------------ROTATES------------------------------------------------------

template<typename T, typename key_type>
typename node_t<T, key_type>::unique_ptr_node_t
node_t<T, key_type>::balance_subtree(unique_ptr_node_t& cur_node, T key) {

    if(!cur_node)
        throw("Invalid ptr");

    int delta = find_balance_fact(cur_node);
    if (delta > 1) {
        if (key < cur_node->right_->key_) { //complicated condition
            // std::cout << "RR rotate";
            cur_node->right_ = rotate_to_right(cur_node->right_);
            cur_node->right_->parent_ = cur_node.get();
        }
        // std::cout << "Left_rotate"<< std::endl;
        return rotate_to_left(cur_node);
    }
    else if (delta < -1) {
        if (key > cur_node->left_->key_) {
            // std::cout << "LL rotate";
            cur_node->left_ = rotate_to_left(cur_node->left_);
            cur_node->left_->parent_ = cur_node.get();
        }
        // std::cout << "Right_rotate"<< std::endl;
        return rotate_to_right(cur_node);
    }
    else
        return std::move(cur_node);
}

template<typename T, typename key_type>
typename node_t<T, key_type>::unique_ptr_node_t
node_t<T, key_type>::rotate_to_left(unique_ptr_node_t& cur_node) {

    if(!cur_node)
        throw("Invalid ptr");

    std::unique_ptr<node_t<T, key_type>> root = std::move(cur_node->right_);
    cur_node->right_ = std::move(root->left_);
    if (cur_node->right_) {
        cur_node->right_->parent_ = cur_node.get();
    }
    root->left_ = std::move(cur_node);
    root->left_->parent_ = root.get();

    change_height(root->left_);
    change_height(root);
    root->size_ = root->left_->size_;
    change_size(root->left_);

    return root;
}

template<typename T, typename key_type>
typename node_t<T, key_type>::unique_ptr_node_t
node_t<T, key_type>::rotate_to_right(unique_ptr_node_t& cur_node) {

    if(!cur_node)
        throw("Invalid ptr");

    unique_ptr_node_t root = std::move(cur_node->left_);
    cur_node->left_ = std::move(root->right_);
    if (cur_node->left_) {
        cur_node->left_->parent_ = cur_node.get();
    }
    root->right_ = std::move(cur_node);
    root->right_->parent_ = root.get();

    change_height(root->right_);
    change_height(root);
    root->size_ = root->right_->size_;
    change_size(root->right_);

    return root;
}

//--------------------RANGES---------------------------------------------------------------

template<typename T, typename key_type>
node_t<T, key_type>*
node_t<T, key_type>::upper_bound(node_t<T, key_type>* cur_node, key_type key) const {

    node_t<T, key_type>* node = nullptr;
    if (cur_node->key_ < key) {
        if (cur_node->right_ != nullptr)
            node = upper_bound(cur_node->right_.get(), key);
        else
            return cur_node;
    }
    else if (cur_node->key_ > key) {
        if (cur_node->left_ != nullptr)
            node = upper_bound(cur_node->left_.get(), key);
        else
            return cur_node;
    }
    else if (cur_node->key_ == key) {
        return cur_node;
    }

    if (node->key_ > key && cur_node->key_ < key) {
        return cur_node;
    }
    return node;
}

template<typename T, typename key_type>
node_t<T, key_type>*
node_t<T, key_type>::lower_bound(node_t<T, key_type>* cur_node, key_type key) const {

    node_t<T, key_type>*  node = nullptr;
    if (cur_node->key_ < key) {
        if (cur_node->right_ != nullptr)
            node = lower_bound(cur_node->right_.get(), key);
        else
            return cur_node;
    }
    else if (cur_node->key_ > key) {
        if (cur_node->left_ != nullptr)
            node = lower_bound(cur_node->left_.get(), key);
        else
            return cur_node;
    }
    else if (cur_node->key_ == key) {
        return cur_node;
    }

    if (node->key_ < key && cur_node->key_ > key) {
        return cur_node;
    }
    return node;
}

//-----------------------------------------------------------------------------------------

template<typename T, typename key_type>
size_t node_t<T, key_type>::define_node_rank(const node_t<T, key_type>* root,
                                             const node_t<T, key_type>* cur_node) const {

    if(cur_node == nullptr)
        throw("Invalid ptr");

    size_t rank = 1;
    if (cur_node->left_ != nullptr) {
        rank += cur_node->left_->size_;
    }
    const node_t<T, key_type>* tmp_node = this;
    while (tmp_node != root) {
        if (tmp_node == tmp_node->parent_->right_.get()) {
            rank += get_size (tmp_node->parent_->left_) + 1;
        }
        tmp_node = tmp_node->parent_;
        // std::cout << "rank: " << rank << "\n";
    }
    return rank;
}

//--------------------WALKING--------------------------------------------------------------

template<typename T, typename key_type>
std::vector<T> node_t<T, key_type>::store_inorder_walk() const {
    std::vector<T> storage;
    std::stack<const node_t<T, key_type>*> node_stk;
    const node_t<T, key_type>* cur_node = this;

    while (cur_node || !node_stk.empty()) {
        if (!node_stk.empty()) {
            cur_node = node_stk.top();
            storage.push_back(cur_node->key_);
            if (cur_node->right_)
                cur_node = cur_node->right_.get();
            else
                cur_node = nullptr;

            node_stk.pop();
        }
        while (cur_node) {
            node_stk.push(cur_node);
            cur_node = cur_node->left_.get();
        }
    }

    return storage;
}

template<typename T, typename key_type>
void node_t<T, key_type>::graphviz_dump(graphviz::dump_graph_t& tree_dump) const {
    tree_dump.graph_node.print_node(this, tree_dump.graphviz_strm);

    if (left_ != nullptr)
    {
        tree_dump.graph_edge.fillcolor = "#7FC7FF";
        tree_dump.graph_edge.color     = "#7FC7FF";
        tree_dump.graph_edge.print_edge(this, left_, tree_dump.graphviz_strm);
        left_->graphviz_dump(tree_dump);
    }
    if (right_ != nullptr)
    {
        tree_dump.graph_edge.fillcolor = "#DC143C";
        tree_dump.graph_edge.color     = "#DC143C";
        tree_dump.graph_edge.print_edge(this, right_, tree_dump.graphviz_strm);
        right_->graphviz_dump(tree_dump);
    }
}
}

