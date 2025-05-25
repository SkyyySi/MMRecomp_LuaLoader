# Additional documentation for `mod_recomp.h`

## An explanation for the `MEM_[…]` function-like macros.

`MEM_[…]` is needed to dereference a pointer to values in N64 memory from native
code. When an N64 function calls a native function with a pointer or with an
array, the native function will only get an offset for `rdram`. Attempting to
directly dereference this pointer from native code (i.e. `*((T *)offset)`,
where `T` is the integer type that's being pointed to) is invalid and will
most likely cause the game to crash due to a segmentation fault. It is also
incorrect to directly use this pointer to index into the N64 memory view
(i.e. `rdram[offset]`). This will probably not crash, but the values you'll
get this way will be in completely the wrong order, which is especially
problematic if the given offset is a pointer to a `NULL`-terminated array.
Instead of doing either of those, this macro must be used.

### Examples

> **Todo**
> Test if the example code snippets below actually work.

Passing a byte-array with an explicit size from N64 code to native code.

#### N64 code

```c
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

RECOMP_IMPORT(".", void MyLib_MyFunc(const u8 *data, s32 data_size));

RECOMP_HOOK("Player_Init") void my_player_init_hook(Actor *thisx, PlayState *play) {
    const u8 my_array[] = { 0xDe, 0xad, 0xBe, 0xef };
    MyLib_MyFunc(my_array, (s32)sizeof(my_array));
}
```

#### Native code

```c
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "mod_recomp.h"

RECOMP_EXPORT void MyLib_MyFunc(uint8_t *rdram, recomp_context *ctx) {
    // Argument #1 is the pointer to the array's first item.
    gpr data = ctx->r4;
    // Argument #2 holds the size of the array.
    size_t data_size = (size_t)MEM_W(ctx->r5, 0);

    // We create a local copy of the data since that is both much easier to
    // pass around between other (potentially external) host functions, as
    // well as there being far less risk of messing up actually reading the
    // data, for example, by trying to directly index into `ctx->r4`.
    uint8_t *data_native = (uint8_t *)malloc((data_size + 1) * sizeof(uint8_t));

    for (size_t i = 0; i < data_size; i++) {
        data_native[i] = MEM_BU(data, 0);
    }

    data_native[data_size] = 0;

    // Now do something useful with the data here...
    printf("data_native[%zu] = {\n", data_size);
    for (size_t i = 0; i < data_size; i++) {
        printf("    [%zu] = 0x%02"PRIx8",\n", i, data_native[i]);
    }
    printf("};\n");

    free(data);
    data = NULL;
}
```

Passing a `NULL`-terminated C-string from N64 code to native code.

#### N64 code

```c
// TODO
```

#### Native code

```c
// TODO
```
