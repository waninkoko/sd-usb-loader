#include <stdio.h>
#include <string.h>
#include <ogcsys.h>

#include "partition.h"
#include "sdhc.h"
#include "usbstorage.h"
#include "utils.h"
#include "wbfs.h"

/* 'partition table' structure */
typedef struct {
	/* Zero bytes */
	u8 padding[446];

	/* Partition table entries */
	partitionEntry entries[MAX_PARTITIONS];
} ATTRIBUTE_PACKED partitionTable;


s32 Partition_GetEntries(u32 device, partitionEntry *outbuf, u32 *outval)
{
	static partitionTable table ATTRIBUTE_ALIGN(32);

	u32 cnt, sector_size;
	s32 ret;

	/* Read from specified device */
	switch (device) {
	case WBFS_DEVICE_USB: {
		/* Get sector size */
		ret = USBStorage_GetCapacity(&sector_size);
		if (ret < 0)
			return ret;

		/* Read partition table */
		ret = USBStorage_ReadSectors(0, 1, &table);
		if (ret < 0)
			return ret;

		break;
	}

	case WBFS_DEVICE_SDHC: {
		/* SDHC sector size */
		sector_size = SDHC_SECTOR_SIZE;

		/* Read partition table */
		ret = SDHC_ReadSectors(0, 1, &table);
		if (!ret)
			return -1;

		break;
	}

	default:
		return -1;
	}


	/* Swap endianess */
	for (cnt = 0; cnt < 4; cnt++) {
		partitionEntry *entry = &table.entries[cnt];

		entry->sector = swap32(entry->sector);
		entry->size   = swap32(entry->size);
	}

	/* Set partition entries */
	memcpy(outbuf, table.entries, sizeof(table.entries));

	/* Set sector size */
	*outval = sector_size;

	return 0;
}
