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

//Airship background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //The ship itself
	Gfx_Tex tex_cloud0; //cloud 0
	Gfx_Tex tex_cloud1; //cloud 1

	//variable for clouds moviment
	s16 cloudsmove;

} Back_World2_3;

//World 1 background functions
void Back_World2_3_DrawBG(StageBack *back)
{
	Back_World2_3 *this = (Back_World2_3*)back;
	
	fixed_t fx, fy;
	
	fx = stage.camera.x;
	fy = stage.camera.y;

	//increase this for cloud move
	this->cloudsmove--;

	if (this->cloudsmove <= -895)
	this->cloudsmove = 0;

	//Draw Clouds
	RECT cloud_src = {0, 0, 224, 81};
	RECT_FIXED cloud_dst = {
		FIXED_DEC(-160 + this->cloudsmove,1) - fx,
		FIXED_DEC(-65,1) - fy,
		FIXED_DEC(cloud_src.w*2,1),
		FIXED_DEC(cloud_src.h*2,1)
	};

	//i dunno what put in variable of for
	for (u8 dunno = 0; dunno < 3; dunno++)
	{
	//increase dst.x
	if (dunno != 0)
	cloud_dst.x += cloud_dst.w - FIXED_DEC(1,1);

	//if be a even number use cloud0,else use cloud 1
	if ((dunno & 1) == 0)
	Stage_DrawTex(&this->tex_cloud0, &cloud_src, &cloud_dst, stage.camera.bzoom);
	else
	Stage_DrawTex(&this->tex_cloud1, &cloud_src, &cloud_dst, stage.camera.bzoom);
}

	FntPrint("cloud x %d", this->cloudsmove);
}

void Back_World2_3_DrawFG(StageBack *back)
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

	Gfx_SetClear(0, 13, 15);
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
	Gfx_LoadTex(&this->tex_cloud0, Archive_Find(arc_back, "cloud0.tim"), 0);
	Gfx_LoadTex(&this->tex_cloud1, Archive_Find(arc_back, "cloud1.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
