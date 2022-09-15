/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "world2_2.h"

#include "../../archive.h"
#include "../../mem.h"
#include "../../stage.h"

//Athletic

//World 2-2 structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back; //The stage

} Back_World2_2;

//World 2-2 background functions
void Back_World2_2_DrawBG(StageBack *back)
{
	Back_World2_2 *this = (Back_World2_2*)back;
	
	fixed_t fx, fy;
	
	fx = stage.camera.x;
	fy = stage.camera.y;

	//Draw background
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT back_src = {0, 0, 160, 81};
	RECT_FIXED back_dst = {
		FIXED_DEC(-160,1) - fx,
		FIXED_DEC(-65,1) - fy,
		FIXED_DEC(back_src.w*2,1),
		FIXED_DEC(back_src.h*2,1)
	};
	
	Stage_DrawTex(&this->tex_back, &back_src, &back_dst, stage.camera.bzoom);

	//Draw Clud
	RECT clud_src = {0, 114, 32, 24}; //shoutout to UNSTOPP4BLE for pointing out that the src.y isn't 144 lmao
	RECT_FIXED clud_dst = {
		FIXED_DEC(-84,1),
		FIXED_DEC(-62,1),
		FIXED_DEC(64,1),
		FIXED_DEC(48,1)
	};

	Stage_DrawTex(&this->tex_back, &clud_src, &clud_dst, stage.camera.bzoom);

	//blue bg
	Gfx_SetClear(148, 148, 255);
}

void Back_World2_2_Free(StageBack *back)
{
	Back_World2_2 *this = (Back_World2_2*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_World2_2_New(void)
{
	//Allocate background structure
	Back_World2_2 *this = (Back_World2_2*)Mem_Alloc(sizeof(Back_World2_2));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_World2_2_DrawBG;
	this->back.free = Back_World2_2_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WORLD2\\ATHLETIC.ARC;1");
	Gfx_LoadTex(&this->tex_back, Archive_Find(arc_back, "back.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
