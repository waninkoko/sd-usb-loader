#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ogcsys.h>
#include <fat.h>
#include <sys/stat.h>

/* Constants */
#define SDHC_MOUNT	"sdhc"

/* Disc interfaces */
extern const DISC_INTERFACE __io_sdhc;


s32 Fat_MountSDHC(void)
{
	s32 ret;

	/* Initialize SDHC interface */
	ret = __io_sdhc.startup();
	if (!ret)
		return -1;

	/* Mount device */
	ret = fatMountSimple(SDHC_MOUNT, &__io_sdhc);
	if (!ret)
		return -2;

	return 0;
}

s32 Fat_UnmountSDHC(void)
{
	s32 ret;

	/* Unmount device */
	fatUnmount(SDHC_MOUNT);

	/* Shutdown SDHC interface */
	ret = __io_sdhc.shutdown();
	if (!ret)
		return -1;

	return 0;
}

s32 Fat_ReadFile(const char *filepath, void **outbuf)
{
	FILE *fp     = NULL;
	void *buffer = NULL;

	struct stat filestat;
	u32         filelen;

	s32 ret;

	/* Get filestats */
	stat(filepath, &filestat);

	/* Get filesize */
	filelen = filestat.st_size;

	/* Allocate memory */
	buffer = memalign(32, filelen);
	if (!buffer)
		goto err;

	/* Open file */
	fp = fopen(filepath, "rb");
	if (!fp)
		goto err;

	/* Read file */
	ret = fread(buffer, 1, filelen, fp);
	if (ret != filelen)
		goto err;

	/* Set pointer */
	*outbuf = buffer;

	goto out;

err:
	/* Free memory */
	if (buffer)
		free(buffer);

	/* Error code */
	ret = -1;

out:
	/* Close file */
	if (fp)
		fclose(fp);

	return ret;
}
