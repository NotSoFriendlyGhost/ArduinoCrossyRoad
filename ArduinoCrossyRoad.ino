#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

const int PIN = 7;    //Set the data pin on the Arduino
const int SIZE = 16;  //We are using a 16x16 pixel matrix

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(SIZE, SIZE, PIN,                                                            //This defines our matrix.
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,  //Don't worry about these details
                                               NEO_GRB + NEO_KHZ800);


unsigned long obstacleUpdate; // Used to time obstacle speed

void reset(); // Define function

struct Player {
  uint16_t playerColor;
  int topLeftX = 7;
  int topLeftY = 15;
  Player(int x, int y, uint16_t c) {
    topLeftX = x;
    topLeftY = y;
    playerColor = c;
  }
  void draw() { // Draw player on screen
    matrix.drawPixel(topLeftX, topLeftY, playerColor);
    matrix.drawPixel(topLeftX, topLeftY + 1, 0);
  }
  void forward() { // Move up, end game if win
    if (topLeftY > 0) {
      topLeftY--;
    }
    else{
      win();
    }
  }
  void splat() { // Reset position if collision detected
    uint16_t red = matrix.Color(255, 0, 0);
    matrix.drawPixel(topLeftX, topLeftY, red);
    matrix.drawPixel(topLeftX, topLeftY + 1, red);
    matrix.drawPixel(topLeftX, topLeftY - 1, red);
    matrix.drawPixel(topLeftX + 1, topLeftY, red);
    matrix.drawPixel(topLeftX - 1, topLeftY, red);
  }
  void win(){ // Fill screen with player color
    matrix.fillScreen(playerColor);
    matrix.show();
    delay(2000);
    reset();
  }
};

uint16_t obstacle[16][16];
uint16_t obsColors[] = { matrix.Color(255, 255, 0), matrix.Color(255, 0, 255), matrix.Color(255, 165, 0) };

struct Obstacle {
  uint16_t obsColor;
  int direction;
  int posX;
  int posY;
  int length;
  Obstacle() {
    posY = random() % 14 + 1;
    int d = random() % 2;
    if (d == 0) {
      direction = -1;
    }
    if (d == 1) {
      direction = 1;
    }
    posX = random() % 14 + 1;
    int colorNum = random() % 3;
    obsColor = obsColors[colorNum];
    length = random() % 3 + 1;
  }
  void update() { // Move obstacle depending on direction
    if (direction == -1) {
      for (int i = posX; i < min(posX + length, 16); i++) {
        obstacle[posY][i] = 0;
      }
      if (posX == 0) {
        respawn();
        return;
      } else
        posX--;
      for (int i = posX; i < min(posX + length, 16); i++) {
        obstacle[posY][i] = obsColor;
      }
    }
    if (direction == 1) {
      for (int i = posX; i > max(posX - length, -1); i--) {
        obstacle[posY][i] = 0;
      }
      if (posX == 15) {
        respawn();
        return;
      } else
        posX++;
      for (int i = posX; i > max(posX - length, -1); i--) {
        obstacle[posY][i] = obsColor;
      }
    }
  }
  void respawn() { // Move obstacle to new random position and direction
    posY = random() % 14 + 1;
    int d = random() % 2;
    if (d == 0) {
      direction = -1;
      posX = 15;
    }
    if (d == 1) {
      direction = 1;
      posX = 0;
    }
    int colorNum = random() % 3;
    obsColor = obsColors[colorNum];
    length = random() % 3 + 1;
  }
};

int startButton = 6;
int lButton = 8;
int rButton = 9;

int singlePlayer = 11;
int twoPlayer = 12;

int p1LED = 2;
int p2LED = 3;

const int numObs = 25;
struct Obstacle obsObjects[numObs];

Player p1(5, 15, matrix.Color(255, 255, 255));
Player p2(10, 15, matrix.Color(0, 0, 255));

bool gameOver = false;
bool won = 0;

// reset game
void reset(){
  p1.topLeftX = 5;
  p1.topLeftY = 15;
  p2.topLeftX = 10;
  p2.topLeftY = 15;
  p1.draw();
  p2.draw();
  for (int i = 0; i < numObs; i++) {
    obsObjects[i].posY = random() % 14 + 1;
    int d = random() % 2;
    if (d == 0) {
      obsObjects[i].direction = -1;
    }
    if (d == 1) {
      obsObjects[i].direction = 1;
    }
    obsObjects[i].posX = random() % 14 + 1;
    int colorNum = random() % 3;
    obsObjects[i].obsColor = obsColors[colorNum];
    obsObjects[i].length = random() % 3 + 1;
  }
  while(digitalRead(startButton)==LOW){ // Wait until start button pressed

  }
  return;

}

// Draw all objects
void updateScreen() {
  matrix.fillScreen(0);
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      if (obstacle[i][j] != 0) {
        matrix.drawPixel(j, i, obstacle[i][j]);
      }
    }
  }
  p1.draw();
  p2.draw();
  matrix.show();
  delay(50);
}

// Check buttons
void checkInput() {
  if (digitalRead(lButton) == HIGH) {
    p1.forward();
    if (digitalRead(rButton) == HIGH) {
      p2.forward();
    }
    delay(100);
  }
  if (digitalRead(rButton) == HIGH) {
    p2.forward();
    if (digitalRead(lButton) == HIGH) {
      p1.forward();
    }
    delay(100);
  }
}

void checkCollision() {
  if (obstacle[p1.topLeftY][p1.topLeftX]) {
    p1.splat();
    matrix.show();
    p1.topLeftX = 5;
    p1.topLeftY = 15;
    delay(2000);
  }
  if (obstacle[p2.topLeftY][p2.topLeftX]) {
    p2.splat();
    matrix.show();
    p2.topLeftX = 10;
    p2.topLeftY = 15;
    delay(2000);
  }
}

void setup() {
  obstacle[1][0] = 1;
  obstacle[3][5] = 1;
  // put your setup code here, to run once:
  matrix.begin();            //This starts the matrix
  matrix.setBrightness(40);  //This sets the brightness of the pixel
  pinMode(lButton, INPUT);
  pinMode(rButton, INPUT);
  pinMode(startButton, INPUT);
  pinMode(p1LED, OUTPUT);
  pinMode(p2LED, OUTPUT);
  digitalWrite(p1LED, HIGH);
  digitalWrite(p2LED, HIGH);
  randomSeed(analogRead(A0));
  reset();
  obstacleUpdate = millis();
  matrix.fillScreen(matrix.Color(0, 255, 0));
  matrix.show();
  delay(2000);
}

void loop() {
  updateScreen();
  if (millis() - obstacleUpdate >= 200) {
    for (int i = 0; i < numObs; i++) {
      obsObjects[i].update();
    }
    obstacleUpdate = millis();
  }
  checkInput();
  checkCollision();
}
