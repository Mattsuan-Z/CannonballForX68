#pragma once

#include "stdint.hpp"

class AssetLoader;

class hwtiles
{
public:
    enum
    {
        LEFT,
        RIGHT,
        CENTRE,
    };

    uint8_t text_ram[0x1000]; // Text RAM
    uint8_t tile_ram[0x10000]; // Tile RAM

    hwtiles(void);
    ~hwtiles(void);

    void init(uint8_t* src_tiles, const bool hires);
    void patch_tiles(AssetLoader* patch);
    void restore_tiles();
    void set_x_clamp(const uint16_t);
    void update_tile_values();
    void render_tile_layer(uint16_t*, uint8_t, uint8_t);
    void render_text_layer(uint16_t*, uint8_t);
    void render_all_tiles(uint16_t*);
#ifndef OR_X68K
    void render_tile_layer_reference(uint16_t*, uint8_t, uint8_t);
    void render_text_layer_reference(uint16_t*, uint8_t);
#endif

    uint8_t text_bank() const {return tile_banks[0];}   // r027: the text layer's tile bank
    uint32_t text_row(unsigned code,unsigned row) const {return tiles[code*8+row];}   // r027b: one 8-pixel row of a tile
// Read-only access to the same decoded tile rows used by the normal renderer.
    uint32_t background_row(uint16_t select,unsigned tx,unsigned ty,unsigned row,uint16_t& colour) const {
        unsigned shift=(ty>=32?8:0)+(tx>=64?4:0),pg=(select>>shift)&15;
        unsigned addr=pg*4096+(ty&31)*128+(tx&63)*2;
        unsigned data=(tile_ram[addr]<<8)|tile_ram[addr+1];
        if(data&0x8000)return 0;
        unsigned code=data&8191;code=(tile_banks[code>>12]*4096+(code&4095))&8191;
        if(!code)return 0;
        colour=((data>>6)&127)*8;
        return tiles[code*8+row];
    }

private:
    int16_t x_clamp;
    
    // S16 Width, ignoring widescreen related scaling.
    uint16_t s16_width_noscale;

    static const int TILES_LENGTH = 0x10000;
    uint32_t tiles[TILES_LENGTH];        // Converted tiles
    uint32_t tiles_backup[TILES_LENGTH]; // Converted tiles (backup without patch)

    uint16_t page[4];
    uint16_t scroll_x[4];
    uint16_t scroll_y[4];

    uint8_t tile_banks[2];

    static const uint16_t NUM_TILES = 0x2000; // Length of graphic asset / 24
    static const uint16_t TILEMAP_COLOUR_OFFSET = 0x1c00;
    
    void (hwtiles::*render8x8_tile_mask)(
        uint16_t *buf,
        uint16_t nTileNumber, 
        uint16_t StartX, 
        uint16_t StartY, 
        uint16_t nTilePalette, 
        uint16_t nColourDepth, 
        uint16_t nMaskColour, 
        uint16_t nPaletteOffset); 
        
    void (hwtiles::*render8x8_tile_mask_clip)(
        uint16_t *buf,
        uint16_t nTileNumber, 
        int16_t StartX, 
        int16_t StartY, 
        uint16_t nTilePalette, 
        uint16_t nColourDepth, 
        uint16_t nMaskColour, 
        uint16_t nPaletteOffset); 
        
    void render8x8_tile_mask_lores(
        uint16_t *buf,
        uint16_t nTileNumber, 
        uint16_t StartX, 
        uint16_t StartY, 
        uint16_t nTilePalette, 
        uint16_t nColourDepth, 
        uint16_t nMaskColour, 
        uint16_t nPaletteOffset); 

    void render8x8_tile_mask_clip_lores(
        uint16_t *buf,
        uint16_t nTileNumber, 
        int16_t StartX, 
        int16_t StartY, 
        uint16_t nTilePalette, 
        uint16_t nColourDepth, 
        uint16_t nMaskColour, 
        uint16_t nPaletteOffset);
        
    void render8x8_tile_mask_hires(
        uint16_t *buf,
        uint16_t nTileNumber, 
        uint16_t StartX, 
        uint16_t StartY, 
        uint16_t nTilePalette, 
        uint16_t nColourDepth, 
        uint16_t nMaskColour, 
        uint16_t nPaletteOffset); 
        
    void render8x8_tile_mask_clip_hires(
        uint16_t *buf,
        uint16_t nTileNumber, 
        int16_t StartX, 
        int16_t StartY, 
        uint16_t nTilePalette, 
        uint16_t nColourDepth, 
        uint16_t nMaskColour, 
        uint16_t nPaletteOffset);
        
    inline void set_pixel_x4(uint16_t *buf, uint32_t data);
};