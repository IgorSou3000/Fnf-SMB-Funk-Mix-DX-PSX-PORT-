/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_AUDIO_H
#define PSXF_GUARD_AUDIO_H

#include "psx.h"

//XA enumerations
typedef enum
{
	XA_Menu,   //MENU.XA
	XA_World1A, //WORLD1A.XA
	XA_World1B, //WORLD1B.XA
	XA_World2A, //WORLD2A.XA
	XA_World2B, //WORLD2B.XA
	XA_Free1A, //FREE1A.XA
	XA_Free1B, //FREE1B.XA
	XA_Free2A, //FREE2A.XA
	XA_Free2B, //FREE2B.XA
	XA_SecretA, //SECRETA.XA
	XA_SecretB, //SECRETB.XA
	XA_MX, //MX.XA

	XA_Max,
} XA_File;

typedef enum
{
	//MENU.XA
	XA_Title, //Title
	XA_Main, //Main menu
	XA_Freeplay,     //Freeplay
	XA_Options,     //Options
	//WORLD1A.XA
	XA_Mushroom_Plains, //Mushroom Plains
	XA_Bricks_And_Lifts,   //Bricks And Lifts
	//WORLD1B.XA
	XA_Lethal_Lava_Lair, //Lethal Lava Lair

	//WORLD2A.XA
	XA_Deep_Deep_Voyage, //Deep Deep Voyage
	XA_Hop_Hop_Heights,   //Hop Hop Heights
	//WORLD2B.XA
	XA_Koopa_Armada, //Koopa Armada

	//FREE1A.XA
	XA_2_Player_Game, //2 Player Game
	XA_Destruction_Dance,   //Destruction Dance
	//FREE1B.XA
	XA_Portal_Power, //Portal Power

	//FREE2A.XA
	XA_Bullet_Time, //Bullet Time
	XA_Boo_Blitz,   //Boo Blitz
	//FREE2B.XA
	XA_Cross_Console_Clash, //Cross Console Clash

	//SECRETA.XA
	XA_Wrong_Warp, //Wrong Warp
	XA_First_Level,   //First Level
	//SECRETB.XA
	XA_Green_Screen, //Green Screen
	XA_Balls, //Balls

	//MX.XA
	XA_Game_Over, //Game Over
	
	XA_TrackMax,
} XA_Track;

//Audio functions
void Audio_Init(void);
void Audio_Quit(void);
void Audio_PlayXA_Track(XA_Track track, u8 volume, u8 channel, boolean loop);
void Audio_SeekXA_Track(XA_Track track);
void Audio_PauseXA(void);
void Audio_StopXA(void);
void Audio_ChannelXA(u8 channel);
s32 Audio_TellXA_Sector(void);
s32 Audio_TellXA_Milli(void);
boolean Audio_PlayingXA(void);
void Audio_WaitPlayXA(void);
void Audio_ProcessXA(void);
void findFreeChannel(void);
u32 Audio_LoadVAGData(u32 *sound, u32 sound_size);
void AudioPlayVAG(int channel, u32 addr);
void Audio_PlaySoundOnChannel(u32 addr, u32 channel);
void Audio_PlaySound(u32 addr);
void Audio_ClearAlloc(void);

#endif
