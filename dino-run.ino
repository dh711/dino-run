#include <LiquidCrystal.h>

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
  B00100,
  B01100,
  B11111,
  B00111,
  B00011,
  B00001,
  B00000
};

const int buttonPin = 13;
const int jumpDuration = 600;
unsigned long int startTime;
bool isJumping = false;
int cactusPos = 16;
int buttonState = LOW;
int gameState = 0;
int Li = 16;
int Lii = 0;

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

String scrollLCDLeft(String str) {
  String result, temp = "                " + str + "                ";
  result = temp.substring(Li, Lii);
  Li++;
  Lii++;
  if (Li > temp.length()) {
    Li = 16;
  	Lii = 0;
  }
  delay(100);
  return result;
}

void splashScreen() {
  lcd.setCursor(1, 0);
  lcd.print("Dinosaur Jump!");
  lcd.setCursor(0, 1);
  lcd.print(scrollLCDLeft("Press BUTTON to start"));
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

unsigned long int currJump = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; 
int lastButtonState = LOW;
int score = 0;

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

int game() {
  lcd.setCursor(12, 0);
  lcd.print(score);
  
  if (button() == 1) {
    jump();
    isJumping = true;
    currJump = millis();
    Serial.print(currJump);
  }

  if (isJumping) {
    if (millis() - currJump >= jumpDuration) {
      unjump();
      isJumping = false;
    }
  } else {
    lcd.setCursor(2,1);
    lcd.write(1);
  }

  // Terrain movement.
  lcd.setCursor(cactusPos, 1);
  lcd.write(" ");
  cactusPos--;
  lcd.setCursor(cactusPos, 1);
  lcd.write(byte(0));
  delay(125);
  if (cactusPos < 0) cactusPos = 16;
  
  if (isJumping == false && cactusPos == 2) {
    gameState = 2;
    lcd.clear();
  	return score;
  }
  
  score++;
}

void gameOver() {
  lcd.setCursor(3, 0);
  lcd.print("GAME OVER!");
  lcd.setCursor(0, 1);
  String display = "Score: " + String(score) + ". Press BUTTON to play again!";
  lcd.print(scrollLCDLeft(display));
  if (button() == 1) {
  	score = 0;
    gameState = 1;
    lcd.clear();
  }
}

void loop() {
  if (gameState == 0)
    splashScreen();
  
  if (gameState == 1)
  	game();
  
  if (gameState == 2)
    gameOver();
}
