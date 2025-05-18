#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "./shared/LuaLoader/lib.h"

#define WITH_SIZE(STRING_LITERAL) (STRING_LITERAL), (sizeof(STRING_LITERAL))

//int counter = 0;

/* #define LOGF(...) \
fprintf(stderr, "[%s() :: %s:%d] >>> " __VA_ARGS__); */

#define LOGF(...) \
recomp_printf("[%s() :: %s:%d] >>> " __VA_ARGS__);

#define LOG_TYPE_SIZE(TYPE) \
recomp_printf("[%s() :: %s:%d] >>> sizeof(%s) = %d;\n", (#TYPE), sizeof(TYPE));

RECOMP_HOOK("Player_Init") void my_player_init_hook(Actor *thisx, PlayState *play) {
	LOG_TYPE_SIZE(int);
	LOG_TYPE_SIZE(size_t);
	//counter++;
	//if (counter < 3) return;

	void *L = NULL;//LuaLoader_Init();

	char script_code[] = "print(\"Hello, world!\")\n";

	LuaLoader_InvokeScriptCode_Args args = {
		.L = (u32)L,
		.script_code = (u32)script_code,
		.script_code_size = (u32)(sizeof(script_code)),
	};

	u64 args_ptr = (u64)(&args);

	//recomp_printf(">>> LuaLoader_InvokeScriptCode returned %ld\n", LuaLoader_InvokeScriptCode(L, &script_code, sizeof(script_code)));
	recomp_printf(">>> LuaLoader_InvokeScriptCode returned %d\n", LuaLoader_InvokeScriptCode(
		(args_ptr & 0x00000000FFFFFFFFull),
		(args_ptr & 0xFFFFFFFF00000000ull) >> 32
	));

	/* // Reduce health by one heart
	bool player_has_health_remaining = Health_ChangeBy(play, -16);

	recomp_printf(
		">>> bool player_has_health_remaining = %s;\n",
		(player_has_health_remaining ? "true" : "false")
	); */

	LuaLoader_Deinit(L);
}
