/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "freeplay1_2.h"

#include "../../archive.h"
#include "../../mem.h"
#include "../../stage.h"

//Wrecking

//Wrecking background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_bg; //background and bomb
	u8 bomb1anim, bomb2anim;

} Back_Freeplay1_2;

//bomb stuff
static void Bomb(Back_Freeplay1_2 *this, boolean explosion, u8 *anim, s16 x)
{
	fixed_t fx, fy;

	fx = stage.camera.x;
	fy = stage.camera.y;

	//le code au chocolate
	if (explosion && *anim <= 26)
	(*anim)++;
	
	//is that whitty's reference!??!!!?
	RECT bomb_src = {0 + (*anim/5)*25, 88, 25, 25};  
	RECT_FIXED bomb_dst = {
		FIXED_DEC(x, 1) - fx,
		FIXED_DEC(-14,1) - fy,
		FIXED_DEC(bomb_src.w*2 + 1,1),
		FIXED_DEC(bomb_src.h*2 + 1,1)
	};

	FntPrint("anim %d", *anim);

	//draw bomb
	if (*anim < 26)
	Stage_DrawTex(&this->tex_bg, &bomb_src, &bomb_dst, stage.camera.bzoom);
}

//Freeplay2 background functions
void Back_Freeplay1_2_DrawBG(StageBack *back) //Destruction Dance
{
	Back_Freeplay1_2 *this = (Back_Freeplay1_2*)back;

	fixed_t fx, fy;

	fx = stage.camera.x;
	fy = stage.camera.y;

	//draw bombs

	//spike bomb
	Bomb(this, stage.song_step >= 660, &this->bomb1anim, -144);

	//bf bomb
	Bomb(this, stage.song_step >= 1098, &this->bomb2anim, 0);
	
	//Draw background
	RECT bg_src = {0, 0, 160, 81};
	RECT_FIXED bg_dst = {
	    FIXED_DEC(-160,1) - fx,
		FIXED_DEC(-65,1) - fy,
		FIXED_DEC(bg_src.w*2 + 1,1),  
		FIXED_DEC(bg_src.h*2 + 1,1)
	};

	Stage_DrawTex(&this->tex_bg, &bg_src, &bg_dst, stage.camera.bzoom);
}

void Back_Freeplay1_2_Free(StageBack *back)
{
	Back_Freeplay1_2 *this = (Back_Freeplay1_2*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Freeplay1_2_New(void)
{
	//Allocate background structure
	Back_Freeplay1_2 *this = (Back_Freeplay1_2*)Mem_Alloc(sizeof(Back_Freeplay1_2));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Freeplay1_2_DrawBG;
	this->back.free = Back_Freeplay1_2_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\FREE1\\WRECK.ARC;1");
	Gfx_LoadTex(&this->tex_bg, Archive_Find(arc_back, "bg.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
