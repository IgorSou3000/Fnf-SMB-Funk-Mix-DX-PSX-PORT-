/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "luigi2p.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Luigi 2 Player Game character structure
enum
{
	Luigi2P_ArcMain_Luigi,
	
	Luigi2P_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Luigi2P_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Luigi2P;

//Luigi 2 Player Game character definitions
static const CharFrame char_luigi_frame[] = {
	{Luigi2P_ArcMain_Luigi, {  2,   7,  22,  29}, {17, 29}}, //0 idle 1
	{Luigi2P_ArcMain_Luigi, { 27,   5,  20,  31}, {16, 31}}, //1 idle 2
	{Luigi2P_ArcMain_Luigi, { 52,   4,  22,  32}, {18, 32}}, //2 idle 3
	{Luigi2P_ArcMain_Luigi, { 77,   7,  22,  29}, {17, 29}}, //3 idle 4
	{Luigi2P_ArcMain_Luigi, {  3,  46,  20,  31}, {16, 31}}, //4 idle 5
	{Luigi2P_ArcMain_Luigi, { 27,  45,  20,  32}, {16, 32}}, //5 idle 6

	{Luigi2P_ArcMain_Luigi, { 52,  45,  22,  32}, {20, 32}}, //6 left 1
	{Luigi2P_ArcMain_Luigi, { 78,  46,  21,  31}, {19, 31}}, //7 left 2

	{Luigi2P_ArcMain_Luigi, {  2,  94,  22,  23}, {18, 23}}, //8 down 1
	{Luigi2P_ArcMain_Luigi, { 28,  94,  21,  23}, {17, 23}}, //9 down 2

	{Luigi2P_ArcMain_Luigi, { 50,  82,  21,  35}, {20, 35}}, //10  up 1
	{Luigi2P_ArcMain_Luigi, { 76,  83,  22,  34}, {20, 34}}, //11 up 2

	{Luigi2P_ArcMain_Luigi, {  3, 127,  21,  33}, {14, 33}}, //12 right 1
	{Luigi2P_ArcMain_Luigi, { 29, 128,  20,  32}, {14, 32}}, //13 right 2
};

static const Animation char_luigi_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  2,  3,  4,  5,  5, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 6,  7,  ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){12, 13, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Luigi 2 Player Game character functions
void Char_Luigi2P_SetFrame(void *user, u8 frame)
{
	Char_Luigi2P *this = (Char_Luigi2P*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_luigi_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Luigi2P_Tick(Character *character)
{
	Char_Luigi2P *this = (Char_Luigi2P*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Luigi2P_SetFrame);
	Character_Draw(character, &this->tex, &char_luigi_frame[this->frame]);
}

void Char_Luigi2P_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Luigi2P_Free(Character *character)
{
	Char_Luigi2P *this = (Char_Luigi2P*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Luigi2P_New(fixed_t x, fixed_t y)
{
	//Allocate luigi object
	Char_Luigi2P *this = Mem_Alloc(sizeof(Char_Luigi2P));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Luigi2P_New] Failed to allocate luigi object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Luigi2P_Tick;
	this->character.set_anim = Char_Luigi2P_SetAnim;
	this->character.free = Char_Luigi2P_Free;
	
	Animatable_Init(&this->character.animatable, char_luigi_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(10,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\LUIGI2P.ARC;1");

	const char **pathp = (const char *[]){
		"luigi.tim", //Luigi2P_ArcMain_Luigi
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
