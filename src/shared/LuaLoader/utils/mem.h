#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__MEM_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__MEM_H_ 1

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>

#include "../mod_recomp.h"
#include "./logging.h"
#include "./types.h"

static void cleanup_free(void *ptr_to_object_arg) {
	void **ptr_to_object = ptr_to_object_arg;
	if (ptr_to_object == NULL) return;

	void *object = *ptr_to_object;
	if (object == NULL) return;

	//LOG("Freeing object at 0x%016"PRIx64, (u64)object);
	free(object);
	object = NULL;
	*ptr_to_object = NULL;
}

#define AUTO_FREE __attribute__((__cleanup__(cleanup_free)))

#endif
