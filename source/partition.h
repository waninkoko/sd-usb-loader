#ifndef _PARTITION_H_
#define _PARTITION_H_

/* 'partition entry' structure */
typedef struct {
	/* Boot indicator */
	u8 boot;

	/* Starting CHS */
	u8 start[3];

	/* Partition type */
	u8 type;

	/* Ending CHS */
	u8 end[3];

	/* Partition sector */
	u32 sector;

	/* Partition size */
	u32 size;
} ATTRIBUTE_PACKED partitionEntry;

/* Constants */
#define MAX_PARTITIONS		4

/* Prototypes */
s32 Partition_GetEntries(u32, partitionEntry *, u32 *);

#endif
