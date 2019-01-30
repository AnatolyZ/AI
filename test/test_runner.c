/*
 * test_runner.c
 *
 *  Created on: 30 џэт. 2019 у.
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
