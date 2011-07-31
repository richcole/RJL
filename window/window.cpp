#include <SDL/SDL.h>
#include <stdio.h>

void error(char const* msg) {
  fprintf(stdout, "%s", msg);
};

int render(char *pixels, int width, int height) {
  int x, y;
  for(y=0;y<height;++y) {
    for(x=0;x<width;++x) {
      pixels[(y*width*3)+(x*3)] = x;
      pixels[(y*width*3)+(x*3)+1] = y;
    }
  }
}

int is_key(SDL_keysym* keysym, SDLKey c) {
  return keysym->sym == c;
}

int handle_event(SDL_Event *event) {
  switch(event->type) {
  case SDL_KEYDOWN:
    if ( is_key(&event->key.keysym, SDLK_q) ) {
      return 1;
    }
    break;
  case SDL_QUIT:
    return 1;
    break;
  }
  return 0;
}

int main( int argc, char* argv[] )
{
  SDL_Surface *screen ;
  SDL_Event event;
  int width = 640;
  int height = 480;
  int finished = 0;

  // Init
  if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    error("Error initializing SDL");
 
  screen = SDL_SetVideoMode(640, 480, 24, SDL_HWSURFACE | SDL_DOUBLEBUF);

  while( ! finished ) {
    while ( ! finished && SDL_PollEvent(&event) ) {
      finished = handle_event(&event);
    }
    render((char *)screen->pixels, width, height);
    SDL_Flip(screen);
  }
  
  SDL_Quit();
  return 0;
}
