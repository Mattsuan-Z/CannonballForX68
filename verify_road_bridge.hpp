/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE.txt). Other notices below are kept as they were. */
// Host check (r022): road rows against the software road renderer.
// One-road lines shown from graphic page 1 must match exactly. On two-road lines the second
// road comes from the 80-step BG atlas, so pixels covered by it are counted, not failed; lines
// where the 512-wide page would wrap use the atlas and are counted as approximate.
#include <vector>
static void verify_road_bridge(){
 uint16_t saved[2048];memcpy(saved,hwroad.ramBuff,sizeof saved);unsigned control=hwroad.road_control;
 uint16_t pal[8192];for(unsigned i=0;i<8192;i++)pal[i]=i;
 std::vector<uint16_t>a(320*224),b(320*224);
 const int cols[]={-2048,-321,-320,-129,-128,-1,0,1,96,191,192,250,383,384,511,512,2047};
 static RoadFrame f;unsigned cases=0;unsigned long exact_rows=0,other_rows=0,atlas_rows=0,hidden_rows=0,offscreen_rows=0,other_pixels=0,atlas_pixels=0;
 for(unsigned c=0;c<1024;c++){
  unsigned data[2]={unsigned(((c*37+11)&255)*2),unsigned(((c*101+7)&255)*2)};
  if(c%11==0)data[0]|=2048;if(c%13==0)data[1]|=2048;
  if(c%5==0)data[0]|=512;
  hwroad.road_control=c&3;memset(hwroad.ramBuff,0,sizeof saved);
  for(unsigned y=0;y<256;y++){hwroad.ramBuff[y]=data[0]+((y*3)&~1&255)*0;hwroad.ramBuff[256+y]=data[1];}
  for(unsigned y=0;y<256;y++){hwroad.ramBuff[y]=(data[0]&~0x1fe)|((((data[0]>>1)+y)&255)<<1);hwroad.ramBuff[256+y]=(data[1]&~0x1fe)|((((data[1]>>1)+y*3)&255)<<1);}
  for(unsigned i=0;i<512;i++){hwroad.ramBuff[0x200+i]=(cols[c%17]+(int)(i%7)*13+0x5f8)&4095;hwroad.ramBuff[0x400+i]=(cols[(c*7+3)%17]+0x5f8)&4095;hwroad.ramBuff[0x600+i]=(c*997+i*37)&0xffff;}
  std::fill(a.begin(),a.end(),0);std::fill(b.begin(),b.end(),0);
  road_build(hwroad,pal,f);road_stream(f);
  road_preview(f,a.data());
  hwroad.render_background_lores(b.data());hwroad.render_foreground_lores(b.data());
  for(unsigned y=0;y<224;y++){
   unsigned m=f.mode[y];
   if(m==0)++hidden_rows;else if(m==1)++exact_rows;else if(m==2)++other_rows;else if(m==3||m==5)++atlas_rows;else ++offscreen_rows;
   if(f.rows[y].next!=(y==223?1023:72+(y+1)*2)){fprintf(stderr,"ROAD IRQ LINE FAIL\n");exit(5);}
   for(unsigned x=0;x<320;x++){
    unsigned i=y*320+x;if(a[i]==b[i])continue;
    if(m==3||m==5){++atlas_pixels;continue;}
    if(m==2){
     // allowed only where the second road (BG1) has a road pixel
     unsigned bgv=0;{uint32_t xy=f.rows[y].bg1;unsigned sx=((xy>>16)+x)&511,sy=((xy&511)+y)&511;unsigned code=road_atlas.map1[(sy>>3)*64+(sx>>3)]&255;unsigned bb=road_atlas.pcg[code*32+(sy&7)*4+((sx&7)>>1)];bgv=sx&1?bb&15:bb>>4;}
     if(bgv){++other_pixels;continue;}
    }
    fprintf(stderr,"ROAD COMPOSE FAIL case=%u control=%u mode=%u x=%u y=%u actual=%u expected=%u\n",c,unsigned(hwroad.road_control),m,x,y,a[i],b[i]);exit(5);
   }
  }
  ++cases;
 }
 memcpy(hwroad.ramBuff,saved,sizeof saved);hwroad.road_control=control;
 printf("ROAD COMPOSITION PASS cases=%u rows exact=%lu with_second_road=%lu atlas_wrap=%lu hidden=%lu offscreen=%lu approx_pixels second_road=%lu atlas=%lu\n",cases,exact_rows,other_rows,atlas_rows,hidden_rows,offscreen_rows,other_pixels,atlas_pixels);
}
