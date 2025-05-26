#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__DEBUG__PPRINT_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__DEBUG__PPRINT_H_ 1

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>

void pprint_hexdump(const uint8_t *restrict data, size_t data_size) {
	printf("Hexdump for 0x%016"PRIx64":\n", (uint64_t)data);
	printf("       | 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f | 0 1 2 3 4 5 6 7 8 9 a b c d e f\n");
	printf("-------+-------------------------------------------------+--------------------------------\n");

	if (data_size == 0) {
		return;
	}

	for (size_t i = 0ULL; i < data_size; i++) {
		if (i % 16 == 0) {
			printf("0x%04zx | ", i);
		}

		uint8_t byte = data[i];
		if (byte) {
			printf("%02"PRIx8, data[i]);
		} else {
			printf("\e[2m00\e[22m");
		}

		if (i % 16 == 15) {
			printf(" |");
			for (int j = 15; j >= 0; j--) {
				uint8_t byte = data[i - j];
				if ((32 <= byte) && (byte <= 126)) {
					printf(" %c", byte);
				} else {
					printf(" \e[2m.\e[22m");
				}
			}
			printf("\n");
		} else {
			printf(" ");
		}
	}

	printf("\n");
}

#endif
