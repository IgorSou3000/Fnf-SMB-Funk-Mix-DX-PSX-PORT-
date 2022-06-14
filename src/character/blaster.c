/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "blaster.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

#include "../stage/world2/world2_3.h"

//Blaster character structure
enum
{
	Blaster_ArcMain_Main,
	
	Blaster_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Blaster_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;

	u16 *bullets;
} Char_Blaster;

Char_Blaster ass;

//Blaster character definitions
static const CharFrame char_blaster_frame[] = {
	{Blaster_ArcMain_Main, {  0,   0,  21,  23}, {  0, 0}}, //0 idle 1
	{Blaster_ArcMain_Main, { 24,   0,  21,  23}, {  0, 0}}, //1 idle 2
	{Blaster_ArcMain_Main, { 48,   0,  20,  24}, { -1, 1}}, //2 idle 3

	{Blaster_ArcMain_Main, {  0,  27,  24,  24}, {  2,-1}}, //3 fire 0
	{Blaster_ArcMain_Main, { 27,  27,  23,  24}, {  1,-1}}, //4 fire 1
	{Blaster_ArcMain_Main, { 53,  27,  20,  24}, { -2, 1}}, //5 fire 2
};

static const Animation char_blaster_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, ASCR_BACK, 1}},         //CharAnim_Idle
	{2, (const u8[]){ 3, 4, 5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Blaster character functions
void Char_Blaster_SetFrame(void *user, u8 frame)
{
	Char_Blaster *this = (Char_Blaster*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_blaster_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Blaster_Tick(Character *character)
{
	Char_Blaster *this = (Char_Blaster*)character;


	//Initialize Pico test
	this->bullets = ((Back_World2_3*)stage.back)->bullet_chart;
	

	if (stage.note_scroll >= 0)
	{
		//Scroll through Pico chart
		u16 substep = stage.note_scroll >> FIXED_SHIFT;
		while (substep >= ((*this->bullets) & 0x7FFF))
		{
			//Play animation and bump speakers
			character->set_anim(character, ((*this->bullets) & 0x8000) ? CharAnim_Left : CharAnim_Left);
			this->bullets++;
		}
	}
	else
	{
		//Perform idle dance
		if (stage.song_step & 0x7)
			Character_PerformIdle(character);
	}




	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Blaster_SetFrame);
	Character_Draw(character, &this->tex, &char_blaster_frame[this->frame]);
}

void Char_Blaster_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_Blaster_Free(Character *character)
{
	Char_Blaster *this = (Char_Blaster*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BlasterBro_New(fixed_t x, fixed_t y)
{
	//Allocate blaster object
	Char_Blaster *this = Mem_Alloc(sizeof(Char_Blaster));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Blaster_New] Failed to allocate blaster object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Blaster_Tick;
	this->character.set_anim = Char_Blaster_SetAnim;
	this->character.free = Char_Blaster_Free;
	
	Animatable_Init(&this->character.animatable, char_blaster_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 15;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BLAST.ARC;1");
	
	const char **pathp = (const char *[]){
		"blaster.tim", //Blaster_ArcMain_Idle0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;

	this->bullets = ((Back_World2_3*)stage.back)->bullet_chart;
	
	return (Character*)this;
}