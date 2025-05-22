#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

//#include "./shared/LuaLoader/lib.h"

#define PREEXEC(FUNCTION_NAME) RECOMP_HOOK(#FUNCTION_NAME)

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

RECOMP_IMPORT(".", u32 LuaLoader_Init(void));

/* static void format_bits_u64(u64 x, char *out_buffer, unsigned long buffer_size) {
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
} */

/* RECOMP_HOOK("Player_Init") void my_player_init_hook(Actor *thisx, PlayState *play) {
	char bits[72];

	u32 L = LuaLoader_Init();
	format_bits_u64(L, WITH_SIZE(bits));
	LOG("0x%016lx || 0b%s || %20lu", L, bits, L);
} */

//#define PRINTF_ 

#define PRINTF_S8  "(" "\e[33m" "s8"  "\e[39m" ")" "\e[3;36m" "%hd"        "\e[23;39m"
#define PRINTF_S16 "(" "\e[33m" "s16" "\e[39m" ")" "\e[3;35m" "%d"         "\e[23;39m"
#define PRINTF_S32 "(" "\e[33m" "s32" "\e[39m" ")" "\e[3;34m" "%ldL"       "\e[23;39m"
#define PRINTF_S64 "(" "\e[33m" "s64" "\e[39m" ")" "\e[3;33m" "%lldLL"     "\e[23;39m"
#define PRINTF_U8  "(" "\e[33m" "u8"  "\e[39m" ")" "\e[3;36m" "0x%02xU"    "\e[23;39m"
#define PRINTF_U16 "(" "\e[33m" "u16" "\e[39m" ")" "\e[3;35m" "0x%04xU"    "\e[23;39m"
#define PRINTF_U32 "(" "\e[33m" "u32" "\e[39m" ")" "\e[3;34m" "0x%08xULL"  "\e[23;39m"
#define PRINTF_U64 "(" "\e[33m" "u64" "\e[39m" ")" "\e[3;33m" "0x%016xULL" "\e[23;39m"
#define PRINTF_F32 "(" "\e[33m" "f32" "\e[39m" ")" "\e[3;34m" "%.7gf"      "\e[23;39m"
#define PRINTF_F64 "(" "\e[33m" "f64" "\e[39m" ")" "\e[3;33m" "%.14g"      "\e[23;39m"

#define PRINTF_PTR "\e[1;3;31m" "0x%08xLU" "\e[22;23;39m"

#define PRINTF_VEC3F "(Vec3f){ .x = "PRINTF_F32", .y = "PRINTF_F32", .z = "PRINTF_F32" }"

void print_actor_info(const Actor *restrict const thisx) {
	// Using seperate calls to `recomp_printf()` for each line of displayed text
	// prevents issues cause by reaching stack size limits.

	recomp_printf("*("PRINTF_PTR") == (Actor){\n", thisx);
	recomp_printf("    " "\e[35m" "." "\e[31m" "id" "\e[39m" "                     " "\e[35m" "=" "\e[39m" " " PRINTF_S16        ",\n", thisx->id);
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

/* PREEXEC(Player_Init) void preexec_Player_Init(Actor *thisx, PlayState *play) {
	static int counter = 0;
	counter++;
	if (counter <= 2) return;

	print_hook_func_args(thisx, play, "Player_Init");

	Actor_Kill(thisx);
} */

#define HOOK_FUNCTION_CALL(FUNCTION_NAME) \
PREEXEC(FUNCTION_NAME) void HOOK_FUNCTION_CALL_IMPL__##FUNCTION_NAME(u32 arg1, u32 arg2, u32 arg3) { \
	recomp_printf(#FUNCTION_NAME"(0x%08x, 0x%08x, 0x%08x);\n", arg1, arg2, arg3); \
}

//HOOK_FUNCTION_CALL(Sram_OpenSave);

PREEXEC(FileSelect_LoadGame) void PREEXEC_FileSelect_LoadGame(GameState *thisx) {
	// 0 for File 1 and 1 for File 2
	s16 selectedFileIndex = *((s16*)(((u8*)thisx) + 0x2448EUL));
	recomp_printf("FileSelect_LoadGame(thisx=0x%08x); --> selectedFileIndex=0x%08x\n", thisx, selectedFileIndex);
}
PREEXEC(Sram_OpenSave) void PREEXEC_Sram_OpenSave(struct FileSelectState *fileSelect, SramContext *sramCtx) {
	recomp_printf("Sram_OpenSave(fileSelect=0x%08x, sramCtx=0x%08x);\n", fileSelect, sramCtx);
}

/* PREEXEC(EnBombers_Update) void preexec_EnBombers_Update(Actor *thisx, PlayState *play) {
	print_hook_func_args(thisx, play, "EnBombers_Update");

	//Actor_Kill(thisx);
} */
