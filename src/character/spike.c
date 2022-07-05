/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "spike.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Spike character structure
enum
{
	Spike_ArcMain_Spike,
	
	Spike_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Spike_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Spike;

//Spike character definitions
static const CharFrame char_spike_frame[] = {
	{Spike_ArcMain_Spike, {  6,  12,  26,  28}, {11, 28}}, //0 idle 1
	{Spike_ArcMain_Spike, { 46,  10,  27,  30}, {12, 30}}, //1 idle 2
	{Spike_ArcMain_Spike, { 87,   7,  26,  33}, {11, 33}}, //2 idle 3
	{Spike_ArcMain_Spike, {128,   8,  25,  32}, {10, 32}}, //3 idle 4

	{Spike_ArcMain_Spike, {  2,  49,  34,  31}, {19, 31}}, //4 left 1
	{Spike_ArcMain_Spike, { 44,  49,  34,  31}, {18, 31}}, //5 left 2

	{Spike_ArcMain_Spike, { 85,  54,  28,  26}, {12, 26}}, //6 down 1
	{Spike_ArcMain_Spike, {126,  52,  28,  28}, {12, 28}}, //7 down 2

	{Spike_ArcMain_Spike, { 10,  84,  17,  36}, { 8, 36}}, //8  up 1
	{Spike_ArcMain_Spike, { 52,  87,  17,  33}, { 8, 33}}, //9  up 2

	{Spike_ArcMain_Spike, { 85,  88,  34,  32}, {11, 32}}, //10 right 1
	{Spike_ArcMain_Spike, {128,  88,  29,  32}, {11, 32}}, //11 right 2
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

//Spike 2 Player Game character functions
void Char_Spike_SetFrame(void *user, u8 frame)
{
	Char_Spike *this = (Char_Spike*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_spike_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Spike_Tick(Character *character)
{
	Char_Spike *this = (Char_Spike*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Spike_SetFrame);
	Character_Draw(character, &this->tex, &char_spike_frame[this->frame]);
}

void Char_Spike_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Spike_Free(Character *character)
{
	Char_Spike *this = (Char_Spike*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Spike_New(fixed_t x, fixed_t y)
{
	//Allocate spike object
	Char_Spike *this = Mem_Alloc(sizeof(Char_Spike));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Spike_New] Failed to allocate spike object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Spike_Tick;
	this->character.set_anim = Char_Spike_SetAnim;
	this->character.free = Char_Spike_Free;
	
	Animatable_Init(&this->character.animatable, char_spike_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(10,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SPIKE.ARC;1");

	const char **pathp = (const char *[]){
		"spike.tim", //Spike_ArcMain_Spike
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
