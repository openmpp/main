/**
* @file    rb_tree.h
* Implementation of a red-black tree with order statistics
*
* The implementation follows very closely "Introduction to Algorithms" 3rd edition, CLRS
* specifically:
* 
* Chapter 12: Binary Trees
* Chapter 13: Red-Black Trees
* Chapter 14: Augmenting data structures
* 
* link containing 3rd edition text and code (chapter 13):
* https://www.d.umn.edu/~ddunham/cs5521f13/notes/ch13.txt
* link to earlier edition, with diagrams but out-of-date code listing:
* http://staff.ustc.edu.cn/~csli/graduate/algorithms/book6/chap14.htm
*
* The function and symbol names below align closely with the pseudocode in CLRS,
* so a reader of this code can follow the explanations in the book.
* The code is a line-by-line translation of the pseudocode in the book,
* apart from code added to maintain the size of the subtree rooted at each node.
* That added code is identified by a preceding comment.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <functional>

template<typename T>
class rb_node
{
public:
    enum colors {
        RED = 0,
        BLACK
    };

	rb_node()
	{
	}

    rb_node(T key, rb_node * NIL)
        : left(NIL)
        , right(NIL)
        , p(NIL)
        , key(key)
        , color(BLACK)
        , size(0)
    {
    }

	rb_node * left;
	rb_node * right;
	rb_node * p;
	T key;
	bool color;
    size_t size;
};

template<typename T, typename CMP = std::less<T> >
class rb_tree
{
public:
    typedef rb_node<T> node_type;

    enum colors {
        RED = rb_node<T>::RED,
        BLACK = rb_node<T>::BLACK
    };

    rb_tree()
    {
		// Create tree-specific NIL instance.
		// rb_delete and rb_transplant use the p member of
		// rb_node for intermediate storage, which requires
		// a distinct NIL instance for each rb_tree.
		NIL = new node_type;
		NIL->color = BLACK;
		NIL->left = NIL;
		NIL->right = NIL;
		NIL->p = NIL;
		NIL->size = 0;

		root = NIL;
    }

	~rb_tree()
	{
		delete NIL;
	}

	/**
	* Minimum value of subtree rooted at a node
	*
	* Source: CLRS 13.2 Querying a binary search tree
	*
	* @return The node with the minimum value.
	*/
	node_type * tree_minimum(node_type * x)
	{
		while (x->left != NIL) {
			x = x->left;
		}
		return x;
	}

	/**
	* Maximum value of subtree rooted at a node
	*
	* Source: CLRS 13.2 Querying a binary search tree
	*
	* @return The node with the minimum value.
	*/
	node_type * tree_maximum(node_type * x)
	{
		while (x->right != NIL) {
			x = x->right;
		}
		return x;
	}

	/**
	* Get the successor node
	*
	* Source: CLRS 13.2 Querying a binary search tree
	*
	* @return The next node, or NIL if this node is the maximum.
	*/
	node_type * tree_successor(node_type * x)
	{
		if (x->right != NIL) {
			return x->right->tree_minimum();
		}
		auto y = x->p;
		while (y != NIL && x == y->right) {
			x = y;
			y = x->p;
		}
		return y;
	}

	/**
	* Get the predecessor node
	*
	* Source: CLRS 13.2 Querying a binary search tree
	*
	* @return The previous node, or NIL if this node is the minimum.
	*/
	node_type * tree_predecessor(node_type * x)
	{
		if (x->left != NIL) {
			return x->left->tree_maximum();
		}
		auto y = x->p;
		while (y != NIL && x == y->left) {
			x = y;
			y = x->p;
		}
		return y;
	}

	/**
     * Find node with a given key
     * 
     * Source: CLRS 12.2 Querying a binary search tree
     *
     * @return The node with the given key, NIL if not found
     */
    node_type * iterative_tree_search(T k) const
    {
        auto x = root;
        while (x != NIL && k != x->key) {
            if (CMP()(k, x->key)) {
                x = x->left;
            }
            else {
                x = x->right;
            }
        }
        return x;
    }

    /**
     * Left rotate a subtree at a node
     * 
     * Source: CLRS 13.2 Rotations
     */
    void left_rotate(node_type * x)
    {
	    auto y = x->right;
	    x->right = y->left;
        if (y->left != NIL) {
            y->left->p = x;
        }
	    y->p = x->p;
        if (x->p == NIL) {
            root = y;
        }
        else if (x->p->left == x) {
            x->p->left = y;
        }
        else {
            x->p->right = y;
        }
	    y->left = x;
	    x->p = y;
        // Maintain sub-tree sizes
	    y->size = x->size;
	    x->size = x->left->size + x->right->size +1;
    }

    /**
     * Right rotate a subtree at a node
     * 
     * Source: CLRS 13.2 Rotations
     */
    void right_rotate(node_type * y) {
        auto x = y->left;
        y->left = x->right;
        if (x->right != NIL) {
            x->right->p = y;
        }
        x->p = y->p;
        if (y->p == NIL) {
            root = x;
        }
        else if (y->p->left == y) {
            y->p->left = x;
        }
        else {
            y->p->right = x;
        }
        x->right = y;
        y->p = x;
        // Maintain sub-tree sizes
        x->size = y->size;
        y->size = y->left->size + y->right->size + 1;
    }

    /**
     * Insert a node into a red-black tree
     * 
     * Source: CLRS 13.3 Insertion and Deletion
     */
    void rb_insert(node_type * z)
    {
        assert(z != NIL);
        node_type * y = NIL;
        auto x = root;
	    while (x != NIL) {
		    y = x;
        	// Maintain sub-tree sizes
		    y->size++;
            if (CMP()(z->key, x->key)) {
                x = x->left;
            }
            else {
                assert(CMP()(x->key, z->key)); // z must not be already in the tree
                x = x->right;
            }
	    }
	    z->p = y;
        if (y == NIL) {
            root = z;
        }
        else if (CMP()(z->key, y->key)) {
            y->left = z;
        }
        else {
            y->right = z;
        }
	    z->left = NIL;
	    z->right = NIL;
	    z->color = RED;
	    z->size = 1;
        //assert(iterative_tree_search(z->key) != NIL); // node absent after insertion
        rb_insert_fixup(z);
        //assert(iterative_tree_search(z->key) != NIL); // node absent after insertion
    }

    /**
     * Modify a tree after node insertion to restore red-black tree properties
     * 
     * Source: CLRS 13.3 Insertion
     */
    void rb_insert_fixup(node_type * z)
    {
        while (z->p->color == RED) {
	        if (z->p == z->p->p->left) {
		        auto y = z->p->p->right;
		        if (y->color == RED) {
			        z->p->color = BLACK;
			        y->color = BLACK;
			        z->p->p->color = RED;
			        z = z->p->p;
		        }
		        else {
			        if (z == z->p->right) {
				        z = z->p;
				        left_rotate(z);
			        }
			        z->p->color = BLACK;
			        z->p->p->color = RED;
			        right_rotate(z->p->p);
		        }
	        }
	        else {
		        auto y = z->p->p->left;
		        if (y->color == RED) {
			        z->p->color = BLACK;
			        y->color = BLACK;
			        z->p->p->color = RED;
			        z = z->p->p;
		        }
		        else {
			        if (z == z->p->left) {
				        z = z->p;
				        right_rotate(z);
			        }
			        z->p->color = BLACK;
			        z->p->p->color = RED;
			        left_rotate(z->p->p);
		        }
	        }
        }
        root->color = BLACK;
    }

    /**
     * Move a subtree
     * 
     * Source: CLRS 13.4 Deletion
     */
    void rb_transplant(node_type * u, node_type * v)
    {
        if (u->p == NIL) {
            root = v;
        }
        else if (u == u->p->left) {
            u->p->left = v;
        }
        else {
            u->p->right = v;
        }
        v->p = u->p;
    }

    /**
     * Delete a node
     * 
     * Source: CLRS 13.4 Deletion
     */
    void rb_delete(node_type * z)
    {
        assert(z != NIL);
	    // Maintain sub-tree sizes
	    if (z != root) {
		    auto tmp = z->p;
		    while (tmp != root) {
			    tmp->size--;
			    tmp = tmp->p;
		    }
		    root->size--;
	    }
	    auto y = z;
	    int y_original_color = y->color;
	    node_type * x = NIL;
	    if (z->left == NIL) {
		    x = z->right;
		    rb_transplant(z, z->right);
	    }
	    else if (z->right == NIL) {
		    x = z->left;
		    rb_transplant(z, z->left);
	    }
	    else {
		    y = tree_minimum(z->right);
		    y_original_color = y->color;
		    x = y->right;
		    if (y->p == z) {
			    x->p = y;
		    }
		    else {
			    rb_transplant(y, y->right);
			    y->right = z->right;
			    y->right->p = y;
        	    // Maintain sub-tree sizes
			    auto tmp = x->p;
			    while (tmp != y) {
				    tmp->size--;
				    tmp = tmp->p;
			    }
			    y->size = y->left->size + 1;
		    }
		    rb_transplant(z, y);
		    y->left = z->left;
		    y->left->p = y;
		    y->color = z->color;
        	// Maintain sub-tree sizes
		    y->size = y->left->size + y->right->size + 1;
	    }
	    if (y_original_color == BLACK)
		    rb_delete_fixup(x);
    }

    /**
     * Modify a tree after node deletion to restore red-black tree properties
     * 
     * Source: CLRS 13.3 Insertion
     */
    void rb_delete_fixup(node_type * x) {
	    while (x != root && x->color == BLACK) {
		    if (x == x->p->left) {
			    auto w = x->p->right;
			    if (w->color == RED) {
				    w->color = BLACK;
				    x->p->color = RED;
				    left_rotate(x->p);
				    w = x->p->right;
			    }
			    if (w->left->color == BLACK && w->right->color == BLACK) {
				    w->color = RED;
				    x = x->p;
			    }
			    else {
			 	    if (w->right->color == BLACK) {
					    w->left->color = BLACK;
					    w->color = RED;
					    right_rotate(w);
					    w = x->p->right;
				    }
				    w->color = x->p->color;
				    x->p->color = BLACK;
				    w->right->color = BLACK;
				    left_rotate(x->p);
				    x = root;
			    }
		    }
		    else {
			    auto w = x->p->left;
			    if (w->color == RED) {
				    w->color = BLACK;
				    x->p->color = RED;
				    right_rotate(x->p);
				    w = x->p->left;
			    }
			    if (w->left->color == BLACK && w->right->color == BLACK) {
				    w->color = RED;
				    x = x->p;
			    }
			    else {
				    if (w->left->color == BLACK) {
					    w->right->color = BLACK;
					    w->color = RED;
					    left_rotate(w);
					    w = x->p->left;
				    }
				    w->color = x->p->color;
				    x->p->color = BLACK;
				    w->left->color = BLACK;
				    right_rotate(x->p);
				    x = root;
			    }
		    }
	    }
	    x->color = BLACK;
    }

    /**
     * Retrieve element based on rank (order statistic select)
     * 
     * Source: CLRS 14.1 Dynamic order statistics
     * The code is an unwound version of the pseudocode in CLRS.
     */
    node_type * os_select(size_t i)
    {
        assert(i > 0 && i <= root->size); // invalid order statistic
        auto x = root;
        size_t r = x->left->size + 1;
        while (i != r) {
            if (i < r) {
                x = x->left;
            }
            else {
                i = i - r;
                x = x->right;
            }
            r = x->left->size + 1;
        }
        return x;
    }

    /**
     * Determine rank of node in tree (order statistic)
     * 
     * Source: CLRS 14.1 Dynamic order statistics
     */
    size_t os_rank(node_type * x)
    {
        size_t r = x->left->size + 1;
        auto y = x;
        while (y != root) {
            if (y == y->p->right) {
                r += y->p->left->size + 1;
            }
            y = y->p;
        }
        return r;
    }

    /**
     * Clears this object to its blank/initial state.
     */
    void clear(node_type * x)
    {
        if (x == NIL) {
            return;
        }
        if (x->left != NIL) {
            clear(x->left);
        }
        if (x->right != NIL) {
            clear(x->right);
        }
        delete x;
    }

    // Interface functions

    size_t size()
    {
        return root->size;
    }

    node_type * get_first()
    {
        return root->tree_minimum();
    }

    node_type * root;

	node_type * NIL;
};
