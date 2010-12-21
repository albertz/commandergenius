#include <stdio.h>
#include "test.h"

int test::initCount = 0;
int test::initCount2 = 12345;

test::test()
{
	initCount++;
	__android_log_print(ANDROID_LOG_INFO, "==TEST==", "test::test(): initCount %d initCount2 %d", initCount, initCount2);
}

test::~test()
{
	initCount--;
	__android_log_print(ANDROID_LOG_INFO, "==TEST==", "test::~test(): initCount %d initCount2 %d", initCount, initCount2);
}

