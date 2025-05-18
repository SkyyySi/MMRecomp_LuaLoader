#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

//#include "./shared/LuaLoader/lib.h"
RECOMP_IMPORT(".", u32 LuaLoader_Init(void));

#define WITH_SIZE(STRING_LITERAL) (STRING_LITERAL), (sizeof(STRING_LITERAL))

#define LOG(...) { \
	recomp_printf("\e[1;32m>>>\e[22;39m "); \
	recomp_printf(__VA_ARGS__); \
	recomp_printf("\e[0m \e[1;35m<<<\e[22;39m \e[36m[\e[32m%s\e[39m() \e[35m::\e[39m \e[33m%s\e[35m:\e[34m%u\e[36m]\e[0m\n", (__func__), (__FILE__), (__LINE__)); \
}

#define LOG_TYPE_SIZE(TYPE) \
LOG("sizeof("#TYPE") = %llu", sizeof(TYPE));

/*
sizeof(char) = 1
sizeof(int) = 4
sizeof(long int) = 4
sizeof(size_t) = 4
sizeof(void*) = 4
sizeof(s8) = 1
sizeof(s16) = 2
sizeof(s32) = 4
sizeof(s64) = 8
sizeof(u8) = 1
sizeof(u16) = 2
sizeof(u32) = 4
sizeof(u64) = 8
*/

static void format_bits_u64(u64 x, char *out_buffer, unsigned long buffer_size) {
	if ((out_buffer == NULL) || (buffer_size < 72)) {
		return;
	}

	int offset = 0;
	for (int i = 0; i < 64; i++) {
		out_buffer[i + offset] = (x & (1ULL << (63 - i))) ? '1' : '0';

		if ((i > 0) && ((i % 8) == 7)) {
			offset++;
			out_buffer[i + offset] = '.';
		}
	}

	out_buffer[buffer_size - 1] = '\0';
}

RECOMP_HOOK("Player_Init") void my_player_init_hook(Actor *thisx, PlayState *play) {
	char bits[72];

	u32 L = LuaLoader_Init();
	format_bits_u64(L, WITH_SIZE(bits));
	LOG("0x%016lx || 0b%s || %20lu", L, bits, L);

	/* u64 foo = 0xFFFFFFFFFFFFFFFFUL;
	format_bits_u64(foo, WITH_SIZE(bits));
	LOG("foo  = 0x%016llx || 0b%s", foo, bits);
	format_bits_u64(((u64)(&foo)), WITH_SIZE(bits));
	LOG("&foo = 0x%016llx || 0b%s", ((u64)(&foo)), bits); */

	/* void *L_as_ptr = (void *)L_as_u64;
	format_bits_u64((u64)L_as_ptr, WITH_SIZE(bits));
	LOG("0x%016llx || 0b%s", (u64)L_as_ptr, bits); */

	/* char script_code[] = "print(\"Hello, world!\")\n"; */

	/* LuaLoader_InvokeScriptCode_Args args = {
		.L = (u64)L,
		.script_code = (u64)script_code,
		.script_code_size = (u32)(sizeof(script_code)),
	}; */

	/* u64 args_ptr = (u64)(&args); */

	//recomp_printf(">>> LuaLoader_InvokeScriptCode returned %ld\n", LuaLoader_InvokeScriptCode(L, &script_code, sizeof(script_code)));
	/* recomp_printf(">>> LuaLoader_InvokeScriptCode returned %d\n", LuaLoader_InvokeScriptCode(
		(args_ptr & 0x00000000FFFFFFFFull),
		(args_ptr & 0xFFFFFFFF00000000ull) >> 32
	)); */

	/* // Reduce health by one heart
	bool player_has_health_remaining = Health_ChangeBy(play, -16);

	recomp_printf(
		">>> bool player_has_health_remaining = %s;\n",
		(player_has_health_remaining ? "true" : "false")
	); */

	/* LuaLoader_Deinit(L); */
}
