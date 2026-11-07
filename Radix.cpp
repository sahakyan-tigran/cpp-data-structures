// Demo for radix::sort. Build with:
//     g++ -std=c++17 Radix.cpp -o radix

#include "Radix.h"

#include <iostream>

int main() {
	int a[] = { 7, 4, -3, 5, 8, 0, -1, 6 };
	const int size = static_cast<int>(sizeof(a) / sizeof(a[0]));

	radix::sort(a, size);

	for (int i = 0; i < size; ++i) {
		std::cout << a[i] << (i + 1 == size ? '\n' : ' ');
	}
	return 0;
}
