#ifndef _WDVD_H_
#define _WDVD_H_

/* Prototypes */
s32 WDVD_Init(void);
s32 WDVD_Close(void);
s32 WDVD_GetHandle(void);
s32 WDVD_Reset(void);
s32 WDVD_ReadDiskId(void *);
s32 WDVD_Seek(u64);
s32 WDVD_Offset(u64);
s32 WDVD_StopLaser(void);
s32 WDVD_StopMotor(void);
s32 WDVD_OpenPartition(u64);
s32 WDVD_ClosePartition(void);
s32 WDVD_UnencryptedRead(void *, u32, u64);
s32 WDVD_Read(void *, u32, u64);
s32 WDVD_WaitForDisc(void);
s32 WDVD_GetCoverStatus(u32 *);
s32 WDVD_SetWBFSMode(u32, u8 *);

#endif

