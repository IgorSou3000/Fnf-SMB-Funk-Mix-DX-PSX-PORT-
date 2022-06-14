/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "world2_3.h"

#include "../../archive.h"
#include "../../mem.h"
#include "../../stage.h"

//Bowser's airship

//World 1 background functions
void Back_World2_3_DrawBG(StageBack *back) // Mushroom Plain
{
	Back_World2_3 *this = (Back_World2_3*)back;
	
	fixed_t fx, fy;
	
	//Draw background
	fx = stage.camera.x;
	fy = stage.camera.y;
}

void Back_World2_3_DrawFG(StageBack *back) //ship
{
	Back_World2_3 *this = (Back_World2_3*)back;

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

void Back_World2_3_Free(StageBack *back)
{
	Back_World2_3 *this = (Back_World2_3*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_World2_3_New(void)
{
	//Allocate background structure
	Back_World2_3 *this = (Back_World2_3*)Mem_Alloc(sizeof(Back_World2_3));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_World2_3_DrawFG;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_World2_3_DrawBG;
	this->back.free = Back_World2_3_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WORLD2\\SHIP.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "ship.tim"), 0);
	Mem_Free(arc_back);

	//Load Bullets chart
	this->bullet_chart = (u16*)IO_Read("\\WORLD2\\BULLETS.CHT;1");
	

	Gfx_SetClear(0, 13, 15);
	
	return (StageBack*)this;
}
