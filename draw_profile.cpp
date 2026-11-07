/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE.txt). Other notices below are kept as they were. */
#include "draw_profile.hpp"
#ifdef OR_X68K
#include <x68k/iocs.h>
#else
#include <chrono>
#endif
DrawSamples draw_samples[2][3];
uint32_t draw_profile_errors=0;
static bool active=false,setup_seen=false;
static unsigned active_mode,active_group,seen;
static uint32_t last_clock;
static DrawSamples* active_samples=nullptr;
#ifdef DRAW_PROFILE_TEST_CLOCK
extern uint32_t draw_profile_test_now();
#endif
static uint32_t profile_clock(){
#ifdef DRAW_PROFILE_TEST_CLOCK
    return draw_profile_test_now();
#elif defined(OR_X68K)
    return (uint32_t)_iocs_ontime().sec;
#else
    return (uint32_t)((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()/10)%8640000);
#endif
}
static void charge(unsigned part,uint32_t now){
    uint32_t elapsed=now>=last_clock?now-last_clock:8640000UL-last_clock+now;
    DrawSamples& s=*active_samples;
    s.cs[part]+=elapsed;s.total_cs+=elapsed;last_clock=now;
}
void draw_profile_begin(unsigned mode,int state,bool scene_enabled){
    if(active)draw_profile_errors++;
    active=true;active_mode=mode;active_group=state>=9&&state<=11?0:state==12?1:2;
    seen=0;setup_seen=false;active_samples=&draw_samples[active_mode][active_group];
    DrawSamples& s=draw_samples[active_mode][active_group];s.frames++;if(scene_enabled)s.scene_frames++;
    last_clock=profile_clock();
}
void draw_profile_mark(unsigned part){
    if(!active)return;
    if(part>=DRAW_PARTS){draw_profile_errors++;return;}
    if(seen&(1U<<part))draw_profile_errors++;
    seen|=1U<<part;
    uint32_t now=profile_clock();charge(part,now);
    active_samples->calls[part]++;
    if(part==DRAW_SETUP)setup_seen=true;
}
void draw_profile_end(){
    if(!active){draw_profile_errors++;return;}
    charge(DRAW_SETUP,profile_clock());
    if(!setup_seen)active_samples->calls[DRAW_SETUP]++;
    active=false;
}
static void add(DrawSamples& a,const DrawSamples& b){
    a.frames+=b.frames;a.scene_frames+=b.scene_frames;a.total_cs+=b.total_cs;
    for(unsigned i=0;i<DRAW_PARTS;i++){a.cs[i]+=b.cs[i];a.calls[i]+=b.calls[i];}
}
static void print_sample(FILE* out,const char* label,const DrawSamples& s){
    static const char* part_names[DRAW_PARTS]={"setup","fill","bg_tiles","fg_tiles","road","sprites","hud"};
    unsigned top=0;for(unsigned i=1;i<DRAW_PARTS;i++)if(s.cs[i]>s.cs[top])top=i;
    uint32_t tenths=s.total_cs?(uint32_t)((uint64_t)s.cs[top]*1000/s.total_cs):0;
    fprintf(out,"%s frames=%lu scene=%lu profiled_cs=%lu\n",label,(unsigned long)s.frames,(unsigned long)s.scene_frames,(unsigned long)s.total_cs);
    fprintf(out,"fill_cs=%lu bg_cs=%lu fg_cs=%lu\n",(unsigned long)s.cs[DRAW_FILL],(unsigned long)s.cs[DRAW_BG],(unsigned long)s.cs[DRAW_FG]);
    fprintf(out,"road_cs=%lu sprite_cs=%lu hud_cs=%lu\n",(unsigned long)s.cs[DRAW_ROAD],(unsigned long)s.cs[DRAW_SPRITES],(unsigned long)s.cs[DRAW_HUD]);
    fprintf(out,"setup_cs=%lu TOP=%s share=%lu.%lu%%\n",(unsigned long)s.cs[DRAW_SETUP],s.total_cs?part_names[top]:"insufficient_samples",(unsigned long)(tenths/10),(unsigned long)(tenths%10));
}
void draw_profile_print(FILE* out,bool detailed){
    if(!out)return;
    static const char* group_names[3]={"START","RACE","OTHER"};
    fprintf(out,"DRAW DETAIL unit=1/100sec errors=%lu\n",(unsigned long)draw_profile_errors);
    for(unsigned m=0;m<2;m++){
        DrawSamples sum;for(unsigned g=0;g<3;g++)add(sum,draw_samples[m][g]);
        if(!sum.frames)continue;
        char label[32];sprintf(label,"PROFILE%u",m?60:30);print_sample(out,label,sum);
        if(detailed)for(unsigned g=0;g<3;g++)if(draw_samples[m][g].frames){
            sprintf(label,"PROFILE%u_%s",m?60:30,group_names[g]);print_sample(out,label,draw_samples[m][g]);
            fprintf(out,"calls setup=%lu fill=%lu bg=%lu fg=%lu road=%lu sprites=%lu hud=%lu\n",(unsigned long)draw_samples[m][g].calls[0],(unsigned long)draw_samples[m][g].calls[1],(unsigned long)draw_samples[m][g].calls[2],(unsigned long)draw_samples[m][g].calls[3],(unsigned long)draw_samples[m][g].calls[4],(unsigned long)draw_samples[m][g].calls[5],(unsigned long)draw_samples[m][g].calls[6]);
        }
    }
}
