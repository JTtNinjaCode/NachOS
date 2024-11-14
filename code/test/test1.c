// #include "syscall.h"
// main()
// 	{
// 		int	n;
// 		for (n=9;n>0;n--) PrintInt(n);

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
// 	}

#define ARRAY_SIZE 10000
#include "syscall.h"
char ary[ARRAY_SIZE];
main() {
	int	n = 0, total = 0;
	// 全部設為 1
	for (n = 0;n < ARRAY_SIZE; n++)
		ary[n] = 1;

	// 全部加起來
	for (n = 0;n < ARRAY_SIZE; n++)
		total = total + ary[n];

	// 預計結果為 10000
	PrintInt(total);
}