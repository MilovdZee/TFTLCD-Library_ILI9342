#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD_ILI9342.h> // Hardware-specific library
#include "TouchScreen.h"

// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0 
#define WHITE           0xFFFF

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0 
#define LCD_RESET A4
Adafruit_TFTLCD_ILI9342 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin

#define TS_MINX 160
#define TS_MINY 145
#define TS_MAXX 870
#define TS_MAXY 948

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Touch screen settings
#define MINPRESSURE 10

// TFT settings
#define LCDWIDTH tft.width()
#define LCDHEIGHT tft.height()

#define BRICKROWS 6
#define BRICKCOLUMNS 10

#define BRICKWIDTH (LCDWIDTH / BRICKCOLUMNS)
#define BRICKHEIGHT (LCDHEIGHT / 32)
#define BRICKCOLOR GREEN

#define BALLSIZE 8
#define BALLCOLOR RED

#define BATWIDTH (LCDWIDTH / 5)
#define BATHEIGHT (LCDHEIGHT / 25)
#define BATCOLOR YELLOW

#define SCORE_HEIGHT 50

#define DELAY 15

int bricks [BRICKROWS][BRICKCOLUMNS];

int score = 0;
int max_score = 0;
int visible_bricks;

struct ball {
  int x;
  int y;
  int x_vel;
  int y_vel;
} ball;

struct bat {
  int x;
  int y;
} bat;

void start_ball() {
  // Set the initial ball position and speed
  ball.x = LCDWIDTH / 2;
  ball.y = LCDHEIGHT * 3 / 4;
  ball.x_vel = BALLSIZE / 2;
  ball.y_vel = BALLSIZE / 2;
}

void start_game() {
  start_ball();

  // Set the initial bat position
  bat.x = LCDWIDTH / 2;
  bat.y = LCDHEIGHT * 5 / 6;
  
  // Initialize all bricks
  for (int l=0; l<BRICKROWS; l++) {
    for (int c=0; c<BRICKCOLUMNS; c++) {
      bricks[l][c] = BRICKCOLOR;
    }
  }
  visible_bricks = BRICKROWS * BRICKCOLUMNS;

  // Erase the screen
  tft.fillScreen(BLACK);

  draw_bricks();
  draw_score();
  draw_bat(BATCOLOR);
}

void draw_bricks() {
  for (int row=0; row<BRICKROWS; row++)
    for (int column=0; column<BRICKCOLUMNS; column++) {
        tft.fillRect(column*BRICKWIDTH + 2, SCORE_HEIGHT + row*BRICKHEIGHT + 2, BRICKWIDTH -4, BRICKHEIGHT - 4, bricks[row][column]);
    }
}

void handle_brick_collision() {
  // find the relevant brick
  int row = (ball.y - SCORE_HEIGHT) / BRICKHEIGHT;
  int column = ball.x / BRICKWIDTH;
  if(row < BRICKROWS && column < BRICKCOLUMNS) {
    if(bricks[row][column] != BLACK) {
      bricks[row][column] = BLACK;
      tft.fillRect(column*BRICKWIDTH + 2, SCORE_HEIGHT + row*BRICKHEIGHT + 2, BRICKWIDTH -4, BRICKHEIGHT - 4, bricks[row][column]);
      visible_bricks--;
      if(visible_bricks == 0) {
        show_end();
      }

      // Bounce the ball
      draw_ball(BLACK);
      ball.y -= 2 * ball.y_vel;
      ball.y_vel *= -1;
      draw_ball(BALLCOLOR);

      // Increase score
      score++;
      if(score > max_score) max_score = score;
      draw_score();
    }
  }
}

void handle_bat_collision() {
  int bat_left = bat.x - BATWIDTH / 2;
  int bat_right = bat.x + BATWIDTH / 2;
  int bat_top = bat.y + BATHEIGHT / 2;
  int bat_bottom = bat.y - BATHEIGHT / 2;

  int ball_left = ball.x - BALLSIZE / 2;
  int ball_right = ball.x + BALLSIZE / 2;
  int ball_top = ball.y + BALLSIZE / 2;
  int ball_bottom = ball.y - BALLSIZE / 2;
  
  if(ball_left <= bat_right && ball_right >= bat_left && 
     ball_bottom <= bat_top && ball_top >= bat_bottom) {
      // Bounce the ball
      draw_ball(BLACK);
      ball.y -= 2 * ball.y_vel;
      ball.y_vel *= -1;
      draw_ball(BALLCOLOR);

      draw_bat(BATCOLOR);
  }
}

void draw_ball(int c) {
  tft.fillRect(ball.x - BALLSIZE/2, ball.y - BALLSIZE/2, BALLSIZE, BALLSIZE, c);
}

void draw_bat(int c) {
  tft.fillRect(bat.x - BATWIDTH/2, bat.y - BATHEIGHT/2, BATWIDTH, BATHEIGHT, c);
}

void move_ball() {
  // Erase existing ball position
  draw_ball(BLACK);
  
  ball.x = ball.x + ball.x_vel;
  ball.y = ball.y + ball.y_vel;

  if(ball.x + BALLSIZE / 2 >= LCDWIDTH || ball.x - BALLSIZE / 2 <= 0) {
    ball.x -= 2 * ball.x_vel;
    ball.x_vel *= -1;
  }

  if(ball.y + BALLSIZE / 2 >= LCDHEIGHT || ball.y - BALLSIZE / 2 <= SCORE_HEIGHT) {
    if(ball.y + BALLSIZE / 2 >= LCDHEIGHT) {
      // Ball hits the bottom
      score = 0;
      draw_score();
    }
    ball.y -= 2 * ball.y_vel;
    ball.y_vel *= -1;
  }

  // Draw the ball in the new position
  draw_ball(BALLCOLOR);
}

void move_bat() {
  // Read touch screen position
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE) {
    // Erase the bat at the current position
    draw_bat(BLACK);

    // Map the touch location to the TFT location
    bat.x = map(p.x, TS_MINX, TS_MAXX, LCDWIDTH, 0);
    bat.y = map(p.y, TS_MINY, TS_MAXY, 0, LCDHEIGHT);

    // Draw the bat at the new position
    draw_bat(BATCOLOR);
  }
}

int previous_score = score;
int previous_max_score = score;
void draw_score() {
  tft.setTextSize(3);

  // Erase old score
  tft.setCursor(0,0);
  tft.setTextColor(BLACK);
  tft.print("Score: ");
  tft.println(previous_score);
  tft.print("Max: ");
  tft.println(previous_max_score);

  // Print new score
  tft.setCursor(0,0);
  tft.setTextColor(BLUE);
  tft.print("Score: ");
  tft.println(score);
  tft.print("Max: ");
  tft.println(max_score);
  
  previous_score = score;
  previous_max_score = max_score;
}

void show_end() {
  tft.fillScreen(BLACK);
  tft.fillScreen(RED);
  tft.fillScreen(GREEN);
  tft.fillScreen(BLUE);
  tft.fillScreen(BLACK);

  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.setTextColor(YELLOW);
  tft.println("SCORE");
  tft.println();
  tft.setTextSize(12);
  tft.print(max_score);

  delay(5000);
  start_game();
}

void setup() {
  Serial.begin(9600);
  Serial.println("Breakout");
  
  tft.reset();
  uint16_t identifier = tft.readID();
  if(identifier != 0x9342) {
    Serial.print("Only works for ILI 9342. Received: 0x");
    Serial.println(identifier);
    return;
  }
  tft.begin();

  tft.setRotation(1);

  start_game();
}

void loop() {
  move_ball();
  move_bat();
  
  handle_brick_collision();
  handle_bat_collision();
  
  delay(DELAY);
}


