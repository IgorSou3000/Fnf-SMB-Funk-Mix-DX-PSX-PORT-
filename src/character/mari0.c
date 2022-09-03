/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mari0.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Mari0 character structure
enum
{
	Mari0_ArcMain_Portal,
	
	Mari0_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Mari0_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Mari0;

//Mari0 character definitions
static const CharFrame char_mari0_frame[] = {
	{Mari0_ArcMain_Portal, {  0,   0,   34,  26}, {  0,  0}}, //0 idle
	{Mari0_ArcMain_Portal, { 34,   0,   34,  26}, {  0,  0}}, //1 idle
	{Mari0_ArcMain_Portal, { 68,   0,   34,  26}, {  0,  0}}, //2 idle
	{Mari0_ArcMain_Portal, {102,   0,   34,  26}, {  0,  0}}, //3 idle
	{Mari0_ArcMain_Portal, {  0,  26,   34,  26}, {  0,  0}}, //4 idle
	{Mari0_ArcMain_Portal, { 34,  26,   34,  26}, {  0,  0}}, //5 idle

	{Mari0_ArcMain_Portal, {  0,  52,   34,  26}, {  0,  0}}, //6 left
	{Mari0_ArcMain_Portal, { 34,  52,   34,  26}, {  0,  0}}, //7 left

	{Mari0_ArcMain_Portal, {  0,  78,   34,  26}, {  0,  0}}, //8 down
	{Mari0_ArcMain_Portal, { 34,  78,   34,  26}, {  0,  0}}, //9 down

	{Mari0_ArcMain_Portal, { 68,  26,   34,  26}, {  0,  0}}, //10 up
	{Mari0_ArcMain_Portal, {102,  26,   34,  26}, {  0,  0}}, //11 up

	{Mari0_ArcMain_Portal, { 68,  52,   34,  26}, {  0,  0}}, //12 right
	{Mari0_ArcMain_Portal, {102,  52,   34,  26}, {  0,  0}}, //13 right

	{Mari0_ArcMain_Portal, { 68,  78,   34,  26}, {  0,  0}}, //14 schüt
	{Mari0_ArcMain_Portal, {102,  78,   34,  26}, {  0,  0}}, //15 schüt
};

static const Animation char_mari0_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 6, 7, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 8, 9, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 10, 11, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 12, 13, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
	{2, (const u8[]){ 14, 15, ASCR_BACK, 1}},        //Shoot portal gun
};

//Mari0 character functions
void Char_Mari0_SetFrame(void *user, u8 frame)
{
	Char_Mari0 *this = (Char_Mari0*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_mari0_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Mari0_Tick(Character *character)
{
	Char_Mari0 *this = (Char_Mari0*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Mari0_SetFrame);
	Character_Draw(character, &this->tex, &char_mari0_frame[this->frame]);

	//handle portal gun shooting
	//some of these are probably off lmao
	switch (stage.song_step)
	{
		case 127:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 254:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 334:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 350:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 366:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 398:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 414:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 430:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 446:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 492:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 524:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
		case 556:
		{
			character->set_anim(character, CharAnim_Special); //Shoot
			break;
		}
	}
}

void Char_Mari0_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Mari0_Free(Character *character)
{
	Char_Mari0 *this = (Char_Mari0*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Mari0_New(fixed_t x, fixed_t y)
{
	//Allocate mari0 object
	Char_Mari0 *this = Mem_Alloc(sizeof(Char_Mari0));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Mari0_New] Failed to allocate mari0 object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Mari0_Tick;
	this->character.set_anim = Char_Mari0_SetAnim;
	this->character.free = Char_Mari0_Free;
	
	Animatable_Init(&this->character.animatable, char_mari0_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MARI0.ARC;1");
	
	const char **pathp = (const char *[]){
		"portal.tim", //Mari0_ArcMain_Small
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
