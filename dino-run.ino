#include <LiquidCrystal.h>
#include <EEPROM.h>

// Initialize the library with the numbers of the interface pins.
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Bitmaps.
// Cactus.
byte CACTUS[] = {
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
byte DINO[] = {
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
byte CROW[] = {
	B00000,
	B00000,
	B00100,
	B01100,
	B11111,
	B00111,
	B00011,
	B00001
};

// Flying Crow.
byte CROW_FLYING[] = {
	B00001,
	B00011,
	B00111,
	B01111,
	B11111,
	B00000,
	B00000,
	B00000
};

// Constants.
const int BUTTON_PIN = 13;
const int JUMP_DURATION = 600;

// Variables.
bool isJumping = false;
unsigned long int currJump = 0;
int delayTime = 150;

int obstacle = 1;
int obstacleXPos = 16;
int obstacleYPos = 1;
int luck;

int buttonState = LOW;
int gameState = 0;

int L1 = 16;
int L2 = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; 
int lastButtonState = LOW;

int score = 0;
int maxScore;
int maxFlag=0;

void writeIntIntoEEPROM(int address, int number) { 
	EEPROM.write(address, number >> 8);
	EEPROM.write(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address) {
	return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void setup() {
	// Initializing LCD.
	lcd.begin(16 ,2);

	// Creating characters.
	lcd.createChar(0, DINO);
	lcd.createChar(1, CACTUS);
	lcd.createChar(2, CROW);
	lcd.createChar(3, CROW_FLYING);

	pinMode(BUTTON_PIN, INPUT);
	buttonState = digitalRead(BUTTON_PIN);
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
		if (millis() - currJump >= JUMP_DURATION) {
		unjump();
		isJumping = false;
		}
	} 
	// Else make sure the dinosaur does not clip and disappear. 
	else {
		lcd.setCursor(2,1);
		lcd.write(byte(0));
	}

	renderObstacle();
	
	// Checking for collisions.
	if (obstacleXPos == 2) {
		if ((isJumping == false && obstacle == 1) || (isJumping == true && obstacle == 2)) {
			obstacle = 1;
          	obstacleYPos = 1;
            gameState = 2;
			L1 = 16;
			L2 = 0;
			lcd.clear();
			return score;
		}
	}
	
	score++;
	
	// Increase speed as game progresses.
	if (score % 100 == 1)
		delayTime = delayTime - 10;
}

void gameOver() {
	maxScore = readIntFromEEPROM(0);
	
	// Display "GAME OVER!" message.
	lcd.setCursor(3, 0);
	lcd.print("GAME OVER!");
	lcd.setCursor(0, 1);

	if (maxFlag == 1) {
		String display = "Score: " + String(score) + ". NEW HIGH SCORE! Press BUTTON to play again!";
		lcd.print(scrollLCDLeft(display));
	}
	else {
		if (score <= maxScore) {
			String display = "Score: " + String(score) + ". Press BUTTON to play again!";
			lcd.print(scrollLCDLeft(display));
			maxFlag = 0;
		}
		else {
			String display = "New High Score: " + String(score) + ". Press BUTTON to play again!";
			lcd.print(scrollLCDLeft(display));
			writeIntIntoEEPROM(0, score);
			maxFlag = 1;
		}
	}
	
	// Restart game on button press.
	if (button() == 1) {
		score = 0;
		delayTime = 150;
		maxFlag = 0;
		gameState = 1;
		L1 = 16;
		L2 = 0;
		lcd.clear();
	}
}

void jump() {
	lcd.setCursor(2, 1);
	lcd.write(" ");
	lcd.setCursor(2, 0);
	lcd.write(byte(0));
}

void unjump() {
	lcd.setCursor(2, 0);
	lcd.write(" ");
	lcd.setCursor(2, 1);
	lcd.write(byte(0));
}

void renderObstacle() {
	// Move terrain and create obstacles.
	lcd.setCursor(obstacleXPos, obstacleYPos);
	lcd.write(" ");
	obstacleXPos--;

	// Reset obstatcle if out of frame.
	if (obstacleXPos < 0) {
		obstacleXPos = 16;
		randomSeed(score);
		luck = random(5);
	}

	// Randomly choose between crows and cacti when score > 100.
	if (score > 100) {
		if (luck % 2 == 0) {
			obstacle = 1;
			obstacleYPos = 1;
		}
		else {
			obstacle = 2;
			obstacleYPos = 0;
		}
	}

	lcd.setCursor(obstacleXPos, obstacleYPos);
	
	// If obstacle is a crow, add animation.
	if (obstacle == 2 && obstacleXPos % 2 == 0)
		lcd.write(byte(3));
	else
		lcd.write(byte(obstacle));

	delay(delayTime);
}

/* 
	Utility Functions. 
*/

int button() 
{
	int reading = digitalRead(BUTTON_PIN);
	
	// If the switch is pressed, due to noise or pressing, reset the debouncing timer.
	if (reading != lastButtonState)
		lastDebounceTime = millis();

	if ((millis() - lastDebounceTime) > debounceDelay) {
		buttonState = reading;     
		if (buttonState == HIGH) {
			lastButtonState = reading;
			return 1;
		}
	}
	
	lastButtonState = reading;
}

String scrollLCDLeft(String str) {
	String result, temp = "                " + str + "                ";
	result = temp.substring(L1, L2);
	
	L1++;
	L2++;
	
	if (L1 > temp.length()) {
		L1 = 16;
		L2 = 0;
	}

	delay(150);
	return result;
}
