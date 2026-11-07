#pragma once

#include <stdexcept>

// Singly-linked LIFO stack. Each node points at the element pushed before it.
template <typename T>
class LinkedStack {
public:
	LinkedStack();
	LinkedStack(const LinkedStack<T>&);
	LinkedStack<T>& operator=(const LinkedStack<T>&);
	~LinkedStack();

	bool is_empty() const { return m_size == 0; }
	int size() const { return m_size; }

	T& top();
	const T& top() const;

	void push(const T&);
	void pop();
	void clear() { deallocate(); }

private:
	// Note: no `template <typename T>` here. Re-declaring T would shadow the
	// class template parameter, which is ill-formed (MSVC accepted it, GCC and
	// Clang reject it).
	struct Node {
		T m_info;
		Node* m_previous;

		explicit Node(const T& info = T()) : m_info(info), m_previous(nullptr) {}
		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;
	};

	void deallocate();
	void copy_from(const LinkedStack<T>&);

private:
	int m_size;
	Node* m_top;
};

template <typename T>
LinkedStack<T>::LinkedStack() : m_size(0), m_top(nullptr) {}

template <typename T>
LinkedStack<T>::LinkedStack(const LinkedStack<T>& object) : m_size(0), m_top(nullptr) {
	copy_from(object);
}

template <typename T>
LinkedStack<T>& LinkedStack<T>::operator=(const LinkedStack<T>& object) {
	if (this != &object) {
		deallocate();
		copy_from(object);
	}
	return *this;
}

template <typename T>
LinkedStack<T>::~LinkedStack() {
	deallocate();
}

template <typename T>
T& LinkedStack<T>::top() {
	if (is_empty()) {
		throw std::out_of_range("LinkedStack::top on an empty stack");
	}
	return m_top->m_info;
}

template <typename T>
const T& LinkedStack<T>::top() const {
	if (is_empty()) {
		throw std::out_of_range("LinkedStack::top on an empty stack");
	}
	return m_top->m_info;
}

template <typename T>
void LinkedStack<T>::push(const T& element) {
	Node* tmp = new Node(element);
	tmp->m_previous = m_top;
	m_top = tmp;
	++m_size;
}

template <typename T>
void LinkedStack<T>::pop() {
	if (is_empty()) {
		throw std::out_of_range("LinkedStack::pop on an empty stack");
	}
	Node* previous = m_top->m_previous;
	delete m_top;
	m_top = previous;
	--m_size;
}

template <typename T>
void LinkedStack<T>::copy_from(const LinkedStack<T>& object) {
	const Node* source = object.m_top;
	Node* tail = nullptr;
	while (source != nullptr) {
		Node* fresh = new Node(source->m_info);
		if (tail == nullptr) {
			m_top = fresh;
		}
		else {
			tail->m_previous = fresh;
		}
		tail = fresh;
		source = source->m_previous;
	}
	m_size = object.m_size;
}

template <typename T>
void LinkedStack<T>::deallocate() {
	while (m_top != nullptr) {
		Node* previous = m_top->m_previous;
		delete m_top;
		m_top = previous;
	}
	m_size = 0;              // leaving these stale left dangling state behind
}
