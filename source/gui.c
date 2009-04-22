#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>

#include "fat.h"
#include "video.h"

/* Constants */
#define CONSOLE_XCOORD		258
#define CONSOLE_YCOORD		112
#define CONSOLE_WIDTH		354
#define CONSOLE_HEIGHT		304

#define COVER_XCOORD		24
#define COVER_YCOORD		104
#define COVER_WIDTH		160
#define COVER_HEIGHT		225

#define BACKGROUND_WIDTH	640
#define BACKGROUND_HEIGHT	480

#define USBLOADER_PATH		"sdhc:/usb-loader"


s32 __Gui_GetPngDimensions(void *img, u32 *w, u32 *h)
{
	IMGCTX   ctx = NULL;
	PNGUPROP imgProp;

	s32 ret;

	/* Select PNG data */
	ctx = PNGU_SelectImageFromBuffer(img);
	if (!ctx) {
		ret = -1;
		goto out;
	}

	/* Get image properties */
	ret = PNGU_GetImageProperties(ctx, &imgProp);
	if (ret != PNGU_OK) {
		ret = -1;
		goto out;
	}

	/* Set image width and height */
	*w = imgProp.imgWidth;
	*h = imgProp.imgHeight;

	/* Success */
	ret = 0;

out:
	/* Free memory */
	if (ctx)
		PNGU_ReleaseImageContext(ctx);

	return ret;
}

s32 __Gui_DrawPng(void *img, u32 x, u32 y)
{
	IMGCTX   ctx = NULL;
	PNGUPROP imgProp;

	s32 ret;

	/* Select PNG data */
	ctx = PNGU_SelectImageFromBuffer(img);
	if (!ctx) {
		ret = -1;
		goto out;
	}

	/* Get image properties */
	ret = PNGU_GetImageProperties(ctx, &imgProp);
	if (ret != PNGU_OK) {
		ret = -1;
		goto out;
	}

	/* Draw image */
	Video_DrawPng(ctx, imgProp, x, y);

	/* Success */
	ret = 0;

out:
	/* Free memory */
	if (ctx)
		PNGU_ReleaseImageContext(ctx);

	return ret;
}


void Gui_InitConsole(void)
{
	/* Initialize console */
	Con_Init(CONSOLE_XCOORD, CONSOLE_YCOORD, CONSOLE_WIDTH, CONSOLE_HEIGHT);
}

void Gui_DrawBackground(void)
{
	extern char bgImg[];

	void *imgData = (void *)bgImg;

	s32 ret;

	/* Try to open background image from SD */
	ret = Fat_ReadFile(USBLOADER_PATH "/background.png", &imgData);
	if (ret > 0) {
		u32 width, height;

		/* Get image dimensions */
		__Gui_GetPngDimensions(imgData, &width, &height);

		/* Background is too big, use default background */
		if ((width > BACKGROUND_WIDTH) || (height > BACKGROUND_HEIGHT))
			imgData = (void *)bgImg;
	}

	/* Draw background */
	__Gui_DrawPng(imgData, 0, 0);

	/* Free memory */
	if (imgData != bgImg)
		free(imgData);
} 

void Gui_DrawCover(u8 *discid)
{
	extern char coverImg[];

	void *imgData = (void *)coverImg;

	char filepath[128];
	s32  ret;

	/* Generate cover filepath */
	sprintf(filepath, USBLOADER_PATH "/covers/%s.png", discid);

	/* Open cover */
	ret = Fat_ReadFile(filepath, &imgData);
	if (ret > 0) {
		u32 width, height;

		/* Get image dimensions */
		__Gui_GetPngDimensions(imgData, &width, &height);

		/* Image is too big, use default image */
		if ((width > COVER_WIDTH) || (height > COVER_HEIGHT))
			imgData = (void *)coverImg;
	}

	/* Draw cover */
	__Gui_DrawPng(imgData, COVER_XCOORD, COVER_YCOORD);

	/* Free memory */
	if (imgData != coverImg)
		free(imgData);
}
