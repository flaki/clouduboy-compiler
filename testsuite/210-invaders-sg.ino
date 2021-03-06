#include <SPI.h>
#include <EEPROM.h>

#include <avr/pgmspace.h>

#include <Arduboy.h>

Arduboy arduboy;

// frame counter, 2-byte unsigned int, max 65536
unsigned int _microcanvas_frame_counter = 0;

// sprintf() textbuffer for drawText
char _microcanvas_textbuffer[32];

// global state machine
unsigned int _microcanvas_state;

// global current drawing color
unsigned int _microcanvas_fill_color = WHITE;

#define LENGTHOF(x)  (sizeof(x) / sizeof(x[0]))

PROGMEM const unsigned char gfx_invader[] = {
  /*9x8x3*/ 0x70, 0x38, 0xea, 0x3c, 0x78, 0x3c, 0xea, 0x38, 0x70, 0x70, 0x38, 0x6a, 0xbc, 0x38, 0xbc, 0x6a, 0x38, 0x70, 0x18, 0x2a, 0x25, 0xec, 0x38, 0x6d, 0x66, 0xa8, 0x18 };
PROGMEM const unsigned char gfx_defender[] = {
  /*9x6*/ 0x38, 0x30, 0x3c, 0x2e, 0x27, 0x2e, 0x3c, 0x30, 0x38 };
PROGMEM const unsigned char gfx_invader_2[] = {
  /*9x8*/ 0x28, 0x10, 0xba, 0x6a, 0x3f, 0x6a, 0xba, 0x10, 0x28 };
PROGMEM const unsigned char gfx_rocket[] = {
  /*1x3*/ 0x07 };
PROGMEM const unsigned char gfx_bomb[] = {
  /*5x5x2*/ 0x08, 0x1a, 0x1c, 0x1a, 0x08, 0x08, 0x18, 0x1e, 0x18, 0x08 };
PROGMEM const unsigned char gfx_intro[] = {
  /*64x64*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0xe0, 0xf0, 0xf8, 0xf8, 0xfc, 0xfc, 0x7c, 0x7c, 0xbc, 0xbc, 0xbc, 0xdc, 0xdc, 0xd8, 0xd8, 0xd0, 0xd0, 0xb0, 0xa0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xd0, 0xd8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfd, 0xf9, 0xf9, 0xf9, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0x7d, 0x7d, 0x7d, 0x7c, 0x3c, 0x81, 0xc5, 0xdd, 0xbd, 0xbf, 0x3b, 0x7b, 0xfb, 0xf7, 0xf7, 0xf7, 0xef, 0xdf, 0xbf, 0xbf, 0x7e, 0xfc, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe7, 0xe7, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x9f, 0xe0, 0xff, 0xff, 0xff, 0xfc, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xf9, 0xf3, 0xcf, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0xfc, 0xf1, 0xc7, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0xf0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x03, 0x03, 0x01, 0x01, 0x01, 0x81, 0x81, 0xc1, 0xe1, 0xf1, 0xf1, 0xf9, 0xf9, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x9f, 0xc7, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x8f, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x38, 0x01, 0xff, 0xff, 0xff, 0x7f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf8, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xbf, 0xdf, 0xef, 0xf3, 0xfd, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x9f, 0xef, 0xf7, 0xf9, 0xfc, 0x7e, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0xff, 0x1f, 0x47, 0x38, 0x1f, 0x0f, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xf9, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x7c, 0x7f, 0x3f, 0x3f, 0x1f, 0x1f, 0x0f, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x70, 0xfc, 0xfe, 0xfe, 0xdf, 0x8f, 0x8f, 0x0f, 0x3e, 0x3e, 0x1c, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x1f, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xcf, 0x3f, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x80, 0x80, 0xc1, 0xc3, 0x83, 0x07, 0x0f, 0x0f, 0x0f, 0x9f, 0xfe, 0xfe, 0xfc, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0f, 0x0f, 0x1f, 0x3f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x07, 0x07, 0x0f, 0x0f, 0x0f, 0x07, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00 };


const int INVADER_WAVES = 4;
const int INVADER_ALIENS = 8;
const int DEFENDER_WIN_ANIMATION_DURATION = 60;
const int GAME_TIMER_ANIMATION_DURATION = 60 * 1 + 60 * 24;
const byte GFX_INVADER_WIDTH = 9;
const byte GFX_INVADER_HEIGHT = 8;
const byte GFX_INVADER_FRAMES = 3;
const byte GFX_INVADER_FRAMESIZE = 9;
const byte GFX_INVADER_2_WIDTH = 9;
const byte GFX_INVADER_2_HEIGHT = 8;
const byte GFX_INVADER_2_FRAMES = 0;
const byte GFX_INVADER_2_FRAMESIZE = 9;
const byte GFX_DEFENDER_WIDTH = 9;
const byte GFX_DEFENDER_HEIGHT = 6;
const byte GFX_DEFENDER_FRAMES = 0;
const byte GFX_DEFENDER_FRAMESIZE = 9;
const byte GFX_ROCKET_WIDTH = 1;
const byte GFX_ROCKET_HEIGHT = 3;
const byte GFX_ROCKET_FRAMES = 0;
const byte GFX_ROCKET_FRAMESIZE = 1;
const byte GFX_BOMB_WIDTH = 5;
const byte GFX_BOMB_HEIGHT = 5;
const byte GFX_BOMB_FRAMES = 2;
const byte GFX_BOMB_FRAMESIZE = 5;
const byte GFX_INTRO_WIDTH = 64;
const byte GFX_INTRO_HEIGHT = 64;
const byte GFX_INTRO_FRAMES = 0;
const int GFX_INTRO_FRAMESIZE = 512;

int rocket_x;
int rocket_y;
int turret_x;
int invaders[ INVADER_WAVES * INVADER_ALIENS ];
int total_invaders;
int game_over;
int intro = true;
int intro_after = 0;
int defeat_after = 0;
int victory_after = 0;
int defender_win_animation_start = 0;
int game_timer_animation_start = 0;

// t: current time, b: beginning value, c: change in value, d: duration
int ease_cubic_in(int x, int t, int b, int c, int d) {
  float td= (float)t/(float)d;
  return (int)(c * td*td*td + b +.5);
}

boolean collides(
  const unsigned char* s1, int x1,int y1, int s1_width, int s1_height,
  const unsigned char* s2, int x2,int y2, int s2_width, int s2_height,
  boolean precise
) {
  boolean result = false;

  // Basic collision rectangle
  int cx = x1>x2 ? x1 : x2;
  int cw = x1>x2 ? x2+s2_width-x1 : x1+s1_width-x2;

  int cy = y1>y2 ? y1 : y2;
  int ch = y1>y2 ? y2+s2_height-y1 : y1+s1_height-y2;

  if (cw>0 && ch>0) {
    result = true;
  }

  // No bounding rect collision or no precise check requested
  if (!precise || !result) {
    return result;
  }

  // TODO: pixel-by-pixel collision test

  return result;
}

void defeat() {
////// FUNCTION BODY //////
arduboy.drawBitmap( WIDTH / 2 - GFX_INVADER_WIDTH / 2, HEIGHT / 2 - GFX_INVADER_HEIGHT * 2, gfx_invader + GFX_INVADER_FRAMESIZE*(_microcanvas_frame_counter / 15 & 1), GFX_INVADER_WIDTH, GFX_INVADER_HEIGHT, WHITE );
_microcanvas_fill_color = WHITE;
{
arduboy.setTextSize( 1 );
arduboy.setCursor( WIDTH / 2 - strlen(" OH NO!") * 6 / 2, HEIGHT / 2 );
arduboy.print( " OH NO!" );
};
if (defeat_after == 0) defeat_after = _microcanvas_frame_counter;
if (defeat_after > 0 && (_microcanvas_frame_counter - defeat_after) > 3 * 60) {
  _microcanvas_fill_color = WHITE;
  {
arduboy.setTextSize( 1 );
arduboy.setCursor( WIDTH / 2 - strlen("press [A] to retry") * 6 / 2, HEIGHT / 4 * 3 );
arduboy.print( "press [A] to retry" );
};
  if (arduboy.pressed( A_BUTTON )) {
  defeat_after = 0;
  setup();
}
}

}
void victory() {
////// FUNCTION BODY //////
if (!defender_win_animation_start) defender_win_animation_start = _microcanvas_frame_counter;
if (defender_win_animation_remaining() > 0) {
  int turret_y = defender_win_animation();
  arduboy.drawBitmap( turret_x - 5, turret_y, gfx_defender, GFX_DEFENDER_WIDTH, GFX_DEFENDER_HEIGHT, WHITE );
}
_microcanvas_fill_color = WHITE;
{
arduboy.setTextSize( 1 );
arduboy.setCursor( WIDTH / 2 - strlen(" HUMANITY PREVAILS!") * 6 / 2, HEIGHT / 2 );
arduboy.print( " HUMANITY PREVAILS!" );
};
arduboy.drawBitmap( WIDTH / 2 + 40, HEIGHT / 2 - 6, gfx_invader + GFX_INVADER_FRAMESIZE*(2), GFX_INVADER_WIDTH, GFX_INVADER_HEIGHT, WHITE );
if (victory_after == 0) victory_after = _microcanvas_frame_counter;
if (victory_after > 0 && (_microcanvas_frame_counter - victory_after) > 2 * 60) {
  int fall = (((_microcanvas_frame_counter - victory_after) - 2 * 60) * 2 + HEIGHT / 2) + 5;
  if (fall < HEIGHT) {
  arduboy.drawBitmap( WIDTH / 2 - 55, fall, gfx_invader + GFX_INVADER_FRAMESIZE*(2), GFX_INVADER_WIDTH, GFX_INVADER_HEIGHT, WHITE );
} else {
  _microcanvas_fill_color = WHITE;
  {
arduboy.setTextSize( 1 );
arduboy.setCursor( WIDTH / 2 - strlen("[A] to play again") * 6 / 2, HEIGHT / 4 * 3 );
arduboy.print( "[A] to play again" );
};
  if (arduboy.pressed( A_BUTTON )) {
  victory_after = 0;
  setup();
}
}
} else {
  arduboy.drawBitmap( WIDTH / 2 - 55, HEIGHT / 2 + 5, gfx_invader + GFX_INVADER_FRAMESIZE*(2), GFX_INVADER_WIDTH, GFX_INVADER_HEIGHT, WHITE );
}

}
int playgame() {
////// FUNCTION BODY //////
arduboy.clear();
int invasion = game_timer_animation();
int invader_x = invader_animation();
int start_x = (WIDTH - (INVADER_ALIENS * (GFX_INVADER_WIDTH + 4) - 4)) / 2;
int y = 0;
while (y < INVADER_WAVES) {
  int x = 0;
  while (x < INVADER_ALIENS) {
  if (invaders[ x + INVADER_ALIENS * y ]) {
  int d_y = invasion + y * (GFX_INVADER_HEIGHT + 1);
  if ((d_y + GFX_INVADER_HEIGHT) >= (HEIGHT - GFX_DEFENDER_HEIGHT)) {
  game_over = true;
  return;
}
  if (y % 2) {
  arduboy.drawBitmap( (start_x + (invader_x - 3)) + x * (GFX_INVADER_WIDTH + 4), d_y, gfx_invader + GFX_INVADER_FRAMESIZE*((invader_x >> 1) & 1), GFX_INVADER_WIDTH, GFX_INVADER_HEIGHT, WHITE );
  if (rocket_y >= 3 && collides( gfx_invader + GFX_INVADER_FRAMESIZE*((invader_x >> 1) & 1), (start_x + (invader_x - 3)) + x * (GFX_INVADER_WIDTH + 4), d_y, GFX_INVADER_WIDTH, GFX_INVADER_HEIGHT, gfx_rocket, rocket_x, rocket_y, GFX_ROCKET_WIDTH, GFX_ROCKET_HEIGHT, "false" )) {
  invaders[ x + INVADER_ALIENS * y ] = 0;
  total_invaders--;
  rocket_y = 0;
}
} else {
  arduboy.drawBitmap( (start_x - (invader_x - 3)) + x * (GFX_INVADER_WIDTH + 4), d_y, gfx_invader_2, GFX_INVADER_2_WIDTH, GFX_INVADER_2_HEIGHT, WHITE );
  if (rocket_y >= 3 && collides( gfx_invader_2, (start_x - (invader_x - 3)) + x * (GFX_INVADER_WIDTH + 4), d_y, GFX_INVADER_2_WIDTH, GFX_INVADER_2_HEIGHT, gfx_rocket, rocket_x, rocket_y, GFX_ROCKET_WIDTH, GFX_ROCKET_HEIGHT, "false" )) {
  invaders[ x + 8 * y ] = 0;
  total_invaders--;
  rocket_y = 0;
}
}
}
  x = x + 1;
}
  y = y + 1;
}
arduboy.drawBitmap( turret_x - 5, HEIGHT - GFX_DEFENDER_HEIGHT, gfx_defender, GFX_DEFENDER_WIDTH, GFX_DEFENDER_HEIGHT, WHITE );
if (rocket_y >= 3) {
  arduboy.fillRect( rocket_x, rocket_y, 1, 2, WHITE );
}
arduboy.fillRect( WIDTH / 2 - 7, 0, 13, 8, BLACK );
_microcanvas_fill_color = WHITE;
{
arduboy.setTextSize( 1 );
arduboy.setCursor( WIDTH / 2 - strlen(_microcanvas_textbuffer) * 6 / 2, 0 );
sprintf( _microcanvas_textbuffer, "%u", (int)floor( game_timer_animation_remaining() / 60 ) );
arduboy.print( _microcanvas_textbuffer );
};

}
int invader_animation() {
////// FUNCTION BODY //////
int t = _microcanvas_frame_counter % 120;
int x;
if (t < 60) {
  x = (0 + ((6 - 0) * (t - 0) * 10 / 60 + 5) / 10) | 0;
} else if (t < 120) {
  x = (6 + ((0 - 6) * (t - 60) * 10 / 60 + 5) / 10) | 0;
}
return x;

}
int defender_win_animation() {
////// FUNCTION BODY //////
int t = min( _microcanvas_frame_counter - defender_win_animation_start, DEFENDER_WIN_ANIMATION_DURATION );
int x = (ease_cubic_in( 0, t, 10 * (HEIGHT - GFX_DEFENDER_HEIGHT), -10 * (HEIGHT - GFX_DEFENDER_HEIGHT), DEFENDER_WIN_ANIMATION_DURATION ) + 5) / 10 | 0;
return x;

}
int defender_win_animation_remaining() {
////// FUNCTION BODY //////
return max( 60 - (_microcanvas_frame_counter - defender_win_animation_start), 0 );

}
int game_timer_animation() {
////// FUNCTION BODY //////
int t = min( _microcanvas_frame_counter - game_timer_animation_start, GAME_TIMER_ANIMATION_DURATION );
int duration_1 = 60 * 1; int duration_2 = 60 * 24;
int x;
if (t < duration_1) {
  x = (ease_cubic_in( 0, t, 10 * 0, 10 * 0, duration_1 ) + 5) / 10 | 0;
} else if (t <= (duration_1 + duration_2)) {
  x = (ease_cubic_in( 0, t - duration_1, 10 * 0, 10 * (((HEIGHT - GFX_DEFENDER_HEIGHT) - GFX_INVADER_HEIGHT) + 3), duration_2 ) + 5) / 10 | 0;
}
return x;

}
int game_timer_animation_remaining() {
////// FUNCTION BODY //////
return max( GAME_TIMER_ANIMATION_DURATION - (_microcanvas_frame_counter - game_timer_animation_start), 0 );

}

void setup() {
  _microcanvas_frame_counter = 0;

  // cpuLoad() will only be 0 right after boot
  if (!arduboy.cpuLoad()) arduboy.begin();

////// CUSTOM SETUP //////
turret_x = WIDTH / 2;
rocket_x = 0;
rocket_y = 0;
for (unsigned int _a_fill_idx_ = 0; _a_fill_idx_ < LENGTHOF( invaders ); ++_a_fill_idx_) invaders[_a_fill_idx_] = 1;
total_invaders = LENGTHOF( invaders );
game_over = false;
defender_win_animation_start = 0;
game_timer_animation_start = 0;
}

void loop() {
  if (!arduboy.nextFrame()) return;

  ++_microcanvas_frame_counter;
  if (_microcanvas_frame_counter==60000) _microcanvas_frame_counter = 0;

////// LOOP CONTENTS TO FOLLOW //////
arduboy.clear();
if (intro_after == 0) intro_after = _microcanvas_frame_counter;
if (intro) {
  if ((_microcanvas_frame_counter - intro_after) < 2 * 60) {
  arduboy.drawBitmap( WIDTH / 2 - 64 / 2, HEIGHT / 2 - 64 / 2, gfx_intro, GFX_INTRO_WIDTH, GFX_INTRO_HEIGHT, WHITE );
} else {
  intro = false;
  game_timer_animation_start = _microcanvas_frame_counter;
}
  arduboy.display(); return;
}
if (rocket_y < 3) {
  if (total_invaders > 0 && arduboy.pressed( A_BUTTON )) {
  rocket_y = HEIGHT - 3;
  rocket_x = turret_x - 1;
}
}
if (rocket_y >= 3) {
  rocket_y = rocket_y - 3;
}
if (total_invaders > 0 && !game_over) {
  if (arduboy.pressed( LEFT_BUTTON )) {
  turret_x = turret_x - 3;
}
  if (arduboy.pressed( RIGHT_BUTTON )) {
  turret_x = turret_x + 3;
}
  if (turret_x < 0) {
  turret_x = 0;
}
  if (turret_x > (WIDTH - GFX_DEFENDER_WIDTH / 2)) {
  turret_x = WIDTH - GFX_DEFENDER_WIDTH / 2;
}
  playgame();
} else {
  if (game_over) {
  defeat();
} else {
  victory();
}
}
////// END OF LOOP CONTENTS //////

  arduboy.display();
}

