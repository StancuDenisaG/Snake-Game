#include <LiquidCrystal.h>
#include "LedControl.h"
#include <EEPROM.h>

// LCD variables
const byte RS = 9;
const byte enable = 8;
const byte d4 = 7;
const byte d5 = 3;
const byte d6 = 5;
const byte d7 = 4;
const byte lcdbrightness = 6;
LiquidCrystal lcd(RS,enable,d4,d5,d6,d7);


// joystick variables
const int pinSW = 2;  
const int pinX = A1;  
const int pinY = A0;  

byte swState = LOW;
int xValue = 0;
int yValue = 0;

const int lowerThreshold = 200;
const int upperThreshold = 800;
bool joyMoved = 0;

const int debounceDelay = 50;
unsigned long int lastDebounce = 0;

byte lastReading = LOW;
byte reading = LOW;

// matrix variables
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin,1);  //DIN, CLK, LOAD, No.

//buzzer
const int buzzerPin = 13;
const int buzzerDead = 350;
const int buzzerJoystickMove = 600;
const int buzzerEatFood = 500;



byte matrixBright = 2;
const int maxMatrixBrigthness;
byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;
unsigned long long lastMoved = 0;
bool matrixChanged = true;
byte xLastFood = 0;
byte yLastFood = 0;
unsigned long previousMillis = 0;
int namePos=0;

struct Player {
  int score;
  char name[3];
};

//eeprom variables
byte matrixBrightness = 1;
int LCDbrightness;
byte difficulty = 1;
byte audio = 1;

Player dummyPlayer;

Player standard = {0, 'AAA'};

const long delayPeriod = 2000;

String mainMenu[5] = {
  "Start",
  "High scores",
  "Settings",
  "About",
  "How to play"  
};

String settingsMenu[7] = {
  "Player name",
  "Difficulty",
  "Reset highscores",
  "LCD brightness",
  "Matrix brightness",
  "Audio",
  "Return to menu"
};


int state = 0;
int lastState = 0;
// current menu pos
int menuPos = 0;
//settings menu pos
int setPos = 0;
byte leaderboard = 0;
byte leaderboardPos = 0;


int highscores[5];
String highscoreNames[5] = { "", "", "", "", "" };


//game variables
int snakeSpeed = 700; 
int place = 0; //highsore place
int maxlength = 3;
char direction;
int row = 4;
int col = 4;
bool restart = true;
bool dead = false;
int matrix [8][8]; 
int foodPos [8][8];
bool createFood = true;
int xFood; 
int yFood; 
int foodBlink = 200; 
bool blinking = false;
unsigned long lastFood;
unsigned long lastTime;


void sounds(const int sound){
      if (audio ==1){
      tone(buzzerPin, sound, 30); 
    }
}
void setup() {

  matrixBrightness = EEPROM.read(1); 
  LCDbrightness = EEPROM.read(2);
  audio = EEPROM.read(3);
  difficulty = EEPROM.read(4);




  pinMode(pinSW, INPUT_PULLUP);  // activate pull-up resistor on the push-button pin
  analogWrite(lcdbrightness, LCDbrightness * 85);
  lc.shutdown(0, false);  // turn off power saving,enables display
  lc.setIntensity(0, matrixBrightness * 3);  // sets brightness(0~15 possiblevalues)
  lc.clearDisplay(0);  // clear screen
 

  Serial.begin(9600);


  lcd.begin(16, 2);
  lcd.print("WELCOME!");
  lcd.setCursor(4, 1);
  lcd.print("Have fun!");


  delay(delayPeriod);
  lcd.clear();
  dummyPlayer.score = 0;
  for(int i=0; i<3; i++){
      lcd.print(dummyPlayer.name[i]='A');

  
  }


}


void loop() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  reading = digitalRead(pinSW);
  if (state == 0) {

    printMenu();

  } else if (state == 1) {
    matrixGame();
    gamerMenu();
  } else if (state == 2) {
    scoresSection();
  } else if (state == 3) {
    settingsSection();
  } else if (state == 4) {  
    aboutSection(); 
  } else if (state == 5) {
    howToPlaySection();
  } else if (state == 6) {
    lcdBrightness();
  } else if (state == 7) {
    setMatrixBrightness();
  } else if (state == 8) {
    setDifficulty();
  } else if (state == 9) {
    changeName();
  } else if (state == 10) {
    setAudio();
  } else if (state == 11) {
    resetHighscores();
  }
}

void printMenu() {
  mainMenuMovement();
  joystickButton();
  lcd.setCursor(0, 0);
  lcd.print("      Menu    ");
  lcd.setCursor(0, 1);
  lcd.print(">");
  lcd.print(mainMenu[menuPos]);

}


void gamerMenu() {
  lc.setLed(0, row, col, true);
  matrix[row][col] = 1;
  lcd.setCursor(0, 0);
  for (int i = 0; i < 3; i++) {
    lcd.print(dummyPlayer.name[i]);
  }

  lcd.print("      ");
  lcd.print("Lvl: ");
  lcd.print(difficulty);
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(dummyPlayer.score);
  lcd.print("/");
  lcd.print(standard.score);
  lcd.print("    ");
  joystickButton();
}
void matrixGame() {

  if (dead == false) {
    xValue = analogRead(pinX);
    yValue = analogRead(pinY);
    updatePositions();
    food();
    movement();

  } else //game over
  {
    sounds(buzzerDead);

    if (dummyPlayer.score > standard.score) {
      standard.score = dummyPlayer.score;
      dummyPlayer.score = 0;

    } else {
      dummyPlayer.score = 0;
    }
    updateHighscores();

    endGame();
    delay(500);
    col = 4;
    row = 4;
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {

        matrix[x][y] = 0;

      }
    }
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {

        foodPos[x][y] = 0;

      }
    }
    maxlength = 3;

    dead = false;
    lc.clearDisplay(0);
    lc.setLed(0, row, col, true);
    matrix[row][col] = 1;

  }
}
void food() {
  if (createFood == true) { //generate food
    createFood = false;
    yFood = random(8);
    xFood = random(8);
    while (matrix[xFood][yFood] != 0) {
      yFood = random(8);
      xFood = random(8);
    }
    foodPos[xFood][yFood] = 1;
  }
  if (xFood == row && yFood == col && createFood == false) { // collect food
    createFood = true;
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {

        foodPos[x][y] = 0;

      }
    }
    dummyPlayer.score = dummyPlayer.score + difficulty;
    maxlength++;

    if (dummyPlayer.score > 3 && difficulty == 1) {
      difficulty++;
    } else if (dummyPlayer.score > 5 && difficulty == 2) {
      difficulty++;
    }

    if (audio == 1) {
      tone(buzzerPin, buzzerEatFood, 30); //sound everytime score goes up
    }
    if (difficulty == 2) {
      snakeSpeed = snakeSpeed - 20;
    } else if (difficulty == 3) {
      snakeSpeed = snakeSpeed / 2;
    }

  }
  if (createFood == false) { // blink
    if (blinking == false && millis() - lastFood > foodBlink) {
      lc.setLed(0, xFood, yFood, true);
      lastFood = millis();
      blinking = true;
    } else if (blinking == true && millis() - lastFood > foodBlink) {
      lc.setLed(0, xFood, yFood, false);
      lastFood = millis();
      blinking = false;
    }
  }

}
void movement(){
  
  if (millis()- lastTime > snakeSpeed){
    lastTime = millis();
  switch (direction){
    case 'U':
      col --;

      break;
    case 'L':
      row --;
      break;
    case 'D':
       col ++;
      break;
     
    case 'R':
      row ++;
      break;
    default:

      break;
  
  }
  if (direction == 'L' ||direction == 'R' ||direction == 'D' ||direction == 'U' ){
  if (row <0 || row >7 || col < 0 || col > 7) //check if snake hit a wall
  { 
      restart = true;
      dead = true;
     
      }

  
  

  else if(matrix [row][col] > 0 && matrix [row][col] != maxlength ) //check if snake hits itself 
   {
    restart = true;
    dead = true;
       
  }
  else{
      for(int x = 0; x <8 ; x ++){    //remove last part of snake
  	    for(int y = 0 ; y <8 ; y++){
          if (matrix[x][y] == maxlength){
            matrix[x][y] = 0;
            lc.setLed(0,x,y,false);
          }
        }

      }

     // move numbers back
     for(int l = maxlength - 1; l > 0 ; l --){
       for(int x = 0; x <8 ; x ++){    
  	    for(int y = 0 ; y <8 ; y++){
          if (matrix[x][y] == l){
            matrix[x][y] = l + 1;
            
          }
        }
      
     }
     //set new point
     matrix [row][col] = 1;
     lc.setLed(0,row,col,true);

    }
    }
   }
  }
  }
void updatePositions(){
  if (xValue > upperThreshold){
    if(direction != 'U'){
      direction = 'D';
  }}

  else if(xValue < lowerThreshold){
    if(direction != 'D'){
      direction = 'U';
    }
  }
  
  else if (yValue > upperThreshold){
    if (direction != 'R'){
    direction = 'L';
    }
  }
  else if (yValue < lowerThreshold)
  {
    if (direction != 'L'){
    direction = 'R';
  }}
  else if(restart == true){
    restart = false;
    direction= 'p';

  }
}


void endGame() {
if (place > 0){
     lcd.setCursor(0,0);
     lcd.print(" Congratulation!! ");
     lcd.setCursor(0,1);
     lcd.print("    You're #");
     lcd.print(place);   
 
     }
    else{
     lcd.setCursor(0,0);
     lcd.print("  Better luck   ");
     lcd.setCursor(0,1);
     lcd.print("    next time!");
     lcd.print(place);   
  
    }

}


void scoresSection() {
  getHighscores();  
  
  lcd.setCursor(0, 0);
  lcd.print("High Scores");

  if (yValue > upperThreshold  && joyMoved == 0) { 
    leaderboardPos = (leaderboardPos != 0) ? leaderboardPos - 1 : 4;
    joyMoved++;
    sounds(buzzerJoystickMove);
  } else if (yValue < lowerThreshold && joyMoved == 0) { 
    leaderboardPos = (leaderboardPos != 4) ? leaderboardPos + 1 : 0;
    joyMoved++;
    sounds(buzzerJoystickMove);
  } else if (yValue < upperThreshold && yValue > lowerThreshold) {
    joyMoved = 0;
  }
  
  lcd.setCursor(0, 1);
  lcd.print(leaderboardPos + 1);
  lcd.print(".");
  lcd.print(highscoreNames[leaderboardPos]);
  lcd.print(" - ");
  lcd.print(highscores[leaderboardPos]);
  lcd.print("   ");
  
  joystickButton();    
}



void settingsSection () {
  
  lcd.setCursor(0, 0);
  lcd.print("    Settings    ");
  if (yValue > upperThreshold  && joyMoved == 0 ) { 
    if (setPos >= 0) {
      setPos--;
    } 
    else setPos = 6;

    joyMoved++;
    sounds(buzzerJoystickMove);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(settingsMenu[setPos]);

  } else if (yValue < lowerThreshold && joyMoved == 0) { 
    if (setPos <= 6) {
      setPos++;
    } 
    else setPos = 0;
    
    joyMoved++;
    sounds(buzzerJoystickMove);
    lcd.clear();
    lcd.setCursor(0, 1);
     lcd.print(settingsMenu[setPos]);

  } else if (yValue < upperThreshold && yValue > lowerThreshold) {
      joyMoved = 0;
    }
    
  joystickButton();    


}


void mainMenuMovement() {

  if (yValue > upperThreshold  && joyMoved == 0) { 
    if (menuPos != 0) {
      menuPos--;
    } 
    else menuPos = 4;

    joyMoved++;
    sounds(buzzerJoystickMove);
    lcd.clear();

  } else if (yValue < lowerThreshold && joyMoved == 0) { 
    if (menuPos != 4) {
      menuPos++;
    } 
    else menuPos = 0;
    
    joyMoved++;
    sounds(buzzerJoystickMove);
    lcd.clear();

  } else if (yValue < upperThreshold && yValue > lowerThreshold) {
      joyMoved = 0;
    }
}



void joystickButton() {
  if (lastReading != reading) {
      lastDebounce = millis();
    }
  if ((millis() - lastDebounce) >= debounceDelay) {
    if (swState != reading) {
      swState = reading;

      if (!swState) {
          switch(menuPos) {
            case 0:             //start game
              if (state == 0) {
                state = 1;
                lcd.clear();
              } else  {
                state = 0;
                lcd.clear();
              }
              break;
            case 1:
              if (state == 0) { //show high scores
                state = 2;
                lcd.clear();
              } else {
                state = 0;
                lcd.clear();
              }
              break;
            case 2:  //settings
               if (state == 0 ) {
                state = 3;
                lcd.clear();
              } else if (state != 0 && setPos == 6 ) { //return to main menu 
                state = 0;
                lcd.clear();
              } else if (setPos == 0) {  //change player name
                state=9;
                lcd.clear();
                setPos =6;
              } else if (setPos == 1) { //set difficulty 
                state=8;
                lcd.clear();
                setPos =6;
              } else if (setPos == 2) {//reset high score
                state=11;
                lcd.clear();
                setPos =6;
              } else if ( setPos == 3) {  //lcd brightness
                state=6;
                lcd.clear();
                setPos =6;
                
              } else if (setPos == 4) { //matrix brightness
                state=7;
                lcd.clear();
                setPos =6;
              } else if (setPos == 5) { //audio 
                 state=10;
               
                setPos =6;
                 lcd.clear();}
  
              break;
            case 3:  //about section
              if (state == 0) {
                state = 4;
                lcd.clear();
              } else {
                state = 0;
                lcd.clear();
              }
              break;
            case 4: //how to play
             if (state == 0) {
                state = 5;
                lcd.clear();
              } else {
                lcd.clear();
                state = 0;
               
              }
              break;
          }
        }
        }
      }

  lastReading = reading;
}

void aboutSection(){
 
  lcd.setCursor(0, 0);
  lcd.print("Denisa:  Snake");
  scrollText("github.com/StancuDenisaG", 1);
  joystickButton();  


 
}
void howToPlaySection(){
 
  lcd.setCursor(0, 0);
  lcd.print("Instructions:");
  scrollText(" Once the game starts move joystick. The game ends if the snake eats itself or goes through walls ", 1);
  joystickButton();
};



char letterPos(char *text, int pos)  
{
  if (pos<16) return ' '; 
  else if (pos>=16 && pos<16+strlen(text))
    return text[pos-16]; 
  else return ' ';  
}

void scrollText(char *text, int line) //scrolls text on a chosen line
{
  char currenttext[16+1];
  static unsigned long nextscroll[2];
  static int posCounter[2]; 
  int i;
  if (millis()>nextscroll[line])
  {
    nextscroll[line]=millis()+500; 

    for (i=0;i<16;i++)
    currenttext[i]=letterPos(text,posCounter[line]+i);
    currenttext[16]=0;    
    lcd.setCursor(0,line);
    lcd.print(currenttext);
  
    posCounter[line]++;  //moving left to right
    if (posCounter[line]==strlen(text)+16){
      posCounter[line]=0;

    } 

  }
}

void changeName() {


  joystickButton();
  lcd.setCursor(0, 1);
  lcd.print(dummyPlayer.name);
  changeLetter();
}

void setNamePos() {
  if (yValue < lowerThreshold && joyMoved == 0 && namePos > 0 ) {
 
      namePos--;
      joyMoved++;
     lcd.setCursor(namePos+1, 0);
     lcd.print(' ');
         
      lcd.setCursor(namePos, 0);
      lcd.print('*');
      sounds(buzzerJoystickMove);      

    }

  
   else if (yValue > upperThreshold && joyMoved == 0 && namePos<2) {
      namePos++;
      joyMoved++;
      lcd.setCursor(namePos-1, 0);
      lcd.print(' ');
      lcd.setCursor(namePos, 0);
      lcd.print('*');

      sounds(buzzerJoystickMove);
  } else if (yValue < upperThreshold && yValue > lowerThreshold) {
    joyMoved = 0;
  

  }
}
void changeLetter() {
  char name[3];
  strcpy(name, dummyPlayer.name);
  lcd.setCursor(0, 1);
  lcd.print(name);
  if (xValue < lowerThreshold && joyMoved == 0) {
    if (name[namePos] > 'A') {
      name[namePos]--;
      joyMoved++;

      lcd.setCursor(namePos, 1);
      lcd.print(name[namePos]);
    } else if (name[namePos] == 'A') {
      name[namePos] = 'Z';
      joyMoved++;

    }
  } else if (xValue > upperThreshold && joyMoved == 0) {
    if (name[namePos] < 'Z') {
      name[namePos]++;
      joyMoved++;

    } else if (name[namePos] == 'Z') {
      name[namePos] = 'A';
      joyMoved++;

    }

  } else if (xValue < upperThreshold && xValue > lowerThreshold) {
    joyMoved = 0;

    delay(150);
    setNamePos();
    

  }

  strcpy(dummyPlayer.name, name);

}

void setDifficulty(){
  lcd.setCursor(3, 0);
  lcd.print("Difficulty");  
  lcd.setCursor(9, 1);
  lcd.print(difficulty);
  joystickButton();

  if(xValue < lowerThreshold &&  joyMoved == 0){
    if (difficulty > 1){
      difficulty--;
      joyMoved++;
      sounds(buzzerJoystickMove);
      lcd.setCursor(9, 1);
      lcd.print(difficulty);

  
      }       
      }         
  else if (xValue > upperThreshold &&  joyMoved == 0){
    if (difficulty < 3){    
    difficulty++;
    joyMoved++;
    sounds(buzzerJoystickMove);
    lcd.setCursor(9, 1);
    lcd.print(difficulty);   

    }
  }
  else if (xValue < upperThreshold && xValue > lowerThreshold) {
    joyMoved = 0;
    EEPROM.update(4, difficulty);
    } 

  
};

void lcdBrightness(){
   lcd.setCursor(7, 0);
  lcd.print("LCD");
  lcd.setCursor(8, 1);
  lcd.print(LCDbrightness);
   joystickButton();
 
  if(xValue < lowerThreshold &&  joyMoved == 0){
    if (LCDbrightness > 1){
      LCDbrightness--;
      sounds(buzzerJoystickMove);
      joyMoved++;
      lcd.setCursor(8, 1);
      lcd.print(LCDbrightness);}       
      }         
  else if (xValue > upperThreshold &&  joyMoved == 0){
    if (LCDbrightness < 3){    
    LCDbrightness++;
    joyMoved++;
    sounds(buzzerJoystickMove);
    lcd.setCursor(8, 1);
    lcd.print(LCDbrightness);   
    }
  }
  else if (xValue < upperThreshold && xValue > lowerThreshold) {
      joyMoved = 0;
      analogWrite(lcdbrightness, LCDbrightness * 85);
      EEPROM.update(2, LCDbrightness);
    }  

};

void setMatrixBrightness(){
  joystickButton();
  lcd.setCursor(4, 0);
  lcd.print("Matrix");
  lcd.setCursor(5, 1);
  lcd.print(matrixBrightness);
  if(xValue < lowerThreshold &&  joyMoved == 0){
    if (matrixBrightness > 1){
      matrixBrightness--;
      sounds(buzzerJoystickMove);
      joyMoved++;
      lcd.setCursor(5, 1);
      lcd.print(matrixBrightness);
  }       
  }         
  else if (xValue > upperThreshold &&  joyMoved == 0){
    if (matrixBrightness < 3){    
    matrixBrightness++;
    joyMoved++;
    sounds(buzzerJoystickMove);
    lcd.setCursor(5, 1);
    lcd.print(matrixBrightness);   
    }
    
  }
else if (xValue < upperThreshold && xValue > lowerThreshold) {
      joyMoved = 0;
      lc.setIntensity(0, 5 * matrixBrightness);
      EEPROM.update(1, matrixBrightness);
    }    
};

void setAudio() {
  joystickButton();
  lcd.setCursor(5, 0);
  lcd.print("Audio");
  lcd.setCursor(6, 1);
  if (audio == 1) {

    lcd.print("ON ");
    Serial.println(audio);
  } else {

    lcd.print("OFF");
  }

  if (xValue < lowerThreshold && joyMoved == 0) {
    if (audio == 1) {
      audio = 0;
      lcd.setCursor(6, 1);
      lcd.print("OFF");
      joyMoved++;
      sounds(buzzerJoystickMove);
    } else {
      audio = 1;

      lcd.setCursor(6, 1);
      lcd.print("ON ");
      joyMoved++;
      sounds(buzzerJoystickMove);
    }
  } else if (xValue > upperThreshold && joyMoved == 0) {
    if (audio == 1) {
      audio = 0;
      lcd.setCursor(6, 1);
      lcd.print("OFF");
      joyMoved++;
      sounds(buzzerJoystickMove);

    } else {
      audio = 1;
      lcd.setCursor(6, 1);
      lcd.print("ON ");
      joyMoved++;
      sounds(buzzerJoystickMove);

    }
  } else if (xValue < upperThreshold && xValue > lowerThreshold) {
    joyMoved = 0;

    EEPROM.update(3, audio);
  }
};
void getSettingsFromEEPROM() {
  matrixBrightness = EEPROM.read(1); 
  LCDbrightness = EEPROM.read(2);
  audio = EEPROM.read(3);
  difficulty = EEPROM.read(4);
 
}

void resetHighscores() {
  for (int i = 1; i <= 5; ++i) {
    EEPROM.update(5 *i, 0);
    EEPROM.update(i * 5 + 1, 'A');
    EEPROM.update(i * 5 + 2, 'B');
    EEPROM.update(i * 5 + 3, 'C');
  };

  scrollText("High scores have been reset",0);
  lcd.setCursor(0, 1);
   lcd.print("Press to return");
  joystickButton();
}
void getHighscores() {
  for (int i = 1; i <= 5; ++i) {
    highscores[i - 1] = EEPROM.read(5 * i);
    int firstLetter = EEPROM.read(i * 5 + 1);
    int secondLetter = EEPROM.read(i* 5 + 2);
    int thirdLetter = EEPROM.read(i* 5 + 3);
    String name = "";
    name += char(firstLetter);
    name += char(secondLetter);
    name += char(thirdLetter);
    highscoreNames[i - 1] = name;
    Serial.print(highscores[i - 1]);
    Serial.print(" - ");
    Serial.println(highscoreNames[i - 1]);
  }
}

void updateHighscores() {
  int score = standard .score;
  char name[3];
  strcpy(name, dummyPlayer.name);
  for (int i = 1; i <= 5; ++i) {
    highscores[i - 1] = EEPROM.read(5 * i);

  }
  if (standard .score > highscores[0]) {
    place = 1;
    for (int i = 5; i > 1; i--) {
      EEPROM.update(5 * i, EEPROM.read(5 * (i - 1)));
      EEPROM.update(5 * i + 1, EEPROM.read(5 * (i - 1) + 1));
      EEPROM.update(5 * i + 2, EEPROM.read(5 * (i - 1) + 2));
      EEPROM.update(5 * i + 3, EEPROM.read(5 * (i - 1) + 3));
    }

    EEPROM.update(5, score);
    EEPROM.update(6, name[0]);
    EEPROM.update(7, name[1]);
    EEPROM.update(8, name[2]);

  } else if (standard .score > highscores[1]) {
    place = 2;
    for (int i = 5; i > 2; i--) {
      EEPROM.update(5 * i, EEPROM.read(5 * (i - 1)));
      EEPROM.update(5 * i + 1, EEPROM.read(5 * (i - 1) + 1));
      EEPROM.update(5 * i + 2, EEPROM.read(5 * (i - 1) + 2));
      EEPROM.update(5 * i + 3, EEPROM.read(5 * (i - 1) + 3));
    }

    EEPROM.update(10, score);
    EEPROM.update(11, name[0]);
    EEPROM.update(12, name[1]);
    EEPROM.update(13, name[2]);

  } else if (standard .score > highscores[2]) {
    place = 3;
    for (int i = 5; i > 3; i--) {
      EEPROM.update(5 * i, EEPROM.read(5 * (i - 1)));
      EEPROM.update(5 * i + 1, EEPROM.read(5 * (i - 1) + 1));
      EEPROM.update(5 * i + 2, EEPROM.read(5 * (i - 1) + 2));
      EEPROM.update(5 * i + 3, EEPROM.read(5 * (i - 1) + 3));
    }

    EEPROM.update(15, score);
    EEPROM.update(16, name[0]);
    EEPROM.update(17, name[1]);
    EEPROM.update(18, name[2]);
  } else if (standard .score > highscores[3]) {
    place = 4;
    EEPROM.update(25, score);
    EEPROM.update(26, name[0]);
    EEPROM.update(27, name[1]);
    EEPROM.update(28, name[2]);

    EEPROM.update(20, score);
    EEPROM.update(21, name[0]);
    EEPROM.update(22, name[1]);
    EEPROM.update(23, name[2]);

  } else if (standard .score > highscores[4]) {
    place = 5;
    EEPROM.update(25, score);
    EEPROM.update(26, name[0]);
    EEPROM.update(27, name[1]);
    EEPROM.update(28, name[2]);

  }

}



