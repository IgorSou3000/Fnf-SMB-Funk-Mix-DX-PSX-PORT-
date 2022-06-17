/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mx.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

MX mx;

//MX character structure
enum
{
	MX_ArcMain_Main, //Fake Mario sprites
	MX_ArcMain_Near, //Phase 2 sprites
	MX_ArcMain_Run, //Running sprites
	
	MX_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[MX_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_MX;

//MX character definitions
static const CharFrame char_mx_frame[] = {
	{MX_ArcMain_Main, {  0,   0,  16,  32}, {  0, 0}}, //0 idle

	{MX_ArcMain_Main, { 19,   0,  17,  32}, {  2, 0}}, //1 left 1
	{MX_ArcMain_Main, { 39,   0,  17,  32}, {  1, 0}}, //2 left 2

	{MX_ArcMain_Main, { 59,   0,  18,  29}, {  1,-3}}, //3 down 1
	{MX_ArcMain_Main, { 80,   0,  17,  31}, {  0,-1}}, //4 down 2

	{MX_ArcMain_Main, {100,   0,  16,  35}, {  0, 3}}, //5 up 1
	{MX_ArcMain_Main, {119,   0,  16,  33}, {  0, 1}}, //6 up 2

	{MX_ArcMain_Main, {138,   0,  18,  32}, {  0, 0}}, //7 right 1
	{MX_ArcMain_Main, {159,   0,  17,  32}, {  0, 0}}, //8 right 2

	{MX_ArcMain_Main, {179,   0,  17,  32}, {  0, 0}}, //9 trans 1
	{MX_ArcMain_Main, {199,   0,  23,  25}, {  3,-7}}, //10 trans 2
	{MX_ArcMain_Main, {225,   0,  24,  22}, {  4,-10}}, //11 trans 3
	{MX_ArcMain_Main, {  0,  38,  24,  23}, {  4,-9}}, //12 trans 4

	//Phase 2

	{MX_ArcMain_Near, {  0,   0,  22,  30}, {  1,-2}}, //13 idle
	{MX_ArcMain_Near, { 25,   0,  22,  30}, {  1,-2}}, //14 idle
	{MX_ArcMain_Near, { 50,   0,  21,  30}, {  0,-2}}, //15 idle

	{MX_ArcMain_Near, { 74,   0,  35,  30}, {  7,-2}}, //16 left
	{MX_ArcMain_Near, {112,   0,  32,  30}, {  6,-2}}, //17 left

	{MX_ArcMain_Near, {147,   0,  21,  24}, {  1,-8}}, //18 down
	{MX_ArcMain_Near, {171,   0,  20,  25}, {  1,-7}}, //19 down

	{MX_ArcMain_Near, {194,   0,  18,  34}, { -1, 2}}, //20 up
	{MX_ArcMain_Near, {215,   0,  18,  33}, { -1, 1}}, //21 up

	{MX_ArcMain_Near, {  0,  37,  24,  30}, {  0,-2}}, //22 right
	{MX_ArcMain_Near, { 27,  37,  23,  30}, {  0,-2}}, //23 right

	//haha funny laff

	{MX_ArcMain_Near, { 53,  37,  23,  23}, {  2,-9}}, //24 laugh 1 1
	{MX_ArcMain_Near, { 79,  37,  36,  34}, {  7, 2}}, //25 laugh 1 2
	{MX_ArcMain_Near, {118,  37,  37,  33}, {  7, 1}}, //26 laugh 1 3

	{MX_ArcMain_Near, {158,  37,  38,  35}, {  8, 3}}, //27 laugh 2 1
	{MX_ArcMain_Near, {199,  37,  38,  34}, {  7, 2}}, //28 laugh 2 2

	{MX_ArcMain_Near, {  0,  75,  44,  44}, {  8,12}}, //29 laugh 3 1
	{MX_ArcMain_Near, { 47,  75,  46,  41}, { 10, 9}}, //30 laugh 3 2

	//Running

	{MX_ArcMain_Run, {  0,   0,  34,  42}, {  0, 0}}, //31 run
	{MX_ArcMain_Run, { 37,   0,  24,  40}, { -2,-2}}, //32 run
	{MX_ArcMain_Run, { 64,   0,  40,  41}, {  2,-1}}, //33 run
};

//Faker animations
static const Animation char_mario_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, ASCR_BACK, 1}},         //CharAnim_Idle
	{2, (const u8[]){ 1, 2, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 3, 4, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 5, 6, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 7, 8, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
	{2, (const u8[]){9, 10, 11, 12, ASCR_BACK, 1}},	//CharAnim_Special
};

//Phase 2 animations
static const Animation char_near_anim[CharAnim_Max] = {
	{3, (const u8[]){ 15, 14, 14, 14, 15, 13, 14, ASCR_BACK, 1}},         //CharAnim_Idle
	{2, (const u8[]){ 16, 17, ASCR_BACK, 1}},         //CharAnim_Left
	{1, (const u8[]){24, 25, 25, 26, 26, ASCR_BACK, 1}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 18, 19, ASCR_BACK, 1}},         //CharAnim_Down
	{1, (const u8[]){ 27, 28, ASCR_BACK, 1}},   //CharAnim_DownAlt
	{2, (const u8[]){ 20, 21, ASCR_BACK, 1}},         //CharAnim_Up
	{1, (const u8[]){29, 30, ASCR_BACK, 1}},   //CharAnim_UpAlt
	{2, (const u8[]){ 22, 23, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
	{2, (const u8[]){24, 25, 26, 26, 27, 28, 28, 29, 30, 30, 30, 30, 30, 30, ASCR_BACK, 1}},   //CharAnim_Special(laughs)
};

//Running animations
static const Animation char_run_anim[CharAnim_Max] = {
	{2, (const u8[]){ 31, 32, 33, ASCR_REPEAT}},         //CharAnim_Idle
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Left
	{1, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Down
	{1, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Up
	{1, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_Special(jump)
};

//MX character functions
void Char_MX_SetFrame(void *user, u8 frame)
{
	Char_MX *this = (Char_MX*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_mx_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_MX_Tick(Character *character)
{
	Char_MX *this = (Char_MX*)character;
	
		//Perform idle dance
		if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
			Character_PerformIdle(character);
	
	//Animate and draw
	//use fake mario sprites when phase is 0, and use phase 2 sprites when phase is 2
	switch (mx.phase)
	{
		case 0:
		{
			Animatable_Animate(&character->animatable, (void*)this, Char_MX_SetFrame);
			break;
		}
		case 2:
		{
			Animatable_Animate(&character->animatable2, (void*)this, Char_MX_SetFrame);
			break;
		}
		case 3:
		{
			Animatable_Animate(&character->animatable3, (void*)this, Char_MX_SetFrame);
			break;
		}
	}
	Character_Draw(character, &this->tex, &char_mx_frame[this->frame]);

	//step specific stuff
	switch (stage.song_step)
	{
		case 507:
		{
			character->ignore_note = true; //Briefly disable note animations so that the transformation animation can happen during the hold note
			character->set_anim(character, CharAnim_Special); //Transformation animation
			break;
		}
		case 512:
		{
			mx.phase = 2; //start using phase 2 sprites
			character->ignore_note = false; //stop ignoring note animations
			break;
		}
		case 751:
		{
			character->set_anim(character, CharAnim_Special); //Start Laugh's
			break;
		}
	
		case 767: //Start next phase
		{
			mx.phase = 3;
			//reposition mx because the offsets are getting difficult to keep up with
			stage.opponent->x = FIXED_DEC(72,1);
			stage.opponent->y = FIXED_DEC(-18,1);
		}
		default: //do nothing otherwise lol
			break;
	}
}

void Char_MX_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatable2, anim);
	Animatable_SetAnim(&character->animatable3, anim);
	Character_CheckStartSing(character);
}

void Char_MX_Free(Character *character)
{
	Char_MX *this = (Char_MX*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_MX_New(fixed_t x, fixed_t y)
{
	//Allocate mx object
	Char_MX *this = Mem_Alloc(sizeof(Char_MX));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_MX_New] Failed to allocate mx object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_MX_Tick;
	this->character.set_anim = Char_MX_SetAnim;
	this->character.free = Char_MX_Free;
	
	Animatable_Init(&this->character.animatable, char_mario_anim);
	Animatable_Init(&this->character.animatable2, char_near_anim);
	Animatable_Init(&this->character.animatable3, char_run_anim);

	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 15; //does this even do anything in this port

	mx.phase = 0;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MX.ARC;1");
	
	const char **pathp = (const char *[]){
		"mario.tim", //MX_ArcMain_Main
		"near.tim", //MX_ArcMain_Near
		"run.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}