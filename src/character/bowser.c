/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bowser.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Bowser character structure
enum
{
	Bowser_ArcMain_Main,
	
	Bowser_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Bowser_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Bowser;

//Bowser character definitions
static const CharFrame char_bowser_frame[] = {
	{Bowser_ArcMain_Main, {  0,   0,  35,  29}, {  0, 0}}, //0 idle 1
	{Bowser_ArcMain_Main, { 38,   0,  35,  31}, {  0, 2}}, //1 idle 2
	{Bowser_ArcMain_Main, { 76,   0,  35,  32}, {  0, 3}}, //2 idle 3
	{Bowser_ArcMain_Main, {114,   0,  35,  32}, {  1, 3}}, //3 idle 4

	{Bowser_ArcMain_Main, {152,   0,  27,  33}, {  5, 4}}, //4 left 1
	{Bowser_ArcMain_Main, {182,   0,  28,  33}, {  4, 4}}, //5 left 2
	{Bowser_ArcMain_Main, {213,   0,  28,  33}, {  4, 4}}, //6 left 3

	{Bowser_ArcMain_Main, {  0,  36,  39,  22}, {  3,-7}}, //7 down 1
	{Bowser_ArcMain_Main, { 42,  36,  38,  24}, {  3,-5}}, //8 down 2
	{Bowser_ArcMain_Main, { 83,  36,  36,  25}, {  2,-4}}, //9 down 3

	{Bowser_ArcMain_Main, {122,  36,  29,  36}, {  2, 7}}, //10 up 1
	{Bowser_ArcMain_Main, {154,  36,  30,  34}, {  2, 5}}, //11 up 2
	{Bowser_ArcMain_Main, {187,  36,  30,  34}, {  2, 5}}, //12 up 3
	{Bowser_ArcMain_Main, {220,  36,  30,  34}, {  2, 5}}, //13 up 4
	{Bowser_ArcMain_Main, {  0,  75,  30,  34}, {  2, 5}}, //14 up 5
	{Bowser_ArcMain_Main, { 33,  75,  30,  34}, {  2, 5}}, //15 up 6

	{Bowser_ArcMain_Main, { 66,  75,  35,  31}, { -2, 2}}, //16 right 1
	{Bowser_ArcMain_Main, {104,  75,  34,  31}, { -1, 2}}, //17 right 2
	{Bowser_ArcMain_Main, {141,  75,  33,  32}, { -1, 3}}, //18 right 3
};

static const Animation char_bowser_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, ASCR_BACK, 1}},         //CharAnim_Idle
	{2, (const u8[]){ 4, 5, 6, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 7, 8, 9, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){10, 11, 12, 13, 14, 15, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){16, 17, 18, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Bowser character functions
void Char_Bowser_SetFrame(void *user, u8 frame)
{
	Char_Bowser *this = (Char_Bowser*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bowser_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Bowser_Tick(Character *character)
{
	Char_Bowser *this = (Char_Bowser*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Bowser_SetFrame);
	Character_Draw(character, &this->tex, &char_bowser_frame[this->frame]);
}

void Char_Bowser_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Bowser_Free(Character *character)
{
	Char_Bowser *this = (Char_Bowser*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Bowser_New(fixed_t x, fixed_t y)
{
	//Allocate bowser object
	Char_Bowser *this = Mem_Alloc(sizeof(Char_Bowser));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Bowser_New] Failed to allocate bowser object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Bowser_Tick;
	this->character.set_anim = Char_Bowser_SetAnim;
	this->character.free = Char_Bowser_Free;
	
	Animatable_Init(&this->character.animatable, char_bowser_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 15;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\KOOPA.ARC;1");
	
	const char **pathp = (const char *[]){
		"bowser.tim", //Bowser_ArcMain_Idle0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}