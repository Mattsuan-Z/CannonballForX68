/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE / LICENSE.txt). Other notices below are kept as they were. */
// Host-side deterministic game/timer traces. No platform code is called here.
#include <string>
#define STATE_FIELDS(X) \
 X(oroad.road_pos) X(oroad.road_pos_old) X(oroad.pos_fine) X(oroad.pos_fine_old) X(oroad.road_width) \
 X(oroad.road_width_bak) X(oroad.car_x_bak) X(oroad.road_ctrl) X(oroad.tilemap_h_target) X(oroad.horizon_base) \
 X(oroad.horizon_set) X(oroad.horizon_y2) X(oroad.horizon_y_bak) X(oroad.height_lookup) X(oroad.height_lookup_wrk) \
 X(oroad.height_ctrl) X(oroad.height_ctrl2) X(oroad.height_index) X(oroad.height_inc) X(oroad.height_step) \
 X(oroad.height_start) X(oroad.height_end) X(oroad.height_addr) X(oroad.height_delay) X(oroad.step_adjust) \
 X(oroad.do_height_inc) X(oroad.up_mult) X(oroad.down_mult) X(oroad.elevation) X(oroad.horizon_mod) \
 X(oroad.road_p0) X(oroad.road_p1) X(oroad.road_p2) X(oroad.road_p3) X(oroad.stage_loaded) X(oroad.road_load_split) \
 X(oinitengine.car_increment) X(oinitengine.car_x_pos) X(oinitengine.car_x_old) X(oinitengine.road_curve) \
 X(oinitengine.road_curve_next) X(oinitengine.road_type) X(oinitengine.road_type_next) X(oinitengine.road_width_next) \
 X(oinitengine.road_width_adj) X(oinitengine.change_width) X(oinitengine.granular_rem) X(oinitengine.ingame_engine) \
 X(oinitengine.ingame_counter) X(oinitengine.rd_split_state) X(oinitengine.pos_fine_old) \
 X(oinitengine.road_width_orig) X(oinitengine.route_updated) X(oinitengine.end_stage_props) X(oinitengine.route_selected) \
 X(trackloader.wh_offset) X(trackloader.curve_offset) \
 X(oferrari.revs) X(oferrari.torque) X(oferrari.torque_index) X(oferrari.gear_value) X(oferrari.gear_bak) \
 X(oferrari.gear_counter) X(oferrari.rev_adjust) X(oferrari.rev_shift) X(oferrari.accel_value) X(oferrari.accel_value_bak) \
 X(oferrari.brake_value) X(oferrari.brake_subtract) X(oferrari.acc_adjust1) X(oferrari.acc_adjust2) X(oferrari.acc_adjust3) \
 X(oferrari.brake_adjust1) X(oferrari.brake_adjust2) X(oferrari.brake_adjust3) X(oferrari.car_inc_old) \
 X(oferrari.steering_old) X(oferrari.road_width_old) X(oferrari.wheel_state) X(oferrari.gear_smoke) \
 X(oferrari.rev_stop_flag) X(oferrari.revs_post_stop) X(oferrari.acc_post_stop) X(oferrari.car_ctrl_active) \
 X(oferrari.auto_brake) \
 X(oinputs.input_steering) X(oinputs.steering_old) X(oinputs.steering_change) X(oinputs.steering_adjust) \
 X(oinputs.input_acc) X(oinputs.input_brake) X(oinputs.acc_adjust) X(oinputs.brake_adjust) X(oinputs.gear) \
 X(oinputs.crash_input) \
 X(otiles.tilemap_h_scr) X(otiles.fg_h_scroll) X(otiles.bg_h_scroll) X(otiles.fg_v_scroll) X(otiles.tilemap_v_off) \
 X(otiles.vswap_off) X(otiles.page) \
 X(ocrash.skid_counter) X(ocrash.spin_control1) X(ocrash.crash_counter) X(ostats.time_counter) X(ostats.cur_stage)
static FILE* state_trace=NULL;
static FILE* render_trace=NULL;
static uint64_t trace_hash(const void* data,size_t n,uint64_t h=14695981039346656037ULL){
    const unsigned char*p=(const unsigned char*)data;
    for(size_t i=0;i<n;i++){h^=p[i];h*=1099511628211ULL;}
    return h;
}
static void timeline_open(){
    const char* root=getenv("STAGE1_TRACE");if(!root)return;
    std::string base=root;state_trace=fopen((base+".state.csv").c_str(),"w");render_trace=fopen((base+".render.csv").c_str(),"w");
    if(!state_trace||!render_trace){fprintf(stderr,"Cannot create validation trace\n");exit(7);}
    fputs("step,logical,outrun.game_state,outrun.tick_counter,ostats.frame_counter,ostats.score,ostats.extend_play_timer,ostats.credits",state_trace);
#define FIELD_NAME(v) fprintf(state_trace,",%s",#v);
    STATE_FIELDS(FIELD_NAME)
#undef FIELD_NAME
    fputs(",road_x_hash,road_y_hash,road_ram_hash,road_buffer_hash,text_hash,tile_hash,stage_timing_hash,objects_hash\n",state_trace);
    fputs("step,pixels_hash,palette_hash\n",render_trace);
}
static void timeline_step(int step,bool logical){
    if(!state_trace)return;
    fprintf(state_trace,"%d,%d,%d,%u,%d,%u,%d,%u",step,int(logical),int(outrun.game_state),unsigned(outrun.tick_counter),int(ostats.frame_counter),unsigned(ostats.score),int(ostats.extend_play_timer),unsigned(ostats.credits));
#define FIELD_VALUE(v) fprintf(state_trace,",%d",int32_t(v));
    STATE_FIELDS(FIELD_VALUE)
#undef FIELD_VALUE
    uint64_t timing=trace_hash(ostats.stage_counters,sizeof ostats.stage_counters);
    timing=trace_hash(ostats.stage_times,sizeof ostats.stage_times,timing);timing=trace_hash(ostats.routes,sizeof ostats.routes,timing);
    uint64_t objects=14695981039346656037ULL;
    for(int i=0;i<0x62;i++){
        oentry* e=&osprites.jump_table[i];
        int32_t values[]={e->control,e->function_holder,e->draw_props,int32_t(e->z),e->xw1,e->yw,int32_t(e->type),e->x,e->y,e->zoom,e->pal_src};
        objects=trace_hash(values,sizeof values,objects);
    }
    fprintf(state_trace,",%016llx,%016llx,%016llx,%016llx,%016llx,%016llx,%016llx,%016llx\n",
        (unsigned long long)trace_hash(oroad.road_x,0x200*2),(unsigned long long)trace_hash(oroad.road_y,0x1000*2),
        (unsigned long long)trace_hash(hwroad.ram,0x800*2),(unsigned long long)trace_hash(hwroad.ramBuff,0x800*2),
        (unsigned long long)trace_hash(video.tile_layer->text_ram,0x1000),(unsigned long long)trace_hash(video.tile_layer->tile_ram,0x10000),
        (unsigned long long)timing,(unsigned long long)objects);
}
static void timeline_render(int step){
    if(render_trace)fprintf(render_trace,"%d,%016llx,%016llx\n",step,(unsigned long long)trace_hash(video.pixels,config.s16_width*config.s16_height*2),(unsigned long long)trace_hash(display_palette,sizeof display_palette));
}
static void timeline_close(){if(state_trace)fclose(state_trace);if(render_trace)fclose(render_trace);}
#undef STATE_FIELDS
static void verify_profile_counts(){
    if(draw_profile_errors){fprintf(stderr,"PROFILE SCOPE ERROR count=%u\n",unsigned(draw_profile_errors));exit(8);}
    uint32_t total_frames=0,total_scopes=0;
    for(unsigned m=0;m<2;m++){
        uint32_t frames=0;
        for(unsigned g=0;g<3;g++){
            const DrawSamples& s=draw_samples[m][g];uint32_t sum=0;
            if(s.calls[DRAW_SETUP]!=s.frames){fprintf(stderr,"PROFILE SETUP COUNT FAIL\n");exit(8);}
            for(unsigned part=0;part<DRAW_PARTS;part++){
                sum+=s.cs[part];total_scopes+=s.calls[part];
                if(part!=DRAW_SETUP&&s.calls[part]!=s.scene_frames){fprintf(stderr,"PROFILE SCENE COUNT FAIL mode=%u group=%u part=%u\n",m,g,part);exit(8);}
            }
            if(sum!=s.total_cs){fprintf(stderr,"PROFILE TOTAL FAIL\n");exit(8);}
            frames+=s.frames;
        }
        if(frames!=render_schedule.draws[m]){fprintf(stderr,"PROFILE FRAME COUNT FAIL\n");exit(8);}
        total_frames+=frames;
    }
    printf("PROFILE COUNTS PASS frames=%u scopes=%u\n",unsigned(total_frames),unsigned(total_scopes));
}
