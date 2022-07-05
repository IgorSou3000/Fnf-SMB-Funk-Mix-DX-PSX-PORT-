/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "stage.h"

#include "mem.h"
#include "timer.h"
#include "audio.h"
#include "pad.h"
#include "main.h"
#include "random.h"
#include "movie.h"
#include "network.h"
#include "mutil.h"

#include "menu.h"
#include "trans.h"
#include "loadscr.h"

#include "object/combo.h"
#include "object/splash.h"

//Stage constants
//#define STAGE_NOHUD //Disable the HUD

//#define STAGE_FREECAM //Freecam

//Stage definitions
boolean noteshake;

static u32 Sounds[4];

#include "character/bf.h"
#include "character/mario.h"
#include "character/bomb.h"
#include "character/bowser.h"
#include "character/blaster.h"
#include "character/luigi2p.h"
#include "character/spike.h"
#include "character/boo.h"
#include "character/mx.h"

#include "stage/world1/world1_1.h"
#include "stage/world1/world1_2.h"

#include "stage/world2/world2_2.h"
#include "stage/world2/world2_3.h"

#include "stage/freeplay1/freeplay1_1.h"
#include "stage/freeplay1/freeplay1_2.h"

#include "stage/freeplay2/freeplay2_2.h"

#include "stage/gb.h"

#include "stage/world1/world1_1pc.h"

static const StageDef stage_defs[StageId_Max] = {
	#include "stagedef_disc1.h"
};

//Stage state
Stage stage;

//Stage music functions
static void Stage_StartVocal(void)
{
	if (!(stage.flag & STAGE_FLAG_VOCAL_ACTIVE))
	{
		Audio_ChannelXA(stage.stage_def->music_channel);
		stage.flag |= STAGE_FLAG_VOCAL_ACTIVE;
	}
}

static void Stage_CutVocal(void)
{
	if (stage.flag & STAGE_FLAG_VOCAL_ACTIVE)
	{
		Audio_ChannelXA(stage.stage_def->music_channel + 1);
		stage.flag &= ~STAGE_FLAG_VOCAL_ACTIVE;
	}
}

//Stage camera functions
static void Stage_FocusCharacter(Character *ch, fixed_t div)
{
	//Use character focus settings to update target position and zoom
	stage.camera.tx = ch->x + ch->focus_x;
	stage.camera.ty = ch->y + ch->focus_y;
	stage.camera.tz = ch->focus_zoom;
	stage.camera.td = div;
}

static void Stage_ScrollCamera(void)
{
	#ifdef STAGE_FREECAM
		if (pad_state.held & PAD_LEFT)
			stage.camera.x -= FIXED_DEC(2,1);
		if (pad_state.held & PAD_UP)
			stage.camera.y -= FIXED_DEC(2,1);
		if (pad_state.held & PAD_RIGHT)
			stage.camera.x += FIXED_DEC(2,1);
		if (pad_state.held & PAD_DOWN)
			stage.camera.y += FIXED_DEC(2,1);
		if (pad_state.held & PAD_TRIANGLE)
			stage.camera.zoom -= FIXED_DEC(1,100);
		if (pad_state.held & PAD_CROSS)
			stage.camera.zoom += FIXED_DEC(1,100);
	#else
		//Get delta position
		fixed_t dx = stage.camera.tx - stage.camera.x;
		fixed_t dy = stage.camera.ty - stage.camera.y;
		fixed_t dz = stage.camera.tz - stage.camera.zoom;
		
		//Scroll based off current divisor
		stage.camera.x += FIXED_MUL(dx, stage.camera.td);
		stage.camera.y += FIXED_MUL(dy, stage.camera.td);
		stage.camera.zoom += FIXED_MUL(dz, stage.camera.td);
	#endif
	
	//Update other camera stuff
	stage.camera.bzoom = FIXED_MUL(stage.camera.zoom, stage.bump);
}

//normal note x
static int note_x[8] = {
	//BF
	 FIXED_DEC(26,1) + FIXED_DEC(SCREEN_WIDEADD,4),
	 FIXED_DEC(60,1) + FIXED_DEC(SCREEN_WIDEADD,4),//+34
	 FIXED_DEC(94,1) + FIXED_DEC(SCREEN_WIDEADD,4),
	FIXED_DEC(128,1) + FIXED_DEC(SCREEN_WIDEADD,4),
	//Opponent
	FIXED_DEC(-128,1) - FIXED_DEC(SCREEN_WIDEADD,4),
	 FIXED_DEC(-94,1) - FIXED_DEC(SCREEN_WIDEADD,4),//+34
	 FIXED_DEC(-60,1) - FIXED_DEC(SCREEN_WIDEADD,4),
	 FIXED_DEC(-26,1) - FIXED_DEC(SCREEN_WIDEADD,4),
};

static int note_y[8] = {
	//BF
	 FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
	 FIXED_DEC(32 - SCREEN_HEIGHT2, 1),//+34
	 FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
	 FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
	//Opponent
	 FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
	 FIXED_DEC(32 - SCREEN_HEIGHT2, 1),//+34
	 FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
	 FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
};

static const u16 note_key[] = {INPUT_LEFT, INPUT_DOWN, INPUT_UP, INPUT_RIGHT};
static const u8 note_anims[4][3] = {
	{CharAnim_Left,  CharAnim_LeftAlt,  PlayerAnim_LeftMiss},
	{CharAnim_Down,  CharAnim_DownAlt,  PlayerAnim_DownMiss},
	{CharAnim_Up,    CharAnim_UpAlt,    PlayerAnim_UpMiss},
	{CharAnim_Right, CharAnim_RightAlt, PlayerAnim_RightMiss},
};

//Load Notes position
void Stage_Load_Notes(void)
{
	//repeating da normal note position to not fuck middlescroll
	//bf
	note_x[0] = FIXED_DEC(26,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[1] = FIXED_DEC(60,1) + FIXED_DEC(SCREEN_WIDEADD,4); //+34
	note_x[2] = FIXED_DEC(94,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[3] = FIXED_DEC(128,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	//opponent
	note_x[4] = FIXED_DEC(-128,1) - FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[5] = FIXED_DEC(-94,1) - FIXED_DEC(SCREEN_WIDEADD,4); //+34
	note_x[6] = FIXED_DEC(-60,1) - FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[7] = FIXED_DEC(-26,1) - FIXED_DEC(SCREEN_WIDEADD,4);

	//middle note x
	if(stage.middlescroll)
	{
	 //bf
	for (u8 i = 0; i < 4; i++)
	note_x[i] -= FIXED_DEC(78,1);
	 //opponent
	 for (u8 i = 4; i < 8; i++)
	 note_x[i] = FIXED_DEC(-400,1) - FIXED_DEC(SCREEN_WIDEADD,4);
	}

	//swap note if game over
	else if (stage.stage_id == StageId_MX)
	{
		note_x[0] = FIXED_DEC(-128,1) + FIXED_DEC(SCREEN_WIDEADD,4);
		note_x[1] = FIXED_DEC(-94,1) + FIXED_DEC(SCREEN_WIDEADD,4);
		note_x[2] = FIXED_DEC(-60,1) + FIXED_DEC(SCREEN_WIDEADD,4);
		note_x[3] = FIXED_DEC(-26,1) + FIXED_DEC(SCREEN_WIDEADD,4);
		note_x[4] = FIXED_DEC(26,1) + FIXED_DEC(SCREEN_WIDEADD,4);
		note_x[5] = FIXED_DEC(60,1) + FIXED_DEC(SCREEN_WIDEADD,4);
		note_x[6] = FIXED_DEC(94,1) + FIXED_DEC(SCREEN_WIDEADD,4);
		note_x[7] = FIXED_DEC(128,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	}


	//normal note
	else
		{
		 //bf
		 note_x[0] = FIXED_DEC(26,1) + FIXED_DEC(SCREEN_WIDEADD,4);
		 note_x[1] = FIXED_DEC(60,1) + FIXED_DEC(SCREEN_WIDEADD,4); //+34
		 note_x[2] = FIXED_DEC(94,1) + FIXED_DEC(SCREEN_WIDEADD,4);
		 note_x[3] = FIXED_DEC(128,1) + FIXED_DEC(SCREEN_WIDEADD,4);
		 //opponent
		 note_x[4] = FIXED_DEC(-128,1) - FIXED_DEC(SCREEN_WIDEADD,4);
		 note_x[5] = FIXED_DEC(-94,1) - FIXED_DEC(SCREEN_WIDEADD,4); //+34
		 note_x[6] = FIXED_DEC(-60,1) - FIXED_DEC(SCREEN_WIDEADD,4);
		 note_x[7] = FIXED_DEC(-26,1) - FIXED_DEC(SCREEN_WIDEADD,4);
		}
}
//Background movement debug
void Stage_MoveBG(RECT_FIXED *dst)
{
	//using a variable to manage move
	    if (pad_state_2.held & INPUT_LEFT)
		    stage.bgx -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_DOWN)
		    stage.bgy += FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_UP)
		   stage.bgy -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_RIGHT)
		    stage.bgx += FIXED_DEC(1,1);

			dst->x += stage.bgx;
			dst->y += stage.bgy;

			FntPrint("bg x is %d, bg y is %d", dst->x / 1024, dst->y / 1024);
}

//Character movement debug
static void Stage_MoveChar(void)
{
	//move player 1 with second controller's d pad when debug is 2
	if (stage.debug == 2)
	{
		if (pad_state_2.held & INPUT_LEFT)
		    stage.player->x -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_DOWN)
		    stage.player->y += FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_UP)
		    stage.player->y -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_RIGHT)
		    stage.player->x += FIXED_DEC(1,1);
	}

    //move player 2 with second controller's d pad when debug is 3
	if (stage.debug == 3)
	{
		if (pad_state_2.held & INPUT_LEFT)
		    stage.opponent->x -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_DOWN)
		    stage.opponent->y += FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_UP)
		    stage.opponent->y -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_RIGHT)
		    stage.opponent->x += FIXED_DEC(1,1);
	}
	//move player 3 with second controller's d pad when debug is 4
	if (stage.debug == 4)
	{
		if (pad_state_2.held & INPUT_LEFT)
		    stage.gf->x -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_DOWN)
		    stage.gf->y += FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_UP)
		    stage.gf->y -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_RIGHT)
		    stage.gf->x += FIXED_DEC(1,1);
	}
	//move player 4 with second controller's d pad when debug is 5
	if (stage.debug == 5)
	{
		if (pad_state_2.held & INPUT_LEFT)
		    stage.opponent2->x -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_DOWN)
		    stage.opponent2->y += FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_UP)
		    stage.opponent2->y -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_RIGHT)
		    stage.opponent2->x += FIXED_DEC(1,1);
	}
}

//Stage section functions
static void Stage_ChangeBPM(u16 bpm, u16 step)
{
	//Update last BPM
	stage.last_bpm = bpm;
	
	//Update timing base
	if (stage.step_crochet)
		stage.time_base += FIXED_DIV(((fixed_t)step - stage.step_base) << FIXED_SHIFT, stage.step_crochet);
	stage.step_base = step;
	
	//Get new crochet and times
	stage.step_crochet = ((fixed_t)bpm << FIXED_SHIFT) * 8 / 240; //15/12/24
	stage.step_time = FIXED_DIV(FIXED_DEC(12,1), stage.step_crochet);
	
	//Get new crochet based values
	stage.early_safe = stage.late_safe = stage.step_crochet / 6; //10 frames
	stage.late_sus_safe = stage.late_safe;
	stage.early_sus_safe = stage.early_safe * 2 / 5;
}

static Section *Stage_GetPrevSection(Section *section)
{
	if (section > stage.sections)
		return section - 1;
	return NULL;
}

static u16 Stage_GetSectionStart(Section *section)
{
	Section *prev = Stage_GetPrevSection(section);
	if (prev == NULL)
		return 0;
	return prev->end;
}

//Section scroll structure
typedef struct
{
	fixed_t start;   //Seconds
	fixed_t length;  //Seconds
	u16 start_step;  //Sub-steps
	u16 length_step; //Sub-steps
	
	fixed_t size; //Note height
} SectionScroll;

static void Stage_GetSectionScroll(SectionScroll *scroll, Section *section)
{
	//Get BPM
	u16 bpm = section->flag & SECTION_FLAG_BPM_MASK;
	
	//Get section step info
	scroll->start_step = Stage_GetSectionStart(section);
	scroll->length_step = section->end - scroll->start_step;
	
	//Get section time length
	scroll->length = (scroll->length_step * FIXED_DEC(15,1) / 12) * 24 / bpm;
	
	//Get note height
	scroll->size = FIXED_MUL(stage.speed, scroll->length * (12 * 150) / scroll->length_step) + FIXED_UNIT;
}

//Note hit detection
static u8 Stage_HitNote(PlayerState *this, u8 type, fixed_t offset)
{
	//Get hit type
	if (offset < 0)
		offset = -offset;
	
	u8 hit_type;
	if (offset > stage.late_safe * 9 / 11)
		hit_type = 3; //SHIT
	else if (offset > stage.late_safe * 6 / 11)
		hit_type = 2; //BAD
	else if (offset > stage.late_safe * 3 / 11)
		hit_type = 1; //GOOD
	else
		hit_type = 0; //SICK
	
	//Increment combo and score
	this->combo++;
	
	static const s32 score_inc[] = {
		35, //SICK
		20, //GOOD
		10, //BAD
		 5, //SHIT
	};
	this->score += score_inc[hit_type];

	this->min_accuracy += 8;

	this->max_accuracy += 8 + hit_type*2;
	this->refresh_accuracy = true;
	this->refresh_score = true;
	
	//Restore vocals and life
	Stage_StartVocal();
	
	//Create combo object telling of our combo
	Obj_Combo *combo = Obj_Combo_New(
		this->character->focus_x,
		this->character->focus_y,
		hit_type,
		this->combo >= 10 ? this->combo : 0xFFFF
	);
	if (combo != NULL)
		ObjectList_Add(&stage.objlist_fg, (Object*)combo);
	
	//Create note splashes if SICK
	if (hit_type == 0)
	{
		for (int i = 0; i < 3; i++)
		{
			//Create splash object
			Obj_Splash *splash = Obj_Splash_New(
				note_x[type ^ stage.note_swap],
				note_y[type ^ stage.note_swap] * (stage.downscroll ? -1 : 1),
				type & 0x3
			);
			if (splash != NULL)
				ObjectList_Add(&stage.objlist_splash, (Object*)splash);
		}
	}
	
	return hit_type;
}

static void Stage_MissNote(PlayerState *this)
{
	this->max_accuracy += 10;
	this->refresh_accuracy = true;
	this->miss += 1;
	this->refresh_miss = true;
	if (this->combo)
	{
		//Kill combo
		if (stage.gf != NULL && this->combo > 5)
			stage.gf->set_anim(stage.gf, CharAnim_DownAlt); //Cry if we lost a large combo
		this->combo = 0;
		
		//Create combo object telling of our lost combo
		Obj_Combo *combo = Obj_Combo_New(
			this->character->focus_x,
			this->character->focus_y,
			0xFF,
			0
		);
		if (combo != NULL)
			ObjectList_Add(&stage.objlist_fg, (Object*)combo);
	}
}

static void Stage_NoteCheck(PlayerState *this, u8 type)
{
	//Perform note check
	for (Note *note = stage.cur_note;; note++)
	{
		if (!(note->type & NOTE_FLAG_MINE))
		{
			//Check if note can be hit
			fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
			if (note_fp - stage.early_safe > stage.note_scroll)
				break;
			if (note_fp + stage.late_safe < stage.note_scroll)
				continue;
			if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
				continue;
			
			//Hit the note
			note->type |= NOTE_FLAG_HIT;
			
			if (this->character->ignore_note == false)
				this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);

			u8 hit_type = Stage_HitNote(this, type, stage.note_scroll - note_fp);
			this->arrow_hitan[type & 0x3] = stage.step_time;
			(void)hit_type;
			return;
		}
		else
		{
			//Check if mine can be hit
			fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
			if (note_fp - (stage.late_safe * 3 / 5) > stage.note_scroll)
				break;
			if (note_fp + (stage.late_safe * 2 / 5) < stage.note_scroll)
				continue;
			if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
				continue;
			
			//Hit the mine
			note->type |= NOTE_FLAG_HIT;
	
			this->character->powerup--;

			if (this->character->spec & CHAR_SPEC_MISSANIM)
				this->character->set_anim(this->character, note_anims[type & 0x3][2]);
			else
				this->character->set_anim(this->character, note_anims[type & 0x3][0]);
			this->arrow_hitan[type & 0x3] = -1;
			
			return;
		}
	}
	
	//Missed a note
	this->arrow_hitan[type & 0x3] = -1;
	
	if (!stage.ghost)
	{
		if (this->character->spec & CHAR_SPEC_MISSANIM)
			this->character->set_anim(this->character, note_anims[type & 0x3][2]);
		else
			this->character->set_anim(this->character, note_anims[type & 0x3][0]);
		Stage_MissNote(this);
		
		this->character->powerup--;
		this->score -= 1;
		this->refresh_score = true;
	}
}

static void Stage_SustainCheck(PlayerState *this, u8 type)
{
	//Perform note check
	for (Note *note = stage.cur_note;; note++)
	{
		//Check if note can be hit
		fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
		if (note_fp - stage.early_sus_safe > stage.note_scroll)
			break;
		if (note_fp + stage.late_sus_safe < stage.note_scroll)
			continue;
		if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || !(note->type & NOTE_FLAG_SUSTAIN))
			continue;
		
		//Hit the note
		note->type |= NOTE_FLAG_HIT;
		if (this->character->ignore_note == false)
			this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
		
		Stage_StartVocal();
		this->arrow_hitan[type & 0x3] = stage.step_time;
			
	}
}

static void Stage_ProcessPlayer(PlayerState *this, Pad *pad, boolean playing)
{
	//Handle player note presses
	if (!(stage.botplay)) {
		if (playing)
		{
			u8 i = (this->character == stage.opponent) ? NOTE_FLAG_OPPONENT : 0;
			
			this->pad_held = this->character->pad_held = pad->held;
			this->pad_press = pad->press;
			
			if (this->pad_held & INPUT_LEFT)
				Stage_SustainCheck(this, 0 | i);
			if (this->pad_held & INPUT_DOWN)
				Stage_SustainCheck(this, 1 | i);
			if (this->pad_held & INPUT_UP)
				Stage_SustainCheck(this, 2 | i);
			if (this->pad_held & INPUT_RIGHT)
				Stage_SustainCheck(this, 3 | i);
			
			if (this->pad_press & INPUT_LEFT)
				Stage_NoteCheck(this, 0 | i);
			if (this->pad_press & INPUT_DOWN)
				Stage_NoteCheck(this, 1 | i);
			if (this->pad_press & INPUT_UP)
				Stage_NoteCheck(this, 2 | i);
			if (this->pad_press & INPUT_RIGHT)
				Stage_NoteCheck(this, 3 | i);
		}
		else
		{
			this->pad_held = this->character->pad_held = 0;
			this->pad_press = 0;
		}
	}
	
	if (stage.botplay) {
		//Do perfect note checks
		if (playing)
		{
			u8 i = (this->character == stage.opponent) ? NOTE_FLAG_OPPONENT : 0;
			
			u8 hit[4] = {0, 0, 0, 0};
			for (Note *note = stage.cur_note;; note++)
			{
				//Check if note can be hit
				fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
				if (note_fp - stage.early_safe - FIXED_DEC(12,1) > stage.note_scroll)
					break;
				if (note_fp + stage.late_safe < stage.note_scroll)
					continue;
				if ((note->type & NOTE_FLAG_MINE) || (note->type & NOTE_FLAG_OPPONENT) != i)
					continue;
				
				//Handle note hit
				if (!(note->type & NOTE_FLAG_SUSTAIN))
				{
					if (note->type & NOTE_FLAG_HIT)
						continue;
					if (stage.note_scroll >= note_fp)
						hit[note->type & 0x3] |= 1;
					else if (!(hit[note->type & 0x3] & 8))
						hit[note->type & 0x3] |= 2;
				}
				else if (!(hit[note->type & 0x3] & 2))
				{
					if (stage.note_scroll <= note_fp)
						hit[note->type & 0x3] |= 4;
					hit[note->type & 0x3] |= 8;
				}
			}
			
			//Handle input
			this->pad_held = 0;
			this->pad_press = 0;
			
			for (u8 j = 0; j < 4; j++)
			{
				if (hit[j] & 5)
				{
					this->pad_held |= note_key[j];
					Stage_SustainCheck(this, j | i);
				}
				if (hit[j] & 1)
				{
					this->pad_press |= note_key[j];
					Stage_NoteCheck(this, j | i);
				}
			}
			
			this->character->pad_held = this->pad_held;
		}
		else
		{
			this->pad_held = this->character->pad_held = 0;
			this->pad_press = 0;
		}
	}
}

//Stage drawing functions
void Stage_DrawTexCol(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 cr, u8 cg, u8 cb)
{
	fixed_t xz = dst->x;
	fixed_t yz = dst->y;
	fixed_t wz = dst->w;
	fixed_t hz = dst->h;
	
		//Don't draw if HUD and is disabled
		if (tex == &stage.tex_hud0)
		{
			#ifdef STAGE_NOHUD
				return;
			#endif
		}
	
	fixed_t l = (SCREEN_WIDTH2  << FIXED_SHIFT) + FIXED_MUL(xz, zoom);// + FIXED_DEC(1,2);
	fixed_t t = (SCREEN_HEIGHT2 << FIXED_SHIFT) + FIXED_MUL(yz, zoom);// + FIXED_DEC(1,2);
	fixed_t r = l + FIXED_MUL(wz, zoom);
	fixed_t b = t + FIXED_MUL(hz, zoom);
	
	l >>= FIXED_SHIFT;
	t >>= FIXED_SHIFT;
	r >>= FIXED_SHIFT;
	b >>= FIXED_SHIFT;
	
	RECT sdst = {
		l,
		t,
		r - l,
		b - t,
	};
	Gfx_DrawTexCol(tex, src, &sdst, cr, cg, cb);
}

void Stage_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom)
{
	Stage_DrawTexCol(tex, src, dst, zoom, 0x80, 0x80, 0x80);
}

void Stage_DrawTexRotate(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 angle)
{	
	s16 sin = MUtil_Sin(angle);
	s16 cos = MUtil_Cos(angle);
	int pw = dst->w / 2000;
    int ph = dst->h / 2000;

	//Get rotated points
	POINT p0 = {-pw, -ph};
	MUtil_RotatePoint(&p0, sin, cos);
	
	POINT p1 = { pw, -ph};
	MUtil_RotatePoint(&p1, sin, cos);
	
	POINT p2 = {-pw,  ph};
	MUtil_RotatePoint(&p2, sin, cos);
	
	POINT p3 = { pw,  ph};
	MUtil_RotatePoint(&p3, sin, cos);
	
	POINT_FIXED d0 = {
		dst->x + ((fixed_t)p0.x << FIXED_SHIFT),
		dst->y + ((fixed_t)p0.y << FIXED_SHIFT)
	};
	POINT_FIXED d1 = {
		dst->x + ((fixed_t)p1.x << FIXED_SHIFT),
		dst->y + ((fixed_t)p1.y << FIXED_SHIFT)
	};
	POINT_FIXED d2 = {
        dst->x + ((fixed_t)p2.x << FIXED_SHIFT),
		dst->y + ((fixed_t)p2.y << FIXED_SHIFT)
	};
	POINT_FIXED d3 = {
        dst->x + ((fixed_t)p3.x << FIXED_SHIFT),
		dst->y + ((fixed_t)p3.y << FIXED_SHIFT)
	};
	
    Stage_DrawTexArb(tex, src, &d0, &d1, &d2, &d3, zoom);
}

void Stage_BlendTexRotate(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 angle, u8 mode)
{	
	s16 sin = MUtil_Sin(angle);
	s16 cos = MUtil_Cos(angle);
	int pw = dst->w / 2000;
    int ph = dst->h / 2000;

	//Get rotated points
	POINT p0 = {-pw, -ph};
	MUtil_RotatePoint(&p0, sin, cos);
	
	POINT p1 = { pw, -ph};
	MUtil_RotatePoint(&p1, sin, cos);
	
	POINT p2 = {-pw,  ph};
	MUtil_RotatePoint(&p2, sin, cos);
	
	POINT p3 = { pw,  ph};
	MUtil_RotatePoint(&p3, sin, cos);
	
	POINT_FIXED d0 = {
		dst->x + ((fixed_t)p0.x << FIXED_SHIFT),
		dst->y + ((fixed_t)p0.y << FIXED_SHIFT)
	};
	POINT_FIXED d1 = {
		dst->x + ((fixed_t)p1.x << FIXED_SHIFT),
		dst->y + ((fixed_t)p1.y << FIXED_SHIFT)
	};
	POINT_FIXED d2 = {
        dst->x + ((fixed_t)p2.x << FIXED_SHIFT),
		dst->y + ((fixed_t)p2.y << FIXED_SHIFT)
	};
	POINT_FIXED d3 = {
        dst->x + ((fixed_t)p3.x << FIXED_SHIFT),
		dst->y + ((fixed_t)p3.y << FIXED_SHIFT)
	};
	
    Stage_BlendTexArb(tex, src, &d0, &d1, &d2, &d3, zoom, mode);
}

void Stage_BlendTex(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 mode)
{
	fixed_t xz = dst->x;
	fixed_t yz = dst->y;
	fixed_t wz = dst->w;
	fixed_t hz = dst->h;

		//Don't draw if HUD and is disabled
		if (tex == &stage.tex_hud0)
		{
			#ifdef STAGE_NOHUD
				return;
			#endif
		}
	
	fixed_t l = (SCREEN_WIDTH2  << FIXED_SHIFT) + FIXED_MUL(xz, zoom);// + FIXED_DEC(1,2);
	fixed_t t = (SCREEN_HEIGHT2 << FIXED_SHIFT) + FIXED_MUL(yz, zoom);// + FIXED_DEC(1,2);
	fixed_t r = l + FIXED_MUL(wz, zoom);
	fixed_t b = t + FIXED_MUL(hz, zoom);
	
	l >>= FIXED_SHIFT;
	t >>= FIXED_SHIFT;
	r >>= FIXED_SHIFT;
	b >>= FIXED_SHIFT;
	
	RECT sdst = {
		l,
		t,
		r - l,
		b - t,
	};
	Gfx_BlendTex(tex, src, &sdst, mode);
}

void Stage_DrawTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom)
{
	//Don't draw if HUD and HUD is disabled
	#ifdef STAGE_NOHUD
		if (tex == &stage.tex_hud0)
			return;
	#endif
	
	//Get screen-space points
	POINT s0 = {SCREEN_WIDTH2 + (FIXED_MUL(p0->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p0->y, zoom) >> FIXED_SHIFT)};
	POINT s1 = {SCREEN_WIDTH2 + (FIXED_MUL(p1->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p1->y, zoom) >> FIXED_SHIFT)};
	POINT s2 = {SCREEN_WIDTH2 + (FIXED_MUL(p2->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p2->y, zoom) >> FIXED_SHIFT)};
	POINT s3 = {SCREEN_WIDTH2 + (FIXED_MUL(p3->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p3->y, zoom) >> FIXED_SHIFT)};
	
	Gfx_DrawTexArb(tex, src, &s0, &s1, &s2, &s3);
}

void Stage_BlendTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom, u8 mode)
{
	//Don't draw if HUD and HUD is disabled
	#ifdef STAGE_NOHUD
		if (tex == &stage.tex_hud0)
			return;
	#endif
	
	//Get screen-space points
	POINT s0 = {SCREEN_WIDTH2 + (FIXED_MUL(p0->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p0->y, zoom) >> FIXED_SHIFT)};
	POINT s1 = {SCREEN_WIDTH2 + (FIXED_MUL(p1->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p1->y, zoom) >> FIXED_SHIFT)};
	POINT s2 = {SCREEN_WIDTH2 + (FIXED_MUL(p2->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p2->y, zoom) >> FIXED_SHIFT)};
	POINT s3 = {SCREEN_WIDTH2 + (FIXED_MUL(p3->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p3->y, zoom) >> FIXED_SHIFT)};
	
	Gfx_BlendTexArb(tex, src, &s0, &s1, &s2, &s3, mode);
}

//Stage HUD functions
static void Stage_DrawHealth(s16 life, u8 i, s8 ox)
{
	//Check if we should use 'dying' frame
	s8 dying;
	if (ox < 0)
		dying = (life >= 18000) * 46;
	else
		dying = (life <= 2000) * 46;
	
	//Get src and dst
	fixed_t hx = (128 << FIXED_SHIFT) * (10000 - life) / 10000;
	RECT src = {
		(i % 2) * 92 + dying,
		16 + (i / 2) * 46,
		46,
		46
	};
	RECT_FIXED dst = {
		hx + ox * FIXED_DEC(23,1) - FIXED_DEC(23,1),
		FIXED_DEC(SCREEN_HEIGHT2 - 32 + 4 - 23, 1),
		src.w << FIXED_SHIFT,
		src.h << FIXED_SHIFT
	};
	if (stage.downscroll)
		dst.y = -dst.y - dst.h;
	
	dst.y += stage.noteshakey;
	dst.x += stage.noteshakex;

	//Draw life icon
    if (stage.mode == StageMode_Swap)
    {
        dst.w = -dst.w;
		dst.x += FIXED_DEC(46,1);
    }
	else
    {
        dst.w = dst.w;
		dst.x = dst.x;
    }
}

static void Stage_DrawStrum(u8 i, RECT *note_src, RECT_FIXED *note_dst)
{
	(void)note_dst;
	
	PlayerState *this = &stage.player_state[(i & NOTE_FLAG_OPPONENT) != 0];
	i &= 0x3;
	
	if (this->arrow_hitan[i] > 0)
	{
		//Play hit animation
		u8 frame = ((this->arrow_hitan[i] << 1) / stage.step_time) & 1;
		note_src->x = (i + 1) << 5;
		note_src->y = 64 - (frame << 5);
		
		this->arrow_hitan[i] -= timer_dt;
		if (this->arrow_hitan[i] <= 0)
		{
			if (this->pad_held & note_key[i])
				this->arrow_hitan[i] = 1;
			else
				this->arrow_hitan[i] = 0;
		}
	}
	else if (this->arrow_hitan[i] < 0)
	{
		//Play depress animation
		note_src->x = (i + 1) << 5;
		note_src->y = 96;
		if (!(this->pad_held & note_key[i]))
			this->arrow_hitan[i] = 0;
	}
	else
	{
		note_src->x = 0;
		note_src->y = i << 5;
	}
}

static void Stage_DrawNotes(void)
{
	//Check if opponent should draw as bot
	u8 bot = (stage.mode >= StageMode_2P) ? 0 : NOTE_FLAG_OPPONENT;
	
	//Initialize scroll state
	SectionScroll scroll;
	scroll.start = stage.time_base;
	
	Section *scroll_section = stage.section_base;
	Stage_GetSectionScroll(&scroll, scroll_section);
	
	//Push scroll back until cur_note is properly contained
	while (scroll.start_step > stage.cur_note->pos)
	{
		//Look for previous section
		Section *prev_section = Stage_GetPrevSection(scroll_section);
		if (prev_section == NULL)
			break;
		
		//Push scroll back
		scroll_section = prev_section;
		Stage_GetSectionScroll(&scroll, scroll_section);
		scroll.start -= scroll.length;
	}
	
	//Draw notes
	for (Note *note = stage.cur_note; note->pos != 0xFFFF; note++)
	{
		//Update scroll
		while (note->pos >= scroll_section->end)
		{
			//Push scroll forward
			scroll.start += scroll.length;
			Stage_GetSectionScroll(&scroll, ++scroll_section);
		}
		
		//Get note information
		u8 i = (note->type & NOTE_FLAG_OPPONENT) != 0;
		PlayerState *this = &stage.player_state[i];
		
		fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
		fixed_t time = (scroll.start - stage.song_time) + (scroll.length * (note->pos - scroll.start_step) / scroll.length_step);
		fixed_t y = note_y[(note->type & 0x7) ^ stage.note_swap] + FIXED_MUL(stage.speed, time * 150);
		
		//Check if went above screen
		if (y < FIXED_DEC(-16 - SCREEN_HEIGHT2, 1))
		{
			//Wait for note to exit late time
			if (note_fp + stage.late_safe >= stage.note_scroll)
				continue;
			
			//Miss note if player's note
			if (!(note->type & (bot | NOTE_FLAG_HIT | NOTE_FLAG_MINE)))
			{
				if (stage.mode < StageMode_Net1 || i == ((stage.mode == StageMode_Net1) ? 0 : 1))
				{
					//Missed note
					Stage_CutVocal();
					Stage_MissNote(this);
					this->character->powerup--;
					
				}
			}
			
			//Update current note
			stage.cur_note++;
		}
		else
		{
			//Don't draw if below screen
			RECT note_src;
			RECT_FIXED note_dst;
			if (y > (FIXED_DEC(SCREEN_HEIGHT,2) + scroll.size) || note->pos == 0xFFFF)
				break;
			
			//Draw note
			if (note->type & NOTE_FLAG_SUSTAIN)
			{
				//Check for sustain clipping
				fixed_t clip;
				y -= scroll.size;
				if ((note->type & (bot | NOTE_FLAG_HIT)) || ((this->pad_held & note_key[note->type & 0x3]) && (note_fp + stage.late_sus_safe >= stage.note_scroll)))
				{
					clip = note_y[(note->type & 0x7) ^ stage.note_swap] - y;
					if (clip < 0)
						clip = 0;
				}
				else
				{
					clip = 0;
				}
				
				//Draw sustain
				if (note->type & NOTE_FLAG_SUSTAIN_END)
				{
					if (clip < (24 << FIXED_SHIFT))
					{
						note_src.x = 160;
						note_src.y = ((note->type & 0x3) << 5) + 4 + (clip >> FIXED_SHIFT);
						note_src.w = 32;
						note_src.h = 28 - (clip >> FIXED_SHIFT);
						
						note_dst.x = stage.noteshakex + note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
						note_dst.y = stage.noteshakey + y + clip;
						note_dst.w = note_src.w << FIXED_SHIFT;
						note_dst.h = (note_src.h << FIXED_SHIFT);
						
						if (stage.downscroll)
						{
							note_dst.y = -note_dst.y;
							note_dst.h = -note_dst.h;
						}
						//draw for opponent
						if (stage.middlescroll && note->type & NOTE_FLAG_OPPONENT)
							Stage_BlendTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump, 1);
						else
							Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
					}
				}
				else
				{
					//Get note height
					fixed_t next_time = (scroll.start - stage.song_time) + (scroll.length * (note->pos + 12 - scroll.start_step) / scroll.length_step);
					fixed_t next_y = note_y[(note->type & 0x7) ^ stage.note_swap] + FIXED_MUL(stage.speed, next_time * 150) - scroll.size;
					fixed_t next_size = next_y - y;
					
					if (clip < next_size)
					{
						note_src.x = 160;
						note_src.y = ((note->type & 0x3) << 5);
						note_src.w = 32;
						note_src.h = 16;
						
						note_dst.x = stage.noteshakex + note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
						note_dst.y = stage.noteshakey + y + clip;
						note_dst.w = note_src.w << FIXED_SHIFT;
						note_dst.h = (next_y - y) - clip;
						
						if (stage.downscroll)
							note_dst.y = -note_dst.y - note_dst.h;
						//draw for opponent
						if (stage.middlescroll && note->type & NOTE_FLAG_OPPONENT)
							Stage_BlendTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump, 1);
						else
							Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
					}
				}
			}
			else if (note->type & NOTE_FLAG_MINE)
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note body
				note_src.x = 192;
				note_src.y = 192;
				note_src.w = 32;
				note_src.h = 32;
				
				note_dst.x = stage.noteshakex + note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
				note_dst.y = stage.noteshakey + y - FIXED_DEC(16,1);
				note_dst.w = note_src.w << FIXED_SHIFT;
				note_dst.h = note_src.h << FIXED_SHIFT;
				
				if (stage.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				//draw for opponent
				if (stage.middlescroll && note->type & NOTE_FLAG_OPPONENT)
					Stage_BlendTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump, 1);
				else
					Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);	
			}
			else
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note
				note_src.x = 32 + ((note->type & 0x3) << 5);
				note_src.y = 0;
				note_src.w = 32;
				note_src.h = 32;
				
				note_dst.x = stage.noteshakex + note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
				note_dst.y = stage.noteshakey + y - FIXED_DEC(16,1);
				note_dst.w = note_src.w << FIXED_SHIFT;
				note_dst.h = note_src.h << FIXED_SHIFT;
				
				if (stage.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				//draw for opponent
				if (stage.middlescroll && note->type & NOTE_FLAG_OPPONENT)
					Stage_BlendTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump, 1);
				else
					Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			}
		}
	}
}

static void Stage_CountDown(void)
{
    //countdown stuff
 if (stage.soundcooldown < 4)
 {
	if (stage.flag & STAGE_FLAG_JUST_STEP)
    {
		if ((stage.song_step & 0x7) == 0) //play a sound every 8 steps
		 stage.playedsound = true;

			if (stage.soundcooldown == 0 && stage.playedsound == true)
				Audio_PlaySound(Sounds[0]);
			if (stage.soundcooldown == 1 && stage.playedsound == true)
				Audio_PlaySound(Sounds[0]);
			if (stage.soundcooldown == 2 && stage.playedsound == true)
				Audio_PlaySound(Sounds[0]);
			if (stage.soundcooldown == 3 && stage.playedsound == true)
				Audio_PlaySound(Sounds[1]);
		
		//increase for another sound and stop play sound
		if (stage.playedsound == true)
		 {
		  stage.soundcooldown++;
		  stage.playedsound = false;
		 }
	}
  }	
}

//Stage loads
static void Stage_SwapChars(void)
{
	if (stage.mode == StageMode_Swap)
	{
		Character *temp = stage.player;
		stage.player = stage.opponent;
		stage.opponent = temp;
	}
}

static void Stage_LoadPlayer(void)
{
	//Load player character
	Character_Free(stage.player);
	stage.player = stage.stage_def->pchar.new(stage.stage_def->pchar.x, stage.stage_def->pchar.y);
}

static void Stage_LoadOpponent(void)
{
	//Load opponent character
	Character_Free(stage.opponent);
	stage.opponent = stage.stage_def->ochar.new(stage.stage_def->ochar.x, stage.stage_def->ochar.y);
}

static void Stage_LoadOpponent2(void)
{
	//Load opponent character
	Character_Free(stage.opponent2);
	if (stage.stage_def->o2char.new != NULL)
		stage.opponent2 = stage.stage_def->o2char.new(stage.stage_def->o2char.x, stage.stage_def->o2char.y);
	else
		stage.opponent2 = NULL;
}

static void Stage_LoadGirlfriend(void)
{
	//Load girlfriend character
	Character_Free(stage.gf);
	if (stage.stage_def->gchar.new != NULL)
		stage.gf = stage.stage_def->gchar.new(stage.stage_def->gchar.x, stage.stage_def->gchar.y);
	else
		stage.gf = NULL;
}

static void Stage_LoadStage(void)
{
	//Load back
	if (stage.back != NULL)
		stage.back->free(stage.back);
	stage.back = stage.stage_def->back();
}

static void Stage_LoadChart(void)
{
	//Load stage data
	char chart_path[64];
	
	//Use standard path convention
	switch(stage.stage_id)
	{
	//freeplay
	case StageId_F1_1:
	case StageId_F1_2:
	case StageId_F1_3:
	case StageId_F2_1:
	case StageId_F2_2:
	case StageId_F2_3:
	sprintf(chart_path, "\\FREE%d\\%d.%d.CHT;1", stage.stage_def->week, stage.stage_def->week, stage.stage_def->week_song);
	break;

	//secret songs
	case StageId_S_1:
	case StageId_S_2:
	case StageId_S_3:
	case StageId_S_4:
	sprintf(chart_path, "\\SECRET\\%d.%d.CHT;1", stage.stage_def->week, stage.stage_def->week_song);
	break;

	//MX song
	case StageId_MX:
	sprintf(chart_path, "\\MX\\%d.%d.CHT;1", stage.stage_def->week, stage.stage_def->week, stage.stage_def->week_song);
	break;
	
	//story songs
	default:
	sprintf(chart_path, "\\WORLD%d\\%d.%d.CHT;1", stage.stage_def->week, stage.stage_def->week, stage.stage_def->week_song);
	break;
	}
	
	if (stage.chart_data != NULL)
		Mem_Free(stage.chart_data);
	stage.chart_data = IO_Read(chart_path);
	u8 *chart_byte = (u8*)stage.chart_data;

		//Directly use section and notes pointers
		stage.sections = (Section*)(chart_byte + 2);
		stage.notes = (Note*)(chart_byte + *((u16*)stage.chart_data));
		
		for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
			stage.num_notes++;
	
	//Swap chart
	if (stage.mode == StageMode_Swap)
	{
		for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
			note->type ^= NOTE_FLAG_OPPONENT;
		for (Section *section = stage.sections;; section++)
		{
			section->flag ^= SECTION_FLAG_OPPFOCUS;
			if (section->end == 0xFFFF)
				break;
		}
	}
	
	//Count max scores
	stage.player_state[0].max_score = 0;
	stage.player_state[1].max_score = 0;
	for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
	{
		if (note->type & (NOTE_FLAG_SUSTAIN | NOTE_FLAG_MINE))
			continue;
		if (note->type & NOTE_FLAG_OPPONENT)
			stage.player_state[1].max_score += 35;
		else
			stage.player_state[0].max_score += 35;
	}
	if (stage.mode >= StageMode_2P && stage.player_state[1].max_score > stage.player_state[0].max_score)
		stage.max_score = stage.player_state[1].max_score;
	else
		stage.max_score = stage.player_state[0].max_score;
	
	stage.cur_section = stage.sections;
	stage.cur_note = stage.notes;
	
	stage.speed = stage.stage_def->speed[0];
	
	stage.step_crochet = 0;
	stage.time_base = 0;
	stage.step_base = 0;
	stage.section_base = stage.cur_section;
	Stage_ChangeBPM(stage.cur_section->flag & SECTION_FLAG_BPM_MASK, 0);
}

static void Stage_LoadSFX(void)
{
		//Load SFX
	 	CdlFILE file;
	  	IO_FindFile(&file, "\\SOUNDS\\COUNT.VAG;1");
	    u32 *data = IO_ReadFile(&file);
	    Sounds[0] = Audio_LoadVAGData(data, file.size);
	    Mem_Free(data);

		IO_FindFile(&file, "\\SOUNDS\\COUNTEND.VAG;1");
	    data = IO_ReadFile(&file);
	    Sounds[1] = Audio_LoadVAGData(data, file.size);
	    Mem_Free(data);
}

static void Stage_LoadMusic(void)
{
	//Offset sing ends
	stage.player->sing_end -= stage.note_scroll;
	stage.opponent->sing_end -= stage.note_scroll;
	if (stage.opponent2 != NULL)
	stage.opponent2->sing_end -= stage.note_scroll;
	if (stage.gf != NULL)
		stage.gf->sing_end -= stage.note_scroll;
	
	//Find music file and begin seeking to it
	Audio_SeekXA_Track(stage.stage_def->music_track);
	
	//Initialize music state
	stage.note_scroll = FIXED_DEC((-5 * 6 - 2) * 12,1);
	stage.song_time = FIXED_DIV(stage.note_scroll, stage.step_crochet);
	stage.interp_time = 0;
	stage.interp_ms = 0;
	stage.interp_speed = 0;
	
	//Offset sing ends again
	stage.player->sing_end += stage.note_scroll;
	stage.opponent->sing_end += stage.note_scroll;
	if (stage.opponent2 != NULL)
		stage.opponent2->sing_end += stage.note_scroll;
	if (stage.gf != NULL)
		stage.gf->sing_end += stage.note_scroll;
}

static void Stage_LoadState(void)
{
	//Initialize stage state
	stage.flag = STAGE_FLAG_VOCAL_ACTIVE;
	
	stage.gf_speed = 1 << 2;
	
	stage.state = StageState_Play;
	
	stage.player_state[0].character = stage.player;
	stage.player_state[1].character = stage.opponent;
	stage.soundcooldown = 0;
	stage.playedsound = false;
	stage.bgx = FIXED_DEC(0,1);
	stage.bgy = FIXED_DEC(0,1);

	//reseting this for avoid bugs
	for (int i = 0; i < 2; i++)
	{
		memset(stage.player_state[i].arrow_hitan, 0, sizeof(stage.player_state[i].arrow_hitan));
		
		stage.player_state[i].character->powerup = 3;
		stage.player_state[i].combo = 0;
		stage.player_state[i].miss = 0;
		stage.player_state[i].accuracy = 0;
		stage.player_state[i].max_accuracy = 0;
		stage.player_state[i].min_accuracy = 0;
		stage.player_state[i].character->ignore_note = false;
		stage.player_state[i].refresh_score = false;
		stage.player_state[i].score = 0;
		strcpy(stage.player_state[i].score_text, "000000");
		
		stage.player_state[i].pad_held = stage.player_state[i].pad_press = 0;
	}
	
	ObjectList_Free(&stage.objlist_splash);
	ObjectList_Free(&stage.objlist_fg);
	ObjectList_Free(&stage.objlist_bg);
}

//Stage functions
void Stage_Load(StageId id, StageDiff difficulty, boolean story)
{
	//Get stage definition
	stage.stage_def = &stage_defs[stage.stage_id = id];
	stage.stage_diff = difficulty;
	stage.story = story;
	
	//Load HUD textures
	switch (stage.stage_id)
	{
		case StageId_MX: //PC port arrows
			Gfx_LoadTex(&stage.tex_hud0, IO_Read("\\STAGE\\HUD0PC.TIM;1"), GFX_LOADTEX_FREE);
			break;
		case StageId_F1_3: //Portal arrows
			Gfx_LoadTex(&stage.tex_hud0, IO_Read("\\STAGE\\HUD0MAR0.TIM;1"), GFX_LOADTEX_FREE);
			break;
		case StageId_S_2: //Super Mario Maker arrows
			Gfx_LoadTex(&stage.tex_hud0, IO_Read("\\STAGE\\HUD0MAKR.TIM;1"), GFX_LOADTEX_FREE);
			break;
		case StageId_S_3: //Game Boy arrows
			Gfx_LoadTex(&stage.tex_hud0, IO_Read("\\STAGE\\HUD0GB.TIM;1"), GFX_LOADTEX_FREE);
			break;
		default: //Mario arrows
			Gfx_LoadTex(&stage.tex_hud0, IO_Read("\\STAGE\\HUD0MARO.TIM;1"), GFX_LOADTEX_FREE);
			break;
	}
	Gfx_LoadTex(&stage.tex_bg, IO_Read("\\STAGE\\BG.TIM;1"), GFX_LOADTEX_FREE);
	//Load stage background
	Stage_LoadStage();

	//Load SFX
	Stage_LoadSFX();

	//Load Notes Position
	Stage_Load_Notes();

	//load fonts
	FontData_Load(&stage.font_smb1, Font_SMB1);

	//Load characters
	Stage_LoadPlayer();
	Stage_LoadOpponent();
	Stage_LoadOpponent2();	
	Stage_LoadGirlfriend();
	Stage_SwapChars();
	
	//Load stage chart
	Stage_LoadChart();
	
	//Initialize stage state
	stage.story = story;
	
	Stage_LoadState();
	
	//Initialize 
	/*
	if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
		Stage_FocusCharacter(stage.opponent, FIXED_UNIT);
	else
		Stage_FocusCharacter(stage.player, FIXED_UNIT);
	*/
	stage.camera.x = stage.camera.tx;
	stage.camera.y = stage.camera.ty;
	stage.camera.zoom = stage.camera.tz = FIXED_DEC(1,1);
	
	stage.bump = FIXED_UNIT;
	stage.sbump = FIXED_UNIT;
	
	//Initialize stage according to mode
	stage.note_swap = (stage.mode == StageMode_Swap && (!(stage.middlescroll))) ? 4 : 0;
	
	//Load music
	stage.note_scroll = 0;
	Stage_LoadMusic();
	
	//Test offset
	stage.offset = 0;
}

void Stage_Unload(void)
{
	//Disable net mode to not break the game
	if (stage.mode >= StageMode_Net1)
		stage.mode = StageMode_Normal;
	
	//Unload stage background
	if (stage.back != NULL)
		stage.back->free(stage.back);
	stage.back = NULL;
	
	//Unload stage data
	Mem_Free(stage.chart_data);
	stage.chart_data = NULL;
	
	//Free objects
	ObjectList_Free(&stage.objlist_splash);
	ObjectList_Free(&stage.objlist_fg);
	ObjectList_Free(&stage.objlist_bg);
	
	//Free characters
	Character_Free(stage.player);
	stage.player = NULL;
	Character_Free(stage.opponent);
	stage.opponent = NULL;
	Character_Free(stage.opponent2);
	stage.opponent2 = NULL;
	Character_Free(stage.gf);
	stage.gf = NULL;
}

static boolean Stage_NextLoad(void)
{
	u8 load = stage.stage_def->next_load;
	if (load == 0)
	{
		//Do stage transition if full reload
		stage.trans = StageTrans_NextSong;
		Trans_Start();
		return false;
	}
	else
	{
		//Get stage definition
		stage.stage_def = &stage_defs[stage.stage_id = stage.stage_def->next_stage];
		
		//Load stage background
		if (load & STAGE_LOAD_STAGE)
			Stage_LoadStage();
		
		//Load characters
		Stage_SwapChars();
		if (load & STAGE_LOAD_PLAYER)
		{
			Stage_LoadPlayer();
		}
		else
		{
			stage.player->x = stage.stage_def->pchar.x;
			stage.player->y = stage.stage_def->pchar.y;
		}
		if (load & STAGE_LOAD_OPPONENT)
		{
			Stage_LoadOpponent();
		}
		else
		{
			stage.opponent->x = stage.stage_def->ochar.x;
			stage.opponent->y = stage.stage_def->ochar.y;
		}
		Stage_SwapChars();
		if (load & STAGE_LOAD_GIRLFRIEND)
		{
			Stage_LoadGirlfriend();
		}
		else if (stage.gf != NULL)
		{
			stage.gf->x = stage.stage_def->gchar.x;
			stage.gf->y = stage.stage_def->gchar.y;
		}
		
		//Load stage chart
		Stage_LoadChart();
		
		//Initialize stage state
		Stage_LoadState();
		
		//Load music
		Stage_LoadMusic();
		
		//Reset timer
		Timer_Reset();
		return true;
	}
}

void Stage_Tick(void)
{
	SeamLoad:;
	
	//Tick transition
	
		//Return to menu when start is pressed
		if (pad_state.press & PAD_START && stage.state == StageState_Play)
		{
			stage.trans = StageTrans_Menu;
			Trans_Start();
		}
		else if (pad_state.press & (PAD_START | PAD_CROSS) && stage.state != StageState_Play)
		{
			stage.trans = StageTrans_Reload;
			Trans_Start();
		}
		else if (pad_state.press & PAD_CIRCLE && stage.state != StageState_Play)
		{
			stage.trans = StageTrans_Menu;
			Trans_Start();
		}
	
	
	
	if (Trans_Tick())
	{
		switch (stage.trans)
		{
			case StageTrans_Menu:
				//Load appropriate menu
				Stage_Unload();
				
				LoadScr_Start();
		
					if (stage.stage_id <= StageId_LastVanilla)
					{
						if (stage.story)
							Menu_Load(MenuPage_Story);
						else
							Menu_Load(MenuPage_Freeplay);
					}
					else
					{
						Menu_Load(MenuPage_Credits);
					}
				
				LoadScr_End();
				
				gameloop = GameLoop_Menu;
				return;
			case StageTrans_NextSong:
				//Load next song
				Stage_Unload();
				
				LoadScr_Start();
				Stage_Load(stage.stage_def->next_stage, stage.stage_diff, stage.story);
				LoadScr_End();
				break;
			case StageTrans_Reload:
				//Reload song
				Stage_Unload();
				
				LoadScr_Start();
				Stage_Load(stage.stage_id, stage.stage_diff, stage.story);
				LoadScr_End();
				break;
			case StageTrans_Disconnect:
				return;
		}
	}
	
	switch (stage.state)
	{
		case StageState_Play:
		{  
			Stage_CountDown();

			if (stage.botplay)
			{
				//Draw botplay
				RECT bot_fill = {174, 225, 67, 16};
				RECT_FIXED bot_dst = {FIXED_DEC(-33,1), FIXED_DEC(-60,1), FIXED_DEC(67,1), FIXED_DEC(16,1)};
				
				bot_dst.w = bot_fill.w << FIXED_SHIFT;

				bot_dst.y += stage.noteshakey;
				bot_dst.x += stage.noteshakex;
				
				Stage_DrawTex(&stage.tex_hud0, &bot_fill, &bot_dst, stage.bump);
			}

			if (noteshake) 
			{
				stage.noteshakex = RandomRange(FIXED_DEC(-5,1),FIXED_DEC(5,1));
				stage.noteshakey = RandomRange(FIXED_DEC(-5,1),FIXED_DEC(5,1));
			}
			else
			{
				stage.noteshakex = 0;
				stage.noteshakey = 0;
			}

			//Clear per-frame flags
			stage.flag &= ~(STAGE_FLAG_JUST_STEP | STAGE_FLAG_SCORE_REFRESH);
			
			//Get song position
			boolean playing;
			fixed_t next_scroll;

			//Debug shit, press SELECT with a 2 controller to go to different modes
			if (pad_state_2.press & DEBUG_SWITCH)
			{
				if (stage.debug < 5)
				    stage.debug += 1;
				else 
				    stage.debug = 0;
			}
			switch (stage.debug)
			{
				case 1: //step counter
			        FntPrint("current step is %d\n", stage.song_step);
					break;
				case 2: //Player 1 (bf) position
				    FntPrint("player1 pos X %d Y %d", stage.player->x/1024, stage.player->y/1024);
					Stage_MoveChar();
					break;
				case 3: //Player 2 (dad) position
				    FntPrint("player2 pos X %d Y %d", stage.opponent->x/1024, stage.opponent->y/1024);
					Stage_MoveChar();
					break;
				case 4: //Player 3 (gf) position
				    FntPrint("player3 pos X %d Y %d", stage.gf->x/1024, stage.gf->y/1024);
					Stage_MoveChar();
					break;
				case 5: //Player 4 (Blaster Bro and Tails) postion
					FntPrint("player4 pos X %d Y %d", stage.opponent2->x/1024, stage.opponent2->y/1024);
					Stage_MoveChar();
					break;
			}		
			
				const fixed_t interp_int = FIXED_UNIT * 8 / 75;
				if (stage.note_scroll < 0)
				{
					//Play countdown sequence
					stage.song_time += timer_dt;
					
					//Update song
					if (stage.song_time >= 0)
					{
						//Song has started
						playing = true;
						Audio_PlayXA_Track(stage.stage_def->music_track, 0x40, stage.stage_def->music_channel, 0);
						
						//Update song time
						fixed_t audio_time = (fixed_t)Audio_TellXA_Milli() - stage.offset;
						if (audio_time < 0)
							audio_time = 0;
						stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
						stage.interp_time = 0;
						stage.song_time = stage.interp_ms;
					}
					else
					{
						//Still scrolling
						playing = false;
					}
					
					//Update scroll
					next_scroll = FIXED_MUL(stage.song_time, stage.step_crochet);
				}
				else if (Audio_PlayingXA())
				{
					fixed_t audio_time_pof = (fixed_t)Audio_TellXA_Milli();
					fixed_t audio_time = (audio_time_pof > 0) ? (audio_time_pof - stage.offset) : 0;
					
					if (stage.expsync)
					{
						//Get playing song position
						if (audio_time_pof > 0)
						{
							stage.song_time += timer_dt;
							stage.interp_time += timer_dt;
						}
						
						if (stage.interp_time >= interp_int)
						{
							//Update interp state
							while (stage.interp_time >= interp_int)
								stage.interp_time -= interp_int;
							stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
						}
						
						//Resync
						fixed_t next_time = stage.interp_ms + stage.interp_time;
						if (stage.song_time >= next_time + FIXED_DEC(25,1000) || stage.song_time <= next_time - FIXED_DEC(25,1000))
						{
							stage.song_time = next_time;
						}
						else
						{
							if (stage.song_time < next_time - FIXED_DEC(1,1000))
								stage.song_time += FIXED_DEC(1,1000);
							if (stage.song_time > next_time + FIXED_DEC(1,1000))
								stage.song_time -= FIXED_DEC(1,1000);
						}
					}
					else
					{
						//Old sync
						stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
						stage.interp_time = 0;
						stage.song_time = stage.interp_ms;
					}
					
					playing = true;
					
					//Update scroll
					next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
				}
				else
				{
					//Song has ended
					playing = false;
					stage.song_time += timer_dt;
					
					//Update scroll
					next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
					
					//Transition to menu or next song
					if (stage.story && stage.stage_def->next_stage != stage.stage_id)
					{
						if (Stage_NextLoad())
							goto SeamLoad;
					}
					else
					{
						stage.trans = StageTrans_Menu;
						Trans_Start();
					}
				}
			
			
			RecalcScroll:;
			//Update song scroll and step
			if (next_scroll > stage.note_scroll)
			{
				if (((stage.note_scroll / 12) & FIXED_UAND) != ((next_scroll / 12) & FIXED_UAND))
					stage.flag |= STAGE_FLAG_JUST_STEP;
				stage.note_scroll = next_scroll;
				stage.song_step = (stage.note_scroll >> FIXED_SHIFT);
				if (stage.note_scroll < 0)
					stage.song_step -= 11;
				stage.song_step /= 12;
			}
			
			//Update section
			if (stage.note_scroll >= 0)
			{
				//Check if current section has ended
				u16 end = stage.cur_section->end;
				if ((stage.note_scroll >> FIXED_SHIFT) >= end)
				{
					//Increment section pointer
					stage.cur_section++;
					
					//Update BPM
					u16 next_bpm = stage.cur_section->flag & SECTION_FLAG_BPM_MASK;
					Stage_ChangeBPM(next_bpm, end);
					stage.section_base = stage.cur_section;
					
					//Recalculate scroll based off new BPM
					next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
					goto RecalcScroll;
				}
			}
			
			//Handle bump
			if ((stage.bump = FIXED_UNIT + FIXED_MUL(stage.bump - FIXED_UNIT, FIXED_DEC(95,100))) <= FIXED_DEC(1003,1000))
				stage.bump = FIXED_UNIT;
			stage.sbump = FIXED_UNIT + FIXED_MUL(stage.sbump - FIXED_UNIT, FIXED_DEC(60,100));
			
			if (playing && (stage.flag & STAGE_FLAG_JUST_STEP))
			{
				//Check if screen should bump
				boolean is_bump_step = (stage.song_step & 0xF) == 0;
				
				//Bump screen
				if (is_bump_step)
					stage.bump = FIXED_DEC(100,100);

				//Bump life every 4 steps
				if ((stage.song_step & 0x3) == 0)
					stage.sbump = FIXED_DEC(103,100);
			}
			
			//Scroll camera
			/*
			if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
				Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
			else
				Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
			*/
			Stage_ScrollCamera();
			
			switch (stage.mode)
			{
				case StageMode_Normal:
				case StageMode_Swap:
				{
					//Handle player 1 inputs
					Stage_ProcessPlayer(&stage.player_state[0], &pad_state, playing);
					
					//Handle opponent notes
					u8 opponent_anote = CharAnim_Idle;
					u8 opponent_snote = CharAnim_Idle;
					
					for (Note *note = stage.cur_note;; note++)
					{
						if (note->pos > (stage.note_scroll >> FIXED_SHIFT))
							break;
						
						//Opponent note hits
						if (playing && (note->type & NOTE_FLAG_OPPONENT) && !(note->type & NOTE_FLAG_HIT) && !(note->type & NOTE_FLAG_MINE))
						{
							//Opponent hits note
							stage.player_state[1].arrow_hitan[note->type & 0x3] = stage.step_time;
							Stage_StartVocal();
							if (note->type & NOTE_FLAG_SUSTAIN)
								opponent_snote = note_anims[note->type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0];
							else
								opponent_anote = note_anims[note->type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0];
							note->type |= NOTE_FLAG_HIT;
						}
					}
					if (stage.player_state[1].character->ignore_note == false)
					{
						if (opponent_anote != CharAnim_Idle)
							stage.opponent->set_anim(stage.opponent, opponent_anote);
						else if (opponent_snote != CharAnim_Idle)
							stage.opponent->set_anim(stage.opponent, opponent_snote);
					}
					break;
				}
				case StageMode_2P:
				{
					//Handle player 1 and 2 inputs
					Stage_ProcessPlayer(&stage.player_state[0], &pad_state, playing);
					Stage_ProcessPlayer(&stage.player_state[1], &pad_state_2, playing);
					break;
				}
			}
			
			//Tick note splashes
			ObjectList_Tick(&stage.objlist_splash);
			
			//Draw stage notes
			Stage_DrawNotes();
			
			//Draw note HUD
			RECT note_src = {0, 0, 32, 32};
			RECT_FIXED note_dst = {0, 0 + stage.noteshakey, FIXED_DEC(32,1), FIXED_DEC(32,1)};
			
			for (u8 i = 0; i < 4; i++)
			{
				//BF
				note_dst.x = stage.noteshakex + note_x[i ^ stage.note_swap] - FIXED_DEC(16,1);
				note_dst.y = stage.noteshakey + note_y[i ^ stage.note_swap] - FIXED_DEC(16,1);
				if (stage.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				
				Stage_DrawStrum(i, &note_src, &note_dst);

				Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				
				//Opponent
				note_dst.x = stage.noteshakex + note_x[(i | 0x4) ^ stage.note_swap] - FIXED_DEC(16,1);
				note_dst.y = stage.noteshakey + note_y[(i | 0x4) ^ stage.note_swap] - FIXED_DEC(16,1);
				
				if (stage.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				Stage_DrawStrum(i | 4, &note_src, &note_dst);

				if (stage.middlescroll)
					Stage_BlendTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump, 1);
				else
					Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			}
			
			
			 //Draw information
			for (int i = 0; i < ((stage.mode >= StageMode_2P) ? 2 : 1); i++)
			{
				PlayerState *this = &stage.player_state[i];

				this->accuracy = (this->min_accuracy * 100) / (this->max_accuracy);
				
				//Get string representing number
				if (this->refresh_score)
				{
					if (this->score != 0 && this->score <= 9)
						sprintf(this->score_text, "0000%d0", this->score * stage.max_score / this->max_score);

					else if (this->score >= 10 && this->score <= 99)
						sprintf(this->score_text, "000%d0", this->score * stage.max_score / this->max_score);
					
					else if (this->score >= 100 && this->score <= 999)
						sprintf(this->score_text, "00%d0", this->score * stage.max_score / this->max_score);

					else if (this->score >= 1000 && this->score <= 9999)
					sprintf(this->score_text, "0%d0", this->score * stage.max_score / this->max_score);

					else if (this->score >= 10000 && this->score <= 99999)
					sprintf(this->score_text, "%d0", this->score * stage.max_score / this->max_score);

					else
						sprintf(this->score_text, "000000");
					this->refresh_score = false;
				}
				
				//Draw text
				stage.font_smb1.draw_col(&stage.font_smb1,
					this->score_text,
					(stage.mode == StageMode_2P && i == 0) ? 60 : 10, 
					(stage.downscroll) ? 5 : 216,
					FontAlign_Left,
					(this->score < 0) ? 255 : 0x80,  //if score be negative draw red, else normal color
					(this->score < 0) ?   0 : 0x80,
					(this->score < 0) ?   0 : 0x80
				);
			}
			
			if (stage.mode < StageMode_2P)
			{
				//Perform life checks
				if (stage.player_state[0].character->powerup <= 0)
				{
					//Player has died
					stage.player_state[0].character->powerup = 0;
						
					stage.state = StageState_Dead;
				}
				if (stage.player_state[0].character->powerup > 3)
					stage.player_state[0].character->powerup = 3;
			}
			
			//making black for cover any green
			Gfx_SetClear(0, 0, 0);

			//draw the black square
			RECT bg_src = {0, 0, 160, 120};

			RECT_FIXED bg_dst = {
			FIXED_DEC(-161,1), 
			FIXED_DEC(-120,1), 
			FIXED_DEC(321,1),
			FIXED_DEC(240,1)
		};
		//inverting square and using camera for move all the sprites/bg
			if (stage.downscroll)
			{
			stage.camera.y = FIXED_DEC(31,1);
			bg_dst.y += bg_dst.h;
			bg_dst.h = -bg_dst.h;
			}
			if (stage.stage_id != StageId_S_3)
				Stage_DrawTex(&stage.tex_bg, &bg_src, &bg_dst, stage.bump);
			
			//Draw stage foreground
			if (stage.back->draw_fg != NULL)
				stage.back->draw_fg(stage.back);
			
			//Tick foreground objects
			ObjectList_Tick(&stage.objlist_fg);
			
			//Tick characters
			stage.player->tick(stage.player);
			//just for mx, draw BEHIND the bg if phase 4
			if (mx.phase != 4)
				stage.opponent->tick(stage.opponent);
			
			//Draw stage middle
			if (stage.back->draw_md != NULL)
				stage.back->draw_md(stage.back);
			
			//Tick girlfriend
			if (stage.gf != NULL)
				stage.gf->tick(stage.gf);

			//Tick opponent2
			if (stage.opponent2 != NULL)
				stage.opponent2->tick(stage.opponent2);
			
			//Tick background objects
			ObjectList_Tick(&stage.objlist_bg);
			
			//Draw stage background
			if (stage.back->draw_bg != NULL)
				stage.back->draw_bg(stage.back);

			//Draw MX if phase 4
			if (mx.phase == 4)
				stage.opponent->tick(stage.opponent);
			break;
		}
		case StageState_Dead: //Start BREAK animation and reading extra data from CD
		{
			//Stop music immediately
			Audio_StopXA();
			
			//Unload stage data
			Mem_Free(stage.chart_data);
			stage.chart_data = NULL;
			
			//Free background
			stage.back->free(stage.back);
			stage.back = NULL;
			
			//Free objects
			ObjectList_Free(&stage.objlist_fg);
			ObjectList_Free(&stage.objlist_bg);
			
			//Free opponent and girlfriend
			Stage_SwapChars();
			Character_Free(stage.opponent);
			stage.opponent = NULL;
            Character_Free(stage.opponent2);
			stage.opponent2 = NULL;
			Character_Free(stage.gf);
			stage.gf = NULL;
			
			//Reset stage state
			stage.flag = 0;
			stage.bump = stage.sbump = FIXED_UNIT;
			
			//Change background colour to black
			Gfx_SetClear(0, 0, 0);
			
			stage.song_time = 0;
			
			stage.state = StageState_DeadLoad;
		}
	//Fallthrough
		case StageState_DeadLoad:
		{
			//tick player
			if (stage.song_time < FIXED_UNIT)
				stage.song_time += FIXED_UNIT / 60;
			stage.player->tick(stage.player);
			
			stage.state = StageState_DeadDrop;
			break;
		}
		case StageState_DeadDrop:
		{
			//tick player
			stage.player->tick(stage.player);
			break;
		}
		case StageState_DeadRetry:
		{
			//tick player
			stage.player->tick(stage.player);
			break;
		}
		default:
			break;
	}
}