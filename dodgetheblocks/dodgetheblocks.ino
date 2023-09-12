#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <time.h>

U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);

#define BLUEBUTTON 5
#define GREENBUTTON 4
#define REDBUTTON 3
#define YELLOWBUTTON 2

class Player{
  public:
    float playerX;
    float playerY;
    int playerSpeed = 2;
    int playerScore = 0;
    int playerLives = 3;

  Player(){
    playerX = 64;
    playerY = 56;
  }

  void render(){
    u8g2.drawCircle(playerX, playerY, 2);
  }

  void update(){
    if(digitalRead(BLUEBUTTON) == LOW){
      playerX -= playerSpeed;
    }

    if(digitalRead(GREENBUTTON) == LOW){
      playerX += playerSpeed;
    }

    //limit the player to screen bounds
    if(playerX < 0) playerX = 0;
    if(playerX > 128) playerX = 128;
  }

  void reset(){
    playerScore = 0;
    playerLives = 3;
    playerX = 64;
    playerY = 56;
  }
};

class Obstacle{
  public:
    float obstacleX;
    float obstacleY;
    int obstacleSpeed;

  Obstacle(){
    obstacleX = rand() % 128;
    obstacleY = 0; //set to 0
    obstacleSpeed = 2 + rand() % 3;
  }

  void render(){
    u8g2.drawBox(obstacleX, obstacleY, 8, 8);
  }

  void update(){
    obstacleY += obstacleSpeed;

    if(obstacleY > 70){
      obstacleX = rand() % 128;
      obstacleY = 0; //set to 0
    }
  }

  void reset(){
    obstacleX = rand() % 128;
    obstacleY = 0;
    obstacleSpeed = 2 + rand() % 3;
  }
};

class Coin{
  public: 
    float coinX;
    float coinY;
    int coinSpeed;

  Coin(){
    coinX = rand() % 128;
    coinY = 0;
    coinSpeed = 1 + rand() % 3;
  }

  void render(){
    u8g2.drawCircle(coinX, coinY, 4);
  }

  void update(){
    coinY += coinSpeed;

    if(coinY > 70){
      reset();
    }
  }

  void reset(){
    coinX = rand() % 128;
    coinY = 0;
    coinSpeed = 1 + rand() % 3;
  }
};

Player *player = new Player();

const int obstacleAmount = 5;
const int coinAmount = 2;

Obstacle obstacles[obstacleAmount];
Coin coins[coinAmount];

int gameState = 0; //0 is starting screen, 1 is main game, 2 is end screen

void restartGame(){
  player->reset();

  for(int i = 0; i < obstacleAmount; i++){
    obstacles[i].reset();
  }

  for(int i = 0; i < coinAmount; i++){
    coins[i].reset();
  }

    gameState = 1;
}

void setup() {
  srand(time(NULL));

  u8g2.begin();
  pinMode(BLUEBUTTON, INPUT_PULLUP);
  pinMode(GREENBUTTON, INPUT_PULLUP);
  pinMode(REDBUTTON, INPUT_PULLUP);
  pinMode(YELLOWBUTTON, INPUT_PULLUP);

  for(int i = 0; i < obstacleAmount; i++){
    Obstacle *newObstacle = new Obstacle();
    obstacles[i] = *newObstacle;
  }

  for(int i = 0; i < coinAmount; i++){
    Coin *newCoin = new Coin();
    coins[i] = *newCoin;
  }
}

void loop() {
  if(gameState == 0){
    if(digitalRead(REDBUTTON) == LOW){
      gameState = 1;
    }
  } else if(gameState == 1){
    player->update();

    for(int i = 0; i < obstacleAmount; i++){
      obstacles[i].update();

      if(((player->playerX >= obstacles[i].obstacleX) && (player->playerX <= obstacles[i].obstacleX + 8)) && ((player->playerY >= obstacles[i].obstacleY) && (player->playerY <= obstacles[i].obstacleY + 8))){
        player->playerLives--;
        obstacles[i].reset();
      }
    }

    for(int i = 0; i < coinAmount; i++){
      coins[i].update();

      if(((player->playerX >= coins[i].coinX - 4) && (player->playerX <= coins[i].coinX + 4)) && ((player->playerY >= coins[i].coinY - 4) && (player->playerY <= coins[i].coinY + 4))){
        player->playerScore++;
        coins[i].reset();
      }
    }

    if(player->playerLives == 0){
      gameState = 2;
    }
  } else if(gameState == 2){
    if(digitalRead(REDBUTTON) == LOW){
      restartGame();
    }
  }

  u8g2.firstPage();
  do {
    if(gameState == 1){
      u8g2.setFont(u8g2_font_6x12_mr);
      u8g2.setCursor(1, 10);
      u8g2.print("Lives: ");
      u8g2.print(player->playerLives);
      u8g2.print("  Score: ");
      u8g2.print(player->playerScore);

      //render each obstacle
      for(int i = 0; i < obstacleAmount; i++){
        obstacles[i].render();
      }

      for(int i = 0; i < coinAmount; i++){
        coins[i].render();
      }

      player->render();
    } else if (gameState == 2) {
      u8g2.setFont(u8g2_font_6x12_mr);
      u8g2.setCursor(30, 32);
      u8g2.print("Game Over :(");
      u8g2.setCursor(30, 42);
      u8g2.print("Score: ");
      u8g2.print(player->playerScore);
      u8g2.setCursor(6, 52);
      u8g2.print("Press RED to restart");
    } else if (gameState == 0){
      u8g2.setFont(u8g2_font_6x12_mr);
      u8g2.setCursor(16, 22);
      u8g2.print("Dodge the Blocks!");
      u8g2.setCursor(14, 42);
      u8g2.print("Press RED to start");
    }
  } while ( u8g2.nextPage() );
  delay(50);
}
