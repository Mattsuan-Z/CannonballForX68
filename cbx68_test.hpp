/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE / LICENSE.txt). Other notices below are kept as they were. */
// Cannonball for x68: DISPLAY TEST (CBX68 TEST). Needs no ROM: the road, background, sprite and
// font pictures are simple shapes made here, and the game itself (Cannonball) does not run.
// It drives the same X68000 display code as the game (raster road, 4-bit background planes,
// sprites reused down the screen, text plane), so it shows whether the display works on a
// machine or emulator. Nothing here is taken from or imitates the arcade game.
#pragma once
#include <cmath>
static bool cbx68_test=false;
static unsigned cbx68_test_key=0;
static inline uint16_t cbx68_rgb(unsigned r,unsigned g,unsigned b){return uint16_t(((g&31)<<11)|((r&31)<<6)|((b&31)<<1));}
// 5x7 font for the text (own design): 7 rows of 5 bits per character
static const char CBX68_FONT_CHARS[]=" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-:./";
static const uint8_t CBX68_FONT[][7]={
 {0,0,0,0,0,0,0},
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},{31,16,16,30,16,16,31},
 {31,16,16,30,16,16,16},{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},{7,2,2,2,2,18,12},
 {17,18,20,24,20,18,17},{16,16,16,16,16,16,31},{17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
 {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
 {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},{17,17,10,4,4,4,4},
 {31,1,2,4,8,16,31},
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{31,2,4,2,1,17,14},{2,6,10,18,31,2,2},
 {31,16,30,1,1,17,14},{6,8,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},{14,17,17,15,1,2,12},
 {0,0,0,31,0,0,0},{0,12,12,0,12,12,0},{0,0,0,0,0,12,12},{1,2,2,4,8,8,16}};
static unsigned cbx68_char_tile(char c){const char*p=strchr(CBX68_FONT_CHARS,c);return p&&c?unsigned(p-CBX68_FONT_CHARS):0;}
// tile codes: font 1..41 (palette group 0), far layer group 16 (codes 1024+), near layer group 20 (codes 1280+)
enum {TF_BODY=1024+1,TF_SNOW=1024+2,TN_HILL=1280+1,TN_TREE=1280+2,TN_LIGHT=1280+3};
static void cbx68_tile_set(uint8_t*tiles,unsigned t,const uint8_t*rows8x8){   // 8 rows x 8 pixels (0..7)
 for(unsigned r=0;r<8;r++){uint8_t p0=0,p1=0,p2=0;for(unsigned x=0;x<8;x++){unsigned c=rows8x8[r*8+x];uint8_t bit=uint8_t(0x80>>x);if(c&1)p0|=bit;if(c&2)p1|=bit;if(c&4)p2|=bit;}
  tiles[t*8+r]=p0;tiles[0x10000+t*8+r]=p1;tiles[0x20000+t*8+r]=p2;}}
static void cbx68_test_assets(){
 // road: 256 widths, centre at x=256: 0 surface, 1 edge line, 2 centre line, 3 off the road
 assets.road.init(0x10000);uint8_t*road=assets.road.asset;
 for(unsigned r=0;r<256;r++){const unsigned hw=8+r*230/255,edge=hw/10+1,mid=hw/40+1;
  for(unsigned x=0;x<512;x++){unsigned d=x>256?x-256:256-x,c=d>=hw?3:(d+edge>=hw?1:(d<mid?2:0));
   if(c&1)road[r*64+x/8]|=uint8_t(0x80>>(x&7));if(c&2)road[0x4000+r*64+x/8]|=uint8_t(0x80>>(x&7));}}
 memcpy(road+0x8000,road,0x8000);
 // tiles: font and background pieces
 assets.tiles.init(0x30000);uint8_t*tiles=assets.tiles.asset;uint8_t px[64];
 for(unsigned i=1;i<sizeof CBX68_FONT/7;i++){memset(px,0,64);for(unsigned y=0;y<7;y++)for(unsigned x=0;x<5;x++)if(CBX68_FONT[i][y]&(16>>x)){px[y*8+x+1]=1;px[(y+1)*8+x+2]=px[(y+1)*8+x+2]?px[(y+1)*8+x+2]:2;}
  for(unsigned y=0;y<7;y++)for(unsigned x=0;x<5;x++)if(CBX68_FONT[i][y]&(16>>x))px[y*8+x+1]=1;
  cbx68_tile_set(tiles,i,px);}
 for(unsigned k=0;k<64;k++)px[k]=1;cbx68_tile_set(tiles,TF_BODY,px);
 for(unsigned k=0;k<64;k++)px[k]=(k/8<3)?2:1;cbx68_tile_set(tiles,TF_SNOW,px);
 for(unsigned k=0;k<64;k++)px[k]=1;cbx68_tile_set(tiles,TN_HILL,px);
 for(unsigned k=0;k<64;k++){unsigned x=k%8,y=k/8;px[k]=(y<6&&x+1>=unsigned(4-int(y)/2)&&x<=unsigned(4+int(y)/2))?2:1;}cbx68_tile_set(tiles,TN_TREE,px);
 for(unsigned k=0;k<64;k++)px[k]=((k%8+k/8)&3)?1:3;cbx68_tile_set(tiles,TN_LIGHT,px);
 // sprites: 4 shapes of 32x32, 5 words a row (the 5th ends the row), bank 0
 assets.sprites.init(0x100000);uint8_t*spr=assets.sprites.asset;
 auto put=[&](unsigned word,uint32_t w){spr[word*4]=uint8_t(w);spr[word*4+1]=uint8_t(w>>8);spr[word*4+2]=uint8_t(w>>16);spr[word*4+3]=uint8_t(w>>24);};
 for(unsigned s=0;s<4;s++)for(unsigned y=0;y<32;y++){
  for(unsigned wx=0;wx<4;wx++){uint32_t w=0;for(unsigned i=0;i<8;i++){int x=int(wx*8+i),dx=x-16,dy=int(y)-16;unsigned c=0;
    if(s==0){int d=dx*dx+dy*dy;c=d<225?(d<64?1:d<144?2:3):0;}                                   // ball
    else if(s==1){c=(y<24&&std::abs(dx)<=int(y)/2+1)?(((y/4)&1)?4:5):(y>=24&&std::abs(dx)<3)?6:0;} // tree
    else if(s==2){c=(y<20)?((x<2||x>29||y<2||y>17)?7:(((x/4+y/4)&1)?8:9)):(std::abs(dx)<2?7:0);}   // sign
    else {c=(std::abs(dx)+std::abs(dy)<15)?((std::abs(dx)+std::abs(dy)<8)?10:11):0;}             // diamond
    w=(w<<4)|c;}
   put(s*256+y*5+wx,w);}
  put(s*256+y*5+4,0xffffffffu);}
 // palette (X68000 colour words)
 for(unsigned i=0;i<16;i++){display_palette[0x400+i]=cbx68_rgb(i&1?11:13,i&1?11:13,i&1?12:14);}
 display_palette[0x402]=display_palette[0x403]=cbx68_rgb(31,31,31);display_palette[0x404]=display_palette[0x405]=cbx68_rgb(31,28,8);
 display_palette[0x406]=cbx68_rgb(28,6,6);display_palette[0x407]=cbx68_rgb(31,31,31);
 for(unsigned i=0;i<16;i++)display_palette[0x420+i]=i&1?cbx68_rgb(6,20,6):cbx68_rgb(8,24,8);
 for(unsigned i=0;i<128;i++)display_palette[0x780+i]=cbx68_rgb(4+i/8,10+i/6,31);
 display_palette[1]=cbx68_rgb(31,31,31);display_palette[2]=cbx68_rgb(4,4,8);                 // text group 0
 display_palette[9]=cbx68_rgb(31,30,6);display_palette[10]=cbx68_rgb(8,6,2);                 // text group 1
 display_palette[17]=cbx68_rgb(8,30,31);display_palette[18]=cbx68_rgb(2,6,10);               // text group 2
 display_palette[16*8+1]=cbx68_rgb(14,12,20);display_palette[16*8+2]=cbx68_rgb(30,30,31);    // far: mountains, snow
 display_palette[20*8+1]=cbx68_rgb(6,16,6);display_palette[20*8+2]=cbx68_rgb(3,10,3);display_palette[20*8+3]=cbx68_rgb(20,28,10);
 const uint16_t sp[16]={0,cbx68_rgb(31,24,8),cbx68_rgb(28,14,4),cbx68_rgb(18,6,2),cbx68_rgb(4,22,6),cbx68_rgb(2,14,4),cbx68_rgb(14,8,4),
  cbx68_rgb(31,31,31),cbx68_rgb(4,10,28),cbx68_rgb(28,4,8),cbx68_rgb(20,8,30),cbx68_rgb(31,20,31),0,0,0,0};
 for(unsigned b=0;b<4;b++)for(unsigned n=0;n<16;n++){uint16_t c=sp[n];if(b&&n)c=uint16_t(((c>>1)&0x7bde)+(b*0x0842));display_palette[0x800+b*16+n]=n?c:0;}
}
static void cbx68_text(unsigned row,unsigned col,unsigned colour,const char*s){
 uint8_t*tr=video.tile_layer->text_ram;
 for(unsigned i=0;s[i]&&col+i<40;i++){unsigned a=row*128+(col+i+24)*2;unsigned code=0x8000|(colour<<9)|cbx68_char_tile(s[i]);tr[a]=uint8_t(code>>8);tr[a+1]=uint8_t(code);}}
static void cbx68_word(unsigned a,unsigned v){uint8_t*tr=video.tile_layer->text_ram;tr[a]=uint8_t(v>>8);tr[a+1]=uint8_t(v);}
static void cbx68_test_setup(){
 // background: far layer pages 4..7 (mountains), near layer pages 0..3 (hills and trees); ground at layer row 255
 uint8_t*tram=video.tile_layer->tile_ram;memset(tram,0,0x8000);
 auto cell=[&](unsigned layer_first_page,unsigned cx,unsigned cy,unsigned code){unsigned pg=layer_first_page+(cy>=32?2:0)+(cx>=64?1:0);unsigned a=pg*4096+(cy&31)*128+(cx&63)*2;tram[a]=uint8_t(code>>8);tram[a+1]=uint8_t(code);};
 for(unsigned cx=0;cx<128;cx++){
  unsigned hf=2+unsigned(6+5*std::sin(cx*0.11)+3*std::sin(cx*0.37+1.0));        // mountains, cells high
  for(unsigned k=0;k<hf;k++)cell(4,cx,31-k,k+1==hf?TF_SNOW:TF_BODY);
  unsigned hn=1+unsigned(2+2*std::sin(cx*0.23+2.0));
  for(unsigned k=0;k<hn;k++)cell(0,cx,31-k,k+1==hn?((cx%3)?TN_TREE:TN_LIGHT):TN_HILL);
 }
 cbx68_word(0xe80,0x3210);cbx68_word(0xe82,0x7654);
 memset(video.tile_layer->text_ram,0,0xe00);
 cbx68_text(1,4,2,"CANNONBALL FOR X68  DISPLAY TEST");
 cbx68_text(2,8,0,"NO ROM - SHAPES MADE HERE");
 cbx68_text(26,1,1,"ESC: QUIT  1-4: DISPLAY");
 hwroad.road_control=4;video.sprite_layer->set_x_clip(false);
}
// One frame: the road bends and rises, both background layers scroll, shapes come up the road.
static void cbx68_test_frame(int frame){
 const double t=frame/60.0;
 const int horizon=110+int(18*std::sin(t*0.7));
 const double curve=std::sin(t*0.45);
 uint16_t*rb=hwroad.ramBuff;const int bend=int(curve*130);   // integer per line (no float in the loop)
 for(int y=0;y<224;y++){
  if(y<horizon){rb[y]=uint16_t(0x800|unsigned(std::min(127,y*127/std::max(1,horizon))));rb[0x200+y]=0x5f8;rb[0x600+y]=0;}
  else{const int depth=std::max(1,223-horizon),dy=y-horizon;const unsigned row=unsigned(std::min(255,dy*255/depth));
   const int far=((depth-dy)<<8)/depth;                                  // 256 at the horizon .. 0 at the bottom
   rb[y]=uint16_t(row<<1);rb[0x200+y]=uint16_t((0x5f8+96+((bend*far*far)>>16))&0xfff);
   const bool band=(((10240/(dy+1))+frame*1024/60)>>8)&1;rb[0x600+y]=uint16_t(band?0x010f:0x0000);}
  rb[0x100+y]=0x800;
 }
 // background scroll: far slowly, near faster, with the curve; rows follow the horizon
 const int fx=int(t*8+curve*40),nx=int(t*30+curve*120);
 cbx68_word(0xe9a,unsigned(192-fx)&0x3ff);cbx68_word(0xe98,unsigned(192-nx)&0x3ff);
 cbx68_word(0xe92,unsigned(256-horizon)&511);cbx68_word(0xe90,unsigned(256-horizon)&511);
 // shapes on both sides of the road, far ones first (near ones are drawn over them)
 unsigned n=0;const int count=24;
 for(int k=count-1;k>=0;k--){
  const double p=std::fmod(t*0.35+k/double(count),1.0);          // 0 far .. 1 near
  const double s=0.05+p*p*2.6;const int rows=std::max(2,std::min(256,int(32*s)));
  const int depth=223-horizon;const int ybot=horizon+int(depth*p*p);if(ybot>=224)continue;
  const double far=1.0-p*p;const int cxr=160-int(curve*130*far*far);
  const int side=(k&1)?1:-1;const int half=int((8+230*p*p)*0.5)+rows/2+4;
  const int x=cxr+side*half-rows/2;const unsigned shape=unsigned(k%4);
  const unsigned z=unsigned(32*0x200/rows);
  osprite&e=osprites.sprite_entries[n];uint16_t*r=e.data;e.scratch=n;
  r[0]=uint16_t((ybot-rows+256)&511);r[1]=uint16_t(shape*256);r[2]=uint16_t(5<<9);r[3]=uint16_t(0x3000|std::min(2047u,z));
  r[4]=uint16_t(0x8000|0x4000|0x2000|std::min(2047u,z));r[5]=uint16_t(((rows-1)<<8)|(shape&3));r[6]=uint16_t(x+190);
  ++n;
 }
 osprites.sprite_count=uint16_t(n);
 char line[48];snprintf(line,sizeof line,"FRAME %06d  KEY %03u",frame,cbx68_test_key);cbx68_text(25,1,0,line);
}
