#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>

#include "disc.h"
#include "gui.h"
#include "menu.h"
#include "restart.h"
#include "subsystem.h"
#include "sys.h"
#include "video.h"


int main(int argc, char **argv)
{
	s32 ret;

	/* Load Custom IOS */
	ret = IOS_ReloadIOS(249);

	/* Initialize system */
	Sys_Init();

	/* Initialize subsystems */
	Subsystem_Init();

	/* Set video mode */
	Video_SetMode();

	/* Initialize console */
	Gui_InitConsole();

	/* Show background */
	Gui_DrawBackground();

	/* Check if Custom IOS is loaded */
	if (ret < 0) {
		printf("[+] ERROR:\n");
		printf("    Custom IOS could not be loaded! (ret = %d)\n", ret);

		goto out;
	}

	/* Initialize DIP module */
	ret = Disc_Init();
	if (ret < 0) {
		printf("[+] ERROR:\n");
		printf("    Could not initialize DIP module! (ret = %d)\n", ret);

		goto out;
	}

	/* Menu loop */
	Menu_Loop();

out:
	/* Restart */
	Restart_Wait();

	return 0;
}
