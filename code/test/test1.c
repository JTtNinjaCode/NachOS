// #include "syscall.h"
// main()
// 	{
// 		int	n;
// 		for (n=9;n>0;n--);
// }

#define ARRAY_SIZE 10000
#include "syscall.h"
unsigned char ary[ARRAY_SIZE];
main() {
	// 把陣列內的所有值都設為 5，但每次迴圈都戳一下 ary[3250]
	// 由於 ary[3250] 會一直特定戳某個 page，使得它在整個過程中不會被換走
	// 另外，page 是直接先放進去的，ary[3250] 的位置在 (3250 + 512) \ 128 = 29
	// 496 是因為此程式碼的長度就是 512 byte，而 data 區段緊接在 code 區段後
	// 因此合理的情況是，29 page 會一直待在 RAM 裡面不會被踢出去
	int	n = 0, total = 0;
	PrintInt(&ary[3250]); // 印出 ary[3250] 的地址，因為是剛開始，它正好等同於 physical addr 的地址
	for (n = 0;n < ARRAY_SIZE; n++) {
		ary[n] = 5;
		ary[3250] = 5; // 戳
	}
	for (n = 0;n < ARRAY_SIZE; n++) {
		total += ary[n];
		ary[3250] = 5; // 戳
	}
	PrintInt(total);
}

// #define ARRAY_SIZE 10000
// #include "syscall.h"
// unsigned char ary[ARRAY_SIZE];
// int total;
// main() {
// 	int	n = 0, total = 0;
// 	// 全部設為 5
// 	for (n = 0;n < ARRAY_SIZE; n++) {
// 		ary[n] = 5;
// 		PrintInt(n);
// 	}
// 	for (n = 0;n < ARRAY_SIZE; n++) {
// 		total += ary[n];
// 	}
// 	PrintInt(total);
// }