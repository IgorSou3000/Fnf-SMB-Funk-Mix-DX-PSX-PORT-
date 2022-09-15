/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "marioccc.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Mario Cross Console Crash player types
enum
{
	MarioCCC_ArcMain_MarioCCC_Small,
	MarioCCC_ArcMain_MarioCCC,
	MarioCCC_ArcMain_MarioCCC_Fire,
	
	MarioCCC_ArcMain_Max,
};

enum
{
	MarioCCC_ArcDead_Dead1, //Mic Drop
	MarioCCC_ArcDead_Dead2, //Twitch
	MarioCCC_ArcDead_Retry, //Retry prompt
	
	MarioCCC_ArcDead_Max,
};

#define MarioCCC_Arc_Max MarioCCC_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[MarioCCC_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
} Char_MarioCCC;

//Mario Cross Console Crash player definitions
static const CharFrame char_marioccc_frame[] = {
	//mario small
	{MarioCCC_ArcMain_MarioCCC_Small, {  0,   0,  48,  48}, { 30,  30}}, //0 idle 1
	{MarioCCC_ArcMain_MarioCCC_Small, { 48,   0,  48,  48}, { 30,  30}}, //1 idle 2
	{MarioCCC_ArcMain_MarioCCC_Small, { 96,   0,  48,  48}, { 30,  30}}, //2 idle 3
	{MarioCCC_ArcMain_MarioCCC_Small, {144,   0,  48,  48}, { 30,  30}}, //3 idle 4

	{MarioCCC_ArcMain_MarioCCC_Small, { 96,  48,  48,  48}, { 30,  30}}, //4 left 1
	{MarioCCC_ArcMain_MarioCCC_Small, {144,  48,  48,  48}, { 30,  30}}, //5 left 2

	{MarioCCC_ArcMain_MarioCCC_Small, { 96,  96,  48,  48}, { 30,  30}}, //6 down 1
	{MarioCCC_ArcMain_MarioCCC_Small, {144,  96,  48,  48}, { 30,  30}}, //7 down 2

	{MarioCCC_ArcMain_MarioCCC_Small, {  0,  96,  48,  48}, { 30,  30}}, //8 up 1
	{MarioCCC_ArcMain_MarioCCC_Small, { 48,  96,  48,  48}, { 30,  30}}, //9 up 2

	{MarioCCC_ArcMain_MarioCCC_Small, {  0,  48,  48,  48}, { 30,  30}}, //10 right 1
	{MarioCCC_ArcMain_MarioCCC_Small, { 48,  48,  48,  48}, { 30,  30}}, //11 right 2

	{MarioCCC_ArcMain_MarioCCC_Small, {  0, 144,  48,  48}, { 30,  30}}, //12 peace 1
	{MarioCCC_ArcMain_MarioCCC_Small, { 48, 144,  48,  48}, { 30,  30}}, //3  peace 2
	{MarioCCC_ArcMain_MarioCCC_Small, { 96, 144,  48,  48}, { 30,  30}}, //14 peace 3
	{MarioCCC_ArcMain_MarioCCC_Small, {144, 144,  48,  48}, { 30,  30}}, //15 peace 4
	{MarioCCC_ArcMain_MarioCCC_Small, {  0, 192,  48,  48}, { 30,  30}}, //16 peace 5
	{MarioCCC_ArcMain_MarioCCC_Small, { 48, 192,  48,  48}, { 30,  30}}, //17 peace 6
	{MarioCCC_ArcMain_MarioCCC_Small, { 96, 192,  48,  48}, { 30,  30}}, //18 peace 7

	{MarioCCC_ArcMain_MarioCCC_Small, {144, 192,  48,  48}, { 30,  30}}, //19 jump 1

	//mario normal
	{MarioCCC_ArcMain_MarioCCC, {  0,   0,  48,  48}, { 30,  30}}, //20 idle 1
	{MarioCCC_ArcMain_MarioCCC, { 48,   0,  48,  48}, { 30,  30}}, //21 idle 2
	{MarioCCC_ArcMain_MarioCCC, { 96,   0,  48,  48}, { 30,  30}}, //22 idle 3
	{MarioCCC_ArcMain_MarioCCC, {144,   0,  48,  48}, { 30,  30}}, //23 idle 4

	{MarioCCC_ArcMain_MarioCCC, { 96,  48,  48,  48}, { 30,  30}}, //24 left 1
	{MarioCCC_ArcMain_MarioCCC, {144,  48,  48,  48}, { 30,  30}}, //25 left 2

	{MarioCCC_ArcMain_MarioCCC, { 96,  96,  48,  48}, { 30,  30}}, //26 down 1
	{MarioCCC_ArcMain_MarioCCC, {144,  96,  48,  48}, { 30,  30}}, //27 down 2

	{MarioCCC_ArcMain_MarioCCC, {  0,  96,  48,  48}, { 30,  30}}, //28 up 1
	{MarioCCC_ArcMain_MarioCCC, { 48,  96,  48,  48}, { 30,  30}}, //29 up 2

	{MarioCCC_ArcMain_MarioCCC, {  0,  48,  48,  48}, { 30,  30}}, //30 right 1
	{MarioCCC_ArcMain_MarioCCC, { 48,  48,  48,  48}, { 30,  30}}, //31 right 2

	{MarioCCC_ArcMain_MarioCCC, {  0, 144,  48,  48}, { 30,  30}}, //32 peace 1
	{MarioCCC_ArcMain_MarioCCC, { 48, 144,  48,  48}, { 30,  30}}, //33 peace 2
	{MarioCCC_ArcMain_MarioCCC, { 96, 144,  48,  48}, { 30,  30}}, //34 peace 3
	{MarioCCC_ArcMain_MarioCCC, {144, 144,  48,  48}, { 30,  30}}, //35 peace 4
	{MarioCCC_ArcMain_MarioCCC, {  0, 192,  48,  48}, { 30,  30}}, //36 peace 5
	{MarioCCC_ArcMain_MarioCCC, { 48, 192,  48,  48}, { 30,  30}}, //37 peace 6
	{MarioCCC_ArcMain_MarioCCC, { 96, 192,  48,  48}, { 30,  30}}, //38 peace 7

	{MarioCCC_ArcMain_MarioCCC, {144, 192,  48,  48}, { 30,  30}}, //39 jump 1

	//mario fire
	{MarioCCC_ArcMain_MarioCCC_Fire, {  0,   0,  48,  48}, { 30,  30}}, //40 idle 1
	{MarioCCC_ArcMain_MarioCCC_Fire, { 48,   0,  48,  48}, { 30,  30}}, //41 idle 2
	{MarioCCC_ArcMain_MarioCCC_Fire, { 96,   0,  48,  48}, { 30,  30}}, //42 idle 3
	{MarioCCC_ArcMain_MarioCCC_Fire, {144,   0,  48,  48}, { 30,  30}}, //43 idle 4

	{MarioCCC_ArcMain_MarioCCC_Fire, { 96,  48,  48,  48}, { 30,  30}}, //44 left 1
	{MarioCCC_ArcMain_MarioCCC_Fire, {144,  48,  48,  48}, { 30,  30}}, //45 left 2

	{MarioCCC_ArcMain_MarioCCC_Fire, { 96,  96,  48,  48}, { 30,  30}}, //46 down 1
	{MarioCCC_ArcMain_MarioCCC_Fire, {144,  96,  48,  48}, { 30,  30}}, //47 down 2

	{MarioCCC_ArcMain_MarioCCC_Fire, {  0,  96,  48,  48}, { 30,  30}}, //48 up 1
	{MarioCCC_ArcMain_MarioCCC_Fire, { 48,  96,  48,  48}, { 30,  30}}, //49 up 2

	{MarioCCC_ArcMain_MarioCCC_Fire, {  0,  48,  48,  48}, { 30,  30}}, //50 right 1
	{MarioCCC_ArcMain_MarioCCC_Fire, { 48,  48,  48,  48}, { 30,  30}}, //51 right 2

	{MarioCCC_ArcMain_MarioCCC_Fire, {  0, 144,  48,  48}, { 30,  30}}, //52 peace 1
	{MarioCCC_ArcMain_MarioCCC_Fire, { 48, 144,  48,  48}, { 30,  30}}, //53  peace 2
	{MarioCCC_ArcMain_MarioCCC_Fire, { 96, 144,  48,  48}, { 30,  30}}, //54 peace 3
	{MarioCCC_ArcMain_MarioCCC_Fire, {144, 144,  48,  48}, { 30,  30}}, //55 peace 4
	{MarioCCC_ArcMain_MarioCCC_Fire, {  0, 192,  48,  48}, { 30,  30}}, //56 peace 5
	{MarioCCC_ArcMain_MarioCCC_Fire, { 48, 192,  48,  48}, { 30,  30}}, //57 peace 6
	{MarioCCC_ArcMain_MarioCCC_Fire, { 96, 192,  48,  48}, { 30,  30}}, //58 peace 7

	{MarioCCC_ArcMain_MarioCCC_Fire, {144, 192,  48,  48}, { 30,  30}}, //59 jump 1
};

static const Animation char_marioccc_small_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{2, (const u8[]){12, 13, 14, 15, 16, 17, 18, ASCR_BACK, 1}},  //CharAnim_Special
};
static const Animation char_marioccc_anim[CharAnim_Max] = {
	{2, (const u8[]){20, 21, 22, 23, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){24, 25, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){26, 27, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){28, 29, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){30, 31, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{2, (const u8[]){32, 33, 34, 35, 36, 37, 38, ASCR_BACK, 1}},     //CharAnim_Special
};
static const Animation char_marioccc_fire_anim[CharAnim_Max] = {
	{2, (const u8[]){40, 41, 42, 43, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){44, 45, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){46, 47, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){48, 49, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){50, 51, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{2, (const u8[]){52, 53, 54, 55, 56, 57, 58, ASCR_BACK, 1}},		 //CharAnim_Special
};


//Mario Cross Console Crash player functions
void Char_MarioCCC_SetFrame(void *user, u8 frame)
{
	Char_MarioCCC *this = (Char_MarioCCC*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_marioccc_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_MarioCCC_Tick(Character *character)
{
	Char_MarioCCC *this = (Char_MarioCCC*)character;

	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if ((Animatable_Ended(&character->animatable) || Animatable_Ended(&character->animatable2) || Animatable_Ended(&character->animatable3)) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
		
		//Stage specific animations
		if (stage.note_scroll >= 0)
		{
			if(stage.song_step == 326)
				character->set_anim(character, CharAnim_Special); //peace
		}
	}
	
	//Animate and draw character
	switch (character->powerup -1)
	{
	case 2:
	Animatable_Animate(&character->animatable3, (void*)this, Char_MarioCCC_SetFrame);
	break;
	case 1:
	Animatable_Animate(&character->animatable2, (void*)this, Char_MarioCCC_SetFrame);
	break;
	case 0:
	Animatable_Animate(&character->animatable, (void*)this, Char_MarioCCC_SetFrame);
	break;
	default:
	Animatable_Animate(&character->animatable, (void*)this, Char_MarioCCC_SetFrame);
	break;
	}

	Character_Draw(character, &this->tex, &char_marioccc_frame[this->frame]);
}

void Char_MarioCCC_SetAnim(Character *character, u8 anim)
{
	Char_MarioCCC *this = (Char_MarioCCC*)character;
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatable2, anim);
	Animatable_SetAnim(&character->animatable3, anim);
	Character_CheckStartSing(character);
}

void Char_MarioCCC_Free(Character *character)
{
	Char_MarioCCC *this = (Char_MarioCCC*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_MarioCCC_New(fixed_t x, fixed_t y)
{
	//Allocate Mario Cross Console Crash object
	Char_MarioCCC *this = Mem_Alloc(sizeof(Char_MarioCCC));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_MarioCCC_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_MarioCCC_Tick;
	this->character.set_anim = Char_MarioCCC_SetAnim;
	this->character.free = Char_MarioCCC_Free;
	
	Animatable_Init(&this->character.animatable, char_marioccc_small_anim);
	Animatable_Init(&this->character.animatable2, char_marioccc_anim);
	Animatable_Init(&this->character.animatable3, char_marioccc_fire_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MARIOCCC.ARC;1");
	
	const char **pathp = (const char *[]){
		"mario-s.tim",   //MarioCCC_ArcMain_MarioCCC-Small
		"mario.tim",   //MarioCCC_ArcMain_MarioCCC
		"mario-f.tim",   //MarioCCC_ArcMain_MarioCCC-Fire
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	return (Character*)this;
}
