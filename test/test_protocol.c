/*
 * test_protocol.c
 *
 *  Created on: 30 џэт. 2019 у.
 *      Author: AZharkov
 */

/* Includes */
#include "test_protocol.h"
/* -------- */

void TestCalculateFCS() {
	uint8_t buf[3];
	memset(buf, 0x00, sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x01;
	buf[2] = 0x01;
	TEST_ASSERT_EQUAL_HEX8(0x03, CalculateFCS(buf, 3));

	memset(buf, 0x00, sizeof(buf));
	buf[0] = 0xAA;
	buf[1] = 0x0F;
	buf[2] = 0xCD;
	TEST_ASSERT_EQUAL_HEX8(0x86, CalculateFCS(buf, 3));
}

void TestGetNextMaster() {
	profibus_MPI_t test_prot;
	test_prot.own_address = 0x03U;
	test_prot.bus_masters = 0x00;
	SET_BIT(test_prot.bus_masters, (0x01U << 0x05U));
	SET_BIT(test_prot.bus_masters, (0x01U << 0x09U));
	SET_BIT(test_prot.bus_masters, (0x01U << 0x0FU));
	TEST_ASSERT_EQUAL_HEX8(0x05, GetNextMaster(&test_prot));

	test_prot.own_address = 0x06U;
	TEST_ASSERT_EQUAL_HEX8(0x09, GetNextMaster(&test_prot));

	test_prot.own_address = 0x0DU;
	TEST_ASSERT_EQUAL_HEX8(0x0F, GetNextMaster(&test_prot));

	test_prot.own_address = 0x12U;
	TEST_ASSERT_EQUAL_HEX8(0x05, GetNextMaster(&test_prot));
}

void TestCheckMaster() {
	profibus_MPI_t test_prot;
	test_prot.own_address = 0x03U;
	test_prot.bus_masters = 0x00;
	SET_BIT(test_prot.bus_masters, (0x01U << 0x05U));
	SET_BIT(test_prot.bus_masters, (0x01U << 0x09U));
	SET_BIT(test_prot.bus_masters, (0x01U << 0x0FU));

	TEST_ASSERT_EQUAL_HEX8(0U, CheckMaster(&test_prot,0x00));
	TEST_ASSERT_EQUAL_HEX8(0U, CheckMaster(&test_prot,0x04));
	TEST_ASSERT_EQUAL_HEX8(1U, CheckMaster(&test_prot,0x05));
	TEST_ASSERT_EQUAL_HEX8(1U, CheckMaster(&test_prot,0x09));
	TEST_ASSERT_EQUAL_HEX8(1U, CheckMaster(&test_prot,0x0F));

}

