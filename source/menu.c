#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>

#include "disc.h"
#include "fat.h"
#include "gui.h"
#include "menu.h"
#include "partition.h"
#include "restart.h"
#include "sys.h"
#include "utils.h"
#include "video.h"
#include "wbfs.h"
#include "wpad.h"

/* Constants */
#define ENTRIES_PER_PAGE	12
#define MAX_CHARACTERS		30

/* Gamelist buffer */
static struct discHdr *gameList = NULL;

/* Gamelist variables */
static s32 gameCnt = 0, gameSelected = 0, gameStart = 0;

/* WBFS device */
static s32 wbfsDev = WBFS_MIN_DEVICE;


s32 __Menu_EntryCmp(const void *a, const void *b)
{
	struct discHdr *hdr1 = (struct discHdr *)a;
	struct discHdr *hdr2 = (struct discHdr *)b;

	/* Compare strings */
	return strcmp(hdr1->title, hdr2->title);
}

s32 __Menu_GetEntries(void)
{
	struct discHdr *buffer = NULL;

	u32 cnt, len;
	s32 ret;

	/* Get list length */
	ret = WBFS_GetCount(&cnt);
	if (ret < 0)
		return ret;

	/* Buffer length */
	len = sizeof(struct discHdr) * cnt;

	/* Allocate memory */
	buffer = (struct discHdr *)memalign(32, len);
	if (!buffer)
		return -1;

	/* Clear buffer */
	memset(buffer, 0, len);

	/* Get header list */
	ret = WBFS_GetHeaders(buffer, cnt, sizeof(struct discHdr));
	if (ret < 0)
		goto err;

	/* Sort entries */
	qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmp);

	/* Free memory */
	if (gameList)
		free(gameList);

	/* Set values */
	gameList = buffer;
	gameCnt  = cnt;

	/* Reset variables */
	gameSelected = gameStart = 0;

	return 0;

err:
	/* Free memory */
	if (buffer)
		free(buffer);

	return ret;
}

char *__Menu_PrintTitle(char *name)
{
	static char buffer[MAX_CHARACTERS + 4];

	/* Clear buffer */
	memset(buffer, 0, sizeof(buffer));

	/* Check string length */
	if (strlen(name) > (MAX_CHARACTERS + 3)) {
		strncpy(buffer, name,  MAX_CHARACTERS);
		strncat(buffer, "...", 3);

		return buffer;
	}

	return name;
}

void __Menu_PrintInfo(struct discHdr *header)
{
	f32 size = 0.0;

	/* Get game size */
	WBFS_GameSize(header->id, &size);

	/* Print game info */
	printf("    %s\n",                    header->title);
	printf("    (%c%c%c%c) (%.2fGB)\n\n", header->id[0], header->id[1], header->id[2], header->id[3], size);
}

void __Menu_MoveList(s8 delta)
{
	s32 index;

	/* No game list */
	if (!gameCnt)
		return;

	/* Select next entry */
	gameSelected += delta;

	/* Out of the list? */
	if (gameSelected <= -1)
		gameSelected = (gameCnt - 1);
	if (gameSelected >= gameCnt)
		gameSelected = 0;

	/* List scrolling */
	index = (gameSelected - gameStart);

	if (index >= ENTRIES_PER_PAGE)
		gameStart += index - (ENTRIES_PER_PAGE - 1);
	if (index <= -1)
		gameStart += index;
}

void __Menu_ShowList(void)
{
	f32 free, used;

	/* Get free space */
	WBFS_DiskSpace(&used, &free);

	printf("[+] Select the game you want to boot:\n\n");

	/* No game list*/
	if (gameCnt) {
		u32 cnt;

		/* Print game list */
		for (cnt = gameStart; cnt < gameCnt; cnt++) {
			struct discHdr *header = &gameList[cnt];

			/* Entries per page limit reached */
			if ((cnt - gameStart) >= ENTRIES_PER_PAGE)
				break;

			/* Print entry */
			printf("\t%2s %s\n", (gameSelected == cnt) ? ">>" : "  ", __Menu_PrintTitle(header->title));
		}
	} else
		printf("\t>> No games found!!\n");

	printf("\n\n");

	/* Print free/used space */
	printf("[+] Free space: %.2fGB\n", free);
	printf("    Used space: %.2fGB\n", used);
}

void __Menu_ShowCover(void)
{
	struct discHdr *header = &gameList[gameSelected];

	/* No game list*/
	if (!gameCnt)
		return;

	/* Draw cover */
	Gui_DrawCover(header->id);
}

void __Menu_Controls(void)
{
	u32 buttons = Wpad_WaitButtons();

	/* UP/DOWN buttons */
	if (buttons & WPAD_BUTTON_UP)
		__Menu_MoveList(-1);

	if (buttons & WPAD_BUTTON_DOWN)
		__Menu_MoveList(1);

	/* LEFT/RIGHT buttons */
	if (buttons & WPAD_BUTTON_LEFT)
		__Menu_MoveList(-ENTRIES_PER_PAGE);

	if (buttons & WPAD_BUTTON_RIGHT)
		__Menu_MoveList(ENTRIES_PER_PAGE);


	/* HOME button */
	if (buttons & WPAD_BUTTON_HOME)
		Restart();

	/* PLUS (+) button */
	if (buttons & WPAD_BUTTON_PLUS)
		Menu_Install();

	/* MINUS (-) button */
	if (buttons & WPAD_BUTTON_MINUS)
		Menu_Remove();

	/* ONE (1) button */
	if (buttons & WPAD_BUTTON_1)
		Menu_Device();

	/* A button */
	if (buttons & WPAD_BUTTON_A)
		Menu_Boot();
}


void Menu_Format(void)
{
	partitionEntry partitions[MAX_PARTITIONS];

	u32 cnt, sector_size;
	s32 ret, selected = 0;

	/* Clear console */
	Con_Clear();

	/* Get partition entries */
	ret = Partition_GetEntries(wbfsDev, partitions, &sector_size);
	if (ret < 0) {
		printf("[+] ERROR: No partitions found! (ret = %d)\n", ret);

		/* Restart */
		Restart_Wait();
	}

loop:
	/* Clear console */
	Con_Clear();

	printf("[+] Selected the partition you want to \n");
	printf("    format:\n\n");

	/* Print partition list */
	for (cnt = 0; cnt < MAX_PARTITIONS; cnt++) {
		partitionEntry *entry = &partitions[cnt];

		/* Calculate size in gigabytes */
		f32 size = entry->size * (sector_size / GB_SIZE);

		/* Selected entry */
		(selected == cnt) ? printf(">> ") : printf("   ");
		fflush(stdout);

		/* Valid partition */
		if (size)
			printf("Partition #%d: (size = %.2fGB)\n",       cnt + 1, size);
		else 
			printf("Partition #%d: (cannot be formatted)\n", cnt + 1);
	}

	partitionEntry *entry = &partitions[selected];
	u32           buttons = Wpad_WaitButtons();

	/* UP/DOWN buttons */
	if (buttons & WPAD_BUTTON_UP) {
		if ((--selected) <= -1)
			selected = MAX_PARTITIONS - 1;
	}

	if (buttons & WPAD_BUTTON_DOWN) {
		if ((++selected) >= MAX_PARTITIONS)
			selected = 0;
	}

	/* B button */
	if (buttons & WPAD_BUTTON_B)
		return;

	/* Valid partition */
	if (entry->size) {
		/* A button */
		if (buttons & WPAD_BUTTON_A)
			goto format;
	}

	goto loop;

format:
	/* Clear console */
	Con_Clear();

	printf("[+] Are you sure you want to format\n");
	printf("    this partition?\n\n");

	printf("    Partition #%d\n",                  selected + 1);
	printf("    (size = %.2fGB - type: %02X)\n\n", entry->size * (sector_size / GB_SIZE), entry->type);

	printf("    Press A button to continue.\n");
	printf("    Press B button to go back.\n\n\n");

	/* Wait for user answer */
	for (;;) {
		u32 buttons = Wpad_WaitButtons();

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;

		/* B button */
		if (buttons & WPAD_BUTTON_B)
			goto loop;
	}

	printf("[+] Formatting, please wait...");
	fflush(stdout);

	/* Format partition */
	ret = WBFS_Format(entry->sector, entry->size);
	if (ret < 0) {
		printf("\n    ERROR! (ret = %d)\n", ret);
		goto out;
	} else
		printf(" OK!\n");

out:
	printf("\n");
	printf("    Press any button to continue...\n");

	/* Wait for any button */
	Wpad_WaitButtons();
}

void Menu_Device(void)
{
	u32 timeout = 30;
	s32 ret;

	/* Ask user for device */
	for (;;) {
		char *devname = "Unknown!";

		/* Set device name */
		switch (wbfsDev) {
		case WBFS_DEVICE_USB:
			devname = "USB Mass Storage Device";
			break;

		case WBFS_DEVICE_SDHC:
			devname = "SD/SDHC Card";
			break;
		}

		/* Clear console */
		Con_Clear();

		printf("[+] Select WBFS device:\n");
		printf("    < %s >\n\n", devname);

		printf("    Press LEFT/RIGHT to select device.\n");
		printf("    Press A button to continue.\n\n");

		u32 buttons = Wpad_WaitButtons();

		/* LEFT/RIGHT buttons */
		if (buttons & WPAD_BUTTON_LEFT) {
			if ((--wbfsDev) < WBFS_MIN_DEVICE)
				wbfsDev = WBFS_MAX_DEVICE;
		}
		if (buttons & WPAD_BUTTON_RIGHT) {
			if ((++wbfsDev) > WBFS_MAX_DEVICE)
				wbfsDev = WBFS_MIN_DEVICE;
		}

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;
	}

	printf("[+] Mounting device, please wait...\n");
	printf("    (%d seconds timeout)\n\n", timeout);
	fflush(stdout);

	/* Initialize WBFS */
	ret = WBFS_Init(wbfsDev, timeout);
	if (ret < 0) {
		printf("    ERROR! (ret = %d)\n", ret);

		/* Restart wait */
		Restart_Wait();
	}

	/* Try to open device */
	while (WBFS_Open() < 0) {
		/* Clear console */
		Con_Clear();

		printf("[+] WARNING:\n\n");

		printf("    No WBFS partition found!\n");
		printf("    You need to format a partition.\n\n");

		printf("    Press A button to format a partition.\n");
		printf("    Press B button to restart.\n\n");

		/* Wait for user answer */
		for (;;) {
			u32 buttons = Wpad_WaitButtons();

			/* A button */
			if (buttons & WPAD_BUTTON_A)
				break;

			/* B button */
			if (buttons & WPAD_BUTTON_B)
				Restart();
		}

		/* Format device */
		Menu_Format();
	}

	/* Get game list */
	__Menu_GetEntries();
}

void Menu_Install(void)
{
	static struct discHdr header ATTRIBUTE_ALIGN(32);

	s32 ret;

	/* Clear console */
	Con_Clear();

	printf("[+] Are you sure you want to install a\n");
	printf("    new Wii game?\n\n");

	printf("    Press A button to continue.\n");
	printf("    Press B button to go back.\n\n\n");

	/* Wait for user answer */
	for (;;) {
		u32 buttons = Wpad_WaitButtons();

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;

		/* B button */
		if (buttons & WPAD_BUTTON_B)
			return;
	}

	/* Disable WBFS mode */
	Disc_SetWBFS(0, NULL);

	printf("[+] Insert the game DVD disc...");
	fflush(stdout);

	/* Wait for disc */
	ret = Disc_Wait();
	if (ret < 0) {
		printf("\n    ERROR! (ret = %d)\n", ret);
		goto out;
	} else
		printf(" OK!\n");

	printf("[+] Opening DVD disc...");
	fflush(stdout);

	/* Open disc */
	ret = Disc_Open();
	if (ret < 0) {
		printf("\n    ERROR! (ret = %d)\n", ret);
		goto out;
	} else
		printf(" OK!\n\n");

	/* Check disc */
	ret = Disc_IsWii();
	if (ret < 0) {
		printf("[+] ERROR: Not a Wii disc!!\n");
		goto out;
	}

	/* Read header */
	Disc_ReadHeader(&header);

	/* Check if game is already installed */
	ret = WBFS_CheckGame(header.id);
	if (ret) {
		printf("[+] ERROR: Game already installed!!\n");
		goto out;
	}

	printf("[+] Installing game, please wait...\n\n");

	printf("    %s\n",           header.title);
	printf("    (%c%c%c%c)\n\n", header.id[0], header.id[1], header.id[2], header.id[3]);

	/* Install game */
	ret = WBFS_AddGame();
	if (ret < 0) {
		printf("[+] Installation ERROR! (ret = %d)\n", ret);
		goto out;
	}

	/* Reload entries */
	__Menu_GetEntries();

out:
	printf("\n");
	printf("    Press any button to continue...\n");

	/* Wait for any button */
	Wpad_WaitButtons();
}

void Menu_Remove(void)
{
	struct discHdr *header = NULL;

	s32 ret;

	/* No game list */
	if (!gameCnt)
		return;

	/* Selected game */
	header = &gameList[gameSelected];

	/* Clear console */
	Con_Clear();

	printf("[+] Are you sure you want to remove this\n");
	printf("    game?\n\n");

	/* Show game info */
	__Menu_PrintInfo(header);

	printf("    Press A button to continue.\n");
	printf("    Press B button to go back.\n\n\n");

	/* Wait for user answer */
	for (;;) {
		u32 buttons = Wpad_WaitButtons();

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;

		/* B button */
		if (buttons & WPAD_BUTTON_B)
			return;
	}

	printf("[+] Removing game, please wait...");
	fflush(stdout);

	/* Remove game */
	ret = WBFS_RemoveGame(header->id);
	if (ret < 0) {
		printf("\n    ERROR! (ret = %d)\n", ret);
		goto out;
	} else
		printf(" OK!\n");

	/* Reload entries */
	__Menu_GetEntries();

out:
	printf("\n");
	printf("    Press any button to continue...\n");

	/* Wait for any button */
	Wpad_WaitButtons();
}

void Menu_Boot(void)
{
	struct discHdr *header = NULL;

	s32 ret;

	/* No game list */
	if (!gameCnt)
		return;

	/* Selected game */
	header = &gameList[gameSelected];

	/* Clear console */
	Con_Clear();

	printf("[+] Are you sure you want to boot this\n");
	printf("    game?\n\n");

	/* Show game info */
	__Menu_PrintInfo(header);

	printf("    Press A button to continue.\n");
	printf("    Press B button to go back.\n\n");

	/* Wait for user answer */
	for (;;) {
		u32 buttons = Wpad_WaitButtons();

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;

		/* B button */
		if (buttons & WPAD_BUTTON_B)
			return;
	}

	printf("\n");
	printf("[+] Booting Wii game, please wait...\n");

	/* Set WBFS mode */
	Disc_SetWBFS(wbfsDev, header->id);

	/* Open disc */
	ret = Disc_Open();
	if (ret < 0) {
		printf("    ERROR: Could not open game! (ret = %d)\n", ret);
		goto out;
	}

	/* Boot Wii disc */
	Disc_WiiBoot();

	printf("    Returned! (ret = %d)\n", ret);

out:
	printf("\n");
	printf("    Press any button to continue...\n");

	/* Wait for button */
	Wpad_WaitButtons();
}


void Menu_Loop(void)
{
	/* Device menu */
	Menu_Device();

	/* Menu loop */
	for (;;) {
		/* Clear console */
		Con_Clear();

		/* Show gamelist */
		__Menu_ShowList();

		/* Show cover */
		__Menu_ShowCover();

		/* Controls */
		__Menu_Controls();
	}
}
