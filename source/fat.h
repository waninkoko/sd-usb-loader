#ifndef _FAT_H_
#define _FAT_H_

/* Prototypes */
s32 Fat_MountSDHC(void);
s32 Fat_UnmountSDHC(void);
s32 Fat_ReadFile(const char *, void **);

#endif
