#pragma once

// Doubly-linked list of ints, with 1-based indices (index 1 is the head).
//
// Every member used to sit above the first `private:` label, which in a
// `class` means private -- the whole API was unreachable from outside.
class LinkedList {
public:
	LinkedList();
	LinkedList(int size, const int* data);
	LinkedList(const LinkedList&);
	LinkedList& operator=(const LinkedList&);
	~LinkedList();

	bool is_empty() const { return m_size == 0; }
	int size() const { return m_size; }

	// 1-based. Throw std::out_of_range when index is outside [1, size()].
	int& at(int index);
	const int& at(int index) const;

	int& front();
	const int& front() const;
	int& back();
	const int& back() const;

	// Insert before the element currently at `index`; index == size() + 1
	// appends. Throws std::out_of_range otherwise.
	void insert(int index, int element);
	void remove(int index);

	void push_back(int element) { insert(m_size + 1, element); }
	void push_front(int element) { insert(1, element); }
	void clear() { deallocate(); }

private:
	struct Node {
		int m_info;
		Node* m_previous;
		Node* m_next;

		explicit Node(int info = 0) : m_info(info), m_previous(nullptr), m_next(nullptr) {}
		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;
	};

	void deallocate();
	void copy_from(const LinkedList&);
	Node* node_at(int index) const;

private:
	int m_size;
	Node* m_head;
	Node* m_tail;
};
