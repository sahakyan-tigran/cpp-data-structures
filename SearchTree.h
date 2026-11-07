#pragma once

#include "BinaryTree.h"

#include <vector>

// Unbalanced binary search tree. Duplicates are allowed and are kept in the
// left subtree, which is what MultiSet relies on.
template <typename T>
class SearchTree {
public:
	SearchTree() : m_root(nullptr), m_size(0) {}
	SearchTree(const SearchTree<T>&);
	SearchTree<T>& operator=(const SearchTree<T>&);
	virtual ~SearchTree();

	// Was `return m_root = nullptr;` -- an assignment, so it always returned
	// false and silently leaked the entire tree on every call. It was also
	// non-const, which made SearchTree(const SearchTree&) fail to compile.
	bool empty() const { return m_root == nullptr; }
	int size() const { return m_size; }

	BinaryTree::Node<T>* find(const T&);
	const BinaryTree::Node<T>* find(const T&) const;
	bool contains(const T& element) const { return find(element) != nullptr; }

	void insert(const T&);
	bool remove(const T&);          // returns whether anything was removed
	void clear();

	// In-order snapshot. Returning values rather than node pointers means a
	// caller can safely mutate the tree while walking the result.
	std::vector<T> to_vector() const;

	template <typename Visit>
	void in_order(Visit visit) const {
		BinaryTree::symmetrical_with_stack(m_root, visit);
	}

private:
	void copy_from(const SearchTree<T>&);
	static BinaryTree::Node<T>* clone(const BinaryTree::Node<T>*);

private:
	BinaryTree::Node<T>* m_root;
	int m_size;
};

template <typename T>
SearchTree<T>::SearchTree(const SearchTree<T>& obj) : m_root(nullptr), m_size(0) {
	copy_from(obj);
}

template <typename T>
SearchTree<T>& SearchTree<T>::operator=(const SearchTree<T>& obj) {
	if (this != &obj) {
		clear();                 // clear() also nulls m_root; the old code left
		copy_from(obj);          // it dangling when obj was empty
	}
	return *this;
}

template <typename T>
SearchTree<T>::~SearchTree() {
	BinaryTree::destroy(m_root);
}

template <typename T>
void SearchTree<T>::clear() {
	BinaryTree::destroy(m_root);
	m_root = nullptr;
	m_size = 0;
}

template <typename T>
BinaryTree::Node<T>* SearchTree<T>::clone(const BinaryTree::Node<T>* source) {
	if (source == nullptr) { return nullptr; }
	BinaryTree::Node<T>* fresh = new BinaryTree::Node<T>(source->m_info);
	try {
		fresh->m_left = clone(source->m_left);
		fresh->m_right = clone(source->m_right);
	}
	catch (...) {
		BinaryTree::destroy(fresh);
		throw;
	}
	return fresh;
}

template <typename T>
void SearchTree<T>::copy_from(const SearchTree<T>& obj) {
	m_root = clone(obj.m_root);
	m_size = obj.m_size;
}

template <typename T>
const BinaryTree::Node<T>* SearchTree<T>::find(const T& element) const {
	const BinaryTree::Node<T>* current = m_root;
	while (current != nullptr && !(current->m_info == element)) {
		current = (current->m_info > element) ? current->m_left : current->m_right;
	}
	return current;
}

template <typename T>
BinaryTree::Node<T>* SearchTree<T>::find(const T& element) {
	const SearchTree<T>* self = this;
	return const_cast<BinaryTree::Node<T>*>(self->find(element));
}

template <typename T>
void SearchTree<T>::insert(const T& element) {
	BinaryTree::Node<T>* parent = nullptr;
	BinaryTree::Node<T>* current = m_root;
	bool go_left = false;

	while (current != nullptr) {
		parent = current;
		go_left = (current->m_info >= element);
		current = go_left ? current->m_left : current->m_right;
	}

	BinaryTree::Node<T>* fresh = new BinaryTree::Node<T>(element);
	if (parent == nullptr) {
		m_root = fresh;
	}
	else if (go_left) {
		parent->m_left = fresh;
	}
	else {
		parent->m_right = fresh;
	}
	++m_size;
}

// Standard BST delete. The previous find_father / child_search / next /
// delete_node arrangement returned father->m_right as an unchecked fallback,
// so it could delete an unrelated node (or the wrong one) and corrupt the tree.
template <typename T>
bool SearchTree<T>::remove(const T& element) {
	BinaryTree::Node<T>* parent = nullptr;
	BinaryTree::Node<T>* target = m_root;

	while (target != nullptr && !(target->m_info == element)) {
		parent = target;
		target = (target->m_info > element) ? target->m_left : target->m_right;
	}
	if (target == nullptr) {
		return false;
	}

	// Two children: swap in the in-order successor, then delete that instead.
	if (target->has_both()) {
		BinaryTree::Node<T>* successor_parent = target;
		BinaryTree::Node<T>* successor = target->m_right;
		while (successor->has_left()) {
			successor_parent = successor;
			successor = successor->m_left;
		}
		target->m_info = successor->m_info;
		parent = successor_parent;
		target = successor;
	}

	// target now has at most one child.
	BinaryTree::Node<T>* child = target->has_left() ? target->m_left : target->m_right;
	if (parent == nullptr) {
		m_root = child;
	}
	else if (parent->m_left == target) {
		parent->m_left = child;
	}
	else {
		parent->m_right = child;
	}

	target->m_left = nullptr;
	target->m_right = nullptr;
	delete target;
	--m_size;
	return true;
}

template <typename T>
std::vector<T> SearchTree<T>::to_vector() const {
	std::vector<T> out;
	out.reserve(static_cast<std::size_t>(m_size));
	BinaryTree::symmetrical_with_stack(m_root,
		[&out](const BinaryTree::Node<T>* n) { out.push_back(n->m_info); });
	return out;
}
