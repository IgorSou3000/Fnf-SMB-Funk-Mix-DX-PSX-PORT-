/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend player types
enum
{
	BF_ArcMain_BF_Small,
	BF_ArcMain_BF,
	BF_ArcMain_BF_Fire,
	
	BF_ArcMain_Max,
};

enum
{
	BF_ArcDead_Dead1, //Mic Drop
	BF_ArcDead_Dead2, //Twitch
	BF_ArcDead_Retry, //Retry prompt
	
	BF_ArcDead_Max,
};

#define BF_Arc_Max BF_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BF_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
} Char_BF;

//Boyfriend player definitions
static const CharFrame char_bf_frame[] = {
	//bf small
	{BF_ArcMain_BF_Small, {  0,   0,  34,  34}, { 30,  30}}, //0 idle 1
	{BF_ArcMain_BF_Small, { 34,   0,  34,  34}, { 30,  30}}, //1 idle 2
	{BF_ArcMain_BF_Small, { 68,   0,  34,  34}, { 30,  30}}, //2 idle 3
	{BF_ArcMain_BF_Small, {102,   0,  34,  34}, { 30,  30}}, //3 idle 4
	{BF_ArcMain_BF_Small, {  0,  34,  34,  34}, { 30,  30}}, //4 idle 5

	{BF_ArcMain_BF_Small, { 34,  34,  34,  34}, { 30,  30}}, //5 left 1
	{BF_ArcMain_BF_Small, { 68,  34,  34,  34}, { 30,  30}}, //6 left 2
	{BF_ArcMain_BF_Small, {102,  34,  34,  34}, { 30,  30}}, //7 left 3

	{BF_ArcMain_BF_Small, { 68, 102,  34,  34}, { 30,  30}}, //8 down 1
	{BF_ArcMain_BF_Small, {102, 102,  34,  34}, { 30,  30}}, //9 down 2
	{BF_ArcMain_BF_Small, {  0, 136,  34,  34}, { 30,  30}}, //10 down 3

	{BF_ArcMain_BF_Small, {  0,  68,  34,  34}, { 30,  30}}, //11 up 1
	{BF_ArcMain_BF_Small, { 34,  68,  34,  34}, { 30,  30}}, //12 up 2
	{BF_ArcMain_BF_Small, { 68,  68,  34,  34}, { 30,  30}}, //13 up 3

	{BF_ArcMain_BF_Small, {102,  68,  34,  34}, { 30,  30}}, //14 right 1
	{BF_ArcMain_BF_Small, {  0, 102,  34,  34}, { 30,  30}}, //15 right 2
	{BF_ArcMain_BF_Small, { 34, 102,  34,  34}, { 30,  30}}, //16 right 3

	{BF_ArcMain_BF_Small, {  6, 105,  14,  20}, { 30,  30}}, //17 walking 1
	{BF_ArcMain_BF_Small, { 30, 106,  14,  19}, { 30,  30}}, //18 walking 2
	{BF_ArcMain_BF_Small, { 55, 105,  15,  20}, { 30,  30}}, //19 walking 3
	{BF_ArcMain_BF_Small, { 77, 106,  16,  19}, { 30,  30}}, //20 walking 4

	//bf normal
	{BF_ArcMain_BF, {  0,   0,  26,  26}, { 26,  26}}, //21 idle 1
	{BF_ArcMain_BF, { 26,   0,  26,  26}, { 26,  26}}, //22 idle 2
	{BF_ArcMain_BF, { 52,   0,  26,  26}, { 26,  26}}, //23 idle 3
	{BF_ArcMain_BF, { 78,   0,  26,  26}, { 26,  26}}, //24 idle 4
	{BF_ArcMain_BF, {  0,  26,  26,  26}, { 26,  26}}, //25 idle 5
	{BF_ArcMain_BF, { 26,  26,  26,  26}, { 26,  26}}, //26 idle 6
	{BF_ArcMain_BF, { 52,  26,  26,  26}, { 26,  26}}, //27 idle 7

	{BF_ArcMain_BF, { 78,  26,  26,  26}, { 26,  26}}, //28 left 1
	{BF_ArcMain_BF, {  0,  52,  26,  26}, { 26,  26}}, //29 left 2
	{BF_ArcMain_BF, { 26,  52,  26,  26}, { 26,  26}}, //30 left 3

	{BF_ArcMain_BF, { 26,  78,  26,  26}, { 26,  26}}, //31 down 1
	{BF_ArcMain_BF, { 52,  78,  26,  26}, { 26,  26}}, //32 down 2
	{BF_ArcMain_BF, { 78,  78,  26,  26}, { 26,  26}}, //33 down 3

	{BF_ArcMain_BF, { 52,  52,  26,  26}, { 26,  26}}, //34 up 1
	{BF_ArcMain_BF, { 78,  52,  26,  26}, { 26,  26}}, //35 up 2
	{BF_ArcMain_BF, {  0,  78,  26,  26}, { 26,  26}}, //36 up 3

	{BF_ArcMain_BF, {  0, 104,  26,  26}, { 26,  26}}, //37 right 1
	{BF_ArcMain_BF, { 26, 104,  26,  26}, { 26,  26}}, //38 right 2
	{BF_ArcMain_BF, { 52, 104,  26,  26}, { 26,  26}}, //39 right 3

	{BF_ArcMain_BF, { 78, 104,  26,  26}, { 26,  26}}, //40 peace 1
	{BF_ArcMain_BF, {  0, 130,  16,  26}, { 26,  26}}, //41 peace 2
	{BF_ArcMain_BF, { 26, 130,  26,  26}, { 26,  26}}, //42 peace 3

	{BF_ArcMain_BF, { 52, 130,  26,  26}, { 26,  26}}, //43 walking 1
	{BF_ArcMain_BF, { 78, 130,  26,  26}, { 26,  26}}, //44 walking 2
	{BF_ArcMain_BF, {  0, 156,  26,  26}, { 26,  26}}, //45 walking 3
	{BF_ArcMain_BF, { 52, 156,  26,  26}, { 26,  26}}, //46 walking 4

	//bf fire
	{BF_ArcMain_BF_Fire, {  0,   0,  26,  26}, { 26,  26}}, //21 idle 1
	{BF_ArcMain_BF_Fire, { 26,   0,  26,  26}, { 26,  26}}, //22 idle 2
	{BF_ArcMain_BF_Fire, { 52,   0,  26,  26}, { 26,  26}}, //23 idle 3
	{BF_ArcMain_BF_Fire, { 78,   0,  26,  26}, { 26,  26}}, //24 idle 4
	{BF_ArcMain_BF_Fire, {  0,  26,  26,  26}, { 26,  26}}, //25 idle 5
	{BF_ArcMain_BF_Fire, { 26,  26,  26,  26}, { 26,  26}}, //26 idle 6
	{BF_ArcMain_BF_Fire, { 52,  26,  26,  26}, { 26,  26}}, //27 idle 7

	{BF_ArcMain_BF_Fire, { 78,  26,  26,  26}, { 26,  26}}, //28 left 1
	{BF_ArcMain_BF_Fire, {  0,  52,  26,  26}, { 26,  26}}, //29 left 2
	{BF_ArcMain_BF_Fire, { 26,  52,  26,  26}, { 26,  26}}, //30 left 3

	{BF_ArcMain_BF_Fire, { 26,  78,  26,  26}, { 26,  26}}, //31 down 1
	{BF_ArcMain_BF_Fire, { 52,  78,  26,  26}, { 26,  26}}, //32 down 2
	{BF_ArcMain_BF_Fire, { 78,  78,  26,  26}, { 26,  26}}, //33 down 3

	{BF_ArcMain_BF_Fire, { 52,  52,  26,  26}, { 26,  26}}, //34 up 1
	{BF_ArcMain_BF_Fire, { 78,  52,  26,  26}, { 26,  26}}, //35 up 2
	{BF_ArcMain_BF_Fire, {  0,  78,  26,  26}, { 26,  26}}, //36 up 3

	{BF_ArcMain_BF_Fire, {  0, 104,  26,  26}, { 26,  26}}, //37 right 1
	{BF_ArcMain_BF_Fire, { 26, 104,  26,  26}, { 26,  26}}, //38 right 2
	{BF_ArcMain_BF_Fire, { 52, 104,  26,  26}, { 26,  26}}, //39 right 3

	{BF_ArcMain_BF_Fire, { 78, 104,  26,  26}, { 26,  26}}, //40 peace 1
	{BF_ArcMain_BF_Fire, {  0, 130,  16,  26}, { 26,  26}}, //41 peace 2
	{BF_ArcMain_BF_Fire, { 26, 130,  26,  26}, { 26,  26}}, //42 peace 3

	{BF_ArcMain_BF_Fire, { 52, 130,  26,  26}, { 26,  26}}, //43 walking 1
	{BF_ArcMain_BF_Fire, { 78, 130,  26,  26}, { 26,  26}}, //44 walking 2
	{BF_ArcMain_BF_Fire, {  0, 156,  26,  26}, { 26,  26}}, //45 walking 3
	{BF_ArcMain_BF_Fire, { 52, 156,  26,  26}, { 26,  26}}, //46 walking 4
};

static const Animation char_bf_small_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5,  6,  7, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9, 10, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){11, 12, 13, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){14, 15, 16, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_Special
};
static const Animation char_bf_anim[CharAnim_Max] = {
	{2, (const u8[]){21, 22, 23, 24, 25, 26, 27, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){28, 29, 30, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){31, 32, 33, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){34, 35, 36, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){37, 38, 39, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_Special
};
static const Animation char_bf_fire_anim[CharAnim_Max] = {
	{2, (const u8[]){47, 48, 49, 50, 51, 52, 53, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){54, 55, 56, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){57, 58, 59, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){60, 61, 62, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){63, 64, 65, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_Special
};


//Boyfriend player functions
void Char_BF_SetFrame(void *user, u8 frame)
{
	Char_BF *this = (Char_BF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BF_Tick(Character *character)
{
	Char_BF *this = (Char_BF*)character;

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
		}
	}
	
	//Animate and draw character
	switch (character->powerup)
	{
	case 2:
	Animatable_Animate(&character->animatable3, (void*)this, Char_BF_SetFrame);
	break;
	case 1:
	Animatable_Animate(&character->animatable2, (void*)this, Char_BF_SetFrame);
	break;
	case 0:
	Animatable_Animate(&character->animatable, (void*)this, Char_BF_SetFrame);
	break;
	default:
	Animatable_Animate(&character->animatable, (void*)this, Char_BF_SetFrame);
	break;
	}

	Character_Draw(character, &this->tex, &char_bf_frame[this->frame]);
}

void Char_BF_SetAnim(Character *character, u8 anim)
{
	Char_BF *this = (Char_BF*)character;
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatable2, anim);
	Animatable_SetAnim(&character->animatable3, anim);
	Character_CheckStartSing(character);
}

void Char_BF_Free(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_BF_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BF *this = Mem_Alloc(sizeof(Char_BF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BF_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BF_Tick;
	this->character.set_anim = Char_BF_SetAnim;
	this->character.free = Char_BF_Free;
	
	Animatable_Init(&this->character.animatable, char_bf_small_anim);
	Animatable_Init(&this->character.animatable2, char_bf_anim);
	Animatable_Init(&this->character.animatable3, char_bf_fire_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BF.ARC;1");
	
	const char **pathp = (const char *[]){
		"bf-small.tim",   //BF_ArcMain_BF-Small
		"bf.tim",   //BF_ArcMain_BF
		"bf-fire.tim",   //BF_ArcMain_BF-Fire
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
