/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "boo.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Boo character structure
enum
{
	Boo_ArcMain_Boo0,
	Boo_ArcMain_Boo1,
	
	Boo_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Boo_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;

	boolean spook; //for alt idle
} Char_Boo;

//Boo character definitions
static const CharFrame char_boo_frame[] = {
	{Boo_ArcMain_Boo0, {  0,   0,  68,  47}, { 0, 0}}, //0 idle 1
	{Boo_ArcMain_Boo0, { 71,   0,  65,  49}, {-3, 2}}, //1 idle 2
	{Boo_ArcMain_Boo0, {139,   0,  66,  49}, {-3, 3}}, //2 idle 3

	{Boo_ArcMain_Boo0, {  0,  52,  68,  50}, { 0, 3}}, //3 spooked 1
	{Boo_ArcMain_Boo0, { 71,  52,  65,  51}, {-3, 4}}, //4 spooked 2
	{Boo_ArcMain_Boo0, {139,  52,  66,  49}, {-3, 3}}, //5 spooked 3

	{Boo_ArcMain_Boo0, {  0, 106,  67,  47}, {-1, 2}}, //6 left 1
	{Boo_ArcMain_Boo0, { 70, 106,  67,  49}, {-2, 2}}, //7 left 2
	{Boo_ArcMain_Boo0, {140, 106,  66,  51}, {-3, 1}}, //8 left 3

	{Boo_ArcMain_Boo0, {  0, 160,  65,  47}, {-2,-3}}, //9 down 1
	{Boo_ArcMain_Boo0, { 68, 160,  64,  49}, {-3,-1}}, //10 down 2
	{Boo_ArcMain_Boo0, {135, 160,  64,  49}, {-3, 0}}, //11 down 3

	{Boo_ArcMain_Boo1, {  0,   0,  66,  51}, {-4,  5}}, //12  up 1
	{Boo_ArcMain_Boo1, { 69,   0,  65,  49}, {-5, 4}}, //13 up 2
	{Boo_ArcMain_Boo1, {137,   0,  65,  49}, {-5, 5}}, //14 up 3

	{Boo_ArcMain_Boo1, {  0,  54,  71,  47}, {-3, 1}}, //15 right 1
	{Boo_ArcMain_Boo1, { 74,  54,  68,  50}, {-5, 3}}, //16 right 2
	{Boo_ArcMain_Boo1, {145,  54,  69,  51}, {-4, 4}}, //17 right 3
};

static const Animation char_boo_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 6, 7, 8, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 9, 10, 11, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 12, 13, 14, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){15, 16, 17, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
	{2, (const u8[]){ 3, 4, 5, ASCR_BACK, 1}},   //Special
};

//Boo character functions
void Char_Boo_SetFrame(void *user, u8 frame)
{
	Char_Boo *this = (Char_Boo*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_boo_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Boo_Tick(Character *character)
{
	Char_Boo *this = (Char_Boo*)character;
	
	//Perform idle dance
	//if spook is zero, do the normal one
	//otherwise, do the spooked idle
	if ((Animatable_Ended(&character->animatable)) && (stage.song_step & 0x7) == 0)
		{
			if (this->spook == 0)
				character->set_anim(character, CharAnim_Idle);
			else
				character->set_anim(character, CharAnim_Special);
		}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Boo_SetFrame);
	Character_Draw(character, &this->tex, &char_boo_frame[this->frame]);

	//Spook stuff
	switch (stage.song_step)
	{
		case 192:
			this->spook = 1;
			break;
		case 1022:
			this->spook = 0;
			break;
	}
}

void Char_Boo_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Boo_Free(Character *character)
{
	Char_Boo *this = (Char_Boo*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Boo_New(fixed_t x, fixed_t y)
{
	//Allocate boo object
	Char_Boo *this = Mem_Alloc(sizeof(Char_Boo));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Boo_New] Failed to allocate boo object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Boo_Tick;
	this->character.set_anim = Char_Boo_SetAnim;
	this->character.free = Char_Boo_Free;
	
	Animatable_Init(&this->character.animatable, char_boo_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(10,1);
	this->character.focus_zoom = FIXED_DEC(1,1);

	this->spook = 0;
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BOO.ARC;1");

	const char **pathp = (const char *[]){
		"boo0.tim", //Boo_ArcMain_Idle0
		"boo1.tim", //Boo_ArcMain_Idle1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
