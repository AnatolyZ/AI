/*
 * test_runner.c
 *
 *  Created on: 30 ���. 2019 �.
 *      Author: AZharkov
 */

/* Includes */
#include "test_runner.h"
/* -------- */

void RunAllTests(void) {
	UNITY_BEGIN();
	RUN_TEST(TestCalculateFCS);
	UNITY_END();
}
