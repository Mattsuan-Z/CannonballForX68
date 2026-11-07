/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE.txt). Other notices below are kept as they were. */
// Cannonball for x68: ROM file loader (replaces the PC version's assetloader.cpp).
// As in Cannonball, the program uses the user's own OutRun (Revision B) ROM files; nothing
// from the ROMs is distributed with it. Each file is read from the ROM folder by name and its
// CRC32 is checked. Messages follow Cannonball's. If any file is missing or wrong, the
// program reports them and stops (see main.cpp).
#include "stdint.hpp"
#include "assetloader.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#ifdef OR_X68K
const char* cbx68_rom_path="ROMS\\";
#else
const char* cbx68_rom_path=std::getenv("CBX68_ROMS")?std::getenv("CBX68_ROMS"):"roms/";
#endif
unsigned cbx68_rom_errors=0;
static uint32_t crc_table[256];static bool crc_ready=false;
static uint32_t crc32_of(const uint8_t*p,size_t n){
    if(!crc_ready){for(uint32_t i=0;i<256;i++){uint32_t c=i;for(int k=0;k<8;k++)c=c&1?0xedb88320UL^(c>>1):c>>1;crc_table[i]=c;}crc_ready=true;}
    uint32_t c=0xffffffffUL;for(size_t i=0;i<n;i++)c=crc_table[(c^p[i])&255]^(c>>8);return c^0xffffffffUL;
}
// ---------------------------------------------------------------------------------------------
// Cannonball's default (data.crc32 = 1): the files in the ROM folder are found by their CRC32,
// so their names do not matter. The folder is listed once; files of a ROM size are checked.
// ---------------------------------------------------------------------------------------------
#ifdef OR_X68K
// Human68k DOS calls _FILES ($FF4E) and _NFILES ($FF4F)
static int dos_files(void*buf,const char*name,int atr){
    register int d0 __asm__("d0");
    __asm__ volatile("move.w %3,-(%%sp)\n\tmove.l %2,-(%%sp)\n\tmove.l %1,-(%%sp)\n\t.dc.w 0xff4e\n\tlea 10(%%sp),%%sp"
        :"=r"(d0):"r"(buf),"r"(name),"r"(atr):"d1","d2","a0","a1","a2","memory","cc");
    return d0;}
static int dos_nfiles(void*buf){
    register int d0 __asm__("d0");
    __asm__ volatile("move.l %1,-(%%sp)\n\t.dc.w 0xff4f\n\taddq.l #4,%%sp":"=r"(d0):"r"(buf):"d1","d2","a0","a1","a2","memory","cc");
    return d0;}
#else
#include <dirent.h>
#endif
struct RomFile{uint32_t crc;char name[64];};
static RomFile rom_files[96];static unsigned rom_file_count=0;static bool rom_listed=false;
static void rom_path_of(char*out,size_t n,const char*name);
static void rom_consider(const char*name,long size){
    if(rom_file_count>=96||(size!=0x8000&&size!=0x10000&&size!=0x20000))return;
    char path[256];rom_path_of(path,sizeof path,name);FILE*f=fopen(path,"rb");if(!f)return;
    static uint8_t buf[0x20000];size_t got=fread(buf,1,sizeof buf,f);fclose(f);if(long(got)!=size)return;
    RomFile&r=rom_files[rom_file_count++];r.crc=crc32_of(buf,got);snprintf(r.name,sizeof r.name,"%s",name);
}
static void rom_list(){
    if(rom_listed)return;rom_listed=true;
#ifdef OR_X68K
    char pattern[256];rom_path_of(pattern,sizeof pattern,"*.*");
    static uint8_t fb[64];
    for(int r=dos_files(fb,pattern,0x20);r>=0;r=dos_nfiles(fb)){
        const long size=long((uint32_t(fb[26])<<24)|(uint32_t(fb[27])<<16)|(uint32_t(fb[28])<<8)|fb[29]);
        rom_consider((const char*)fb+30,size);}
#else
    char dir[256];snprintf(dir,sizeof dir,"%s",cbx68_rom_path);if(!*dir)snprintf(dir,sizeof dir,".");
    if(DIR*d=opendir(dir)){while(dirent*e=readdir(d)){char path[256];rom_path_of(path,sizeof path,e->d_name);
        if(FILE*f=fopen(path,"rb")){fseek(f,0,SEEK_END);long n=ftell(f);fclose(f);rom_consider(e->d_name,n);}}closedir(d);}
#endif
}
static void rom_path_of(char*out,size_t n,const char*name){
    const size_t pl=strlen(cbx68_rom_path);const char last=pl?cbx68_rom_path[pl-1]:'\\';
#ifdef OR_X68K
    const char*sep=(!pl||last=='\\'||last=='/'||last==':')?"":"\\";
#else
    const char*sep=(!pl||last=='/'||last=='\\')?"":"/";
#endif
    snprintf(out,n,"%s%s%s",cbx68_rom_path,sep,name);}
AssetLoader::AssetLoader():asset(NULL),length(0),loaded(false){}
AssetLoader::~AssetLoader(){delete[] asset;}
void AssetLoader::init(uint32_t n){length=n;asset=new uint8_t[n]();load=&AssetLoader::load_crc32;}   // Cannonball's default: by CRC32
void AssetLoader::unload(){delete[]asset;asset=NULL;}
int AssetLoader::load_asset(const char*name,const int off,const int len,const int crc,const uint8_t mode,const bool verbose){
    char path[256];rom_path_of(path,sizeof path,name);
    FILE*f=fopen(path,"rb");
    if(!f){if(verbose)printf("cannot open asset: %s\n",path);++cbx68_rom_errors;return 1;}
    uint8_t*buf=new uint8_t[len];size_t got=fread(buf,1,len,f);fclose(f);
    uint32_t found=crc32_of(buf,got);
    if(got!=size_t(len)||found!=uint32_t(crc)){
        if(verbose)printf("%s has incorrect checksum.\nExpected: %lu Found: %lu\n",name,(unsigned long)uint32_t(crc),(unsigned long)found);
        delete[]buf;++cbx68_rom_errors;return 1;}
    if(off<0||unsigned(off)+unsigned(len-1)*mode>=length){delete[]buf;++cbx68_rom_errors;return 1;}
    for(int i=0;i<len;i++)asset[off+i*mode]=buf[i];
    delete[]buf;loaded=true;return 0;
}
int AssetLoader::load_crc32(const char*debug,const int off,const int len,const int crc,const uint8_t mode,const bool verbose){
    rom_list();
    for(unsigned i=0;i<rom_file_count;i++)if(rom_files[i].crc==uint32_t(crc))return load_asset(rom_files[i].name,off,len,crc,mode,verbose);
    if(verbose)printf("Unable to locate asset in path: %s possible name: %s crc32: 0x%lx\n",cbx68_rom_path,debug,(unsigned long)uint32_t(crc));
    ++cbx68_rom_errors;return 1;
}
int AssetLoader::load_binary(const char*name){
    FILE*f=fopen(name,"rb");if(!f)return 1;fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);if(n<=0){fclose(f);return 1;}
    init(n);loaded=fread(asset,1,n,f)==unsigned(n);fclose(f);return loaded?0:1;
}
