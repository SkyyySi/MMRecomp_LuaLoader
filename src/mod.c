#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "./shared/LuaLoader/lib.h"

RECOMP_HOOK("Player_Init") void my_player_init_hook(Actor *thisx, PlayState *play) {
	LuaLoader_MyFunction();
}
