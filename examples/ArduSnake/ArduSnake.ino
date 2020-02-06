/******************************************
 * Snake game for Arduino Uno and Adafruit 2,8" touch screen for Arduino
 * 
 * Written by Arnau/mas886/RedRedNose
 * Webpage: http://redrednose.xyz/
 * 
 * MIT license, all text above must be included in any redistribution
 * 
 * Adafruit libraries used:
 *  Adafruit_ILI9341(Screen controller library): https://github.com/adafruit/Adafruit_ILI9341
 *  Adafruit-GFX-Library(Graphics Library): https://github.com/adafruit/Adafruit-GFX-Library
 *  Adafruit_STMPE610(Touchscreen): https://github.com/adafruit/Adafruit_STMPE610
 *  
 ******************************************/
 
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD_ILI9342.h>
#include <TouchScreen.h>

// Assign human-readable names to some common 16-bit color values:
#define BLACK     0x0000
#define BLUE      0x001F
#define RED       0xF800
#define GREEN     0x07E0
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define WHITE     0xFFFF
#define LIGHTGREY 0xC618
#define MAROON    0x7800
#define NAVY      0x000F
#define DARKCYAN  0x03EF

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 160
#define TS_MINY 145
#define TS_MAXX 870
#define TS_MAXY 948

#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
Adafruit_TFTLCD_ILI9342 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

//1=main menu,2=game,3=game over
byte screen;

//Size of the snake (on proper situation it would be 22x23=506 positions of the grid, though it's limited by arduino's memory
#define snakesize 250
//Snake's delay between movement(miliseconds)
#define velocity 200

#define minx 10
#define maxx 50
#define miny 240
#define maxy 280

#define gridx 24
#define gridy 23

void setup(void) {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  Serial.println("Snake!");
  tft.begin();

  tft.setRotation(1);
  printMenu();
}

void printScore(int score){
  tft.fillRect(90,297,90,20,BLACK);
  tft.setCursor(90, 297);
  tft.print(score);
}

void updateSnake(byte newPos[], byte oldPos[]){
  tft.fillRect(oldPos[0]*10,oldPos[1]*10,10,10,BLACK);
  tft.fillRect(newPos[0]*10,newPos[1]*10,10,10,WHITE);
}


void printGameOver(int score, byte grid[gridy][gridx]){
  screen=3;
  for(int y=0;y<gridy;y++){
    for(int x=0;x<gridx;x++){
      grid[y][x]=0;
    }
  }
  tft.fillRect(40,55,160,180,LIGHTGREY);
  tft.drawRect(41,56,158,178,MAROON);
  tft.setTextColor(BLACK);
  tft.setTextSize(4);
  tft.setCursor(55, 60);
  tft.print("Game");
  tft.setCursor(90, 90);
  tft.print("Over");
  tft.setTextColor(NAVY);
  tft.setTextSize(3);
  tft.setCursor(70, 120);
  tft.print("Score");
  tft.drawRect(65,150,110,30,NAVY);
  tft.setTextColor(DARKCYAN);
  tft.setCursor(70, 155);
  tft.print(score);
  tft.fillRect(80,185,80,30,RED);
  tft.setCursor(85, 188);
  tft.setTextColor(WHITE);
  tft.print("Play");
}

void printGameScreen(){
  screen=2;
  tft.fillScreen(BLACK);
  tft.drawLine(0, 230, 240, 230, YELLOW);
  //Arrow up-1
  tft.drawLine(30,240,30,280,WHITE);
  tft.drawLine(30,240,10,260,WHITE);
  tft.drawLine(30,240,50,260,WHITE);
  //Arrow right-2
  tft.drawLine(70,260,110,260,WHITE);
  tft.drawLine(110,260,90,240,WHITE);
  tft.drawLine(110,260,90,280,WHITE);
  //Arrow down-3
  tft.drawLine(150,240,150,280,WHITE);
  tft.drawLine(150,280,130,260,WHITE);
  tft.drawLine(150,280,170,260,WHITE);
  //Arrow left-4
  tft.drawLine(190,260,230,260,WHITE);
  tft.drawLine(190,260,210,240,WHITE);
  tft.drawLine(190,260,210,280,WHITE);
  //----------------
  tft.drawLine(0, 290, 240, 290, YELLOW);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(10 , 297);
  tft.println("Score: ");
  printScore(0);
  tft.fillRect(180,295,50,20,RED);
  tft.setCursor(182, 297);
  tft.print("Menu");
}

void printMenu(){
  screen=1;
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);

  tft.setTextSize(5);
  tft.setCursor(30 , 30);
  tft.println("SNAKE!");

  tft.fillRect(50,120,120,50,GREEN);
  tft.setTextSize(3);
  tft.setCursor(63 , 133);
  tft.println("Start");
  
  tft.setCursor(0 , 310);
  tft.setTextSize(1);
  tft.println("Modified by Milo");

  // Somehow the last text does not show correctly without this delay...
  delay(10);
}

bool initgame=false;
byte arrow;
void loop() {
  byte snakeBuffer[snakesize][2]={0};
  byte grid[gridy][gridx]={0};
  long addpos;
  long delpos;
  byte lastarrow;
  int score;
  byte newPos[2];
  byte oldPos[2];
  bool gendot;
  bool incaxys;
  byte increment;
  
  while(true){
    //This if will update the snake position while we are in the game screen
    if (screen==2){
      if((arrow!=0)&&(((lastarrow+2!=arrow)&&(lastarrow-2!=arrow))||(score==0))){
        if((arrow%2)!=0){
          incaxys=true;
        }else{
          incaxys=false;
        }
        if((arrow>1)&&(arrow<4)){
          increment=1;
        }else{
          increment=-1;
        }
        lastarrow=arrow;
      }
      arrow=0;
      newPos[incaxys]+=increment;
      if((newPos[0]<0)|(newPos[0]>23)|(newPos[1]<0)|(newPos[1]>22)){        
        printGameOver(score,grid);
        
      }else{
        snakeBuffer[addpos][0]=newPos[0];
        snakeBuffer[addpos][1]=newPos[1];
        addpos++;
        if(addpos>(snakesize-1)){
          addpos=0;
        }
        grid[newPos[1]][newPos[0]]++;
        grid[oldPos[1]][oldPos[0]]=0;

        //Check various grid events
        switch(grid[newPos[1]][newPos[0]]){
          case 2:
            printGameOver(score, grid);
            break;
          case 4:
            score+=10;
            printScore(score);
            gendot=true;
            if((score/10)<snakesize){
              delpos--;
            }
          default:
            updateSnake(newPos,oldPos);
            delpos++;
            if(delpos>(snakesize-1)){
              delpos=0;
            }
            oldPos[0]=snakeBuffer[delpos][0];
            oldPos[1]=snakeBuffer[delpos][1];
            break;
        }
      }
      //We generate a random dot to feed the snake
      while(gendot){
        byte posy=random(gridy-1);
        byte posx=random(gridx-1);
        if(grid[posy][posx]==0){
          tft.fillRect(posx*10,posy*10,10,10,CYAN);
          grid[posy][posx]=3;
          gendot=false;
        }
      }
    }
    handle_touch_screen();
    if (initgame){
      //Set variables for the game initialization
      initgame=false;
      arrow=2;
      lastarrow=1;
      score=0;
      addpos=1;
      delpos=0;
      incaxys=false;
      increment=1;
      gendot=true;
      newPos[0]=-1;newPos[1]=0;
      printGameScreen();
    }
  }
}

void handle_touch_screen() {
  for(int i=0;i<velocity/10;i++) {
    delay(10);
    TSPoint p = ts.getPoint();
    pinMode(XM, OUTPUT);
    pinMode(XP, OUTPUT);
    pinMode(YM, OUTPUT);
    pinMode(YP, OUTPUT);
  
    // Button control, only triggered when there's data from the touchscreen
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      // Retrieve a point  
      p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
      p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
        
      switch(screen){
        case 1:
          //menu and start game parameters
          if((p.x>=50)&&(p.x<=170)&&(p.y>=120)&&(p.y<=170)){
            initgame=true;
          }
          break;
        case 2:
          //Check arrows on in game screen
          if(p.x>=10 && p.x<=230 && p.y>=240 && p.y<=280){
            for(int c=0,g=1;c<=180;c+=60,g++){
              if(p.x>=minx+c && p.x<=maxx+c && p.y>=miny && p.y<=maxy){
                arrow=g;
                break;
              }
            }
          } else if((p.x>=180)&&(p.x<=230)&&(p.y>=295)&&(p.y<=315)){
            printMenu();
          }
          break;
        case 3:
          if((p.x>=180)&&(p.x<=230)&&(p.y>=295)&&(p.y<=315)){
            printMenu();
          }else if((p.x>=80)&&(p.x<=160)&&(p.y>=185)&&(p.y<=215)){
            initgame=true;
          }
          break;
      }
    }
  }
}

