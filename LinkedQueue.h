#pragma once

#include <stdexcept>

// Singly-linked FIFO queue, with a tail pointer so push stays O(1).
template <typename T>
class LinkedQueue {
public:
	LinkedQueue();
	LinkedQueue(const LinkedQueue<T>&);
	LinkedQueue<T>& operator=(const LinkedQueue<T>&);
	~LinkedQueue();

	bool is_empty() const { return m_size == 0; }
	int size() const { return m_size; }

	T& front();
	const T& front() const;
	T& back();
	const T& back() const;

	void push(const T&);
	void pop();
	void clear() { deallocate(); }

private:
	// Note: no `template <typename T>` here. Re-declaring T would shadow the
	// class template parameter, which is ill-formed (MSVC accepted it, GCC and
	// Clang reject it).
	struct Node {
		T m_info;
		Node* m_next;

		explicit Node(const T& info = T()) : m_info(info), m_next(nullptr) {}
		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;
	};

	void deallocate();
	void copy_from(const LinkedQueue<T>&);

private:
	int m_size;
	Node* m_front;
	Node* m_rear;
};

template <typename T>
LinkedQueue<T>::LinkedQueue() : m_size(0), m_front(nullptr), m_rear(nullptr) {}

template <typename T>
LinkedQueue<T>::LinkedQueue(const LinkedQueue<T>& object)
	: m_size(0), m_front(nullptr), m_rear(nullptr)
{
	copy_from(object);
}

template <typename T>
LinkedQueue<T>& LinkedQueue<T>::operator=(const LinkedQueue<T>& object) {
	if (this != &object) {
		deallocate();
		copy_from(object);
	}
	return *this;
}

template <typename T>
LinkedQueue<T>::~LinkedQueue() {
	deallocate();
}

template <typename T>
T& LinkedQueue<T>::front() {
	if (is_empty()) {
		throw std::out_of_range("LinkedQueue::front on an empty queue");
	}
	return m_front->m_info;
}

template <typename T>
const T& LinkedQueue<T>::front() const {
	if (is_empty()) {
		throw std::out_of_range("LinkedQueue::front on an empty queue");
	}
	return m_front->m_info;
}

template <typename T>
T& LinkedQueue<T>::back() {
	if (is_empty()) {
		throw std::out_of_range("LinkedQueue::back on an empty queue");
	}
	return m_rear->m_info;
}

template <typename T>
const T& LinkedQueue<T>::back() const {
	if (is_empty()) {
		throw std::out_of_range("LinkedQueue::back on an empty queue");
	}
	return m_rear->m_info;
}

template <typename T>
void LinkedQueue<T>::push(const T& element) {
	Node* tmp = new Node(element);
	if (m_front == nullptr) {
		m_front = tmp;
	}
	else {
		m_rear->m_next = tmp;
	}
	m_rear = tmp;
	++m_size;
}

template <typename T>
void LinkedQueue<T>::pop() {
	if (is_empty()) {
		throw std::out_of_range("LinkedQueue::pop on an empty queue");
	}
	Node* next = m_front->m_next;
	delete m_front;
	m_front = next;
	if (m_front == nullptr) {
		m_rear = nullptr;      // the old rear has just been freed
	}
	--m_size;
}

template <typename T>
void LinkedQueue<T>::copy_from(const LinkedQueue<T>& object) {
	const Node* source = object.m_front;
	while (source != nullptr) {
		push(source->m_info);
		source = source->m_next;
	}
}

template <typename T>
void LinkedQueue<T>::deallocate() {
	while (m_front != nullptr) {
		Node* next = m_front->m_next;
		delete m_front;
		m_front = next;
	}
	m_rear = nullptr;
	m_size = 0;
}
