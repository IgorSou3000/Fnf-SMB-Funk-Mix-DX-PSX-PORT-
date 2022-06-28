/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "gb.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"

//GB background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
} Back_GB;

void Back_Color(StageBack *back)
{
	//Use green bg
	Gfx_SetClear(16, 56, 15);
}

//GB background functions
void Back_GB_Free(StageBack *back)
{
	Back_GB *this = (Back_GB*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_GB_New(void)
{
	//Allocate background structure
	Back_GB *this = (Back_GB*)Mem_Alloc(sizeof(Back_GB));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Color;
	this->back.free = Back_GB_Free;
	
	return (StageBack*)this;
}
