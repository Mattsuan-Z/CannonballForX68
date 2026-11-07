/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE / LICENSE.txt). Other notices below are kept as they were. */
#pragma once
#include <stdint.h>
#include <stdio.h>
enum DrawPart { DRAW_SETUP, DRAW_FILL, DRAW_BG, DRAW_FG, DRAW_ROAD, DRAW_SPRITES, DRAW_HUD, DRAW_PARTS };
struct DrawSamples {
    uint32_t frames=0,scene_frames=0,total_cs=0;
    uint32_t cs[DRAW_PARTS]={};
    uint32_t calls[DRAW_PARTS]={};
};
extern DrawSamples draw_samples[2][3];
extern uint32_t draw_profile_errors;
void draw_profile_begin(unsigned mode,int game_state,bool scene_enabled);
void draw_profile_mark(unsigned completed_part);
void draw_profile_end();
void draw_profile_print(FILE* output,bool detailed);
