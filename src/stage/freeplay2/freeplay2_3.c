/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "freeplay2_3.h"

#include "../../archive.h"
#include "../../mem.h"
#include "../../stage.h"

//Sonic stage

//Sonict background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //The stage itself

	Animatable water_animatable;

	//water state
	u8 water_frame;
} Back_Freeplay2_3;

//water animation and rects
static const CharFrame water_frame[] = {
	{0, {  0, 100,  80,  15}, {  0,   0}}, //0 water anim 1
	{0, { 82, 100,  80,  15}, {  0,   0}}, //1 water anim 2
	{0, {  0, 120,  80,  15}, {  0,   0}}, //2 water anim 3
	{0, { 82, 120,  80,  15}, {  0,   0}}, //3 water anim 4
};

static const Animation water_anim[] = {
	{3, (const u8[]){0, 1, 2, 3, ASCR_BACK, 1}}, //water
};

//water functions
void Freeplay2_3_Water_SetFrame(void *user, u8 frame)
{
	Back_Freeplay2_3 *this = (Back_Freeplay2_3*)user;
	
	//Check if this is a new frame
	if (frame != this->water_frame)
	{
		const CharFrame *cframe;
		//Check if new art shall be loaded
		cframe = &water_frame[this->water_frame = frame];
	}
}

void Freeplay2_3_Water_Draw(Back_Freeplay2_3 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &water_frame[this->water_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, (src.w*2) << FIXED_SHIFT, (src.h*2) << FIXED_SHIFT};

	Stage_DrawTex(&this->tex_back0, &src, &dst, stage.camera.bzoom);
}


//Freeplay2 background functions
void Back_Freeplay2_3_DrawBG(StageBack *back) //Cross Console Clsh
{
	Back_Freeplay2_3 *this = (Back_Freeplay2_3*)back;

	fixed_t fx, fy;
	
	fx = stage.camera.x;
	fy = stage.camera.y;

	//water stuff
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		if (Animatable_Ended(&this->water_animatable))
			Animatable_SetAnim(&this->water_animatable, 0);
	}
	Animatable_Animate(&this->water_animatable, (void*)this, Freeplay2_3_Water_SetFrame);

	Freeplay2_3_Water_Draw(this, FIXED_DEC(-160,1) - fx, FIXED_DEC(33,1) - fy);

	//Draw background
	RECT back_src = {0, 0, 160, 81};
	RECT_FIXED back_dst = {
		FIXED_DEC(-160,1) - fx,
		FIXED_DEC(-65,1) - fy,
		FIXED_DEC(back_src.w*2,1),
		FIXED_DEC(back_src.h*2,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &back_src, &back_dst, stage.camera.bzoom);
}

void Back_Freeplay2_3_Free(StageBack *back)
{
	Back_Freeplay2_3 *this = (Back_Freeplay2_3*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Freeplay2_3_New(void)
{
	//Allocate background structure
	Back_Freeplay2_3 *this = (Back_Freeplay2_3*)Mem_Alloc(sizeof(Back_Freeplay2_3));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Freeplay2_3_DrawBG;
	this->back.free = Back_Freeplay2_3_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\FREE2\\SONIC.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "bg.tim"), 0);
	Mem_Free(arc_back);

	Animatable_Init(&this->water_animatable, water_anim);
	Animatable_SetAnim(&this->water_animatable, 0);
	
	return (StageBack*)this;
}
