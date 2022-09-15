/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "spiwa.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Spike and Waluigi character structure
enum
{
	SpiWa_ArcMain_Spike,
	SpiWa_ArcMain_Waluigi,
	
	SpiWa_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[SpiWa_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_SpiWa;

//Spike and Waluigi character definitions
static const CharFrame char_spiwa_frame[] = {
	{SpiWa_ArcMain_Spike, {  6,  12,  26,  28}, {11, 28}}, //0 idle 1
	{SpiWa_ArcMain_Spike, { 46,  10,  27,  30}, {12, 30}}, //1 idle 2
	{SpiWa_ArcMain_Spike, { 87,   7,  26,  33}, {11, 33}}, //2 idle 3
	{SpiWa_ArcMain_Spike, {128,   8,  25,  32}, {10, 32}}, //3 idle 4

	{SpiWa_ArcMain_Spike, {  2,  49,  34,  31}, {19, 31}}, //4 left 1
	{SpiWa_ArcMain_Spike, { 44,  49,  34,  31}, {18, 31}}, //5 left 2

	{SpiWa_ArcMain_Spike, { 85,  54,  28,  26}, {12, 26}}, //6 down 1
	{SpiWa_ArcMain_Spike, {126,  52,  28,  28}, {12, 28}}, //7 down 2

	{SpiWa_ArcMain_Spike, { 10,  84,  17,  36}, { 8, 36}}, //8  up 1
	{SpiWa_ArcMain_Spike, { 52,  87,  17,  33}, { 8, 33}}, //9  up 2

	{SpiWa_ArcMain_Spike, { 85,  88,  34,  32}, {11, 32}}, //10 right 1
	{SpiWa_ArcMain_Spike, {128,  88,  29,  32}, {11, 32}}, //11 right 2

	{SpiWa_ArcMain_Waluigi, {  9,  10,  28,  35}, {22, 35}}, //12 idle 1
	{SpiWa_ArcMain_Waluigi, { 55,   7,  27,  38}, {21, 38}}, //1 idle 2
	{SpiWa_ArcMain_Waluigi, {102,   7,  26,  38}, {20, 38}}, //2 idle 3
	{SpiWa_ArcMain_Waluigi, {145,   7,  25,  38}, {20, 38}}, //3 idle 4

	{SpiWa_ArcMain_Waluigi, {  5,  51,  35,  39}, {35, 39}}, //4 left 1
	{SpiWa_ArcMain_Waluigi, { 49,  52,  34,  38}, {34, 38}}, //5 left 2

	{SpiWa_ArcMain_Waluigi, { 98,  63,  30,  27}, {26, 27}}, //6 down 1
	{SpiWa_ArcMain_Waluigi, {143,  62,  30,  28}, {26, 28}}, //7 down 2

	{SpiWa_ArcMain_Waluigi, {  7,  92,  27,  46}, {24, 46}}, //8  up 1
	{SpiWa_ArcMain_Waluigi, { 51,  94,  27,  44}, {26, 44}}, //9  up 2

	{SpiWa_ArcMain_Waluigi, { 94, 101,  39,  37}, {23, 37}}, //10 right 1
	{SpiWa_ArcMain_Waluigi, {140, 102,  36,  36}, {23, 36}}, //11 right 2
};

static const Animation char_spike_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5,  ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

static const Animation char_waluigi_anim[CharAnim_Max] = {
	{2, (const u8[]){12, 13, 14, 15,  ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){16, 17,  ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){18, 19, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){20, 21, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){22, 23, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Spike and Waluigi character functions
void Char_SpiWa_SetFrame(void *user, u8 frame)
{
	Char_SpiWa *this = (Char_SpiWa*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_spiwa_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_SpiWa_Tick(Character *character)
{
	Char_SpiWa *this = (Char_SpiWa*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_SpiWa_SetFrame);
	Character_Draw(character, &this->tex, &char_spiwa_frame[this->frame]);
}

void Char_SpiWa_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_SpiWa_Free(Character *character)
{
	Char_SpiWa *this = (Char_SpiWa*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_SpiWa_New(fixed_t x, fixed_t y)
{
	//Allocate spiwa object
	Char_SpiWa *this = Mem_Alloc(sizeof(Char_SpiWa));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_SpiWa_New] Failed to allocate spiwa object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_SpiWa_Tick;
	this->character.set_anim = Char_SpiWa_SetAnim;
	this->character.free = Char_SpiWa_Free;
	
	Animatable_Init(&this->character.animatable, char_waluigi_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(10,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SPIWA.ARC;1");

	const char **pathp = (const char *[]){
		"spike.tim", //SpiWa_ArcMain_Spike
		"waluigi.tim", //SpiWa_ArcMain_Waluigi
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
