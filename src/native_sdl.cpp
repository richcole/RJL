#include "native.h"
#include "boxed_int.h"
#include "exception.h"
#include "frame.h"
#include "array.h"
#include "char_array.h"
#include "sym.h"
#include "mem.h"
#include "func.h"
#include "block.h"
#include "code.h"
#include "context.h"

#include <SDL/SDL.h>
#include <stdio.h>

struct SdlScreenBuffer {
  Fixnum      type;
  SDL_Surface *surface;
};

SdlScreenBuffer *new_sdl_screen_buffer(Object *cxt) {
  SdlScreenBuffer *buf = (SdlScreenBuffer *) 
    context_alloc_buffer(cxt, sizeof(SdlScreenBuffer));
  buf->type      = SdlScreenTypeTag;
  buf->surface   = 0;
  return buf;
};

def_get_buffer(SdlScreen, sdl_screen, SdlScreenTypeTag);
def_set_buffer(SdlScreen, sdl_screen, SdlScreenTypeTag);

Object *sdl_screen(Object *cxt) {
  return context_get(cxt, "Screen");
}

Object* native_sdl_screen_init(Object *cxt, Object *frame, Object *self) {
  if ( SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
    return new_exception(cxt, frame, "Unable to initialized SDL");
  };

  return return_undefined(cxt, frame);
}

Object* native_sdl_screen_new(Object *cxt, Object *frame, Object *self) {
  Object *obj = new_object(cxt, sdl_screen(cxt));
  obj->buffer = (Buffer *) new_sdl_screen_buffer(cxt);
  return return_value(cxt, frame, obj);
};

#define GET_SDL_SCREEN_BUF(buf) \
  SdlScreenBuffer* buf = get_sdl_screen_buffer(self); \
  if ( buf == 0 ) { \
    return new_exception(cxt, frame, "Expected self to be a screen buffer"); \
  }; \
  if ( buf->surface == 0 ) { \
    return new_exception(cxt, frame, "Screen is not initialized"); \
  }; \

Object* native_sdl_screen_set_size(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *size = pop(cxt, stack);
  SdlScreenBuffer* buf = get_sdl_screen_buffer(self); \

  if ( buf == 0 ) { 
    return new_exception(cxt, frame, "Expected self to be a screen buffer"); 
  }; 

  if ( buf->surface != 0 ) {
    return new_exception(cxt, frame, "Resizing of screen is not implemented");
  };

  Fixnum width = boxed_int_to_fixnum(cxt, get(cxt, size, "width"));
  Fixnum height = boxed_int_to_fixnum(cxt, get(cxt, size, "height"));
  buf->surface = SDL_SetVideoMode(width, height, 24, 
    SDL_HWSURFACE | SDL_DOUBLEBUF);

  return return_value(cxt, frame, self);
}

Object* native_sdl_screen_flip(Object *cxt, Object *frame, Object *self) {
  GET_SDL_SCREEN_BUF(buf);
  SDL_Flip(buf->surface);
  return return_undefined(cxt, frame);
}

Object* native_sdl_screen_close(Object *cxt, Object *frame, Object *self) {
  GET_SDL_SCREEN_BUF(buf);
  SDL_Quit();
  return return_undefined(cxt, frame);
}

Object* native_sdl_screen_fill(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Fixnum color = boxed_int_to_fixnum(cxt, pop(cxt, stack));

  GET_SDL_SCREEN_BUF(buf);
  char *pixels = (char *)buf->surface->pixels;
  Fixnum len = buf->surface->w * buf->surface->h * 3;
  for(int i=0; i<len; i+=3) {
    pixels[i]   = (char) (color & 0xff);
    pixels[i+1] = (char) ((color & 0xff00) >> 8);
    pixels[i+2] = (char) ((color & 0xff0000) >> 16);
  }
  
  return return_undefined(cxt, frame);
}

void init_native_sdl_sys(Object *cxt) {
  Object *screen = sdl_screen(cxt);
  set(cxt, screen, "init",    new_func(cxt, native_sdl_screen_init));
  set(cxt, screen, "new",     new_func(cxt, native_sdl_screen_new));
  set(cxt, screen, "size:",   new_func(cxt, native_sdl_screen_set_size));
  set(cxt, screen, "flip",    new_func(cxt, native_sdl_screen_flip));
  set(cxt, screen, "fill:",   new_func(cxt, native_sdl_screen_fill));
  set(cxt, screen, "close:",   new_func(cxt, native_sdl_screen_close));
}
