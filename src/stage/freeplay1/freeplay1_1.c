/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "freeplay1_1.h"

#include "../../archive.h"
#include "../../mem.h"
#include "../../stage.h"

//Sky

//sky background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_coclo; //it a abbreviation of coins cloud, yeah i am stupid
	Gfx_Tex tex_stars; //i don't need say what it is

	//coin 0 state
	u8 coin0_frame;
	
	Animatable coin0_animatable, coin1_animatable;

	//coin 1 state
	u8 coin1_frame;

	//bg anim for coin
	s16 coinmovebg;

	//bg anim for stars
	s16 starsmovebg;

} Back_Freeplay1_1;

//Coin0 animation and rects
static const CharFrame coin0_frame[] = {
	{0, {  2,   4,  32,  14}, {  0,   0}}, //0 coinanim 1
	{0, {  2,  20,  32,  14}, {  0,   0}}, //1 coinanim 2
	{0, {  2,  36,  32,  14}, {  0,   0}}, //2 coinanim 3
};

//Coin1 animation and rects
static const CharFrame coin1_frame[] = {
	{0, { 51,   2,  22,  16}, {  0,   0}}, //0 coinanim 1
	{0, { 51,  20,  22,  16}, {  0,   0}}, //1 coinanim 2
	{0, { 51,  38,  22,  16}, {  0,   0}}, //2 coinanim 3
};

static const Animation coins_anim[] = {
	{2, (const u8[]){0, 1, 2, 1, 0, ASCR_BACK, 1}}, //coinanim
};

//Coin0 functions
void Freeplay1_1_Coin0_SetFrame(void *user, u8 frame)
{
	Back_Freeplay1_1 *this = (Back_Freeplay1_1*)user;
	
	//Check if this is a new frame
	if (frame != this->coin0_frame)
	{
		const CharFrame *cframe;
		//Check if new art shall be loaded
		cframe = &coin0_frame[this->coin0_frame = frame];
		cframe = &coin1_frame[this->coin1_frame = frame];
	}
}

void Freeplay1_1_Coins_Draw(Back_Freeplay1_1 *this, fixed_t x, fixed_t y, char* coin)
{
	//Draw character
	const CharFrame *cframe;
	
	if (strcmp(coin,"coin 0") == 0)
	cframe = &coin0_frame[this->coin0_frame];
	else
	cframe = &coin1_frame[this->coin1_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, (src.w*2) << FIXED_SHIFT, (src.h*2) << FIXED_SHIFT};

	Stage_DrawTex(&this->tex_coclo, &src, &dst, stage.camera.bzoom);
}

//Freeplay2 background functions
void Back_Freeplay1_1_DrawBG(StageBack *back) //2 Player Game
{
	Back_Freeplay1_1 *this = (Back_Freeplay1_1*)back;

	fixed_t fx, fy;

	if (this->coinmovebg <= -640)
	this->coinmovebg = 0;

	if (this->starsmovebg/24 <= -320)
	this->starsmovebg = 0;

	//startin move bg
	this->coinmovebg -= 2;
	this->starsmovebg--;
	
	//Draw background
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT cloud_src = {3, 63, 48, 16};
	RECT_FIXED cloud_dst = {
	    FIXED_DEC(-131,1),
		FIXED_DEC( 19,1),
		FIXED_DEC(cloud_src.w*2 + 1,1) - fx,  
		FIXED_DEC(cloud_src.h*2 + 1,1) - fy
		};

	//cloud for luigi
	Stage_DrawTex(&this->tex_coclo, &cloud_src, &cloud_dst, stage.camera.bzoom);

	//cloud for bf
	cloud_dst.x = FIXED_DEC(25,1);
	cloud_dst.y = FIXED_DEC(54,1);

	Stage_DrawTex(&this->tex_coclo, &cloud_src, &cloud_dst, stage.camera.bzoom);

	//coin0 and coin1
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		if ((stage.song_step % 9) == 0)
		{
		Animatable_SetAnim(&this->coin0_animatable, 0);
		Animatable_SetAnim(&this->coin1_animatable, 0);
		}
	}
	Animatable_Animate(&this->coin0_animatable, (void*)this, Freeplay1_1_Coin0_SetFrame);
	Animatable_Animate(&this->coin1_animatable, (void*)this, Freeplay1_1_Coin0_SetFrame);

	for (u8 i = 0; i <= 2; i++)
	{
	Freeplay1_1_Coins_Draw(this, FIXED_DEC((-138 + (320*i)) + this->coinmovebg,1) - fx, FIXED_DEC(54,1) - fy, "coin 0");
	Freeplay1_1_Coins_Draw(this, FIXED_DEC((-12  + (320*i)) + this->coinmovebg,1) - fx, FIXED_DEC(-27,1) - fy, "coin 0");
	

	Freeplay1_1_Coins_Draw(this, FIXED_DEC((-142 + (320*i)) + this->coinmovebg/2,1) - fx, FIXED_DEC(-55,1) - fy, "coin 1");
	Freeplay1_1_Coins_Draw(this, FIXED_DEC((100  + (320*i)) + this->coinmovebg/2,1) - fx, FIXED_DEC(16,1) - fy, "coin 1");
	  }

	//draw stars bg
	RECT stars_src = {0, 0, 160, 81};
	RECT_FIXED stars_dst = {
		FIXED_DEC(-160 + this->starsmovebg/24,1) - fx,
		FIXED_DEC(-65,1) - fy,
		FIXED_DEC(stars_src.w*2 + 1,1),
		FIXED_DEC(stars_src.h*2 + 1,1)
	};
	
	Stage_DrawTex(&this->tex_stars, &stars_src, &stars_dst, stage.camera.bzoom);

	stars_dst.x += stars_dst.w - FIXED_DEC(1,1);

	Stage_DrawTex(&this->tex_stars, &stars_src, &stars_dst, stage.camera.bzoom);

	//made bg blue
	Gfx_SetClear(16, 16, 43);
}

void Back_Freeplay1_1_Free(StageBack *back)
{
	Back_Freeplay1_1 *this = (Back_Freeplay1_1*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Freeplay1_1_New(void)
{
	//Allocate background structure
	Back_Freeplay1_1 *this = (Back_Freeplay1_1*)Mem_Alloc(sizeof(Back_Freeplay1_1));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Freeplay1_1_DrawBG;
	this->back.free = Back_Freeplay1_1_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\FREE1\\SKY.ARC;1");
	Gfx_LoadTex(&this->tex_coclo, Archive_Find(arc_back, "coclo.tim"), 0);
	Gfx_LoadTex(&this->tex_stars, Archive_Find(arc_back, "stars.tim"), 0);
	Mem_Free(arc_back);

	Animatable_Init(&this->coin0_animatable, coins_anim);
	Animatable_SetAnim(&this->coin0_animatable, 0);

	Animatable_Init(&this->coin1_animatable, coins_anim);
	Animatable_SetAnim(&this->coin1_animatable, 0);
	
	return (StageBack*)this;
}
