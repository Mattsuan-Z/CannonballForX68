/* Cannonball for x68 - X68000 port by Mattsuan-Z@ColtMaz.
   Part of a derivative work of Cannonball (Copyright Chris White and the Cannonball team),
   released under the Cannonball licence (LICENSE / LICENSE.txt). Other notices below are kept as they were. */
// Silent adapter for the deferred audio implementation.
#include "stdint.hpp"
#include "engine/audio/osoundint.hpp"
#include <cstring>
OSoundInt osoundint;
OSoundInt::OSoundInt():pcm(NULL),ym(NULL),has_booted(false),audio_ticks(0),pcm_ram(NULL),sound_counter(0),sounds_queued(0),sound_head(0),sound_tail(0){
    memset(engine_data,0,sizeof engine_data);memset(queue,0,sizeof queue);
}
OSoundInt::~OSoundInt(){}
void OSoundInt::init(){reset();memset(engine_data,0,sizeof engine_data);}
void OSoundInt::reset(){sound_counter=sound_head=sound_tail=sounds_queued=0;audio_ticks=0;}
void OSoundInt::tick(){}
void OSoundInt::play_queued_sound(){}
void OSoundInt::queue_sound_service(uint8_t){}
void OSoundInt::queue_sound(uint8_t){}
void OSoundInt::queue_clear(){sound_head=sound_tail=sounds_queued=0;}
void OSoundInt::add_to_queue(uint8_t){}
