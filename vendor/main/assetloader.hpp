/***************************************************************************
    Binary File Loader. 
    
    Handles loading an individual binary file to memory.
    Supports reading bytes, words and longs from this area of memory.

    Copyright Chris White.
    See license.txt for more details.
***************************************************************************/

#pragma once

class AssetLoader
{

public:
    enum {NORMAL = 1, INTERLEAVE2 = 2, INTERLEAVE4 = 4};

    uint8_t* asset;

    // Size of asset
    uint32_t length;

    // Successfully loaded
    bool loaded;

    AssetLoader();
    ~AssetLoader();
    void init(uint32_t);

    int (AssetLoader::*load)(const char*, const int, const int, const int, const uint8_t, const bool);
    int load_asset(const char* filename, const int offset, const int length, const int expected_crc, const uint8_t mode = NORMAL, const bool verbose = true);
    int load_crc32(const char* debug, const int offset, const int length, const int expected_crc, const uint8_t mode = NORMAL, const bool verbose = true);
    int load_binary(const char* filename);
    void unload(void);

    // ----------------------------------------------------------------------------
    // Used by translated 68000 Code
    // ----------------------------------------------------------------------------

    inline uint32_t read32(uint32_t* addr)
    {    
        uint32_t data = (asset[*addr] << 24) | (asset[*addr+1] << 16) | (asset[*addr+2] << 8) | (asset[*addr+3]);
        *addr += 4;
        return data;
    }

    inline uint16_t read16(uint32_t* addr)
    {
        uint16_t data = (asset[*addr] << 8) | (asset[*addr+1]);
        *addr += 2;
        return data;
    }

    inline uint8_t read8(uint32_t* addr)
    {
        return asset[(*addr)++]; 
    }

    inline uint32_t read32(uint32_t addr)
    {    
        return (asset[addr] << 24) | (asset[addr+1] << 16) | (asset[addr+2] << 8) | asset[addr+3];
    }

    inline uint16_t read16(uint32_t addr)
    {
        return (asset[addr] << 8) | asset[addr+1];
    }

    inline uint8_t read8(uint32_t addr)
    {
        return asset[addr];
    }

    // ----------------------------------------------------------------------------
    // Used by translated Z80 Code
    // Note that the endian is reversed compared with the 68000 code.
    // ----------------------------------------------------------------------------

    inline uint16_t read16(uint16_t* addr)
    {
        uint16_t data = (asset[*addr+1] << 8) | (asset[*addr]);
        *addr += 2;
        return data;
    }

    inline uint8_t read8(uint16_t* addr)
    {
        return asset[(*addr)++]; 
    }

    inline uint16_t read16(uint16_t addr)
    {
        return (asset[addr+1] << 8) | asset[addr];
    }

    inline uint8_t read8(uint16_t addr)
    {
        return asset[addr];
    }

private:
    int create_map();
    int filesize(const char* filename);
};