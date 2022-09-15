/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "freeplay1_3.h"

#include "../../archive.h"
#include "../../mem.h"
#include "../../stage.h"

//Portal test chamber

//World F-3 structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back; //Chamber puzzle and wall

} Back_Freeplay1_3;

//World 2-2 background functions
void Back_Freeplay1_3_DrawBG(StageBack *back)
{
	Back_Freeplay1_3 *this = (Back_Freeplay1_3*)back;
	
	fixed_t fx, fy;
	
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	//Draw puzzle
	RECT chamber_src = {0, 0, 160, 81};
	RECT_FIXED chamber_dst = {
		FIXED_DEC(-160,1) - fx,
		FIXED_DEC(-65,1) - fy,
		FIXED_DEC(chamber_src.w*2,1),
		FIXED_DEC(chamber_src.h*2,1)
	};
	
	Stage_DrawTex(&this->tex_back, &chamber_src, &chamber_dst, stage.camera.bzoom);

	//Draw wall

	RECT wall_src = {0, 84, 160, 81};
	RECT_FIXED wall_dst = {
		FIXED_DEC(-160,1) - fx,
		FIXED_DEC(-65,1) - fy,
		FIXED_DEC(wall_src.w*2,1),
		FIXED_DEC(wall_src.h*2,1)
	};
	
	Stage_DrawTex(&this->tex_back, &wall_src, &wall_dst, stage.camera.bzoom);
}

void Back_Freeplay1_3_Free(StageBack *back)
{
	Back_Freeplay1_3 *this = (Back_Freeplay1_3*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Freeplay1_3_New(void)
{
	//Allocate background structure
	Back_Freeplay1_3 *this = (Back_Freeplay1_3*)Mem_Alloc(sizeof(Back_Freeplay1_3));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Freeplay1_3_DrawBG;
	this->back.free = Back_Freeplay1_3_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\FREE1\\PORTAL.ARC;1");
	Gfx_LoadTex(&this->tex_back, Archive_Find(arc_back, "bg.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
