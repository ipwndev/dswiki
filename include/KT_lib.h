//-----------------------------------------------------------------//
//                                                                 //
//   +--+  /--/ +---------+          +-+  +-+  +-+                 //
//   |  | /  /  |         |          | |  | |  | |                 //
//   |  |/  /   +--+   +--+          | |  +-+  | |                 //
//   |     /       |   |             | |       | +-----+           //
//   |     \       |   |             | |  +-+  | +---+ |           //
//   |  |\  \      |   |             | |  | |  | |   | |           //
//   |  | \  \     |   |   +-------+ | |  | |  | +---+ |           //
//   +--+  \__\    +---+   +-------+ +-+  +-+  +-------+           //
//                                                                 //
//  Noob Lib for EFSv2                                             //
//  BY KRAM AND TOILETKING                                         //
//  04/08/2008                                                     //
//                                                                 //
//-----------------------------------------------------------------//


#ifndef __KT_LIB1__
#define __KT_LIB1__

#ifdef __cplusplus
extern "C" {
#endif


//You cannot use AS_lib and MikMod all at once, therefore you must comment on one of the functions below.

/*//<======= If you don't use AS_lib, just put '/ *' before this comment.


void KT_PlayMyMP3(const char* name);


void KT_PlayMyRaw(const char* name, u8 volume, u8 pan, u8 loop, u8 prio);


void KT_PlayMyVideo(char * filename, u8 screen, int speed, int enable_sound);

//Do not erase this comment*/



/*//<======= If you don't use MikMod, just put '/ *' before this comment.


void KT_ModFree();


void KT_PlayMyMod(const char* name);

//Do not erase this comment*/


void KT_Init(void);


void KT_UseEFS(void);


void KT_UseFAT(void);


void KT_StreamPlayMyVideo(char * name, u8 screen);


int KT_IsStreamVideoPlaying(void);


int KT_GetFrameCount(char * name);


int KT_GetCurrentFrame(void);


void KT_SetCurrentFrame(int frame);


void KT_StopStreamVideo(void);


void KT_StreamVideoUpdate(void);


void KT_DataSav(const char* name, const char* data);


void KT_DataRead(const char* name, int origin, long int offset, int count);


void KT_LoadBmp(int screen, const char* name);


void KT_LoadJpeg(int screen, const char* name);


void KT_LoadGif(int screen, const char* name, s16 x, s16 y);


void KT_CreateSprite(u8 screen, u8 sprite_number, const char* name, u8  obj_shape, u8 obj_size,  u8 color_mode, u8 palette, u8 erase_palette, s16 x, s16 y);


void KT_DeleteSprite(u8 screen, u8 sprite_number);


void KT_ResetSpriteSys(void);


void KT_ResetSpriteSysScreen(int screen);


void KT_ShowBG(u16 screen, u16 bg, const char* name);


void KT_DelBG(u16 screen, u16 bg);


void KT_ResetBgSys(void);


void KT_ResetBgSysScreen(int screen);

#ifdef __cplusplus
}
#endif

#endif
