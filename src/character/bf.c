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
	{BF_ArcMain_BF_Small, {  2,   7,  18,  18}, { 17,  18}}, //0 idle 1
	{BF_ArcMain_BF_Small, { 29,   6,  16,  19}, { 16,  19}}, //1 idle 2
	{BF_ArcMain_BF_Small, { 52,   5,  21,  20}, { 18,  20}}, //2 idle 3
	{BF_ArcMain_BF_Small, { 79,   6,  15,  19}, { 15,  19}}, //3 idle 4

	{BF_ArcMain_BF_Small, {  3,  30,  18,  20}, { 18,  20}}, //4 left 1
	{BF_ArcMain_BF_Small, { 29,  31,  16,  19}, { 16,  19}}, //5 left 2
	{BF_ArcMain_BF_Small, { 54,  31,  16,  19}, { 16,  19}}, //6 left 3

	{BF_ArcMain_BF_Small, { 79,  35,  16,  15}, { 16,  15}}, //7 down 1
	{BF_ArcMain_BF_Small, {  5,  58,  14,  17}, { 14,  17}}, //8 down 2
	{BF_ArcMain_BF_Small, { 30,  58,  14,  17}, { 14,  17}}, //9 down 3

	{BF_ArcMain_BF_Small, { 57,  53,  12,  22}, { 12,  22}}, //10 up 1
	{BF_ArcMain_BF_Small, { 80,  56,  14,  19}, { 14,  19}}, //11 up 2
	{BF_ArcMain_BF_Small, {  6,  81,  14,  19}, { 14,  19}}, //12 up 3

	{BF_ArcMain_BF_Small, { 29,  81,  18,  19}, { 16,  19}}, //13 right 1
	{BF_ArcMain_BF_Small, { 56,  81,  15,  19}, { 14,  19}}, //14 right 2
	{BF_ArcMain_BF_Small, { 80,  81,  15,  19}, { 14,  19}}, //15 right 3

	{BF_ArcMain_BF_Small, {  6, 105,  14,  20}, { 14,  19}}, //16 walking 1
	{BF_ArcMain_BF_Small, { 30, 106,  14,  19}, { 14,  19}}, //17 walking 2
	{BF_ArcMain_BF_Small, { 55, 105,  15,  20}, { 14,  19}}, //18 walking 3
	{BF_ArcMain_BF_Small, { 77, 106,  16,  19}, { 14,  19}}, //19 walking 4

	//bf normal
	{BF_ArcMain_BF, {  4,   9,  18,  16}, { 16,  16}}, //20 idle 1
	{BF_ArcMain_BF, { 29,   8,  17,  17}, { 16,  17}}, //21 idle 2
	{BF_ArcMain_BF, { 54,   7,  17,  18}, { 16,  18}}, //22 idle 3
	{BF_ArcMain_BF, { 79,   5,  18,  20}, { 16,  20}}, //23 idle 4

	{BF_ArcMain_BF, {  3,  31,  18,  19}, { 17,  19}}, //24 left 1
	{BF_ArcMain_BF, { 28,  31,  17,  20}, { 16,  19}}, //25 left 2

	{BF_ArcMain_BF, { 54,  34,  17,  16}, { 16,  16}}, //26 down 1
	{BF_ArcMain_BF, { 80,  32,  16,  18}, { 15,  18}}, //27 down 2

	{BF_ArcMain_BF, {  3,  53,  18,  22}, { 16,  22}}, //28 up 1
	{BF_ArcMain_BF, { 29,  54,  18,  21}, { 16,  21}}, //29 up 2

	{BF_ArcMain_BF, { 56,  55,  17,  20}, { 14,  20}}, //30 right 1
	{BF_ArcMain_BF, { 82,  55,  17,  20}, { 14,  20}}, //31 right 2

	{BF_ArcMain_BF, {  3,  79,  20,  21}, { 14,  19}}, //32 peace 1
	{BF_ArcMain_BF, { 26,  80,  19,  20}, { 14,  19}}, //33 peace 2
	{BF_ArcMain_BF, { 51,  80,  20,  20}, { 14,  19}}, //34 peace 3

	{BF_ArcMain_BF, { 81,  80,  16,  20}, { 14,  19}}, //35 walking 1
	{BF_ArcMain_BF, {  5, 105,  16,  20}, { 14,  19}}, //36 walking 2
	{BF_ArcMain_BF, { 31, 105,  16,  20}, { 14,  19}}, //37 walking 3
	{BF_ArcMain_BF, { 51, 105,  18,  20}, { 14,  19}}, //38 walking 4

	//bf fire
	{BF_ArcMain_BF_Fire, {  4,   9,  18,  16}, { 16,  16}}, //39 idle 1
	{BF_ArcMain_BF_Fire, { 29,   8,  17,  17}, { 16,  17}}, //40 idle 2
	{BF_ArcMain_BF_Fire, { 54,   7,  17,  18}, { 16,  18}}, //41 idle 3
	{BF_ArcMain_BF_Fire, { 79,   5,  18,  20}, { 16,  20}}, //42 idle 4

	{BF_ArcMain_BF_Fire, {  3,  31,  18,  19}, { 17,  19}}, //43 left 1
	{BF_ArcMain_BF_Fire, { 28,  31,  17,  20}, { 16,  19}}, //44 left 2

	{BF_ArcMain_BF_Fire, { 54,  34,  17,  16}, { 16,  16}}, //45 down 1
	{BF_ArcMain_BF_Fire, { 80,  32,  16,  18}, { 15,  18}}, //46 down 2

	{BF_ArcMain_BF_Fire, {  3,  53,  18,  22}, { 16,  22}}, //47 up 1
	{BF_ArcMain_BF_Fire, { 29,  54,  18,  21}, { 16,  21}}, //48 up 2

	{BF_ArcMain_BF_Fire, { 56,  55,  17,  20}, { 14,  20}}, //49 right 1
	{BF_ArcMain_BF_Fire, { 82,  55,  17,  20}, { 14,  20}}, //50 right 2

	{BF_ArcMain_BF_Fire, {  3,  79,  20,  21}, { 14,  19}}, //51 peace 1
	{BF_ArcMain_BF_Fire, { 26,  80,  19,  20}, { 14,  19}}, //52 peace 2
	{BF_ArcMain_BF_Fire, { 51,  80,  20,  20}, { 14,  19}}, //53 peace 3

	{BF_ArcMain_BF_Fire, { 81,  80,  16,  20}, { 14,  19}}, //54 walking 1
	{BF_ArcMain_BF_Fire, {  5, 105,  16,  20}, { 14,  19}}, //55 walking 2
	{BF_ArcMain_BF_Fire, { 31, 105,  16,  20}, { 14,  19}}, //56 walking 3
	{BF_ArcMain_BF_Fire, { 51, 105,  18,  20}, { 14,  19}}, //57 walking 4
};

static const Animation char_bf_small_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5,  6, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8,  9, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){10, 11, 12, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){13, 14, 15, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_Special
};
static const Animation char_bf_anim[CharAnim_Max] = {
	{2, (const u8[]){20, 21, 21, 22, 23, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){24, 25, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){26, 27, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){28, 29, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){30, 31, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_Special
};
static const Animation char_bf_fire_anim[CharAnim_Max] = {
	{2, (const u8[]){39, 40, 40, 41, 42, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){43, 44, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){45, 46, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){47, 48, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){49, 50, ASCR_BACK, 1}},             //CharAnim_Right
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
	switch (character->powerup -1)
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
