// #include "syscall.h"
// main()
// 	{
// 		int	n;
// 		for (n=9;n>0;n--);
			// PrintInt(n);

// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// 		// PrintInt(n);
// }

#define ARRAY_SIZE 10000
#include "syscall.h"
unsigned char ary[ARRAY_SIZE];
int total;
main() {
	int	n = 0, total = 0;
	// 全部設為 1
	for (n = 0;n < ARRAY_SIZE; n++) {
		ary[n] = 5;
		PrintInt(n);
	}
	for (n = 0;n < ARRAY_SIZE; n++) {
		total += ary[n];
	}
	PrintInt(total);
}