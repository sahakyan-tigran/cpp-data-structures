// Self-contained test suite for the containers in this repository.
//
//   g++ -std=c++17 -Wall -Wextra -I.. tests/test_all.cpp -o tests/test_all
//   ./tests/test_all
//
// Exits non-zero if anything fails.

#include "../BinaryTree.h"
#include "../HeapSort.h"
#include "../HeapTree.h"
#include "../LinkedList.h"
#include "../LinkedQueue.h"
#include "../LinkedStack.h"
#include "../MultiSet.h"
#include "../Polynom.h"
#include "../Radix.h"
#include "../SearchTree.h"
#include "../queue.h"
#include "../stack.h"

#include <cstdlib>
#include <iostream>
#include <new>
#include <sstream>
#include <string>
#include <vector>

// Global allocation counters. The destructors and deallocate() helpers in
// this repository used to leave nodes behind; running the whole suite twice
// and requiring new/delete to balance is what catches that.
namespace {
std::size_t g_allocations = 0;
std::size_t g_deallocations = 0;
}

void* operator new(std::size_t n) {
	++g_allocations;
	if (void* p = std::malloc(n ? n : 1)) { return p; }
	throw std::bad_alloc();
}

void* operator new[](std::size_t n) {
	++g_allocations;
	if (void* p = std::malloc(n ? n : 1)) { return p; }
	throw std::bad_alloc();
}

void operator delete(void* p) noexcept {
	if (p != nullptr) { ++g_deallocations; std::free(p); }
}

void operator delete[](void* p) noexcept {
	if (p != nullptr) { ++g_deallocations; std::free(p); }
}

void operator delete(void* p, std::size_t) noexcept { operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { operator delete[](p); }

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool condition, const std::string& what) {
	++g_checks;
	if (!condition) {
		++g_failures;
		std::cout << "  FAIL: " << what << "\n";
	}
}

template <typename T>
void check_eq(const T& actual, const T& expected, const std::string& what) {
	++g_checks;
	if (!(actual == expected)) {
		++g_failures;
		std::cout << "  FAIL: " << what << " (got " << actual
			<< ", expected " << expected << ")\n";
	}
}

template <typename Fn>
bool throws(Fn fn) {
	try { fn(); }
	catch (const std::exception&) { return true; }
	return false;
}

void section(const std::string& name) {
	std::cout << name << "\n";
}

// ---------------------------------------------------------------- Stack ----

void test_stack() {
	section("Stack");

	Stack<int> s;
	check(s.is_empty(), "new stack is empty");
	check_eq(s.size(), 0, "new stack size");

	for (int i = 1; i <= 5; ++i) { s.push(i); }
	check_eq(s.size(), 5, "size after 5 pushes");
	check_eq(s.top(), 5, "top after pushes");

	// The regression this suite exists for: pop() used to throw unconditionally
	// because the `throw` sat outside the empty check.
	s.pop();
	check_eq(s.top(), 4, "top after pop");
	check_eq(s.size(), 4, "size after pop");

	check(throws([] { Stack<int> e; e.pop(); }), "pop on empty throws");
	check(throws([] { Stack<int> e; e.top(); }), "top on empty throws");

	// Growth past the initial capacity.
	Stack<int> big;
	for (int i = 0; i < 1000; ++i) { big.push(i); }
	check_eq(big.size(), 1000, "size after 1000 pushes");
	check_eq(big.top(), 999, "top after growth");
	for (int i = 999; i >= 0; --i) {
		check_eq(big.top(), i, "LIFO order during drain");
		big.pop();
	}
	check(big.is_empty(), "empty after draining");

	// Deep copy, and a non-trivial element type.
	Stack<std::string> words;
	words.push("alpha");
	words.push("beta");
	Stack<std::string> copy = words;
	copy.pop();
	check_eq(words.size(), 2, "copy is independent (source size)");
	check_eq(copy.size(), 1, "copy is independent (copy size)");
	check_eq(words.top(), std::string("beta"), "source unchanged by copy mutation");

	Stack<std::string> assigned;
	assigned.push("x");
	assigned = words;
	check_eq(assigned.size(), 2, "assignment copies size");
	check_eq(assigned.top(), std::string("beta"), "assignment copies contents");

	assigned = assigned;                       // self-assignment must be safe
	check_eq(assigned.size(), 2, "self-assignment keeps size");
	check_eq(assigned.top(), std::string("beta"), "self-assignment keeps contents");
}

// ---------------------------------------------------------------- Queue ----

void test_queue() {
	section("Queue");

	Queue<int> q;
	check(q.is_empty(), "new queue is empty");

	for (int i = 1; i <= 5; ++i) { q.push(i); }
	check_eq(q.front(), 1, "FIFO front");
	check_eq(q.back(), 5, "FIFO back");
	q.pop();
	check_eq(q.front(), 2, "front after pop");
	check_eq(q.size(), 4, "size after pop");

	check(throws([] { Queue<int> e; e.pop(); }), "pop on empty throws");
	check(throws([] { Queue<int> e; e.front(); }), "front on empty throws");

	// Force the buffer to wrap around, then force a growth while wrapped.
	Queue<int> w;
	for (int i = 0; i < 12; ++i) { w.push(i); }
	for (int i = 0; i < 10; ++i) { w.pop(); }      // front is now well inside
	for (int i = 100; i < 140; ++i) { w.push(i); }
	check_eq(w.front(), 10, "front survives wrap + growth");
	check_eq(w.size(), 42, "size survives wrap + growth");
	check_eq(w.back(), 139, "back survives wrap + growth");

	int expected[] = { 10, 11 };
	for (int i = 0; i < 2; ++i) {
		check_eq(w.front(), expected[i], "order preserved across wrap");
		w.pop();
	}
	for (int i = 100; i < 140; ++i) {
		check_eq(w.front(), i, "order preserved after growth");
		w.pop();
	}
	check(w.is_empty(), "drained wrapped queue is empty");

	// This is what the hardcoded `int* tmp` in resize() broke: any T but int.
	Queue<std::string> strings;
	for (int i = 0; i < 100; ++i) { strings.push("item"); }
	check_eq(strings.size(), 100, "queue of std::string grows");
	check_eq(strings.front(), std::string("item"), "queue of std::string content");

	Queue<int> copy = w;
	check(copy.is_empty(), "copy of empty queue is empty");
	Queue<int> src;
	src.push(1);
	src.push(2);
	Queue<int> deep = src;
	deep.pop();
	check_eq(src.size(), 2, "queue copy is independent");
	check_eq(deep.size(), 1, "queue copy is independent (copy)");
}

// ----------------------------------------------------------- LinkedStack ----

void test_linked_stack() {
	section("LinkedStack");

	LinkedStack<int> s;
	check(s.is_empty(), "new linked stack is empty");
	for (int i = 1; i <= 4; ++i) { s.push(i); }
	check_eq(s.top(), 4, "top");
	check_eq(s.size(), 4, "size");

	LinkedStack<int> copy = s;
	check_eq(copy.size(), 4, "copy size");
	check_eq(copy.top(), 4, "copy top");
	copy.pop();
	check_eq(s.size(), 4, "source unaffected by copy pop");

	for (int i = 4; i >= 1; --i) {
		check_eq(s.top(), i, "LIFO order");
		s.pop();
	}
	check(s.is_empty(), "empty after drain");
	check(throws([] { LinkedStack<int> e; e.pop(); }), "pop on empty throws");

	// Assigning an empty stack over a populated one used to leave m_top dangling.
	LinkedStack<int> target;
	target.push(9);
	LinkedStack<int> empty;
	target = empty;
	check(target.is_empty(), "assigning an empty stack clears the target");
	check_eq(target.size(), 0, "assigning an empty stack zeroes size");
}

// ----------------------------------------------------------- LinkedQueue ----

void test_linked_queue() {
	section("LinkedQueue");

	LinkedQueue<int> q;
	for (int i = 1; i <= 4; ++i) { q.push(i); }
	check_eq(q.front(), 1, "front");
	check_eq(q.back(), 4, "back");
	check_eq(q.size(), 4, "size");

	LinkedQueue<int> copy = q;
	check_eq(copy.front(), 1, "copy front");
	check_eq(copy.back(), 4, "copy back");

	for (int i = 1; i <= 4; ++i) {
		check_eq(q.front(), i, "FIFO order");
		q.pop();
	}
	check(q.is_empty(), "empty after drain");

	// Emptying must clear m_rear too, or the next push writes through a
	// freed node.
	q.push(42);
	check_eq(q.front(), 42, "push after full drain: front");
	check_eq(q.back(), 42, "push after full drain: back");
	check_eq(q.size(), 1, "push after full drain: size");

	check(throws([] { LinkedQueue<int> e; e.pop(); }), "pop on empty throws");

	LinkedQueue<int> target;
	target.push(9);
	LinkedQueue<int> empty;
	target = empty;
	check(target.is_empty(), "assigning an empty queue clears the target");
}

// ------------------------------------------------------------ LinkedList ----

void test_linked_list() {
	section("LinkedList");

	// Every member used to be private, so none of this would even compile.
	LinkedList l;
	check(l.is_empty(), "new list is empty");

	l.push_back(10);
	l.push_back(20);
	l.push_back(30);
	check_eq(l.size(), 3, "size after push_back");
	check_eq(l.at(1), 10, "at(1)");
	check_eq(l.at(3), 30, "at(3)");
	check_eq(l.front(), 10, "front");
	check_eq(l.back(), 30, "back");

	l.insert(2, 15);                       // 10, 15, 20, 30
	check_eq(l.size(), 4, "size after insert");
	check_eq(l.at(2), 15, "inserted value");
	check_eq(l.at(3), 20, "element shifted right");

	l.push_front(5);                       // 5, 10, 15, 20, 30
	check_eq(l.front(), 5, "push_front");
	check_eq(l.size(), 5, "size after push_front");

	// remove() used to be `if (m_size != 0) throw`, so it threw on every
	// non-empty list.
	l.remove(1);                           // 10, 15, 20, 30
	check_eq(l.front(), 10, "front after removing head");
	check_eq(l.size(), 4, "size after removing head");

	l.remove(4);                           // 10, 15, 20
	check_eq(l.back(), 20, "back after removing tail");
	check_eq(l.size(), 3, "size after removing tail");

	l.remove(2);                           // 10, 20
	check_eq(l.at(1), 10, "first after removing middle");
	check_eq(l.at(2), 20, "second after removing middle");
	check_eq(l.size(), 2, "size after removing middle");

	check(throws([&l] { l.at(0); }), "at(0) throws");
	check(throws([&l] { l.at(99); }), "at(99) throws");
	check(throws([&l] { l.insert(99, 1); }), "insert out of range throws");
	check(throws([] { LinkedList e; e.remove(1); }), "remove on empty throws");

	int data[] = { 1, 2, 3, 4 };
	LinkedList from_array(4, data);
	check_eq(from_array.size(), 4, "array constructor size");
	check_eq(from_array.at(4), 4, "array constructor content");

	LinkedList copy = from_array;
	copy.remove(1);
	check_eq(from_array.size(), 4, "copy is independent");
	check_eq(copy.size(), 3, "copy is independent (copy)");

	// Assigning an empty list over a populated one must empty it.
	LinkedList target;
	target.push_back(1);
	LinkedList empty;
	target = empty;
	check(target.is_empty(), "assigning an empty list clears the target");
}

// -------------------------------------------------------------- HeapTree ----

void test_heap() {
	section("HeapTree / HeapSort");

	HeapTree<int> h;
	check(h.empty(), "new heap is empty");
	check(throws([] { HeapTree<int> e; e.max_value(); }), "max_value on empty throws");

	int values[] = { 5, 1, 9, 3, 7, 9, 2 };
	for (int v : values) { h.insert(v); }
	check_eq(h.size(), 7, "heap size");
	check_eq(h.max_value(), 9, "max");

	h.remove();
	check_eq(h.max_value(), 9, "max after removing one of two 9s");
	h.remove();
	check_eq(h.max_value(), 7, "max after removing both 9s");

	// operator= had no `return *this;` at all.
	HeapTree<int> a;
	a.insert(1);
	HeapTree<int> b;
	b.insert(100);
	a = b;
	check_eq(a.max_value(), 100, "assignment copies contents");
	check_eq(a.size(), 1, "assignment copies size");

	HeapTree<int>& ref = (a = b);
	check_eq(ref.max_value(), 100, "assignment returns *this");

	// Growth past the initial capacity.
	HeapTree<int> big;
	for (int i = 0; i < 500; ++i) { big.insert(i); }
	check_eq(big.size(), 500, "heap grows");
	check_eq(big.max_value(), 499, "max after growth");

	int unsorted[] = { 9, 4, 7, 1, 8, 2, 6, 3, 5, 0 };
	HeapSort(unsorted, 10);
	bool sorted = true;
	for (int i = 1; i < 10; ++i) {
		if (unsorted[i - 1] > unsorted[i]) { sorted = false; }
	}
	check(sorted, "HeapSort produces ascending order");
	check_eq(unsorted[0], 0, "HeapSort smallest");
	check_eq(unsorted[9], 9, "HeapSort largest");
}

// ------------------------------------------------------------ BinaryTree ----

// Builds the complete 7-node tree
//     4 -> (2, 6),  2 -> (1, 3),  6 -> (5, 7)
// whose in-order walk is 1,2,3,4,5,6,7.
BinaryTree::Node<int>* build_plain() {
	using BinaryTree::Node;
	Node<int>* root = new Node<int>(4);
	root->m_left = new Node<int>(2);
	root->m_right = new Node<int>(6);
	root->m_left->m_left = new Node<int>(1);
	root->m_left->m_right = new Node<int>(3);
	root->m_right->m_left = new Node<int>(5);
	root->m_right->m_right = new Node<int>(7);
	return root;
}

BinaryTree::ExtendedNode<int>* build_extended() {
	using BinaryTree::ExtendedNode;
	ExtendedNode<int>* root = new ExtendedNode<int>(4);
	ExtendedNode<int>* n2 = new ExtendedNode<int>(2);
	ExtendedNode<int>* n6 = new ExtendedNode<int>(6);
	ExtendedNode<int>* n1 = new ExtendedNode<int>(1);
	ExtendedNode<int>* n3 = new ExtendedNode<int>(3);
	ExtendedNode<int>* n5 = new ExtendedNode<int>(5);
	ExtendedNode<int>* n7 = new ExtendedNode<int>(7);
	BinaryTree::attach_left(root, n2);
	BinaryTree::attach_right(root, n6);
	BinaryTree::attach_left(n2, n1);
	BinaryTree::attach_right(n2, n3);
	BinaryTree::attach_left(n6, n5);
	BinaryTree::attach_right(n6, n7);
	return root;
}

std::string join(const std::vector<int>& v) {
	std::ostringstream out;
	for (std::size_t i = 0; i < v.size(); ++i) {
		if (i != 0) { out << ","; }
		out << v[i];
	}
	return out.str();
}

void test_binary_tree() {
	section("BinaryTree");

	using namespace BinaryTree;
	Node<int>* root = build_plain();

	check_eq(height(root), 2, "height in edges");
	check_eq(leaves(root), 4, "leaf count");
	check_eq(points(root), 7, "node count");
	check(is_strict(root), "tree is strict");
	check_eq(find_max(root), 7, "find_max");   // used to return a dangling ref

	// The _rec family used to call the ExtendedNode overloads instead of
	// recursing, so none of these produced the right sequence.
	std::vector<int> seen;
	auto collect = [&seen](const Node<int>* n) { seen.push_back(n->m_info); };

	seen.clear();
	straight_traverse_rec(root, collect);
	check_eq(join(seen), std::string("4,2,1,3,6,5,7"), "pre-order (recursive)");

	seen.clear();
	symmetrical_traverse_rec(root, collect);
	check_eq(join(seen), std::string("1,2,3,4,5,6,7"), "in-order (recursive)");

	seen.clear();
	reverse_traverse_rec(root, collect);
	check_eq(join(seen), std::string("1,3,2,5,7,6,4"), "post-order (recursive)");

	seen.clear();
	straight_with_stack(root, collect);
	check_eq(join(seen), std::string("4,2,1,3,6,5,7"), "pre-order (stack)");

	seen.clear();
	symmetrical_with_stack(root, collect);
	check_eq(join(seen), std::string("1,2,3,4,5,6,7"), "in-order (stack)");

	seen.clear();
	reverse_with_stack(root, collect);
	check_eq(join(seen), std::string("1,3,2,5,7,6,4"), "post-order (stack)");

	seen.clear();
	print_by_level(root, collect);
	check_eq(join(seen), std::string("4,2,6,1,3,5,7"), "level order (queue)");

	// print_i_level had a stray comma operator that dropped the right subtree.
	seen.clear();
	print_by_level_rec(root, collect);
	check_eq(join(seen), std::string("4,2,6,1,3,5,7"), "level order (recursive)");

	destroy(root);

	// Parent-pointer traversals: these did not compile before, because the
	// find_next_* helpers returned Node<T>* from an ExtendedNode<T>* function.
	ExtendedNode<int>* ext = build_extended();

	seen.clear();
	straight_traverse(ext, [&seen](const ExtendedNode<int>* n) { seen.push_back(n->m_info); });
	check_eq(join(seen), std::string("4,2,1,3,6,5,7"), "pre-order (parent pointers)");

	seen.clear();
	symmetrical_traverse(ext, [&seen](const ExtendedNode<int>* n) { seen.push_back(n->m_info); });
	check_eq(join(seen), std::string("1,2,3,4,5,6,7"), "in-order (parent pointers)");

	destroy(static_cast<Node<int>*>(ext));

	// Degenerate shapes.
	check_eq(height(static_cast<Node<int>*>(nullptr)), -1, "height of empty tree");
	check_eq(leaves(static_cast<Node<int>*>(nullptr)), 0, "leaves of empty tree");
	check(is_strict(static_cast<Node<int>*>(nullptr)), "empty tree is strict");

	Node<int>* single = new Node<int>(1);
	check_eq(height(single), 0, "height of single node");
	single->m_left = new Node<int>(2);
	check(!is_strict(single), "one-child tree is not strict");
	destroy(single);
}

// ------------------------------------------------------------ SearchTree ----

void test_search_tree() {
	section("SearchTree");

	SearchTree<int> t;
	check(t.empty(), "new tree is empty");

	// empty() was `return m_root = nullptr;`, which wiped the root on every
	// call. If that regressed, the inserts below would vanish.
	int values[] = { 50, 30, 70, 20, 40, 60, 80 };
	for (int v : values) { t.insert(v); }
	check(!t.empty(), "tree is not empty after inserts");
	check_eq(t.size(), 7, "size after inserts");
	check_eq(join(t.to_vector()), std::string("20,30,40,50,60,70,80"), "in-order is sorted");

	check(t.contains(40), "contains an inserted value");
	check(!t.contains(41), "does not contain a missing value");
	check(t.find(70) != nullptr, "find returns a node");
	check(t.find(71) == nullptr, "find returns null when absent");

	// Each delete case.
	check(t.remove(20), "remove a leaf");
	check_eq(join(t.to_vector()), std::string("30,40,50,60,70,80"), "after removing a leaf");

	check(t.remove(30), "remove a node with one child");
	check_eq(join(t.to_vector()), std::string("40,50,60,70,80"), "after removing a one-child node");

	check(t.remove(70), "remove a node with two children");
	check_eq(join(t.to_vector()), std::string("40,50,60,80"), "after removing a two-child node");

	check(t.remove(50), "remove the root");
	check_eq(join(t.to_vector()), std::string("40,60,80"), "after removing the root");
	check_eq(t.size(), 3, "size tracks removals");

	check(!t.remove(999), "removing a missing value reports false");
	check_eq(t.size(), 3, "size unchanged by a failed removal");

	SearchTree<int> copy = t;
	copy.remove(40);
	check_eq(copy.size(), 2, "copy is independent (copy)");
	check_eq(t.size(), 3, "copy is independent (source)");

	SearchTree<int> assigned;
	assigned.insert(1);
	assigned = t;
	check_eq(join(assigned.to_vector()), std::string("40,60,80"), "assignment copies contents");

	// Assigning an empty tree over a populated one.
	SearchTree<int> empty;
	assigned = empty;
	check(assigned.empty(), "assigning an empty tree clears the target");

	// Duplicates.
	SearchTree<int> dup;
	dup.insert(5);
	dup.insert(5);
	dup.insert(5);
	check_eq(dup.size(), 3, "duplicates are stored");
	check_eq(join(dup.to_vector()), std::string("5,5,5"), "duplicates in order");
	dup.remove(5);
	check_eq(dup.size(), 2, "removing one duplicate");
}

// -------------------------------------------------------------- MultiSet ----

void test_multiset() {
	section("MultiSet");

	int a[] = { 1, 2, 2, 3, 4 };
	int b[] = { 2, 3, 3, 5 };
	MultiSet<int> A(a, 5);
	MultiSet<int> B(b, 4);

	check_eq(A.size(), 5, "A size");
	check_eq(A.count(2), 2, "A counts duplicates");

	// Previously these returned a reference to a by-value parameter.
	MultiSet<int> inter = A & B;
	check_eq(join(inter.elements()), std::string("2,3"), "intersection");

	MultiSet<int> uni = A | B;
	check_eq(join(uni.elements()), std::string("1,2,2,3,3,4,5"), "union");

	MultiSet<int> diff = A - B;
	check_eq(join(diff.elements()), std::string("1,2,4"), "difference");

	// Operands must not be modified by the free operators.
	check_eq(join(A.elements()), std::string("1,2,2,3,4"), "A unchanged");
	check_eq(join(B.elements()), std::string("2,3,3,5"), "B unchanged");

	MultiSet<int> A2(a, 5);
	check(A == A2, "equality");
	check(!(A == B), "inequality");
	check(A != B, "operator!=");

	int sub[] = { 1, 2 };
	MultiSet<int> S(sub, 2);
	check(S < A, "proper subset");
	check(!(A < S), "not a superset of itself the other way");
	check(!(A < A), "a set is not a proper subset of itself");
	check(A > S, "operator>");
	check(A <= A2, "operator<=");
	check(A >= A2, "operator>=");

	// Empty sets must not crash: str_first() used to return null and get
	// dereferenced straight away.
	MultiSet<int> empty;
	check_eq(empty.size(), 0, "empty size");
	check_eq(join(empty.elements()), std::string(""), "empty elements");
	MultiSet<int> still_empty = empty & A;
	check_eq(still_empty.size(), 0, "empty intersection");
	MultiSet<int> from_empty = empty | A;
	check_eq(join(from_empty.elements()), std::string("1,2,2,3,4"), "union with empty");

	std::ostringstream out;
	out << inter;
	check_eq(out.str(), std::string("{2, 3}"), "stream output");
}

// --------------------------------------------------------------- Polynom ----

void test_polynom() {
	section("Polynom");

	// 3x^2 + 2x + 1
	Polynom::Information info;
	info.push_back(std::make_pair(3.0, 2));
	info.push_back(std::make_pair(2.0, 1));
	info.push_back(std::make_pair(1.0, 0));
	Polynom p(info);

	check_eq(p.exponent(), 2, "degree");
	check_eq(p.count_value(0.0), 1.0, "value at 0");
	check_eq(p.count_value(1.0), 6.0, "value at 1");
	check_eq(p.count_value(2.0), 17.0, "value at 2");

	Polynom copy = p;
	check_eq(copy.count_value(2.0), 17.0, "copy evaluates the same");

	Polynom sum = p + p;
	check_eq(sum.count_value(2.0), 34.0, "sum evaluates correctly");
	check_eq(p.count_value(2.0), 17.0, "operand unchanged by +");

	Polynom scaled = p * 2.0;
	check_eq(scaled.count_value(2.0), 34.0, "scalar multiply");

	Polynom product = p * p;
	check_eq(product.count_value(2.0), 289.0, "polynomial multiply");

	// d/dx (3x^2 + 2x + 1) = 6x + 2
	Polynom d = p;
	d.derivative();
	check_eq(d.count_value(0.0), 2.0, "derivative at 0");
	check_eq(d.count_value(1.0), 8.0, "derivative at 1");

	std::ostringstream out;
	out << p;
	check_eq(out.str(), std::string("3x^2 + 2x + 1"), "stream output");

	Polynom zero;
	std::ostringstream zout;
	zout << zero;
	check_eq(zout.str(), std::string("0"), "empty polynomial prints 0");
	check_eq(zero.count_value(5.0), 0.0, "empty polynomial evaluates to 0");
}

// ----------------------------------------------------------------- Radix ----

void test_radix() {
	section("Radix sort");

	int a[] = { 7, 4, 3, 5, 8, 0, 1, 6 };
	radix::sort(a, 8);
	check_eq(join(std::vector<int>(a, a + 8)), std::string("0,1,3,4,5,6,7,8"),
		"sorts non-negative input");

	// The old version sorted by raw bits, so the sign bit put every negative
	// number last.
	int n[] = { 3, -1, 0, -7, 5, -2 };
	radix::sort(n, 6);
	check_eq(join(std::vector<int>(n, n + 6)), std::string("-7,-2,-1,0,3,5"),
		"sorts negative input");

	int one[] = { 42 };
	radix::sort(one, 1);
	check_eq(one[0], 42, "single element");

	radix::sort(nullptr, 0);
	check(true, "null input is a no-op");
}

void run_all() {
	test_stack();
	test_queue();
	test_linked_stack();
	test_linked_queue();
	test_linked_list();
	test_heap();
	test_binary_tree();
	test_search_tree();
	test_multiset();
	test_polynom();
	test_radix();
}

}  // namespace

int main() {
	// The first pass also warms up anything the standard library allocates
	// lazily (locales, stream buffers), so the second pass measures only our
	// own allocations.
	run_all();

	std::size_t allocations_before = g_allocations;
	std::size_t deallocations_before = g_deallocations;

	int checks_before = g_checks;
	int failures_before = g_failures;
	run_all();
	g_checks -= checks_before;
	g_failures -= failures_before;

	// Signed: the warm-up pass can leave a lazily-grown stream buffer that is
	// released during the measured pass, so freed may legitimately exceed
	// allocated by a couple. Only a positive difference is a leak.
	long long allocated = static_cast<long long>(g_allocations - allocations_before);
	long long freed = static_cast<long long>(g_deallocations - deallocations_before);

	std::cout << "\nMemory\n";
	++g_checks;
	if (allocated > freed) {
		++g_failures;
		std::cout << "  FAIL: " << (allocated - freed) << " of " << allocated
			<< " allocations were never freed\n";
	}
	else {
		std::cout << "  " << allocated << " allocations, all freed\n";
	}

	std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
	if (g_failures != 0) {
		std::cout << g_failures << " FAILED\n";
		return 1;
	}
	std::cout << "All tests passed.\n";
	return 0;
}
