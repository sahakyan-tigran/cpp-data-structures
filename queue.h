#pragma once

#include <stdexcept>

// Array-backed FIFO queue implemented as a circular buffer.
//
// Invariant: the live elements are at
//     m_data[(m_front + i) % m_capacity]   for i in [0, m_size)
// Deriving the back from m_front + m_size (rather than storing it) keeps the
// wrap-around cases from drifting out of sync.
template <typename T>
class Queue {
public:
	Queue();
	Queue(const Queue<T>&);
	Queue<T>& operator=(const Queue<T>&);
	~Queue();

	bool is_empty() const { return m_size == 0; }
	int size() const { return m_size; }
	int capacity() const { return m_capacity; }

	T& front();
	const T& front() const;
	T& back();
	const T& back() const;

	void push(const T&);
	void pop();
	void clear();

private:
	void grow();
	int index_of(int offset) const { return (m_front + offset) % m_capacity; }

private:
	static constexpr int INITIAL_CAPACITY = 16;

	int m_capacity;
	int m_size;
	int m_front;
	T* m_data;
};

template <typename T>
Queue<T>::Queue()
	: m_capacity(INITIAL_CAPACITY)
	, m_size(0)
	, m_front(0)
	, m_data(new T[INITIAL_CAPACITY]{})
{}

template <typename T>
Queue<T>::Queue(const Queue<T>& object)
	: m_capacity(object.m_capacity)
	, m_size(object.m_size)
	, m_front(0)                       // re-linearise into the new buffer
	, m_data(new T[object.m_capacity]{})
{
	for (int i = 0; i < m_size; ++i) {
		m_data[i] = object.m_data[object.index_of(i)];
	}
}

template <typename T>
Queue<T>& Queue<T>::operator=(const Queue<T>& object) {
	if (this != &object) {
		T* fresh = new T[object.m_capacity]{};
		try {
			for (int i = 0; i < object.m_size; ++i) {
				fresh[i] = object.m_data[object.index_of(i)];
			}
		}
		catch (...) {
			delete[] fresh;
			throw;
		}
		delete[] m_data;
		m_data = fresh;
		m_capacity = object.m_capacity;
		m_size = object.m_size;
		m_front = 0;
	}
	return *this;
}

template <typename T>
Queue<T>::~Queue() {
	delete[] m_data;
}

template <typename T>
T& Queue<T>::front() {
	if (is_empty()) {
		throw std::out_of_range("Queue::front on an empty queue");
	}
	return m_data[m_front];
}

template <typename T>
const T& Queue<T>::front() const {
	if (is_empty()) {
		throw std::out_of_range("Queue::front on an empty queue");
	}
	return m_data[m_front];
}

template <typename T>
T& Queue<T>::back() {
	if (is_empty()) {
		throw std::out_of_range("Queue::back on an empty queue");
	}
	return m_data[index_of(m_size - 1)];
}

template <typename T>
const T& Queue<T>::back() const {
	if (is_empty()) {
		throw std::out_of_range("Queue::back on an empty queue");
	}
	return m_data[index_of(m_size - 1)];
}

template <typename T>
void Queue<T>::push(const T& element) {
	if (m_size == m_capacity) {
		grow();
	}
	m_data[index_of(m_size)] = element;
	++m_size;
}

template <typename T>
void Queue<T>::pop() {
	if (is_empty()) {
		throw std::out_of_range("Queue::pop on an empty queue");
	}
	m_data[m_front] = T();
	m_front = (m_front + 1) % m_capacity;
	--m_size;
}

template <typename T>
void Queue<T>::clear() {
	for (int i = 0; i < m_size; ++i) {
		m_data[index_of(i)] = T();
	}
	m_size = 0;
	m_front = 0;
}

template <typename T>
void Queue<T>::grow() {
	int new_capacity = m_capacity * 2;
	T* fresh = new T[new_capacity]{};
	try {
		for (int i = 0; i < m_size; ++i) {
			fresh[i] = m_data[index_of(i)];
		}
	}
	catch (...) {
		delete[] fresh;
		throw;
	}
	delete[] m_data;
	m_data = fresh;
	m_capacity = new_capacity;
	m_front = 0;
}
