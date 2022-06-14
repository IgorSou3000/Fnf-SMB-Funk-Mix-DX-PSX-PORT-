/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_WORLD2_3_H
#define PSXF_GUARD_WORLD2_3_H

#include "../../stage.h"

//Airship background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //The ship itself

  //Bullets chart
  u16 *bullet_chart;
} Back_World2_3;

//World1_1 functions
StageBack *Back_World2_3_New();

#endif
