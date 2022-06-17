/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_MX_H
#define PSXF_GUARD_MX_H

#include "../character.h"

//Mario character functions
Character *Char_MX_New(fixed_t x, fixed_t y);

//putting this here so that the stage can access the phase stuff as well
typedef struct
{
  u8 phase; //for... uh... phase stuff
} MX;

extern MX mx;

#endif
