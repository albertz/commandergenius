
#include <android/log.h>

extern void unaligned_test2(int x1, int x2, int x3, int x4, int x5, int x6, int x7, unsigned * data, unsigned * target);
extern void unaligned_test(unsigned * data, unsigned * target);
extern unsigned val0, val1, val2, val3, val4;

unsigned val0 = 1, val1 = 2, val2 = 3, val3 = 4, val4 = 5;

void unaligned_test2(int x1, int x2, int x3, int x4, int x5, int x6, int x7, unsigned * data, unsigned * target)
{
	int xx = x1 + x2 - x3 + x4 - x5 + x6 - x7;
	*target = xx + data[3];
}

void unaligned_test(unsigned * data, unsigned * target)
{
	//__android_log_print(ANDROID_LOG_INFO, "Ballfield", "data %p target %p", data, target);
	int x1 = *data, x2 = *target, x3 = *data + *target, x4 = *data - *target, x5 = *data - *target * 2, x6 = *data + *target * 2, x7 = *target * 3 + *data;
	unaligned_test2(x1, x2, x3, x4, x5, x6, x7, data-3, target);
};

