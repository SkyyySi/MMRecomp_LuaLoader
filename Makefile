BUILD_DIR := build

# Allow the user to specify the compiler and linker on macOS
# as Apple Clang does not support MIPS architecture
ifeq ($(OS),Windows_NT)
    CC      := clang
    LD      := ld.lld
else ifneq ($(shell uname),Darwin)
    CC      := clang
    LD      := ld.lld
else
    CC      ?= clang
    LD      ?= ld.lld
endif

TARGET  := $(BUILD_DIR)/mod.elf

LDSCRIPT := mod.ld
CFLAGS   := -target mips -mips2 -mabi=32 -O2 -G0 -mno-abicalls -mno-odd-spreg -mno-check-zero-division \
			-fomit-frame-pointer -ffast-math -fno-unsafe-math-optimizations -fno-builtin-memset \
			-Wall -Wextra -Wno-incompatible-library-redeclaration -Wno-unused-parameter -Wno-unknown-pragmas -Wno-unused-variable \
			-Wno-missing-braces -Wno-unsupported-floating-point-opt -Werror=section
CPPFLAGS := -nostdinc -D_LANGUAGE_C -DMIPS -DF3DEX_GBI_2 -DF3DEX_GBI_PL -DGBI_DOWHILE -I include -I include/dummy_headers \
			-I mm-decomp/include -I mm-decomp/src -I mm-decomp/extracted/n64-us -I mm-decomp/include/libc
LDFLAGS  := -nostdlib -T $(LDSCRIPT) -Map $(BUILD_DIR)/mod.map --unresolved-symbols=ignore-all --emit-relocs -e 0 --no-nmagic

C_SRCS := $(wildcard src/*.c)
C_OBJS := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o))
C_DEPS := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.d))

$(TARGET): $(C_OBJS) $(LDSCRIPT) | $(BUILD_DIR)
	$(LD) $(C_OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR) $(BUILD_DIR)/src $(BUILD_DIR)/src/shared:
ifeq ($(OS),Windows_NT)
	mkdir $(subst /,\,$(BUILD_DIR))\\src\\shared
else
	mkdir -p -- $(BUILD_DIR)/src/shared
endif

SHARED_LIB_NAME    := LuaLoader
SHARED_LIB_VERSION := 1.0.0

SHARED_LIB_BIN_BASE_PATH := $(BUILD_DIR)/src/shared/$(SHARED_LIB_NAME)-$(SHARED_LIB_VERSION)
SHARED_LIB_SOURCE_PATH := ./src/shared/$(SHARED_LIB_NAME)/lib.c
#ZIG_CFLAGS := -static -shared -DLUA_USE_LINUX -fPIC -I./include -I./src/shared/LuaLoader/lua/src -lm -Wl,-E -ldl # -O2 -g
ZIG_CFLAGS := -std=c23 -static -shared -fPIC
ZIG_LDFLAGS := -I./include -I./src/shared/LuaLoader/lua/src

$(C_OBJS): $(BUILD_DIR)/%.o : %.c | $(BUILD_DIR) $(BUILD_DIR)/src
	$(CC) $(CFLAGS) -ggdb -O0 $(CPPFLAGS) $< -MMD -MF $(@:.o=.d) -c -o $@
#	$(CC) -shared -fPIC -I./include \
#		-o $(BUILD_DIR)/src/shared/$(SHARED_LIB_NAME)-$(SHARED_LIB_VERSION).so \
#		./src/shared/$(SHARED_LIB_NAME)/lib.c
	zig cc \
		$(ZIG_CFLAGS) \
		-ggdb -O0 \
		-target x86_64-linux-gnu \
		-DLUA_USE_LINUX -Wl,-E -ldl \
		-o $(SHARED_LIB_BIN_BASE_PATH).so \
		$(SHARED_LIB_SOURCE_PATH) \
		./src/shared/LuaLoader/lua/src/*.c \
		$(ZIG_LDFLAGS)
#zig cc -target x86_64-linux-gnu $(ZIG_CFLAGS) -march=native -flto -ldl -o $(SHARED_LIB_BIN_BASE_PATH).so    $(SHARED_LIB_SOURCE_PATH) ./src/shared/LuaLoader/lua/src/*.c
#zig cc -target x86_64-windows   $(ZIG_CFLAGS) -march=native -flto -s   -o $(SHARED_LIB_BIN_BASE_PATH).dll   $(SHARED_LIB_SOURCE_PATH) ./src/shared/LuaLoader/lua/src/*.c
#zig cc -target aarch64-macos    $(ZIG_CFLAGS)                          -o $(SHARED_LIB_BIN_BASE_PATH).dylib $(SHARED_LIB_SOURCE_PATH) ./src/shared/LuaLoader/lua/src/*.c

clean:
	rm -rf $(BUILD_DIR)

-include $(C_DEPS)

.PHONY: clean
