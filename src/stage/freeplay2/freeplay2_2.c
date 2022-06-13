/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "freeplay2_2.h"

#include "../../archive.h"
#include "../../mem.h"
#include "../../stage.h"

//Ghost's house

//Ghost background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //The ship itself
} Back_Freeplay2_2;

//Freeplay2 background functions
void Back_Freeplay2_2_DrawBG(StageBack *back) //Boo blitz
{
	Back_Freeplay2_2 *this = (Back_Freeplay2_2*)back;

	fixed_t fx, fy;
	
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
	
	Stage_DrawTex(&this->tex_back0, &back_src, &back_dst, stage.camera.bzoom);
}

void Back_Freeplay2_2_Free(StageBack *back)
{
	Back_Freeplay2_2 *this = (Back_Freeplay2_2*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Freeplay2_2_New(void)
{
	//Allocate background structure
	Back_Freeplay2_2 *this = (Back_Freeplay2_2*)Mem_Alloc(sizeof(Back_Freeplay2_2));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Freeplay2_2_DrawBG;
	this->back.free = Back_Freeplay2_2_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\FREE2\\GHOST.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "bg.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
