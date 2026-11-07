/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE.txt). Other notices below are kept as they were. */
#pragma once
#include <stdint.h>
// Update only colours that change; the pixel conversion remains unchanged.
static uint16_t display_palette[8192];
static inline void display_palette_update(unsigned index,uint32_t c){
    display_palette[index]=((c>>11)&31)<<11 | ((c>>19)&31)<<6 | ((c>>3)&31)<<1;
}
