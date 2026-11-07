/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE / LICENSE.txt). Other notices below are kept as they were. */
#pragma once
#include <stdint.h>
// Keep the 60-step engine timeline; only select which complete frames to draw.
struct RenderSchedule {
    bool draw30 = true;
    uint32_t steps[2] = {0,0};
    uint32_t logic[2] = {0,0};
    uint32_t draws[2] = {0,0};
    unsigned mode() const {return draw30?0:1;}
    bool should_draw(unsigned step) const {return !draw30 || (step&1)!=0;}
    void record(bool logical,bool drawn){unsigned m=mode();steps[m]++;if(logical)logic[m]++;if(drawn)draws[m]++;}
};
static RenderSchedule render_schedule;
