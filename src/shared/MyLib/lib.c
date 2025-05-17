#include <stdio.h>
#include <inttypes.h>

#define DLLEXPORT __attribute__((visibility("default")))

DLLEXPORT const uint32_t recomp_api_version = 1;

DLLEXPORT void MyLib_MyFunction(void) {
	fprintf(stderr, ">>> Hello from shared library function %s(), at %s:%d!\n", __func__, __FILE__, __LINE__);
}
