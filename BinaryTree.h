#pragma once

#include <algorithm>
#include <queue>
#include <stack>
#include <stdexcept>

namespace BinaryTree {

	template <typename T>
	struct Node {
		explicit Node(const T& info = T()) : m_info(info), m_left(nullptr), m_right(nullptr) {}
		Node(const Node<T>&) = delete;
		Node<T>& operator=(const Node<T>&) = delete;
		virtual ~Node() {}

		bool is_leaf() const { return m_left == nullptr && m_right == nullptr; }
		bool has_left() const { return m_left != nullptr; }
		bool has_right() const { return m_right != nullptr; }
		bool has_both() const { return m_left != nullptr && m_right != nullptr; }

		T m_info;
		Node<T>* m_left;
		Node<T>* m_right;
	};

	// A node that also knows its parent, so the traversals below can walk the
	// tree iteratively without a stack.
	template <typename T>
	struct ExtendedNode : public Node<T> {
		explicit ExtendedNode(const T& info = T())
			: Node<T>(info)          // the base is dependent; plain Node(info)
			, m_father(nullptr)      // only compiled on MSVC
		{}

		// m_left/m_right are declared Node<T>* in the base, but in an
		// ExtendedNode tree every child really is an ExtendedNode. These
		// accessors do the downcast once, instead of at every call site --
		// previously the traversals just returned Node<T>* from a function
		// declared to return ExtendedNode<T>*, which did not compile.
		ExtendedNode<T>* left() const { return static_cast<ExtendedNode<T>*>(this->m_left); }
		ExtendedNode<T>* right() const { return static_cast<ExtendedNode<T>*>(this->m_right); }

		ExtendedNode<T>* m_father;
	};

	template <typename T>
	using FuncNode = void(*)(Node<T>*);

	template <typename T>
	using FuncExtendedNode = void(*)(ExtendedNode<T>*);

	// Link helpers that keep m_father consistent. Building an ExtendedNode tree
	// by assigning m_left/m_right directly leaves m_father null and breaks every
	// traversal that walks upwards.
	template <typename T>
	void attach_left(ExtendedNode<T>* parent, ExtendedNode<T>* child) {
		parent->m_left = child;
		if (child != nullptr) { child->m_father = parent; }
	}

	template <typename T>
	void attach_right(ExtendedNode<T>* parent, ExtendedNode<T>* child) {
		parent->m_right = child;
		if (child != nullptr) { child->m_father = parent; }
	}

	// Frees a whole tree. There was no way to do this before, so every example
	// that built a tree with new leaked it.
	template <typename T>
	void destroy(Node<T>* root) {
		if (root == nullptr) { return; }
		destroy(root->m_left);
		destroy(root->m_right);
		delete root;
	}

	// Height in edges: an empty tree is -1, a single node is 0.
	template <typename T>
	int height(const Node<T>* root) {
		if (root == nullptr) { return -1; }
		return 1 + std::max(height(root->m_left), height(root->m_right));
	}

	template <typename T>
	int leaves(const Node<T>* root) {
		if (root == nullptr) { return 0; }
		if (root->is_leaf()) { return 1; }
		return leaves(root->m_left) + leaves(root->m_right);
	}

	// Returns by value. This used to return const T& bound to a local, so every
	// caller read a dangling reference.
	template <typename T>
	T find_max(const Node<T>* root) {
		if (root == nullptr) {
			throw std::invalid_argument("find_max on an empty tree");
		}
		T result = root->m_info;
		if (root->has_left()) {
			T left = find_max(root->m_left);
			if (left > result) { result = left; }
		}
		if (root->has_right()) {
			T right = find_max(root->m_right);
			if (right > result) { result = right; }
		}
		return result;
	}

	template <typename T>
	int points(const Node<T>* root) {
		if (root == nullptr) { return 0; }
		return 1 + points(root->m_left) + points(root->m_right);
	}

	// Strict (full) binary tree: every node has either 0 or 2 children.
	template <typename T>
	bool is_strict(const Node<T>* root) {
		if (root == nullptr) { return true; }
		if (root->is_leaf()) { return true; }
		if (!root->has_both()) { return false; }
		return is_strict(root->m_left) && is_strict(root->m_right);
	}

	// ---- recursive traversals ------------------------------------------------
	// These used to call straight_traverse / symmetrical_traverse /
	// reverse_traverse -- the ExtendedNode overloads -- instead of recursing
	// into themselves.

	template <typename T, typename Visit>
	void straight_traverse_rec(Node<T>* root, Visit visit) {
		if (root == nullptr) { return; }
		visit(root);
		straight_traverse_rec(root->m_left, visit);
		straight_traverse_rec(root->m_right, visit);
	}

	template <typename T, typename Visit>
	void symmetrical_traverse_rec(Node<T>* root, Visit visit) {
		if (root == nullptr) { return; }
		symmetrical_traverse_rec(root->m_left, visit);
		visit(root);
		symmetrical_traverse_rec(root->m_right, visit);
	}

	template <typename T, typename Visit>
	void reverse_traverse_rec(Node<T>* root, Visit visit) {
		if (root == nullptr) { return; }
		reverse_traverse_rec(root->m_left, visit);
		reverse_traverse_rec(root->m_right, visit);
		visit(root);
	}

	template <typename T, typename Visit>
	void print_i_level(Node<T>* root, int level, Visit visit) {
		if (root == nullptr) { return; }
		if (level == 0) {
			visit(root);
			return;
		}
		print_i_level(root->m_left, level - 1, visit);
		print_i_level(root->m_right, level - 1, visit);   // was ...), visit;
	}

	template <typename T, typename Visit>
	void print_by_level_rec(Node<T>* root, Visit visit) {
		if (root == nullptr) { return; }
		int h = height(root);
		for (int i = 0; i <= h; ++i) {
			print_i_level(root, i, visit);
		}
	}

	// ---- parent-pointer traversals -------------------------------------------

	template <typename T>
	ExtendedNode<T>* find_next_straight(ExtendedNode<T>* n) {
		if (n->has_left()) { return n->left(); }
		if (n->has_right()) { return n->right(); }
		// Climb until we find an ancestor with an unvisited right subtree.
		while (n->m_father != nullptr &&
			(n->m_father->right() == n || !n->m_father->has_right())) {
			n = n->m_father;
		}
		if (n->m_father == nullptr) { return nullptr; }
		return n->m_father->right();
	}

	template <typename T>
	ExtendedNode<T>* find_next_symmetrical(ExtendedNode<T>* n) {
		if (n->has_right()) {
			n = n->right();
			while (n->has_left()) { n = n->left(); }
			return n;
		}
		// Was an unguarded while (n == n->m_father->m_right), which
		// dereferenced a null father at the root.
		while (n->m_father != nullptr && n->m_father->right() == n) {
			n = n->m_father;
		}
		return n->m_father;
	}

	template <typename T>
	ExtendedNode<T>* find_next_reverse(ExtendedNode<T>* n) {
		ExtendedNode<T>* father = n->m_father;
		if (father == nullptr) { return nullptr; }
		if (father->left() == n && father->has_right()) {
			ExtendedNode<T>* sibling = father->right();
			while (sibling->has_left() || sibling->has_right()) {
				sibling = sibling->has_left() ? sibling->left() : sibling->right();
			}
			return sibling;
		}
		return father;
	}

	template <typename T, typename Visit>
	void straight_traverse(ExtendedNode<T>* root, Visit visit) {
		for (ExtendedNode<T>* n = root; n != nullptr; n = find_next_straight(n)) {
			visit(n);
		}
	}

	template <typename T, typename Visit>
	void symmetrical_traverse(ExtendedNode<T>* root, Visit visit) {
		if (root == nullptr) { return; }
		while (root->has_left()) { root = root->left(); }
		for (ExtendedNode<T>* n = root; n != nullptr; n = find_next_symmetrical(n)) {
			visit(n);
		}
	}

	template <typename T, typename Visit>
	void reverse_traverse(ExtendedNode<T>* root, Visit visit) {
		if (root == nullptr) { return; }
		ExtendedNode<T>* n = root;
		while (n->has_left() || n->has_right()) {
			n = n->has_left() ? n->left() : n->right();
		}
		for (; n != nullptr; n = find_next_reverse(n)) {
			visit(n);
		}
	}

	// ---- iterative traversals using an explicit container --------------------

	template <typename T, typename Visit>
	void print_by_level(Node<T>* root, Visit visit) {
		if (root == nullptr) { return; }
		std::queue<Node<T>*> q;
		q.push(root);
		while (!q.empty()) {
			Node<T>* current = q.front();
			q.pop();
			visit(current);
			if (current->has_left()) { q.push(current->m_left); }
			if (current->has_right()) { q.push(current->m_right); }
		}
	}

	template <typename T, typename Visit>
	void straight_with_stack(Node<T>* root, Visit visit) {
		if (root == nullptr) { return; }
		std::stack<Node<T>*> s;
		s.push(root);
		while (!s.empty()) {
			Node<T>* current = s.top();
			s.pop();
			visit(current);
			if (current->has_right()) { s.push(current->m_right); }
			if (current->has_left()) { s.push(current->m_left); }
		}
	}

	template <typename T, typename Visit>
	void symmetrical_with_stack(Node<T>* root, Visit visit) {
		std::stack<Node<T>*> s;
		Node<T>* current = root;
		while (current != nullptr || !s.empty()) {
			while (current != nullptr) {
				s.push(current);
				current = current->m_left;
			}
			current = s.top();
			s.pop();
			visit(current);
			current = current->m_right;
		}
	}

	template <typename T, typename Visit>
	void reverse_with_stack(Node<T>* root, Visit visit) {
		if (root == nullptr) { return; }
		std::stack<Node<T>*> s1, s2;
		s1.push(root);
		while (!s1.empty()) {
			Node<T>* current = s1.top();
			s1.pop();
			s2.push(current);
			if (current->has_left()) { s1.push(current->m_left); }
			if (current->has_right()) { s1.push(current->m_right); }
		}
		while (!s2.empty()) {
			visit(s2.top());
			s2.pop();
		}
	}
}
