/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE / LICENSE.txt). Other notices below are kept as they were. */
// Pixel comparisons against the original full-resolution algorithms.
#include <vector>
extern bool render_reference_mode;
static unsigned verified_frames=0;
static uint16_t sprite_before[2048];
static void verify_sprite_before(){video.sprite_layer->save_render_state(sprite_before);}
static void verify_full(int frame){
    const int n=config.s16_width*config.s16_height;
    static std::vector<uint16_t> optimized(n);
    memcpy(optimized.data(),video.pixels,n*2);
    uint16_t sprite_after[2048],sprite_original[2048];
    video.sprite_layer->save_render_state(sprite_after);
    video.sprite_layer->load_render_state(sprite_before);
    render_reference_mode=true;video.prepare_frame();
    video.sprite_layer->save_render_state(sprite_original);
    if(memcmp(sprite_after,sprite_original,sizeof sprite_after)){fprintf(stderr,"SPRITE STATE FAIL frame=%d\n",frame);exit(7);}
    video.sprite_layer->load_render_state(sprite_after);
    if(memcmp(optimized.data(),video.pixels,n*2)){
        for(int i=0;i<n;i++)if(optimized[i]!=video.pixels[i]){
            fprintf(stderr,"FULL PIXEL FAIL frame=%d x=%d y=%d optimized=%u original=%u\n",frame,i%config.s16_width,i/config.s16_width,optimized[i],video.pixels[i]);exit(3);
        }
    }
    for(unsigned i=0;i<8192;i++){
        uint32_t c=g_rgb[i];uint16_t rebuilt=((c>>11)&31)<<11 | ((c>>19)&31)<<6 | ((c>>3)&31)<<1;
        if(display_palette[i]!=rebuilt){fprintf(stderr,"PALETTE CACHE FAIL frame=%d entry=%u\n",frame,i);exit(4);}
    }
    render_reference_mode=false;
    memcpy(video.pixels,optimized.data(),n*2);
    verified_frames++;
    if((frame+1)%1000==0)printf("FULL PIXELS PASS frames=%u pixels_per_frame=%d palette_entries_per_frame=8192\n",verified_frames,n);
}
// Deterministic edge cases for tile wrapping, clipping, priority and dual roads.
static void verify_render_cases(){
    hwtiles* t=video.tile_layer;
    std::vector<uint8_t> saved_text(t->text_ram,t->text_ram+0x1000),saved_tiles(t->tile_ram,t->tile_ram+0x10000);
    std::vector<uint16_t> saved_road(hwroad.ramBuff,hwroad.ramBuff+0x800);
    const unsigned saved_control=hwroad.road_control;
    const int n=config.s16_width*config.s16_height;
    std::vector<uint16_t> a(n),b(n);
    uint32_t seed=0x39051472;
    auto next=[&seed](){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;};
    auto word=[](uint8_t*dst,unsigned off,unsigned value){dst[off]=value>>8;dst[off+1]=value;};
    auto equal=[&a,&b](const char*label,int c){if(a!=b){fprintf(stderr,"RENDER EDGE FAIL kind=%s case=%d\n",label,c);exit(6);}};
    for(unsigned i=0;i<0x10000;i+=2)word(t->tile_ram,i,next());
    for(unsigned i=0;i<0x1000;i+=2)word(t->text_ram,i,next());
    const uint16_t xs[]={0,1,7,8,191,192,193,255,256,511,512,513,1016,1023,0x8000};
    const uint16_t ys[]={0,1,7,8,223,224,255,256,287,288,503,511,0x8000};
    int cases=0;
    for(int c=0;c<80;c++){
        word(t->text_ram,0xe80,next());word(t->text_ram,0xe82,next());
        word(t->text_ram,0xe98,xs[c%15]);word(t->text_ram,0xe9a,xs[(c*7)%15]);
        word(t->text_ram,0xe90,ys[c%13]);word(t->text_ram,0xe92,ys[(c*5)%13]);
        t->update_tile_values();
        for(int layer=0;layer<2;layer++)for(int priority=0;priority<2;priority++){
            std::fill(a.begin(),a.end(),0x55aa);std::fill(b.begin(),b.end(),0x55aa);
            t->render_tile_layer(a.data(),layer,priority);t->render_tile_layer_reference(b.data(),layer,priority);equal("tiles",c);cases++;
        }
        for(int priority=0;priority<2;priority++){
            std::fill(a.begin(),a.end(),0x55aa);std::fill(b.begin(),b.end(),0x55aa);
            t->render_text_layer(a.data(),priority);t->render_text_layer_reference(b.data(),priority);equal("text",c);cases++;
        }
    }
    for(int c=0;c<32;c++){
        for(unsigned i=0;i<0x800;i++)hwroad.ramBuff[i]=next();
        hwroad.road_control=c&7;
        std::fill(a.begin(),a.end(),0x55aa);std::fill(b.begin(),b.end(),0x55aa);
        hwroad.render_foreground_lores(a.data());hwroad.render_foreground_lores_reference(b.data());equal("road",c);cases++;
    }
    memcpy(t->text_ram,saved_text.data(),0x1000);memcpy(t->tile_ram,saved_tiles.data(),0x10000);t->update_tile_values();
    memcpy(hwroad.ramBuff,saved_road.data(),0x1000);hwroad.road_control=saved_control;
    printf("RENDER EDGE PASS cases=%d\n",cases);
}
