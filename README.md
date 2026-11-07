# Data Structures

Classic data structures and algorithms written from scratch in C++ — no STL
containers in the implementations, only in the tests. Each one is a single
header you can drop into a project.

Built as university coursework and since cleaned up: the code is portable
(GCC / Clang / MSVC), covered by a test suite, and checked by CI on Linux,
macOS and Windows.

![tests](https://github.com/sahakyan-tigran/cpp-data-structures/actions/workflows/ci.yml/badge.svg)

## Contents

| Header | What it is |
| --- | --- |
| [`stack.h`](stack.h) | LIFO stack over a growable array |
| [`queue.h`](queue.h) | FIFO queue as a circular buffer |
| [`LinkedStack.h`](LinkedStack.h) | LIFO stack over a singly-linked list |
| [`LinkedQueue.h`](LinkedQueue.h) | FIFO queue over a singly-linked list, O(1) push |
| [`LinkedList.h`](LinkedList.h) | Doubly-linked list of ints, 1-based indices |
| [`HeapTree.h`](HeapTree.h) | Binary max-heap over a 1-indexed array |
| [`HeapSort.h`](HeapSort.h) | In-place heap sort built on `HeapTree` |
| [`BinaryTree.h`](BinaryTree.h) | Tree nodes, metrics and seven traversals |
| [`SearchTree.h`](SearchTree.h) | Unbalanced BST allowing duplicates |
| [`MultiSet.h`](MultiSet.h) | Multiset over a BST, with set algebra |
| [`Polynom.h`](Polynom.h) | Sparse polynomial over a circular linked list |
| [`Radix.h`](Radix.h) | LSD radix sort, one bit per pass |

## Building and testing

Everything except `LinkedList` and `Polynom` is header-only, so most of the
time you just include a header. To run the test suite:

```sh
make test
```

or directly:

```sh
g++ -std=c++17 -Wall -Wextra -I. tests/test_all.cpp LinkedList.cpp Polynom.cpp -o test_all && ./test_all
```

The suite covers ~1200 assertions across every container — ordering, growth
past the initial capacity, deep-copy independence, self-assignment, the error
paths, and the degenerate cases (empty containers, single nodes, removing the
root). It also overrides global `operator new`/`delete`, runs the whole suite
twice, and fails if the two counts do not balance, which is how the destructors
and `deallocate()` helpers are checked for leaks.

There is also a small demo for the radix sort:

```sh
make demo
```

## Usage

```cpp
#include "stack.h"
#include "SearchTree.h"
#include "MultiSet.h"

Stack<std::string> s;
s.push("alpha");
s.push("beta");
s.top();            // "beta"
s.pop();

SearchTree<int> t;
t.insert(50);
t.insert(30);
t.insert(70);
t.to_vector();      // {30, 50, 70}, in-order
t.remove(50);       // handles the leaf / one-child / two-child cases

int a[] = {1, 2, 2, 3};
int b[] = {2, 3, 3, 5};
MultiSet<int> A(a, 4), B(b, 4);
(A & B).elements(); // {2, 3}
(A | B).elements(); // {1, 2, 2, 3, 3, 5}
(A - B).elements(); // {1, 2}
```

### Trees

`BinaryTree.h` provides two node types. `Node<T>` is a plain node; traversals
over it are either recursive (`straight_traverse_rec`, `symmetrical_traverse_rec`,
`reverse_traverse_rec`, `print_by_level_rec`) or use an explicit container
(`straight_with_stack`, `symmetrical_with_stack`, `reverse_with_stack`,
`print_by_level`).

`ExtendedNode<T>` additionally stores a parent pointer, which lets
`straight_traverse`, `symmetrical_traverse` and `reverse_traverse` walk the tree
iteratively with no auxiliary storage at all. Build those trees with
`attach_left` / `attach_right` so the parent links stay consistent, and free
them with `destroy`.

Every traversal takes any callable, so a lambda works:

```cpp
std::vector<int> seen;
BinaryTree::symmetrical_with_stack(root, [&](auto* n) { seen.push_back(n->m_info); });
```

## Notes on the containers

- `MultiSet` set algebra follows the usual multiset definitions, comparing
  multiplicities element by element: intersection takes `min(count_a, count_b)`,
  union takes `max`, difference takes `max(0, count_a - count_b)`.
- `SearchTree` is deliberately unbalanced — inserting sorted input degrades it
  to a linked list. Balancing it is the obvious next exercise.
- `LinkedList` uses 1-based indices, so `at(1)` is the head and
  `insert(size() + 1, x)` appends.
- The containers are copyable but not movable; they predate the point where
  move semantics would have been taught, and adding them would change the
  exercise rather than finish it.

## License

MIT
