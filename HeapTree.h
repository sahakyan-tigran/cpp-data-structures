#pragma once

#include <algorithm>
#include <stdexcept>

// Binary max-heap over an array.
//
// Storage is 1-based: the root lives at m_data[1], the children of i are at
// 2i and 2i+1. Slot 0 is unused, which is why the buffer holds m_capacity + 1
// elements.
template <typename T>
class HeapTree {
public:
	HeapTree();
	HeapTree(const HeapTree<T>&);
	HeapTree<T>& operator=(const HeapTree<T>&);
	~HeapTree();

	bool empty() const { return m_size == 0; }
	int size() const { return m_size; }

	const T& max_value() const;
	void insert(const T&);
	void remove();

private:
	void grow();

private:
	static constexpr int INITIAL_CAPACITY = 64;

	int m_capacity;
	int m_size;
	T* m_data;
};

template <typename T>
HeapTree<T>::HeapTree()
	: m_capacity(INITIAL_CAPACITY)
	, m_size(0)
	, m_data(new T[INITIAL_CAPACITY + 1]{})
{}

template <typename T>
HeapTree<T>::HeapTree(const HeapTree<T>& obj)
	: m_capacity(obj.m_capacity)
	, m_size(obj.m_size)
	, m_data(new T[obj.m_capacity + 1]{})
{
	for (int i = 1; i <= m_size; ++i) {
		m_data[i] = obj.m_data[i];
	}
}

template <typename T>
HeapTree<T>& HeapTree<T>::operator=(const HeapTree<T>& obj) {
	if (this != &obj) {
		T* fresh = new T[obj.m_capacity + 1]{};
		try {
			for (int i = 1; i <= obj.m_size; ++i) {
				fresh[i] = obj.m_data[i];
			}
		}
		catch (...) {
			delete[] fresh;
			throw;
		}
		delete[] m_data;
		m_data = fresh;
		m_capacity = obj.m_capacity;
		m_size = obj.m_size;
	}
	return *this;                 // was missing entirely
}

template <typename T>
HeapTree<T>::~HeapTree() {
	delete[] m_data;
}

template <typename T>
const T& HeapTree<T>::max_value() const {
	if (empty()) {
		throw std::out_of_range("HeapTree::max_value on an empty heap");
	}
	return m_data[1];
}

template <typename T>
void HeapTree<T>::insert(const T& element) {
	if (m_size == m_capacity) {
		grow();
	}
	m_data[++m_size] = element;

	// Sift up.
	int i = m_size;
	while (i != 1 && m_data[i] > m_data[i / 2]) {
		std::swap(m_data[i], m_data[i / 2]);
		i /= 2;
	}
}

template <typename T>
void HeapTree<T>::remove() {
	if (empty()) {
		return;
	}
	m_data[1] = m_data[m_size];
	m_data[m_size] = T();
	--m_size;

	// Sift down.
	int current = 1;
	int child = 2 * current;
	while (child <= m_size) {
		if (child + 1 <= m_size && m_data[child] < m_data[child + 1]) {
			++child;
		}
		if (!(m_data[child] > m_data[current])) {
			break;
		}
		std::swap(m_data[child], m_data[current]);
		current = child;
		child = 2 * current;
	}
}

template <typename T>
void HeapTree<T>::grow() {
	int new_capacity = m_capacity * 2;
	T* fresh = new T[new_capacity + 1]{};
	try {
		for (int i = 1; i <= m_size; ++i) {
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
