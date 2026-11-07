#include "LinkedList.h"

#include <stdexcept>

LinkedList::LinkedList() : m_size(0), m_head(nullptr), m_tail(nullptr) {}

LinkedList::LinkedList(int size, const int* data)
	: m_size(0), m_head(nullptr), m_tail(nullptr)
{
	if (size < 0) {
		throw std::invalid_argument("LinkedList: negative size");
	}
	if (size > 0 && data == nullptr) {
		throw std::invalid_argument("LinkedList: null data with non-zero size");
	}
	for (int i = 0; i < size; ++i) {
		push_back(data[i]);
	}
}

LinkedList::LinkedList(const LinkedList& object)
	: m_size(0), m_head(nullptr), m_tail(nullptr)
{
	copy_from(object);
}

LinkedList& LinkedList::operator=(const LinkedList& object) {
	if (this != &object) {
		deallocate();            // also resets head/tail; copying an empty
		copy_from(object);       // list now correctly empties *this
	}
	return *this;
}

LinkedList::~LinkedList() {
	deallocate();
}

LinkedList::Node* LinkedList::node_at(int index) const {
	if (index < 1 || index > m_size) {
		throw std::out_of_range("LinkedList: index out of range");
	}
	// Walk from whichever end is closer.
	if (index <= m_size / 2) {
		Node* current = m_head;
		for (int i = 1; i < index; ++i) {
			current = current->m_next;
		}
		return current;
	}
	Node* current = m_tail;
	for (int i = m_size; i > index; --i) {
		current = current->m_previous;
	}
	return current;
}

int& LinkedList::at(int index) { return node_at(index)->m_info; }
const int& LinkedList::at(int index) const { return node_at(index)->m_info; }

int& LinkedList::front() { return node_at(1)->m_info; }
const int& LinkedList::front() const { return node_at(1)->m_info; }
int& LinkedList::back() { return node_at(m_size)->m_info; }
const int& LinkedList::back() const { return node_at(m_size)->m_info; }

void LinkedList::insert(int index, int element) {
	// Was: silently clamped an out-of-range index instead of reporting it.
	if (index < 1 || index > m_size + 1) {
		throw std::out_of_range("LinkedList::insert: index out of range");
	}

	Node* fresh = new Node(element);

	if (m_size == 0) {
		m_head = fresh;
		m_tail = fresh;
		m_size = 1;
		return;
	}
	if (index == 1) {
		fresh->m_next = m_head;
		m_head->m_previous = fresh;
		m_head = fresh;
		++m_size;
		return;
	}
	if (index == m_size + 1) {
		fresh->m_previous = m_tail;
		m_tail->m_next = fresh;
		m_tail = fresh;
		++m_size;
		return;
	}

	Node* next = node_at(index);
	Node* previous = next->m_previous;
	previous->m_next = fresh;
	fresh->m_previous = previous;
	fresh->m_next = next;
	next->m_previous = fresh;
	++m_size;
}

void LinkedList::remove(int index) {
	// Was: `if (m_size != 0) throw ...`, i.e. remove() threw on every
	// non-empty list and silently fell through on an empty one.
	if (m_size == 0) {
		throw std::out_of_range("LinkedList::remove on an empty list");
	}
	Node* target = node_at(index);   // validates the index

	if (target->m_previous != nullptr) {
		target->m_previous->m_next = target->m_next;
	}
	else {
		m_head = target->m_next;
	}
	if (target->m_next != nullptr) {
		target->m_next->m_previous = target->m_previous;
	}
	else {
		m_tail = target->m_previous;
	}

	delete target;
	--m_size;
}

void LinkedList::copy_from(const LinkedList& object) {
	for (Node* source = object.m_head; source != nullptr; source = source->m_next) {
		push_back(source->m_info);
	}
}

void LinkedList::deallocate() {
	Node* current = m_head;
	while (current != nullptr) {
		Node* next = current->m_next;
		delete current;
		current = next;
	}
	m_head = nullptr;      // these were left dangling before
	m_tail = nullptr;
	m_size = 0;
}
