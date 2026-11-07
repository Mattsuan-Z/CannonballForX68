#pragma once
#include <cstdint>
typedef uint8_t Uint8; typedef uint16_t Uint16; typedef uint32_t Uint32; typedef int16_t Sint16; typedef int32_t Sint32;
typedef struct SDL_Joystick SDL_Joystick; typedef struct SDL_GameController SDL_GameController; typedef struct SDL_Haptic SDL_Haptic;
typedef struct SDL_Window SDL_Window; typedef struct SDL_Renderer SDL_Renderer; typedef struct SDL_Texture SDL_Texture; typedef struct SDL_Surface SDL_Surface;
typedef void* SDL_GLContext; typedef int SDL_Keycode; typedef int SDL_AudioDeviceID;
typedef struct { int freq; int format; int channels; int samples; void (*callback)(void*,Uint8*,int); void* userdata; } SDL_AudioSpec;
typedef union SDL_Event { int type; } SDL_Event;
typedef struct SDL_mutex SDL_mutex;
typedef struct { int sym; int scancode; int mod; } SDL_Keysym;
typedef struct { int which; int axis; int value; } SDL_JoyAxisEvent;
typedef struct { int which; int button; int state; } SDL_JoyButtonEvent;
typedef struct { int which; int button; int state; } SDL_ControllerButtonEvent;
typedef struct { int which; int axis; int value; } SDL_ControllerAxisEvent;
typedef struct { int which; int hat; int value; } SDL_JoyHatEvent;
typedef int SDL_JoystickID;
typedef int SDL_GameControllerAxis; typedef int SDL_GameControllerButton;
typedef struct { int x, y, w, h; } SDL_Rect;
typedef struct SDL_PixelFormat SDL_PixelFormat;
