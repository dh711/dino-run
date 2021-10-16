#include <LiquidCrystal.h>
#include <EEPROM.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Cactus.
byte cactus[] = {
  B00100,
  B00101,
  B10101,
  B10101,
  B10101,
  B10110,
  B01100,
  B00100
};

// Dinosaur.
byte dino[] = {
  B01110,
  B10111,
  B11110,
  B11111,
  B11000,
  B11111,
  B11010,
  B10010
};

// Crow.
byte crow[] = {
  B00000,
  B00000,
  B00100,
  B01100,
  B11111,
  B00111,
  B00011,
  B00001
};

const int buttonPin = 13;
const int jumpDuration = 600;


unsigned long int startTime;
bool isJumping = false;
int obstaclePos = 16;
int buttonState = LOW;
int gameState = 0;
int Li = 16;
int Lii = 0;
int delayTime = 150;
unsigned long int currJump = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; 
int lastButtonState = LOW;
int score = 0;
int luck;
int maxscore;
int maxFlag=0;

void writeIntIntoEEPROM(int address, int number)
{ 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void setup() {
  // Initializing LCD.
  lcd.begin(16,2);
  Serial.begin(9600);
  
  // Creating characters.
  lcd.createChar(0, cactus);
  lcd.createChar(1, dino);
  lcd.createChar(2, crow);

  pinMode(buttonPin, INPUT);
  buttonState = digitalRead(buttonPin);
}

void loop() {
  // Initial state - Splash screen.
  if (gameState == 0)
    splashScreen();
  
  // Game in progress.
  if (gameState == 1)
  	game();
  
  // Game over.
  if (gameState == 2)
    gameOver();
}

String scrollLCDLeft(String str) {
  String result, temp = "                " + str + "                ";
  result = temp.substring(Li, Lii);
  Li++;
  Lii++;
  if (Li > temp.length()) {
    Li = 16;
  	Lii = 0;
  }
  delay(150);
  return result;
}

void splashScreen() {
  // Display game name.
  lcd.setCursor(1, 0);
  lcd.print("Dinosaur Jump!");
  
  // Scroll start information.
  lcd.setCursor(0, 1);
  lcd.print(scrollLCDLeft("Press BUTTON to start"));
  
  // On button press, start game.
  if (button() == 1) {
  	gameState = 1;
    lcd.clear();
  }
}

void jump() {
  lcd.setCursor(2,1);
  lcd.write(" ");
  lcd.setCursor(2,0);
  lcd.write(byte(1));
}

void unjump() {
  lcd.setCursor(2,0);
  lcd.write(" ");
  lcd.setCursor(2,1);
  lcd.write(byte(1));
}

int button() 
{
     int reading = digitalRead(buttonPin);
     // If the switch changed, due to noise or pressing:
     if (reading != lastButtonState) {
       // reset the debouncing timer
       lastDebounceTime = millis();
     } 
     
     if ((millis() - lastDebounceTime) > debounceDelay) {
       buttonState = reading;     
       if (buttonState == HIGH) {
           lastButtonState = reading;
           return 1;
         }
       }
     lastButtonState = reading;
}

void renderObstacle() {
  // Move terrain and create obstacles.
  int obstacle = 0;
  
  lcd.setCursor(obstaclePos, 1);
  lcd.write(" ");
  obstaclePos--;
  
  // Reset obstatcle if out of frame.
  if (obstaclePos < 0) {
    obstaclePos = 16;
    randomSeed(score);
    luck = random(5);
  }
  
  if (score > 50) {
        if (luck % 2 == 0)
          obstacle = 2;
        else
          obstacle = 0;
  }
  
  lcd.setCursor(obstaclePos, 1);
  lcd.write(byte(obstacle));
  
  delay(delayTime);
}

int game() {
  // Print current score on top right.
  lcd.setCursor(12, 0);
  lcd.print(score);
  
  // On button press, activate jump.
  if (button() == 1 && isJumping == false) {
    jump();
    isJumping = true;
    currJump = millis();
    Serial.print(currJump);
  }
  // If jumping, check if jump duration is over.
  if (isJumping) {
    if (millis() - currJump >= jumpDuration) {
      unjump();
      isJumping = false;
    }
  } 
  // Else make sure the dinosaur doesnot clip and disappear. 
  else {
    lcd.setCursor(2,1);
    lcd.write(byte(1));
  }

  renderObstacle();
  
  if (isJumping == false && obstaclePos == 2) {
    gameState = 2;
    lcd.clear();
  	return score;
  }
  
  score++;
  
  if (score % 100 == 1)
    delayTime = delayTime-10;
}

void gameOver() {
  // Display "GAME OVER!" message.
  maxscore = readIntFromEEPROM(0);
  Serial.println(maxscore);
  lcd.setCursor(3, 0);
  lcd.print("GAME OVER!");
  if (maxFlag == 1) {
    lcd.setCursor(0, 1);
    String display = "New High Score: " + String(score) + ". Press BUTTON to play again!";
    lcd.print(scrollLCDLeft(display));
  }
  else {
    if (score <= maxscore) {
      // Display score.
      lcd.setCursor(0, 1);
      String display = "Score: " + String(score) + ". Press BUTTON to play again!";
      lcd.print(scrollLCDLeft(display));
      maxFlag = 0;
    }
    else {
      lcd.setCursor(0, 1);
      String display = "New High Score: " + String(score) + ". Press BUTTON to play again!";
      lcd.print(scrollLCDLeft(display));
      writeIntIntoEEPROM(0, score);
      maxFlag = 1;
    }
  }
  
  // Restart game on button press.
  if (button() == 1) {
    maxFlag = 0;
    score = 0;
    delayTime = 150;
    gameState = 1;
    lcd.clear();
  }
}
