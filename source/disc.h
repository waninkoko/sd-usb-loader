#ifndef _DISC_H_
#define _DISC_H_

/* Disc header structure */
struct discHdr
{
	/* Game ID */
	u8 id[6];

	/* Game version */
	u16 version;

	/* Audio streaming */
	u8 streaming;
	u8 bufsize;

	/* Padding */
	u8 unused1[14];

	/* Magic word */
	u32 magic;

	/* Padding */
	u8 unused2[4];

	/* Game title */
	char title[64];

	/* Encryption/Hashing */
	u8 encryption;
	u8 h3_verify;

	/* Padding */
	u8 unused3[30];
} ATTRIBUTE_PACKED;

/* Prototypes */
s32  Disc_Init(void);
s32  Disc_Open(void);
s32  Disc_Wait(void);
s32  Disc_SetWBFS(u32, u8 *);
s32  Disc_ReadHeader(void *);
s32  Disc_IsWii(void);
s32  Disc_BootPartition(u64);
s32  Disc_WiiBoot(void);

#endif

