/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mario.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Mario character structure
enum
{
	Mario_ArcMain_Small,
	Mario_ArcMain_Normal,
	Mario_ArcMain_Fire,
	
	Mario_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Mario_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Mario;

//Mario character definitions
static const CharFrame char_mario_frame[] = {
	{Mario_ArcMain_Small, {  2,   5,  17, 15}, {  5, 15}}, //0 idle 1
	{Mario_ArcMain_Small, { 24,   4,  14, 16}, {  4, 16}}, //1 idle 2
	{Mario_ArcMain_Small, { 45,   4,  14, 16}, {  4, 16}}, //2 idle 3

	{Mario_ArcMain_Small, { 62,   4,  17, 16}, {  6, 16}}, //3 left 1
	{Mario_ArcMain_Small, {  2,  24,  16, 16}, {  5, 16}}, //4 left 2

	{Mario_ArcMain_Small, { 22,  28,  18, 12}, {  4, 12}}, //5 down 1
	{Mario_ArcMain_Small, { 42,  28,  17, 12}, {  4, 12}}, //6 down 2

	{Mario_ArcMain_Small, { 63,  21,  14, 19}, {  4, 23}}, //7 up 1
	{Mario_ArcMain_Small, {  2,  43,  16, 17}, {  5, 21}}, //8 up 2

	{Mario_ArcMain_Small, { 20,  44,  21, 16}, {  6, 16}}, //9 right 1
	{Mario_ArcMain_Small, { 44,  44,  19, 16}, {  5, 16}}, //10 right 2
};

static const Animation char_mario_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  2,  2, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 3,  4, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 5,  6, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 7,  8, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 9, 10, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Mario character functions
void Char_Mario_SetFrame(void *user, u8 frame)
{
	Char_Mario *this = (Char_Mario*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_mario_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Mario_Tick(Character *character)
{
	Char_Mario *this = (Char_Mario*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Mario_SetFrame);
	Character_Draw(character, &this->tex, &char_mario_frame[this->frame]);
}

void Char_Mario_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Mario_Free(Character *character)
{
	Char_Mario *this = (Char_Mario*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Mario_New(fixed_t x, fixed_t y)
{
	//Allocate mario object
	Char_Mario *this = Mem_Alloc(sizeof(Char_Mario));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Mario_New] Failed to allocate mario object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Mario_Tick;
	this->character.set_anim = Char_Mario_SetAnim;
	this->character.free = Char_Mario_Free;
	
	Animatable_Init(&this->character.animatable, char_mario_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MARIO.ARC;1");
	
	const char **pathp = (const char *[]){
		"mario-s.tim", //Mario_ArcMain_Small
		"mario.tim", //Mario_ArcMain_Normal
		"mario-f.tim",  //Mario_ArcMain_Fire
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
