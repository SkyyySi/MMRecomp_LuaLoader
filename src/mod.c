#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "./shared/LuaLoader/lib.h"

#define WITH_SIZE(STRING_LITERAL) (STRING_LITERAL), (sizeof(STRING_LITERAL))

//int counter = 0;

RECOMP_HOOK("Player_Init") void my_player_init_hook(Actor *thisx, PlayState *play) {
	//counter++;
	//if (counter < 3) return;

	void *L = LuaLoader_Init();

	char script_code[] = "print(\"Hello, world!\")\n";
	recomp_printf(">>> %s -> &script_code = %p\n", __func__, &script_code);

	recomp_printf(">>> LuaLoader_InvokeScriptCode returned %ld\n", LuaLoader_InvokeScriptCode(L, &script_code, sizeof(script_code)));

	/* // Reduce health by one heart
	bool player_has_health_remaining = Health_ChangeBy(play, -16);

	recomp_printf(
		">>> bool player_has_health_remaining = %s;\n",
		(player_has_health_remaining ? "true" : "false")
	); */

	LuaLoader_Deinit(L);
}
