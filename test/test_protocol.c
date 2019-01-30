/*
 * test_protocol.c
 *
 *  Created on: 30 џэт. 2019 у.
 *      Author: AZharkov
 */


/* Includes */
#include "test_protocol.h"
/* -------- */

void TestCalculateFCS(){
	uint8_t buf[10];
	memset(buf,0x00,sizeof(buf));
	TEST_ASSERT_EQUAL_HEX8(0x00, CalculateFCS(buf,2));
}
