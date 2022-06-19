/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "world1_1pc.h"
#include "../../character/mx.h"

#include "../../archive.h"
#include "../../mem.h"
#include "../../stage.h"

//1-1

//World 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_goal; //Goal
	Gfx_Tex tex_level0; //loop 1 2 3
	Gfx_Tex tex_level1; //loop 4 5 6
	Gfx_Tex tex_level2; //loop 7

	Gfx_Tex tex_loogi; //Staked Luigi Head
	Gfx_Tex tex_wall; //Broken Brick Wall

	s16 movingbg;
} Back_World1_1PC;

//World 1 background functions
void Back_World1_1PC_DrawBG(StageBack *back) // 1-1
{
	Back_World1_1PC *this = (Back_World1_1PC*)back;
	
	fixed_t fx, fy;
	
	//Draw background
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	//End of level
	RECT back_src = {0, 0, 160, 81};
	RECT_FIXED back_dst = {
		FIXED_DEC(-160 + this->movingbg,1) - fx,
		FIXED_DEC(-65,1) - fy,
		FIXED_DEC(back_src.w*2 + 1,1),
		FIXED_DEC(back_src.h*2 + 1,1)
	};
	
	if (mx.phase != 4 && stage.song_step < 800)
		Stage_DrawTex(&this->tex_goal, &back_src, &back_dst, stage.camera.bzoom);

	//And down-a here, is where the disaster begins! Wahoo!
	RECT level0_src = {0,   0, 158, 81};
	RECT level1_src = {0,  84, 160, 81}; //reused for level4
	RECT level2_src = {0, 168, 160, 81}; //reused for level5
	RECT level3_src = {0,   0, 160, 81}; //reused for level6

	if (mx.phase == 3)
	{
		//modified code from the main menu

		//draw the level shit
		Stage_DrawTex(&this->tex_level0, &level0_src, &back_dst, stage.camera.bzoom);
		//manipulating da src y and dst x for draw another cloud
		back_dst.x -= back_dst.w;

			Stage_DrawTex(&this->tex_level0, &level1_src, &back_dst, stage.camera.bzoom);

				//repeating cloud0 but with a different x for a decent loop
				back_dst.x -= back_dst.w;

				Stage_DrawTex(&this->tex_level0, &level2_src, &back_dst, stage.camera.bzoom);

					//repeating cloud1 but with a different x for a decent loop
					back_dst.x -= back_dst.w;

					Stage_DrawTex(&this->tex_level1, &level3_src, &back_dst, stage.camera.bzoom);

						//repeating cloud1 but with a different x for a decent loop
						back_dst.x -= back_dst.w;

						Stage_DrawTex(&this->tex_level1, &level1_src, &back_dst, stage.camera.bzoom); //4

							//repeating cloud1 but with a different x for a decent loop
							back_dst.x -= back_dst.w;

							Stage_DrawTex(&this->tex_level1, &level2_src, &back_dst, stage.camera.bzoom); //5

								//repeating cloud1 but with a different x for a decent loop
								back_dst.x -= back_dst.w;

								Stage_DrawTex(&this->tex_level2, &level3_src, &back_dst, stage.camera.bzoom);

								this->movingbg += 8;
								//it is so big it loops itself lmao
	}

	switch (stage.song_step)
	{
		case 1248: //balck bg
			mx.blackscene = 1;
			break;
		case 1257: //INNOCENCE
			mx.blackscene = 2;
			break;	
		case 1263: //DOESN'T
			mx.blackscene = 3;
			break;
		case 1267: //GET
			mx.blackscene = 4;
			break;
		case 1269: //YOU
			mx.blackscene = 5;
			break;
		case 1271: //FAR
			mx.blackscene = 6;
			break;
		case 1280: //go back to normal
			mx.blackscene = 0;
			break;
		case 1369: //prepare for luigi
			mx.scenetype = 1;
			break;
		case 1533: //Luigi
			mx.blackscene = 1;
			break;
		case 1537: //Oh shit
			mx.blackscene = 0;
			break;
	}








	RECT wall_src = {0, 1, 160, 31};
	RECT_FIXED wall_dst = {
		FIXED_DEC(-160,1) - fx,
		FIXED_DEC(36,1) - fy,
		FIXED_DEC(wall_src.w*2,1),
		FIXED_DEC(wall_src.h*2,1)
	};

	if (mx.phase == 4)
		Stage_DrawTex(&this->tex_wall, &wall_src, &wall_dst, stage.camera.bzoom);

	//make sure transparent bg is black
	Gfx_SetClear(0, 0, 0);

	//FntPrint("Fuck %d", this->movingbg);
}

void Back_World1_1PC_DrawFG(StageBack *back)
{
	Back_World1_1PC *this = (Back_World1_1PC*)back;
	
	fixed_t fx, fy;

	//just a blank black screen
	RECT blackscreen = {
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT
	};

	//Luigi head
	RECT loogi_src = {
		0,
		0,
		47,
		80
	};

	RECT_FIXED loogif_dst = {
		FIXED_DEC(-46,1) - fx,
		FIXED_DEC(-63,1) - fy,
		FIXED_DEC(loogi_src.w*2,1),
		FIXED_DEC(loogi_src.h*2,1)
	};

	//the text is in stage.c, since for SOME REASON fonts won't work here
	if (mx.blackscene > 0 && mx.scenetype == 0)
	{
		Gfx_DrawRect(&blackscreen, 0, 0, 0);
	}

	if (mx.blackscene == 1 && mx.scenetype == 1)
	{
		Stage_DrawTex(&this->tex_loogi, &loogi_src, &loogif_dst, stage.camera.bzoom);
		Gfx_DrawRect(&blackscreen, 0, 0, 0);
	}
}

void Back_World1_1PC_Free(StageBack *back)
{
	Back_World1_1PC *this = (Back_World1_1PC*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_World1_1PC_New(void)
{
	//Allocate background structure
	Back_World1_1PC *this = (Back_World1_1PC*)Mem_Alloc(sizeof(Back_World1_1PC));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_World1_1PC_DrawFG;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_World1_1PC_DrawBG;
	this->back.free = Back_World1_1PC_Free;

	mx.blackscene = 0;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WORLD1\\PC.ARC;1");
	Gfx_LoadTex(&this->tex_goal, Archive_Find(arc_back, "goal.tim"), 0);
	Gfx_LoadTex(&this->tex_level0, Archive_Find(arc_back, "level0.tim"), 0);
	Gfx_LoadTex(&this->tex_level1, Archive_Find(arc_back, "level1.tim"), 0);
	Gfx_LoadTex(&this->tex_level2, Archive_Find(arc_back, "level2.tim"), 0);
	Gfx_LoadTex(&this->tex_loogi, Archive_Find(arc_back, "loogi.tim"), 0);
	Gfx_LoadTex(&this->tex_wall, Archive_Find(arc_back, "wall.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
