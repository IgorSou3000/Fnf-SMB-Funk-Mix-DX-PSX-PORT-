/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bomb.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Bomb character structure
enum
{
	Bomb_ArcMain_Normal,
	Bomb_ArcMain_Mad,
	
	Bomb_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Bomb_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Bomb;

//Bomb character definitions
static const CharFrame char_bomb_frame[] = {
	//bomb normal
	{Bomb_ArcMain_Normal, {  4,   7,  18,  18}, { 16, 18}}, //0 idle 1
	{Bomb_ArcMain_Normal, { 29,   4,  17,  21}, { 16, 21}}, //1 idle 2
	{Bomb_ArcMain_Normal, { 55,   1,  16,  24}, { 16, 24}}, //2 idle 3
	{Bomb_ArcMain_Normal, { 79,   3,  16,  22}, { 16, 22}}, //3 idle 4

	{Bomb_ArcMain_Normal, {  5,  28,  17,  22}, { 17, 22}}, //4 left 1
	{Bomb_ArcMain_Normal, { 30,  27,  16,  23}, { 16, 23}}, //5 left 2

	{Bomb_ArcMain_Normal, { 54,  28,  19,  22}, { 16, 22}}, //6 down 1
	{Bomb_ArcMain_Normal, { 79,  27,  17,  23}, { 15, 23}}, //7 down 2

	{Bomb_ArcMain_Normal, {  4,  51,  16,  24}, { 16, 24}}, //8 up 1
	{Bomb_ArcMain_Normal, { 29,  51,  16,  24}, { 16, 24}}, //9 up 2

	{Bomb_ArcMain_Normal, { 54,  55,  16,  20}, { 14, 20}}, //10 right 1
	{Bomb_ArcMain_Normal, { 79,  52,  17,  23}, { 15, 23}}, //11 right 2


	//bomb mad
	{Bomb_ArcMain_Mad, {  4,   3,  18,  22}, { 16, 22}}, //12 idle 1
	{Bomb_ArcMain_Mad, { 29,   3,  17,  22}, { 16, 22}}, //13 idle 2
	{Bomb_ArcMain_Mad, { 55,   3,  18,  22}, { 16, 22}}, //14 idle 3
	{Bomb_ArcMain_Mad, { 79,   4,  16,  21}, { 16, 21}}, //15 idle 4

	{Bomb_ArcMain_Mad, {  5,  27,  17,  23}, { 17, 23}}, //16 left 1
	{Bomb_ArcMain_Mad, { 30,  27,  16,  23}, { 16, 23}}, //17 left 2

	{Bomb_ArcMain_Mad, { 54,  28,  19,  22}, { 16, 22}}, //18 down 1
	{Bomb_ArcMain_Mad, { 79,  28,  17,  22}, { 15, 22}}, //19 down 2

	{Bomb_ArcMain_Mad, {  4,  52,  16,  23}, { 16, 23}}, //20 up 1
	{Bomb_ArcMain_Mad, { 29,  52,  16,  23}, { 16, 23}}, //21 up 2

	{Bomb_ArcMain_Mad, { 54,  53,  16,  22}, { 14, 22}}, //22 right 1
	{Bomb_ArcMain_Mad, { 79,  53,  17,  22}, { 15, 22}}, //23 right 2

	{Bomb_ArcMain_Mad, {  1,  76,  32,  31}, { 23, 27}}, //24 explosion 1
	{Bomb_ArcMain_Mad, { 39,  76,  32,  31}, { 23, 27}}, //25 explosion 2
};

//bomb
static const Animation char_bomb_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  3, ASCR_BACK, 1}},         //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
	{2, (const u8[]){24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, ASCR_BACK, 1}},    //CharAnim_Special
};
//bomb mad
static const Animation char_bomb_m_anim[CharAnim_Max] = {
	{2, (const u8[]){12, 13, 14, 15, 15, ASCR_BACK, 1}},         //CharAnim_Idle
	{2, (const u8[]){16, 17, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){18, 19, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){20, 21, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){22, 23, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
	{2, (const u8[]){24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, 24, 25, ASCR_BACK, 1}},    //CharAnim_Special
};

//Bomb character functions
void Char_Bomb_SetFrame(void *user, u8 frame)
{
	Char_Bomb *this = (Char_Bomb*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bomb_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Bomb_Tick(Character *character)
{
	Char_Bomb *this = (Char_Bomb*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	
	//event stuff
	//explosion
	if (stage.song_step == 793)
	character->set_anim(character, CharAnim_Special);


	//bomb mad
	if ((stage.song_step >= 384 && stage.mode == StageMode_Normal) || (character->powerup -1 == 0 && stage.mode != StageMode_Normal))
	Animatable_Animate(&character->animatable2, (void*)this, Char_Bomb_SetFrame);

	//normal bomb
	else
	Animatable_Animate(&character->animatable, (void*)this, Char_Bomb_SetFrame);

	//stop drawing after explosion
	if (stage.song_step <= 808)
	Character_Draw(character, &this->tex, &char_bomb_frame[this->frame]);
}

void Char_Bomb_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatable2, anim);
	Character_CheckStartSing(character);
}

void Char_Bomb_Free(Character *character)
{
	Char_Bomb *this = (Char_Bomb*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Bomb_New(fixed_t x, fixed_t y)
{
	//Allocate bomb object
	Char_Bomb *this = Mem_Alloc(sizeof(Char_Bomb));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Bomb_New] Failed to allocate bomb object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Bomb_Tick;
	this->character.set_anim = Char_Bomb_SetAnim;
	this->character.free = Char_Bomb_Free;
	
	Animatable_Init(&this->character.animatable, char_bomb_anim);
	Animatable_Init(&this->character.animatable2, char_bomb_m_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 15;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BOMB.ARC;1");
	
	const char **pathp = (const char *[]){
		"bomb.tim", //Bomb_ArcMain_Normal
		"bomb-m.tim", //Bomb_ArcMain_Mad
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}