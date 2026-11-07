/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE.txt). Other notices below are kept as they were. */
// Host-only workload counter for the sprite layer (r010 Claude comparison build).
// Walks the same sprite list as hwsprites::render_reference without touching pixels,
// so the 68000 kernel can be chosen from counted work instead of guesses.
#ifndef OR_X68K
#include "video.hpp"
#include "hwvideo/hwsprites.hpp"
#include "frontend/config.hpp"
#include <cstdio>
#include <cstring>
static struct SpriteWork {
    unsigned long long draws,sprites,rows,words,words_clear,src_px,src_clear,src_shadow,src_opaque;
    unsigned long long out_clear,out_opaque,out_shadow,out_clipped,rows_clip,rows_endmark,rows_same_addr;
    unsigned long long opaque_runs,clear_runs;
    unsigned long long hz_rows[6],hz_out[6];
    unsigned long long row_words_hist[8];
    unsigned long long path_clear,path_inside,path_edge,rows_wrapped,words_wrapped;
} work;
static int hz_bucket(unsigned hz){return hz<0x100?0:hz<0x200?1:hz==0x200?2:hz<0x400?3:hz<0x800?4:5;}
void hwsprites::count_sprite_work(){
    work.draws++;
    for(uint16_t data=0;data<SPRITE_RAM_SIZE;data+=8){
        if(ramBuff[data]&0x8000)break;
        if((1u<<((ramBuff[data+3]>>12)&3))!=8)continue;
        if(ramBuff[data]&0x5000)continue;
        int bank=(ramBuff[data]>>9)&7;int top=(ramBuff[data]&0x1ff)-0x100;unsigned addr=ramBuff[data+1];
        int pitch=((ramBuff[data+2]>>1)|((ramBuff[data+4]&0x1000)<<3))>>8;int xpos=ramBuff[data+6];
        bool shadow=(ramBuff[data+3]>>14)&1;int vzoom=ramBuff[data+3]&0x7ff;int ydelta=(ramBuff[data+4]&0x8000)?1:-1;
        int flip=(~ramBuff[data+4]>>14)&1;int xdelta=(ramBuff[data+4]&0x2000)?1:-1;int hzoom=ramBuff[data+4]&0x7ff;
        int height=(ramBuff[data+5]>>8)+1;
        if(xpos<0x80&&xdelta<0)xpos+=0x200;xpos-=0xbe;xpos+=config.s16_x_off;
        const uint32_t* sd=sprites+0x10000*(bank%(SPRITES_LENGTH/0x10000));
        if(vzoom<0x40)vzoom=0x40;if(hzoom<0x40)hzoom=0x40;
        work.sprites++;
        int b=hz_bucket(hzoom);unsigned yacc=0;unsigned prev=0xffffffff;
        // Path split of the r010 kernel (same window rules as sprite_fast.hpp).
        const int q=4096/hzoom,r=4096-q*hzoom,most=r?q+1:q,width=config.s16_width;
        const int fast_lo=xdelta>0?x1:x1+most,fast_hi=xdelta>0?(x2<width?x2:width)-most:x2;
        const unsigned reach=xdelta>0?(xpos<width?unsigned(width-xpos):0u):(xpos>=0?unsigned(xpos+1):0u);
        const unsigned words_max=reach/unsigned(q)+1;
        for(int y=top,n=0;n<height;y+=ydelta,n++){
            if(y>=0&&y<config.s16_height){
                work.rows++;work.hz_rows[b]++;if((addr&0xffff)==prev)work.rows_same_addr++;prev=addr&0xffff;
                uint16_t s=flip?addr+1:addr-1;int x=xpos,acc=0,last=-1;bool clip=false,endm=false;unsigned nw=0;
                const uint16_t index=uint16_t(addr);
                const bool wraps=flip?unsigned(index)+1<words_max:unsigned(index)+words_max>0x10000u;
                if(wraps)work.rows_wrapped++;
                while((xdelta>0&&x<config.s16_width)||(xdelta<0&&x>=0)){
                    s=flip?s-1:s+1;uint32_t w=sd[s];nw++;work.words++;bool allc=true;
                    {
                        const int d=xdelta>0?x:x+1;
                        if(wraps)work.words_wrapped++;
                        else if(((w^(w>>1))&0x77777777u)==0)work.path_clear++;
                        else if(d>=fast_lo&&d<=fast_hi)work.path_inside++;
                        else work.path_edge++;
                    }
                    for(int k=0;k<8;k++){
                        unsigned pix=flip?(w>>(4*k))&15:(w>>(28-4*k))&15;work.src_px++;
                        int cls=(pix==0||pix==15)?0:(shadow&&pix==10)?2:1;
                        if(cls)allc=false;
                        if(cls==0)work.src_clear++;else if(cls==2)work.src_shadow++;else work.src_opaque++;
                        if(cls!=last){if(cls==0)work.clear_runs++;else work.opaque_runs++;last=cls;}
                        while(acc<0x200){
                            bool in=x>=x1&&x<x2;if(!in){work.out_clipped++;clip=true;}
                            else if(cls==0)work.out_clear++;else if(cls==2)work.out_shadow++;else work.out_opaque++;
                            work.hz_out[b]++;x+=xdelta;acc+=hzoom;
                        }
                        acc-=0x200;
                    }
                    if(allc)work.words_clear++;
                    if((w&(flip?0x0f000000:0xf0))==(flip?0x0f000000u:0xf0u)){endm=true;break;}
                }
                if(clip)work.rows_clip++;if(endm)work.rows_endmark++;
                work.row_words_hist[nw<1?0:nw<2?1:nw<4?2:nw<8?3:nw<16?4:nw<32?5:nw<64?6:7]++;
            }
            yacc+=vzoom;addr+=pitch*(yacc>>9);yacc&=0x1ff;
        }
    }
}
// Path coverage for the boundary tests: clear, inside, edge, wrapped words, rows of 16+ words.
void hwsprites::sprite_work_paths(unsigned long long* out) const{
    out[0]=work.path_clear;out[1]=work.path_inside;out[2]=work.path_edge;out[3]=work.words_wrapped;
    out[4]=work.row_words_hist[5]+work.row_words_hist[6]+work.row_words_hist[7];out[5]=work.out_shadow;
}
void hwsprites::print_sprite_work(FILE* f){
    if(!f||!work.draws)return;
    const double d=double(work.draws);
#define P(name) fprintf(f,"%-16s total=%llu per_draw=%.1f\n",#name,work.name,work.name/d)
    P(sprites);P(rows);P(words);P(words_clear);P(src_px);P(src_clear);P(src_shadow);P(src_opaque);
    P(out_clear);P(out_opaque);P(out_shadow);P(out_clipped);P(rows_clip);P(rows_endmark);P(rows_same_addr);P(opaque_runs);P(clear_runs);
    P(path_clear);P(path_inside);P(path_edge);P(rows_wrapped);P(words_wrapped);
#undef P
    const char* hn[6]={"hz<0x100","hz<0x200","hz=0x200","hz<0x400","hz<0x800","other"};
    for(int i=0;i<6;i++)fprintf(f,"%-10s rows_per_draw=%.1f out_per_draw=%.1f\n",hn[i],work.hz_rows[i]/d,work.hz_out[i]/d);
    const char* wn[8]={"0","1","2-3","4-7","8-15","16-31","32-63","64+"};
    for(int i=0;i<8;i++)fprintf(f,"row_words %-6s rows_per_draw=%.1f\n",wn[i],work.row_words_hist[i]/d);
}
#endif
