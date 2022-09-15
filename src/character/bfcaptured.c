/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfcaptured.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend player types
enum
{
	BFCaptured_ArcMain_BFCaptured_Small,
	BFCaptured_ArcMain_BFCaptured,
	BFCaptured_ArcMain_BFCaptured_Fire,
	
	BFCaptured_ArcMain_Max,
};

enum
{
	BFCaptured_ArcDead_Dead1, //Mic Drop
	BFCaptured_ArcDead_Dead2, //Twitch
	BFCaptured_ArcDead_Retry, //Retry prompt
	
	BFCaptured_ArcDead_Max,
};

#define BFCaptured_Arc_Max BFCaptured_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BFCaptured_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
} Char_BFCaptured;

//Boyfriend player definitions
static const CharFrame char_bfcaptured_frame[] = {
	//bfcaptured small
	{BFCaptured_ArcMain_BFCaptured_Small, {  0,   0,  46,  26}, { 30,  30}}, //0 idle 1
	{BFCaptured_ArcMain_BFCaptured_Small, { 46,   0,  46,  26}, { 30,  30}}, //1 idle 2
	{BFCaptured_ArcMain_BFCaptured_Small, { 92,   0,  46,  26}, { 30,  30}}, //2 idle 3
	{BFCaptured_ArcMain_BFCaptured_Small, {138,   0,  46,  26}, { 30,  30}}, //3 idle 4
	{BFCaptured_ArcMain_BFCaptured_Small, {  0,  26,  46,  26}, { 30,  30}}, //4 idle 5
	{BFCaptured_ArcMain_BFCaptured_Small, { 46,  26,  46,  26}, { 30,  30}}, //5 idle 6
	{BFCaptured_ArcMain_BFCaptured_Small, { 92,  26,  46,  26}, { 30,  30}}, //6 idle 7

	{BFCaptured_ArcMain_BFCaptured_Small, {138,  26,  46,  26}, { 30,  30}}, //7 left 1
	{BFCaptured_ArcMain_BFCaptured_Small, {  0,  52,  46,  26}, { 30,  30}}, //8 left 2

	{BFCaptured_ArcMain_BFCaptured_Small, {138,  52,  46,  26}, { 30,  30}}, //9 down 1
	{BFCaptured_ArcMain_BFCaptured_Small, {  0,  78,  46,  26}, { 30,  30}}, //10 down 2

	{BFCaptured_ArcMain_BFCaptured_Small, { 46,  52,  46,  26}, { 30,  30}}, //11 up 1
	{BFCaptured_ArcMain_BFCaptured_Small, { 92,  52,  46,  26}, { 30,  30}}, //12 up 2

	{BFCaptured_ArcMain_BFCaptured_Small, { 46,  78,  46,  26}, { 30,  30}}, //13 right 1
	{BFCaptured_ArcMain_BFCaptured_Small, { 92,  78,  46,  26}, { 30,  30}}, //14 right 2

	{BFCaptured_ArcMain_BFCaptured_Small, {138,  78,  46,  26}, { 30,  30}}, //15 peace 1
	{BFCaptured_ArcMain_BFCaptured_Small, {  0, 104,  46,  26}, { 30,  30}}, //16 peace 2
	{BFCaptured_ArcMain_BFCaptured_Small, { 46, 104,  46,  26}, { 30,  30}}, //17 peace 3
	{BFCaptured_ArcMain_BFCaptured_Small, { 92, 104,  46,  26}, { 30,  30}}, //18 peace 4

	//bfcaptured normal
	{BFCaptured_ArcMain_BFCaptured, {  0,   0,  46,  26}, { 30,  30}}, //19 idle 1
	{BFCaptured_ArcMain_BFCaptured, { 46,   0,  46,  26}, { 30,  30}}, //20 idle 2
	{BFCaptured_ArcMain_BFCaptured, { 92,   0,  46,  26}, { 30,  30}}, //21 idle 3
	{BFCaptured_ArcMain_BFCaptured, {138,   0,  46,  26}, { 30,  30}}, //22 idle 4
	{BFCaptured_ArcMain_BFCaptured, {  0,  26,  46,  26}, { 30,  30}}, //23 idle 5
	{BFCaptured_ArcMain_BFCaptured, { 46,  26,  46,  26}, { 30,  30}}, //24 idle 6
	{BFCaptured_ArcMain_BFCaptured, { 92,  26,  46,  26}, { 30,  30}}, //25 idle 7

	{BFCaptured_ArcMain_BFCaptured, {138,  26,  46,  26}, { 30,  30}}, //26 left 1
	{BFCaptured_ArcMain_BFCaptured, {  0,  52,  46,  26}, { 30,  30}}, //27 left 2

	{BFCaptured_ArcMain_BFCaptured, {138,  52,  46,  26}, { 30,  30}}, //28 down 1
	{BFCaptured_ArcMain_BFCaptured, {  0,  78,  46,  26}, { 30,  30}}, //29 down 2

	{BFCaptured_ArcMain_BFCaptured, { 46,  52,  46,  26}, { 30,  30}}, //30 up 1
	{BFCaptured_ArcMain_BFCaptured, { 92,  52,  46,  26}, { 30,  30}}, //31 up 2

	{BFCaptured_ArcMain_BFCaptured, { 46,  78,  46,  26}, { 30,  30}}, //32 right 1
	{BFCaptured_ArcMain_BFCaptured, { 92,  78,  46,  26}, { 30,  30}}, //33 right 2

	{BFCaptured_ArcMain_BFCaptured, {138,  78,  46,  26}, { 30,  30}}, //34 peace 1
	{BFCaptured_ArcMain_BFCaptured, {  0, 104,  46,  26}, { 30,  30}}, //35 peace 2
	{BFCaptured_ArcMain_BFCaptured, { 46, 104,  46,  26}, { 30,  30}}, //36 peace 3
	{BFCaptured_ArcMain_BFCaptured, { 92, 104,  46,  26}, { 30,  30}}, //37 peace 4

	//bfcaptured fire
	{BFCaptured_ArcMain_BFCaptured_Fire, {  0,   0,  46,  26}, { 30,  30}}, //38 idle 1
	{BFCaptured_ArcMain_BFCaptured_Fire, { 46,   0,  46,  26}, { 30,  30}}, //39 idle 2
	{BFCaptured_ArcMain_BFCaptured_Fire, { 92,   0,  46,  26}, { 30,  30}}, //40 idle 3
	{BFCaptured_ArcMain_BFCaptured_Fire, {138,   0,  46,  26}, { 30,  30}}, //41 idle 4
	{BFCaptured_ArcMain_BFCaptured_Fire, {  0,  26,  46,  26}, { 30,  30}}, //42 idle 5
	{BFCaptured_ArcMain_BFCaptured_Fire, { 46,  26,  46,  26}, { 30,  30}}, //43 idle 6
	{BFCaptured_ArcMain_BFCaptured_Fire, { 92,  26,  46,  26}, { 30,  30}}, //44 idle 7

	{BFCaptured_ArcMain_BFCaptured_Fire, {138,  26,  46,  26}, { 30,  30}}, //45 left 1
	{BFCaptured_ArcMain_BFCaptured_Fire, {  0,  52,  46,  26}, { 30,  30}}, //46 left 2

	{BFCaptured_ArcMain_BFCaptured_Fire, {138,  52,  46,  26}, { 30,  30}}, //47 down 1
	{BFCaptured_ArcMain_BFCaptured_Fire, {  0,  78,  46,  26}, { 30,  30}}, //48 down 2

	{BFCaptured_ArcMain_BFCaptured_Fire, { 46,  52,  46,  26}, { 30,  30}}, //49 up 1
	{BFCaptured_ArcMain_BFCaptured_Fire, { 92,  52,  46,  26}, { 30,  30}}, //50 up 2

	{BFCaptured_ArcMain_BFCaptured_Fire, { 46,  78,  46,  26}, { 30,  30}}, //51 right 1
	{BFCaptured_ArcMain_BFCaptured_Fire, { 92,  78,  46,  26}, { 30,  30}}, //52 right 2

	{BFCaptured_ArcMain_BFCaptured_Fire, {138,  78,  46,  26}, { 30,  30}}, //53 peace 1
	{BFCaptured_ArcMain_BFCaptured_Fire, {  0, 104,  46,  26}, { 30,  30}}, //54 peace 2
	{BFCaptured_ArcMain_BFCaptured_Fire, { 46, 104,  46,  26}, { 30,  30}}, //55 peace 3
	{BFCaptured_ArcMain_BFCaptured_Fire, { 92, 104,  46,  26}, { 30,  30}}, //56 peace 4
};

static const Animation char_bfcaptured_small_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5,  6, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 7,  8, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 9, 10, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){11, 12, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){13, 14, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{2, (const u8[]){15, 16, 17, 18, ASCR_BACK, 1}},      //CharAnim_Special
};
static const Animation char_bfcaptured_anim[CharAnim_Max] = {
	{2, (const u8[]){19, 20, 21, 22, 23, 24, 25, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){26, 27, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){28, 29, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){30, 31, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){32, 33, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{2, (const u8[]){34, 35, 36, 37, ASCR_BACK, 1}},     //CharAnim_Special
};
static const Animation char_bfcaptured_fire_anim[CharAnim_Max] = {
	{2, (const u8[]){38, 39, 40, 41, 42, 43, 44, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){45, 46, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){47, 48, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){49, 50, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){51, 52, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	{2, (const u8[]){53, 54, 55, 56, ASCR_BACK, 1}},		 //CharAnim_Special
};


//Boyfriend player functions
void Char_BFCaptured_SetFrame(void *user, u8 frame)
{
	Char_BFCaptured *this = (Char_BFCaptured*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfcaptured_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFCaptured_Tick(Character *character)
{
	Char_BFCaptured *this = (Char_BFCaptured*)character;

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
			if(stage.song_step == 1089)
				character->set_anim(character, CharAnim_Special); //peace
		}
	}
	
	//Animate and draw character
	switch (character->powerup -1)
	{
	case 2:
	Animatable_Animate(&character->animatable3, (void*)this, Char_BFCaptured_SetFrame);
	break;
	case 1:
	Animatable_Animate(&character->animatable2, (void*)this, Char_BFCaptured_SetFrame);
	break;
	case 0:
	Animatable_Animate(&character->animatable, (void*)this, Char_BFCaptured_SetFrame);
	break;
	default:
	Animatable_Animate(&character->animatable, (void*)this, Char_BFCaptured_SetFrame);
	break;
	}

	Character_Draw(character, &this->tex, &char_bfcaptured_frame[this->frame]);
}

void Char_BFCaptured_SetAnim(Character *character, u8 anim)
{
	Char_BFCaptured *this = (Char_BFCaptured*)character;
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatable2, anim);
	Animatable_SetAnim(&character->animatable3, anim);
	Character_CheckStartSing(character);
}

void Char_BFCaptured_Free(Character *character)
{
	Char_BFCaptured *this = (Char_BFCaptured*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_BFCaptured_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BFCaptured *this = Mem_Alloc(sizeof(Char_BFCaptured));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFCaptured_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFCaptured_Tick;
	this->character.set_anim = Char_BFCaptured_SetAnim;
	this->character.free = Char_BFCaptured_Free;
	
	Animatable_Init(&this->character.animatable, char_bfcaptured_small_anim);
	Animatable_Init(&this->character.animatable2, char_bfcaptured_anim);
	Animatable_Init(&this->character.animatable3, char_bfcaptured_fire_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFCAPT.ARC;1");
	
	const char **pathp = (const char *[]){
		"bf-small.tim",   //BFCaptured_ArcMain_BFCaptured-Small
		"bf.tim",   //BFCaptured_ArcMain_BFCaptured
		"bf-fire.tim",   //BFCaptured_ArcMain_BFCaptured-Fire
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
