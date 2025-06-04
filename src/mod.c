#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "./shared/LuaLoader/lib.h"

#define PREEXEC(FUNCTION_NAME, FUNCTION_ARGS) \
void FUNCTION_NAME FUNCTION_ARGS; \
RECOMP_HOOK(#FUNCTION_NAME) void PREEXEC_RECOMP_IMPL__ ## FUNCTION_NAME ## __ FUNCTION_ARGS

#define WITH_SIZE(STRING_LITERAL) (STRING_LITERAL), ((size_t)(sizeof(STRING_LITERAL)))

#define LOG(...) { \
	recomp_printf("\e[1;32m>>>\e[22;39m "); \
	recomp_printf(__VA_ARGS__); \
	recomp_printf("\e[0m \e[1;35m<<<\e[22;39m \e[36m[\e[32m%s\e[39m() \e[35m::\e[39m \e[33m%s\e[35m:\e[34m%u\e[36m]\e[0m\n", (__func__), (__FILE__), (__LINE__)); \
}

#define LOG_TYPE_SIZE(TYPE) \
LOG("sizeof("#TYPE") = %llu", sizeof(TYPE));

#define PRINTF_S8  "(" "\e[3;33m" "s8"  "\e[23;39m" ")" "\e[36m" "%hd"          "\e[39m"
#define PRINTF_S16 "(" "\e[3;33m" "s16" "\e[23;39m" ")" "\e[35m" "%d"           "\e[39m"
#define PRINTF_S32 "(" "\e[3;33m" "s32" "\e[23;39m" ")" "\e[34m" "%ldL"         "\e[39m"
#define PRINTF_S64 "(" "\e[3;33m" "s64" "\e[23;39m" ")" "\e[33m" "%lldLL"       "\e[39m"
#define PRINTF_U8  "(" "\e[3;33m" "u8"  "\e[23;39m" ")" "\e[36m" "0x%02xU"      "\e[39m"
#define PRINTF_U16 "(" "\e[3;33m" "u16" "\e[23;39m" ")" "\e[35m" "0x%04xU"      "\e[39m"
#define PRINTF_U32 "(" "\e[3;33m" "u32" "\e[23;39m" ")" "\e[34m" "0x%08lxUL"    "\e[39m"
#define PRINTF_U64 "(" "\e[3;33m" "u64" "\e[23;39m" ")" "\e[33m" "0x%016llxULL" "\e[39m"
#define PRINTF_F32 "(" "\e[3;33m" "f32" "\e[23;39m" ")" "\e[34m" "%.7gf"        "\e[39m"
#define PRINTF_F64 "(" "\e[3;33m" "f64" "\e[23;39m" ")" "\e[33m" "%.14g"        "\e[39m"

#define PRINTF_PTR "\e[1;3;31m" "0x%08x" "\e[22;23;39m"

#define PRINTF_VEC3F "(" "\e[1;3;36m" "Vec3f" "\e[22;23;39m" ")" "{ "\
"\e[35m.\e[31mx \e[35m=\e[39m "PRINTF_F32", " \
"\e[35m.\e[31my \e[35m=\e[39m "PRINTF_F32", " \
"\e[35m.\e[31mz \e[35m=\e[39m "PRINTF_F32 \
" }"

#define PRINT_STRUCT_MEMBER(STRUCT_PTR, MEMBER_TYPE_ID, MEMBER_NAME, PADDING) \
recomp_printf("    \e[35m.\e[31m" #MEMBER_NAME "\e[39m" PADDING "\e[35m=\e[39m " PRINTF_ ## MEMBER_TYPE_ID "\e[0m,\n", (STRUCT_PTR)->MEMBER_NAME);

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

/* static void format_bits_u64(const u64 *restrict const x_ptr, char *restrict const out_buffer, const size_t buffer_size) {
	if (x_ptr == NULL) {
		LOG("Value of `x_ptr` is NULL!");
		return;
	}

	if (out_buffer == NULL) {
		LOG("Value of `out_buffer` is NULL!");
		return;
	}

	if (buffer_size < 72) {
		LOG("Value of `buffer_size` must be greater than or equal to 72! (got: %u)", buffer_size);
		return;
	}

	const u64 x = *x_ptr;

	int offset = 0;
	for (int i = 0; i < 64; i++) {
		out_buffer[i + offset] = (x & (1ULL << (63 - i))) ? '1' : '0';

		if ((i > 0) && ((i % 8) == 7)) {
			offset++;
			out_buffer[i + offset] = '\'';
		}
	}

	out_buffer[buffer_size - 1] = '\0';
} */

#define SPLIT_DOUBLEWORD(VALUE) \
((u32)(((u64)(VALUE)) & ((u64)(0xFFFFFFFF)))), ((u32)((((u64)(VALUE)) >> ((u64)(32))) & ((u64)(0xFFFFFFFF))))

/* RECOMP_HOOK("Player_Init") void my_player_init_hook(Actor *thisx, PlayState *play) {
	HOST_PTR(lua_State) L = LuaLoader_Init();

	char bits[72];
	format_bits_u64(&L, WITH_SIZE(bits));
	LOG("0x%016llx || 0b%s || %20llu", L, bits, L);

	LuaLoader_InvokeScriptCode_Args *invoke_script_args =
		(LuaLoader_InvokeScriptCode_Args *)recomp_alloc(sizeof(LuaLoader_InvokeScriptCode_Args));
	//invoke_script_args->L = L;
	invoke_script_args->L_low  = ((u32)(((u64)(L)) & ((u64)(0xFFFFFFFF))));
	invoke_script_args->L_high = ((u32)((((u64)(L)) >> ((u64)(32))) & ((u64)(0xFFFFFFFF))));
	invoke_script_args->script_code = "print('Hello, world!')";
	invoke_script_args->script_code_size = sizeof("print('Hello, world!')");
	LOG("invoke_script_args = "PRINTF_PTR, invoke_script_args);
	LuaLoader_InvokeScriptCode(invoke_script_args);

	//LuaLoader_Deinit(L);
	LuaLoader_Deinit(SPLIT_DOUBLEWORD(L));
} */

RECOMP_HOOK("Player_Init") void test_hook(Actor *thisx, PlayState *play) {
	static bool do_run = true;
	if (!do_run) return;
	do_run = false;

	const char script_code[] = "If you can read this, it works!";
	LOG("script_code["PRINTF_S32"] = "PRINTF_PTR, (s32)sizeof(script_code), script_code);

	HOST_PTR(lua_State) L = LuaLoader_Init();

	if (L.u64 == 0ULL) {
		LOG("Expected `L` to be a pointer to `lua_State`, but got NULL instead!");
		return;
	}

	char *script_file_path = recomp_get_config_string("LuaLoader::EntrypointScript");

	if (script_file_path == NULL) {
		LOG("Failed to get the file path of the entrypoint Lua script!");
		goto CleanupLuaVM;
	}

	if (script_file_path[0] == '\0') {
		LOG("The \"Entrypoint Script\" configuration option is currently empty.");
		LOG("Please set it to the filepath of Lua script file you wish to be executed!");
		goto CleanupScriptFilePath;
	}

	/* const char script_code[] = "print('\\027[7mHello from Lua!\\027[27m')";
	LuaLoader_InvokeScriptCodeArgs invoke_script_args = {
		L,
		script_code,
		sizeof(script_code) - 1, // Do not count the terminating NULL-byte.
	};
	LuaLoader_InvokeScriptCode(&invoke_script_args); */
	LuaLoader_InvokeScriptFile(L, script_file_path);

CleanupScriptFilePath:
	recomp_free_config_string(script_file_path);

CleanupLuaVM:
	LuaLoader_Deinit(L);
}

void print_actor_info(const Actor *restrict const thisx) {
	// Using seperate calls to `recomp_printf()` for each line of displayed text
	// prevents issues cause by reaching stack size limits.

	recomp_printf("*("PRINTF_PTR") == (Actor){\n", thisx);
	//recomp_printf("    " "\e[35m" "." "\e[31m" "id" "\e[39m" "                     " "\e[35m" "=" "\e[39m" " " PRINTF_S16        ",\n", thisx->id);
	PRINT_STRUCT_MEMBER(thisx, S16, id, "                     ");
	recomp_printf("    " "\e[35m" "." "\e[31m" "category" "\e[39m" "               " "\e[35m" "=" "\e[39m" " " PRINTF_U8         ",\n", thisx->category);
	recomp_printf("    " "\e[35m" "." "\e[31m" "room" "\e[39m" "                   " "\e[35m" "=" "\e[39m" " " PRINTF_S8         ",\n", thisx->room);
	recomp_printf("    " "\e[35m" "." "\e[31m" "flags" "\e[39m" "                  " "\e[35m" "=" "\e[39m" " " PRINTF_U32        ",\n", thisx->flags);
	recomp_printf("    " "\e[35m" "." "\e[31m" "home" "\e[39m" "                   " "\e[35m" "=" "\e[39m" " " "(PosRot){ ... }" ",\n"/* , thisx->home */);
	recomp_printf("    " "\e[35m" "." "\e[31m" "params" "\e[39m" "                 " "\e[35m" "=" "\e[39m" " " PRINTF_S16        ",\n", thisx->params);
	recomp_printf("    " "\e[35m" "." "\e[31m" "objectSlot" "\e[39m" "             " "\e[35m" "=" "\e[39m" " " PRINTF_S8         ",\n", thisx->objectSlot);
	recomp_printf("    " "\e[35m" "." "\e[31m" "attentionRangeType" "\e[39m" "     " "\e[35m" "=" "\e[39m" " " PRINTF_S8         ",\n", thisx->attentionRangeType);
	recomp_printf("    " "\e[35m" "." "\e[31m" "halfDaysBits" "\e[39m" "           " "\e[35m" "=" "\e[39m" " " PRINTF_S16        ",\n", thisx->halfDaysBits);
	recomp_printf("    " "\e[35m" "." "\e[31m" "world" "\e[39m" "                  " "\e[35m" "=" "\e[39m" " " "(PosRot){ ... }" ",\n"/* , thisx->world */);
	recomp_printf("    " "\e[35m" "." "\e[31m" "csId" "\e[39m" "                   " "\e[35m" "=" "\e[39m" " " PRINTF_S8         ",\n", thisx->csId);
	recomp_printf("    " "\e[35m" "." "\e[31m" "audioFlags" "\e[39m" "             " "\e[35m" "=" "\e[39m" " " PRINTF_U8         ",\n", thisx->audioFlags);
	recomp_printf("    " "\e[35m" "." "\e[31m" "focus" "\e[39m" "                  " "\e[35m" "=" "\e[39m" " " "(PosRot){ ... }" ",\n"/* , thisx->focus */);
	recomp_printf("    " "\e[35m" "." "\e[31m" "sfxId" "\e[39m" "                  " "\e[35m" "=" "\e[39m" " " PRINTF_U16        ",\n", thisx->sfxId);
	recomp_printf("    " "\e[35m" "." "\e[31m" "lockOnArrowOffset" "\e[39m" "      " "\e[35m" "=" "\e[39m" " " PRINTF_F32        ",\n", thisx->lockOnArrowOffset);
	recomp_printf("    " "\e[35m" "." "\e[31m" "scale" "\e[39m" "                  " "\e[35m" "=" "\e[39m" " " PRINTF_VEC3F      ",\n", thisx->scale);
	recomp_printf("    " "\e[35m" "." "\e[31m" "velocity" "\e[39m" "               " "\e[35m" "=" "\e[39m" " " PRINTF_VEC3F      ",\n", thisx->velocity);
	recomp_printf("    " "\e[35m" "." "\e[31m" "speed" "\e[39m" "                  " "\e[35m" "=" "\e[39m" " " PRINTF_F32        ",\n", thisx->speed);
	recomp_printf("    " "\e[35m" "." "\e[31m" "gravity" "\e[39m" "                " "\e[35m" "=" "\e[39m" " " PRINTF_F32        ",\n", thisx->gravity);
	recomp_printf("    " "\e[35m" "." "\e[31m" "terminalVelocity" "\e[39m" "       " "\e[35m" "=" "\e[39m" " " PRINTF_F32        ",\n", thisx->terminalVelocity);
	recomp_printf("    " "\e[35m" "." "\e[31m" "wallPoly" "\e[39m" "               " "\e[35m" "=" "\e[39m" " " "(struct CollisionPoly*)" PRINTF_PTR ",\n", thisx->wallPoly);
	recomp_printf("    " "\e[35m" "." "\e[31m" "floorPoly" "\e[39m" "              " "\e[35m" "=" "\e[39m" " " "(struct CollisionPoly*)" PRINTF_PTR ",\n", thisx->floorPoly);
	recomp_printf("    " "\e[35m" "." "\e[31m" "wallBgId" "\e[39m" "               " "\e[35m" "=" "\e[39m" " " PRINTF_U8  ",\n", thisx->wallBgId);
	recomp_printf("    " "\e[35m" "." "\e[31m" "floorBgId" "\e[39m" "              " "\e[35m" "=" "\e[39m" " " PRINTF_U8  ",\n", thisx->floorBgId);
	recomp_printf("    " "\e[35m" "." "\e[31m" "wallYaw" "\e[39m" "                " "\e[35m" "=" "\e[39m" " " PRINTF_S16 ",\n", thisx->wallYaw);
	recomp_printf("    " "\e[35m" "." "\e[31m" "floorHeight" "\e[39m" "            " "\e[35m" "=" "\e[39m" " " PRINTF_F32 ",\n", thisx->floorHeight);
	recomp_printf("    " "\e[35m" "." "\e[31m" "depthInWater" "\e[39m" "           " "\e[35m" "=" "\e[39m" " " PRINTF_F32 ",\n", thisx->depthInWater);
	recomp_printf("    " "\e[35m" "." "\e[31m" "bgCheckFlags" "\e[39m" "           " "\e[35m" "=" "\e[39m" " " PRINTF_U16 ",\n", thisx->bgCheckFlags);
	recomp_printf("    " "\e[35m" "." "\e[31m" "yawTowardsPlayer" "\e[39m" "       " "\e[35m" "=" "\e[39m" " " PRINTF_S16 ",\n", thisx->yawTowardsPlayer);
	recomp_printf("    " "\e[35m" "." "\e[31m" "xyzDistToPlayerSq" "\e[39m" "      " "\e[35m" "=" "\e[39m" " " PRINTF_F32 ",\n", thisx->xyzDistToPlayerSq);
	recomp_printf("    " "\e[35m" "." "\e[31m" "xzDistToPlayer" "\e[39m" "         " "\e[35m" "=" "\e[39m" " " PRINTF_F32 ",\n", thisx->xzDistToPlayer);
	recomp_printf("    " "\e[35m" "." "\e[31m" "playerHeightRel" "\e[39m" "        " "\e[35m" "=" "\e[39m" " " PRINTF_F32 ",\n", thisx->playerHeightRel);
	recomp_printf("    " "\e[35m" "." "\e[31m" "colChkInfo" "\e[39m" "             " "\e[35m" "=" "\e[39m" " " "(CollisionCheckInfo){ ... }" ",\n"/* , thisx->colChkInfo */);
	recomp_printf("    " "\e[35m" "." "\e[31m" "shape" "\e[39m" "                  " "\e[35m" "=" "\e[39m" " " "(ActorShape){ ... }" ",\n"/* , thisx->shape */);
	recomp_printf("    " "\e[35m" "." "\e[31m" "projectedPos" "\e[39m" "           " "\e[35m" "=" "\e[39m" " " PRINTF_VEC3F ",\n", thisx->projectedPos);
	recomp_printf("    " "\e[35m" "." "\e[31m" "projectedW" "\e[39m" "             " "\e[35m" "=" "\e[39m" " " PRINTF_F32   ",\n", thisx->projectedW);
	recomp_printf("    " "\e[35m" "." "\e[31m" "cullingVolumeDistance" "\e[39m" "  " "\e[35m" "=" "\e[39m" " " PRINTF_F32   ",\n", thisx->cullingVolumeDistance);
	recomp_printf("    " "\e[35m" "." "\e[31m" "cullingVolumeScale" "\e[39m" "     " "\e[35m" "=" "\e[39m" " " PRINTF_F32   ",\n", thisx->cullingVolumeScale);
	recomp_printf("    " "\e[35m" "." "\e[31m" "cullingVolumeDownward" "\e[39m" "  " "\e[35m" "=" "\e[39m" " " PRINTF_F32   ",\n", thisx->cullingVolumeDownward);
	recomp_printf("    " "\e[35m" "." "\e[31m" "prevPos" "\e[39m" "                " "\e[35m" "=" "\e[39m" " " PRINTF_VEC3F ",\n", thisx->prevPos);
	recomp_printf("    " "\e[35m" "." "\e[31m" "isLockedOn" "\e[39m" "             " "\e[35m" "=" "\e[39m" " " PRINTF_U8    ",\n", thisx->isLockedOn);
	recomp_printf("    " "\e[35m" "." "\e[31m" "targetPriority" "\e[39m" "         " "\e[35m" "=" "\e[39m" " " PRINTF_U8    ",\n", thisx->targetPriority);
	recomp_printf("    " "\e[35m" "." "\e[31m" "textId" "\e[39m" "                 " "\e[35m" "=" "\e[39m" " " PRINTF_U16   ",\n", thisx->textId);
	recomp_printf("    " "\e[35m" "." "\e[31m" "freezeTimer" "\e[39m" "            " "\e[35m" "=" "\e[39m" " " PRINTF_U16   ",\n", thisx->freezeTimer);
	recomp_printf("    " "\e[35m" "." "\e[31m" "colorFilterParams" "\e[39m" "      " "\e[35m" "=" "\e[39m" " " PRINTF_U16   ",\n", thisx->colorFilterParams);
	recomp_printf("    " "\e[35m" "." "\e[31m" "colorFilterTimer" "\e[39m" "       " "\e[35m" "=" "\e[39m" " " PRINTF_U8    ",\n", thisx->colorFilterTimer);
	recomp_printf("    " "\e[35m" "." "\e[31m" "isDrawn" "\e[39m" "                " "\e[35m" "=" "\e[39m" " " PRINTF_U8    ",\n", thisx->isDrawn);
	recomp_printf("    " "\e[35m" "." "\e[31m" "dropFlag" "\e[39m" "               " "\e[35m" "=" "\e[39m" " " PRINTF_U8    ",\n", thisx->dropFlag);
	recomp_printf("    " "\e[35m" "." "\e[31m" "hintId" "\e[39m" "                 " "\e[35m" "=" "\e[39m" " " PRINTF_U8    ",\n", thisx->hintId);
	recomp_printf("    " "\e[35m" "." "\e[31m" "parent" "\e[39m" "                 " "\e[35m" "=" "\e[39m" " " "(struct Actor*)" PRINTF_PTR ",\n", thisx->parent);
	recomp_printf("    " "\e[35m" "." "\e[31m" "child" "\e[39m" "                  " "\e[35m" "=" "\e[39m" " " "(struct Actor*)" PRINTF_PTR ",\n", thisx->child);
	recomp_printf("    " "\e[35m" "." "\e[31m" "prev" "\e[39m" "                   " "\e[35m" "=" "\e[39m" " " "(struct Actor*)" PRINTF_PTR ",\n", thisx->prev);
	recomp_printf("    " "\e[35m" "." "\e[31m" "next" "\e[39m" "                   " "\e[35m" "=" "\e[39m" " " "(struct Actor*)" PRINTF_PTR ",\n", thisx->next);
	recomp_printf("    " "\e[35m" "." "\e[31m" "init" "\e[39m" "                   " "\e[35m" "=" "\e[39m" " " "(ActorFunc){ ... }" ",\n"/* , thisx->init */);
	recomp_printf("    " "\e[35m" "." "\e[31m" "destroy" "\e[39m" "                " "\e[35m" "=" "\e[39m" " " "(ActorFunc){ ... }" ",\n"/* , thisx->destroy */);
	recomp_printf("    " "\e[35m" "." "\e[31m" "update" "\e[39m" "                 " "\e[35m" "=" "\e[39m" " " "(ActorFunc){ ... }" ",\n"/* , thisx->update */);
	recomp_printf("    " "\e[35m" "." "\e[31m" "draw" "\e[39m" "                   " "\e[35m" "=" "\e[39m" " " "(ActorFunc){ ... }" ",\n"/* , thisx->draw */);
	recomp_printf("    " "\e[35m" "." "\e[31m" "overlayEntry" "\e[39m" "           " "\e[35m" "=" "\e[39m" " " "(struct ActorOverlay*)" PRINTF_PTR ",\n", thisx->overlayEntry);
	recomp_printf("};\n");
}

void print_hook_func_args(const Actor *restrict const thisx, const PlayState *restrict const play, const char *restrict const funcname) {
	static const char format[] = (
		"\e[1;35m" ">>>" "\e[22;39m"
		" "
		"\e[32m" "%s" "\e[39m"
		"("
		"\e[3;31m" "thisx" "\e[23;35m" "=" "\e[39m" PRINTF_PTR
		", "
		"\e[3;31m" "play"  "\e[23;35m" "=" "\e[39m" PRINTF_PTR
		")"
		"\e[0m\n"
	);

	recomp_printf("\n\n------------------------------------------------------------------------------------------------------------------------\n\n");
	recomp_printf(format, funcname, thisx, play);
	print_actor_info(thisx);
}

/* PREEXEC(Player_Init, (Actor *thisx, PlayState *play)) {
	static int counter = 0;
	counter++;
	if (counter <= 1) return;

	print_hook_func_args(thisx, play, "Player_Init");

	//Actor_Kill(thisx);
} */

/* #define HOOK_FUNCTION_CALL(FUNCTION_NAME) \
RECOMP_HOOK(#FUNCTION_NAME) void HOOK_FUNCTION_CALL_IMPL__ ## FUNCTION_NAME ## __(u32 arg1, u32 arg2, u32 arg3) { \
	recomp_printf(#FUNCTION_NAME"(0x%08x, 0x%08x, 0x%08x);\n", arg1, arg2, arg3); \
}

//HOOK_FUNCTION_CALL(Sram_OpenSave);

PREEXEC(FileSelect_LoadGame, (GameState *thisx)) {
	// 0 for File 1 and 1 for File 2
	s16 selectedFileIndex = *((s16*)(((u8*)thisx) + 0x2448EUL));
	recomp_printf("FileSelect_LoadGame(thisx=0x%08x); --> selectedFileIndex=0x%08x\n", thisx, selectedFileIndex);
}

PREEXEC(Sram_OpenSave, (struct FileSelectState *fileSelect, SramContext *sramCtx)) {
	recomp_printf("Sram_OpenSave(fileSelect=0x%08x, sramCtx=0x%08x);\n", fileSelect, sramCtx);
} */

/* PREEXEC(EnBombers_Update, (Actor *thisx, PlayState *play)) {
	print_hook_func_args(thisx, play, "EnBombers_Update");

	//Actor_Kill(thisx);
} */
