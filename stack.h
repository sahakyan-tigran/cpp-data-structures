#pragma once

#include <stdexcept>
#include <utility>

// Array-backed LIFO stack. Grows geometrically; never shrinks.
template <typename T>
class Stack {
public:
	Stack();
	Stack(const Stack<T>&);
	Stack<T>& operator=(const Stack<T>&);
	~Stack();

	bool is_empty() const { return m_size == 0; }
	int size() const { return m_size; }
	int capacity() const { return m_capacity; }

	T& top();
	const T& top() const;

	void push(const T&);
	void pop();
	void clear();

private:
	void grow();

private:
	static constexpr int INITIAL_CAPACITY = 16;

	int m_capacity;
	int m_size;
	T* m_data;
};

template <typename T>
Stack<T>::Stack()
	: m_capacity(INITIAL_CAPACITY)
	, m_size(0)
	, m_data(new T[INITIAL_CAPACITY]{})
{}

template <typename T>
Stack<T>::Stack(const Stack<T>& object)
	: m_capacity(object.m_capacity)
	, m_size(object.m_size)
	, m_data(new T[object.m_capacity]{})
{
	for (int i = 0; i < m_size; ++i) {
		m_data[i] = object.m_data[i];
	}
}

template <typename T>
Stack<T>& Stack<T>::operator=(const Stack<T>& object) {
	if (this != &object) {
		// Build the copy first: if an element copy throws, *this is untouched.
		T* fresh = new T[object.m_capacity]{};
		try {
			for (int i = 0; i < object.m_size; ++i) {
				fresh[i] = object.m_data[i];
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
	}
	return *this;
}

template <typename T>
Stack<T>::~Stack() {
	delete[] m_data;
}

template <typename T>
T& Stack<T>::top() {
	if (is_empty()) {
		throw std::out_of_range("Stack::top on an empty stack");
	}
	return m_data[m_size - 1];
}

template <typename T>
const T& Stack<T>::top() const {
	if (is_empty()) {
		throw std::out_of_range("Stack::top on an empty stack");
	}
	return m_data[m_size - 1];
}

template <typename T>
void Stack<T>::push(const T& element) {
	if (m_size == m_capacity) {
		grow();
	}
	m_data[m_size] = element;
	++m_size;
}

template <typename T>
void Stack<T>::pop() {
	if (is_empty()) {
		throw std::out_of_range("Stack::pop on an empty stack");
	}
	--m_size;
	m_data[m_size] = T();   // release whatever the slot was holding
}

template <typename T>
void Stack<T>::clear() {
	for (int i = 0; i < m_size; ++i) {
		m_data[i] = T();
	}
	m_size = 0;
}

template <typename T>
void Stack<T>::grow() {
	int new_capacity = m_capacity * 2;
	T* fresh = new T[new_capacity]{};
	try {
		for (int i = 0; i < m_size; ++i) {
			fresh[i] = m_data[i];
		}
	}
	catch (...) {
		delete[] fresh;
		throw;
	}
	delete[] m_data;
	m_data = fresh;
	m_capacity = new_capacity;
}
