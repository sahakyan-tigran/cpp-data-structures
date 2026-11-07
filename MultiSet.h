#pragma once

#include "SearchTree.h"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <vector>

// A multiset backed by a binary search tree: an element may appear more than
// once, and its multiplicity matters.
//
// Set algebra follows the usual multiset definitions, comparing multiplicities
// element by element:
//   a & b   intersection   min(count_a, count_b)
//   a | b   union          max(count_a, count_b)
//   a - b   difference     max(0, count_a - count_b)
template <typename T>
class MultiSet {
public:
	MultiSet() {}
	MultiSet(const T* data, int size);
	explicit MultiSet(const std::vector<T>& data);

	int size() const { return m_tree.size(); }
	bool empty() const { return m_tree.empty(); }
	bool contains(const T& element) const { return m_tree.contains(element); }
	int count(const T& element) const;

	void insert(const T& element) { m_tree.insert(element); }
	bool remove(const T& element) { return m_tree.remove(element); }

	// Sorted ascending, with duplicates.
	std::vector<T> elements() const { return m_tree.to_vector(); }

	MultiSet<T>& operator&=(const MultiSet<T>&);
	MultiSet<T>& operator|=(const MultiSet<T>&);
	MultiSet<T>& operator-=(const MultiSet<T>&);

	bool operator==(const MultiSet<T>&) const;
	bool operator<(const MultiSet<T>&) const;

private:
	void assign(const std::vector<T>& values);

private:
	SearchTree<T> m_tree;
};

template <typename T>
MultiSet<T>::MultiSet(const T* data, int size) {
	for (int i = 0; i < size; ++i) {
		m_tree.insert(data[i]);
	}
}

template <typename T>
MultiSet<T>::MultiSet(const std::vector<T>& data) {
	for (std::size_t i = 0; i < data.size(); ++i) {
		m_tree.insert(data[i]);
	}
}

template <typename T>
void MultiSet<T>::assign(const std::vector<T>& values) {
	m_tree.clear();
	for (std::size_t i = 0; i < values.size(); ++i) {
		m_tree.insert(values[i]);
	}
}

template <typename T>
int MultiSet<T>::count(const T& element) const {
	std::vector<T> values = m_tree.to_vector();
	int n = 0;
	for (std::size_t i = 0; i < values.size(); ++i) {
		if (values[i] == element) { ++n; }
	}
	return n;
}

// Each of these snapshots both trees first. The previous versions walked a
// stack of raw node pointers into m_tree while calling m_tree.remove(), so the
// stack was left holding freed nodes. They also never updated m_size.
template <typename T>
MultiSet<T>& MultiSet<T>::operator&=(const MultiSet<T>& object) {
	std::vector<T> mine = m_tree.to_vector();
	std::vector<T> theirs = object.m_tree.to_vector();
	std::vector<T> result;
	std::set_intersection(mine.begin(), mine.end(),
		theirs.begin(), theirs.end(), std::back_inserter(result));
	assign(result);
	return *this;
}

template <typename T>
MultiSet<T>& MultiSet<T>::operator|=(const MultiSet<T>& object) {
	std::vector<T> mine = m_tree.to_vector();
	std::vector<T> theirs = object.m_tree.to_vector();
	std::vector<T> result;
	std::set_union(mine.begin(), mine.end(),
		theirs.begin(), theirs.end(), std::back_inserter(result));
	assign(result);
	return *this;
}

template <typename T>
MultiSet<T>& MultiSet<T>::operator-=(const MultiSet<T>& object) {
	std::vector<T> mine = m_tree.to_vector();
	std::vector<T> theirs = object.m_tree.to_vector();
	std::vector<T> result;
	std::set_difference(mine.begin(), mine.end(),
		theirs.begin(), theirs.end(), std::back_inserter(result));
	assign(result);
	return *this;
}

template <typename T>
bool MultiSet<T>::operator==(const MultiSet<T>& object) const {
	return m_tree.to_vector() == object.m_tree.to_vector();
}

// Proper subset: every multiplicity is covered by object, and object has more.
template <typename T>
bool MultiSet<T>::operator<(const MultiSet<T>& object) const {
	std::vector<T> mine = m_tree.to_vector();
	std::vector<T> theirs = object.m_tree.to_vector();
	if (mine.size() >= theirs.size()) { return false; }
	return std::includes(theirs.begin(), theirs.end(), mine.begin(), mine.end());
}

// Free operators take the left operand by value and return by value. They used
// to be declared `MultiSet<T>&` while returning that by-value parameter, i.e. a
// reference to a local that had already been destroyed.
template <typename T>
MultiSet<T> operator&(MultiSet<T> obj1, const MultiSet<T>& obj2) {
	return obj1 &= obj2;
}

template <typename T>
MultiSet<T> operator|(MultiSet<T> obj1, const MultiSet<T>& obj2) {
	return obj1 |= obj2;
}

template <typename T>
MultiSet<T> operator-(MultiSet<T> obj1, const MultiSet<T>& obj2) {
	return obj1 -= obj2;
}

template <typename T>
bool operator!=(const MultiSet<T>& obj1, const MultiSet<T>& obj2) {
	return !(obj1 == obj2);
}

template <typename T>
bool operator>(const MultiSet<T>& obj1, const MultiSet<T>& obj2) {
	return obj2 < obj1;
}

template <typename T>
bool operator<=(const MultiSet<T>& obj1, const MultiSet<T>& obj2) {
	return obj1 == obj2 || obj1 < obj2;
}

template <typename T>
bool operator>=(const MultiSet<T>& obj1, const MultiSet<T>& obj2) {
	return obj1 == obj2 || obj2 < obj1;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const MultiSet<T>& object) {
	std::vector<T> values = object.elements();
	out << "{";
	for (std::size_t i = 0; i < values.size(); ++i) {
		if (i != 0) { out << ", "; }
		out << values[i];
	}
	out << "}";
	return out;     // no trailing endl: let the caller decide
}
