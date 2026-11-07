/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE.txt). Other notices below are kept as they were. */
// Offline reference methods: original algorithms retained for pixel comparison.
// Copyright Aaron Giles. All rights reserved. Based on MAME source code.
// See CANNONBALL_LICENSE.txt and the notices in vendor/main/hwvideo.
#ifndef OR_X68K
#include "video.hpp"
#include "frontend/config.hpp"
bool render_reference_mode=false;
void hwtiles::render_tile_layer_reference(uint16_t* buf, uint8_t page_index, uint8_t priority_draw)
{
    int16_t Colour, x, y, Priority = 0;

    uint16_t ActPage = 0;
    uint16_t EffPage = page[page_index];
    uint16_t xScroll = scroll_x[page_index];
    uint16_t yScroll = scroll_y[page_index];

    // Need to support this at each row/column
    if ((xScroll & 0x8000) != 0)
        xScroll = (text_ram[0xf80 + (0x40 * page_index) + 0] << 8) | text_ram[0xf80 + (0x40 * page_index) + 1];
    if ((yScroll & 0x8000) != 0)
        yScroll = (text_ram[0xf16 + (0x40 * page_index) + 0] << 8) | text_ram[0xf16 + (0x40 * page_index) + 1];

    for (int my = 0; my < 64; my++) 
    {
        for (int mx = 0; mx < 128; mx++) 
        {
            if (my < 32 && mx < 64)                    // top left
                ActPage = (EffPage >> 0) & 0x0f;
            if (my < 32 && mx >= 64)                   // top right
                ActPage = (EffPage >> 4) & 0x0f;
            if (my >= 32 && mx < 64)                   // bottom left
                ActPage = (EffPage >> 8) & 0x0f;
            if (my >= 32 && mx >= 64)                  // bottom right page
                ActPage = (EffPage >> 12) & 0x0f;

            uint32_t TileIndex = 64 * 32 * 2 * ActPage + ((2 * 64 * my) & 0xfff) + ((2 * mx) & 0x7f);

            uint16_t Data = (tile_ram[TileIndex + 0] << 8) | tile_ram[TileIndex + 1];

            Priority = (Data >> 15) & 1;

            if (Priority == priority_draw) 
            {
                uint32_t Code = Data & 0x1fff;
                Code = tile_banks[Code / 0x1000] * 0x1000 + Code % 0x1000;
                Code &= (NUM_TILES - 1);

                if (Code == 0) continue;

                Colour = (Data >> 6) & 0x7f;

                x = 8 * mx;
                y = 8 * my;

                // We take into account the internal screen resolution here
                // to account for widescreen mode.
                x -= (x_clamp - xScroll) & 0x3ff;

                if (x < -x_clamp)
                    x += 1024;

                y -= yScroll & 0x1ff;

                if (y < -288)
                    y += 512;

                uint16_t ColourOff = TILEMAP_COLOUR_OFFSET;
                if (Colour >= 0x20)
					ColourOff = 0x100 | TILEMAP_COLOUR_OFFSET;
                if (Colour >= 0x40)
					ColourOff = 0x200 | TILEMAP_COLOUR_OFFSET;
                if (Colour >= 0x60)
					ColourOff = 0x300 | TILEMAP_COLOUR_OFFSET;

                if (x > 7 && x < (s16_width_noscale - 8) && y > 7 && y <= (S16_HEIGHT - 8))
                    (this->*render8x8_tile_mask)(buf, Code, x, y, Colour, 3, 0, ColourOff);
                else if (x > -8 && x < s16_width_noscale && y > -8 && y < S16_HEIGHT)
					(this->*render8x8_tile_mask_clip)(buf, Code, x, y, Colour, 3, 0, ColourOff);
            } // end priority check
        }
    } // end for loop
}

void hwtiles::render_text_layer_reference(uint16_t* buf, uint8_t priority_draw)
{
    uint16_t mx, my, Code, Colour, x, y, Priority, TileIndex = 0;

    for (my = 0; my < 32; my++) 
    {
        for (mx = 0; mx < 64; mx++) 
        {
            Code = (text_ram[TileIndex + 0] << 8) | text_ram[TileIndex + 1];
            Priority = (Code >> 15) & 1;

            if (Priority == priority_draw) 
            {
                Colour = (Code >> 9) & 0x07;
                Code &= 0x1ff;
                Code += tile_banks[0] * 0x1000;
                Code &= (NUM_TILES - 1);

                if (Code != 0) 
                {
                    x = 8 * mx;
                    y = 8 * my;

                    x -= 192;

                    // We also adjust the text layer for wide-screen below. But don't allow painting in the 
                    // wide-screen areas to avoid graphical glitches.
                    if (x > 7 && x < (s16_width_noscale - 8) && y > 7 && y <= (S16_HEIGHT - 8))
                        (this->*render8x8_tile_mask)(buf, Code, x + config.s16_x_off, y, Colour, 3, 0, TILEMAP_COLOUR_OFFSET);
                    else if (x > -8 && x < s16_width_noscale && y >= 0 && y < S16_HEIGHT) 
                        (this->*render8x8_tile_mask_clip)(buf, Code, x + config.s16_x_off, y, Colour, 3, 0, TILEMAP_COLOUR_OFFSET);
                }
            }
            TileIndex += 2;
        }
    }
}


void HWRoad::render_foreground_lores_reference(uint16_t* pixels)
{
    int x, y;
    uint16_t* roadram = ramBuff;
    
    for (y = 0; y < S16_HEIGHT; y++) 
    {
        uint16_t color_table[32];

        static const uint8_t priority_map[2][8] =
        {
            { 0x80,0x81,0x81,0x87,0,0,0,0x00 },
            { 0x81,0x81,0x81,0x8f,0,0,0,0x80 }
        };

        const uint32_t data0 = roadram[0x000 + y];
        const uint32_t data1 = roadram[0x100 + y];

        // if both roads are low priority, skip
        if (((data0 & 0x800) != 0) && ((data1 & 0x800) != 0))
            continue;

        uint16_t* pPixel = pixels + (y * config.s16_width);
        int32_t hpos0, hpos1, color0, color1;
        int32_t control = road_control & 3;

        uint8_t *src0, *src1;
        int32_t bgcolor; // 8 bits

        // get road 0 data
        src0   = ((data0 & 0x800) != 0) ? roads + 256 * 2 * 512 : (roads + (0x000 + ((data0 >> 1) & 0xff)) * 512);
        hpos0  = roadram[0x200 + (((road_control & 4) != 0) ? y : (data0 & 0x1ff))] & 0xfff;
        color0 = roadram[0x600 + (((road_control & 4) != 0) ? y : (data0 & 0x1ff))];

        // get road 1 data
        src1   = ((data1 & 0x800) != 0) ? roads + 256 * 2 * 512 : (roads + (0x100 + ((data1 >> 1) & 0xff)) * 512);
        hpos1  = roadram[0x400 + (((road_control & 4) != 0) ? (0x100 + y) : (data1 & 0x1ff))] & 0xfff;
        color1 = roadram[0x600 + (((road_control & 4) != 0) ? (0x100 + y) : (data1 & 0x1ff))];

        // determine the 5 colors for road 0
        color_table[0x00] = color_offset1 ^ 0x00 ^ ((color0 >> 0) & 1);
        color_table[0x01] = color_offset1 ^ 0x02 ^ ((color0 >> 1) & 1);
        color_table[0x02] = color_offset1 ^ 0x04 ^ ((color0 >> 2) & 1);
        bgcolor = (color0 >> 8) & 0xf;
        color_table[0x03] = ((data0 & 0x200) != 0) ? color_table[0x00] : (color_offset2 ^ 0x00 ^ bgcolor);
        color_table[0x07] = color_offset1 ^ 0x06 ^ ((color0 >> 3) & 1);

        // determine the 5 colors for road 1
        color_table[0x10] = color_offset1 ^ 0x08 ^ ((color1 >> 4) & 1);
        color_table[0x11] = color_offset1 ^ 0x0a ^ ((color1 >> 5) & 1);
        color_table[0x12] = color_offset1 ^ 0x0c ^ ((color1 >> 6) & 1);
        bgcolor = (color1 >> 8) & 0xf;
        color_table[0x13] = ((data1 & 0x200) != 0) ? color_table[0x10] : (color_offset2 ^ 0x10 ^ bgcolor);
        color_table[0x17] = color_offset1 ^ 0x0e ^ ((color1 >> 7) & 1);

        // Shift road dependent on whether we are in widescreen mode or not
        uint16_t s16_x = 0x5f8 + config.s16_x_off;

        // draw the road
        switch (control) 
        {
            case 0:
                if (data0 & 0x800)
                    continue;
                hpos0 = (hpos0 - (s16_x + x_offset)) & 0xfff;
                for (x = 0; x < config.s16_width; x++) 
                {
                    int pix0 = (hpos0 < 0x200) ? src0[hpos0] : 3;
                    pPixel[x] = color_table[0x00 + pix0];
                    hpos0 = (hpos0 + 1) & 0xfff;
                }
                break;

            case 1:
                hpos0 = (hpos0 - (s16_x + x_offset)) & 0xfff;
                hpos1 = (hpos1 - (s16_x + x_offset)) & 0xfff;
                for (x = 0; x < config.s16_width; x++) 
                {
                    int pix0 = (hpos0 < 0x200) ? src0[hpos0] : 3;
                    int pix1 = (hpos1 < 0x200) ? src1[hpos1] : 3;
                    if (((priority_map[0][pix0] >> pix1) & 1) != 0)
                        pPixel[x] = color_table[0x10 + pix1];
                    else
                        pPixel[x] = color_table[0x00 + pix0];

                    hpos0 = (hpos0 + 1) & 0xfff;
                    hpos1 = (hpos1 + 1) & 0xfff;
                }
                break;

            case 2:
                hpos0 = (hpos0 - (s16_x + x_offset)) & 0xfff;
                hpos1 = (hpos1 - (s16_x + x_offset)) & 0xfff;
                for (x = 0; x < config.s16_width; x++) 
                {
                    int pix0 = (hpos0 < 0x200) ? src0[hpos0] : 3;
                    int pix1 = (hpos1 < 0x200) ? src1[hpos1] : 3;
                    if (((priority_map[1][pix0] >> pix1) & 1) != 0)
                        pPixel[x] = color_table[0x10 + pix1];
                    else
                        pPixel[x] = color_table[0x00 + pix0];

                    hpos0 = (hpos0 + 1) & 0xfff;
                    hpos1 = (hpos1 + 1) & 0xfff;
                }
                break;

            case 3:
                if (data1 & 0x800)
                    continue;
                hpos1 = (hpos1 - (s16_x + x_offset)) & 0xfff;
                for (x = 0; x < config.s16_width; x++) 
                {
                    int pix1 = (hpos1 < 0x200) ? src1[hpos1] : 3;
                    pPixel[x] = color_table[0x10 + pix1];
                    hpos1 = (hpos1 + 1) & 0xfff;
                }
                break;
            } // end switch
    } // end for
}


#endif
