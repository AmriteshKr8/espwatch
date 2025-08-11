#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP32Time.h>
#include <esp_sleep.h>
#include <BleKeyboard.h>
#include "esp_bt.h"
#include <TFT_eSPI.h>
#include "esp_wifi.h"
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);

const char form_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
    <head>
      <title>
        Create
      </title>
    </head>
<body>
    <style>
        body{
            font-family: monospace;
            font-weight: bold;
            background: #c7c1c1;
        }
        h2{
            color: #333;
            text-align: center;
            font-size: 2em;
            margin: 20px 10px;
        }
        form{
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
        }
        input[type="text"], textarea{
            width: 80%;
            padding: 10px;
            margin: 5px 0;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
        input[type="submit"]{
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        input[type="submit"]:hover{
            background-color: #45a049;
        }
        textarea{
            resize: none;
        }
        textarea:focus{
            outline: none;
            border-color: #4CAF50;
        }
        input[type="text"]:focus{
            outline: none;
            border-color: #4CAF50;
        }
        input[type="text"]::placeholder, textarea::placeholder{
            color: #aaa;
            font-style: italic;
        }
        input[type="text"]:focus::placeholder, textarea:focus::placeholder{
            color: #4CAF50;
            font-style: normal;
        }
        input[type="text"]:focus, textarea:focus{
            box-shadow: 0 0 5px rgba(76, 175, 80, 0.5);
        }
        input[type="submit"]:active{
            background-color: #3e8e41;
        }
    </style>
  <h2>Create File</h2>
  <form onsubmit="sendMessage(event)">
    <input type="text" id="fileName" placeholder="File Name" required>
    <br>
    <textarea id="fileContent" name="fileContent" rows="20" cols="50" placeholder="File Content"></textarea>
    <br>
    <input type="submit" value="Send">
  </form>
  <script>
    function sendMessage(e) {
      e.preventDefault();
      const fileName = document.getElementById("fileName").value;
      const fileContent = document.getElementById("fileContent").value;
      fetch(`/submit?fileName=${encodeURIComponent(fileName)}&fileContent=${encodeURIComponent(fileContent)}`)
        .then(r => r.text())
        .then(text => alert(text));
    }
  </script>
</body>
</html>
)rawliteral";

// Write to file
void writeFile(const char* path, const char* message) {
  File file = SPIFFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.print(message);
  file.close();
  Serial.println("File written successfully!");
}

// Read from file
String readFile(const char* path) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return "File not Found!";
  }

  String content;
  while (file.available()) {
    content += (char)file.read();
  }
  file.close();
  return content;
}

struct WifiCred {
  String ssid;
  String password;
};

struct WifiCredList {
  WifiCred creds[5];
  int count;
};

WifiCredList readCreds(const char* path = "/wpa_creds.txt") {
  WifiCredList list;
  list.count = 0;

  File file = SPIFFS.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("Failed to open file");
    return list;
  }

  while (file.available() && list.count < 5) {
    String line = file.readStringUntil('\n');
    line.trim();

    int k = line.indexOf(':');
    if (k != -1) {
      list.creds[list.count].ssid = line.substring(0, k);
      list.creds[list.count].password = line.substring(k + 1);
      list.count++;
    }
  }

  file.close();
  return list;
}

#include <JetBrainsMono_ExtraBold24pt7b.h>
#include <JetBrainsMono_ExtraBold18pt7b.h>
#include <JetBrainsMono_ExtraBold16pt7b.h>
#include <JetBrainsMono_ExtraBold14pt7b.h>
#include <JetBrainsMono_ExtraBold30pt7b.h>

BleKeyboard bleKeyboard("ESPWATCH", "AK8", 100);

String ssid;
String pass;

const char* ap_ssid = "ESPWATCH";
const char* ap_password = "ESWT2048";

const int UP = D2;
const int DOWN = D4;
const int SEL = D3;

const char* ssid_list[5];
const char* pass_list[5];

// pong_vars

int xdir = 1;
int ydir = 1;
int velocity = 1;
int prevx = 120;
int prevy = 140;
int newx = 0;
int newy = 0;
int score = 0;
bool game_over = false;

const int court_x = 15;
const int court_y = 15;
const int border_thickness = 1;
const int ball_rad = 4;
const int paddle_length = 40;
const int paddle_thickness = 8;

int paddle_x = 20;
int paddle_y = 40;
int paddle_prev_x = 20;
int paddle_prev_y = 40;
int paddle_velocity = 1;

int menu_index = 0;
int last_menu_index = -1;
const char* wifi_menu_items[] = { "Exit     ", "Status   ", "Disable  ", "AP       ", "STA       ", "HOST_CP  " };
const int wifi_menu_len = 6;
const char* ble_menu_items[] = { "Exit     ", "Remote   ", "Media     ", "Keyb      " };
const int ble_menu_len = 4;
const char* main_menu_items[] = { "Exit     ", "Wifi     ", "Ble      ", "Flash    ", "Stopwatch", "Sync     " };
const int main_menu_len = 6;
const char* game_menu_items[] = { "Exit     ", "Pong     ", "Invaders " };
const int game_menu_len = 3;
int char_counter = 0;
const int char_list_length = 73;
const char char_list[] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
  'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
  'u', 'v', 'w', 'x', 'y', 'z',
  '~', '.', ',', '<', '>', '^', '@', '#', '?', ';', ':'
};


RTC_DATA_ATTR int brightness = 125;
const char* WiFiModes[] = { "OFF", "STA", "AP" };

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
ESP32Time rtc(19800);

TFT_eSPI tft = TFT_eSPI();

RTC_DATA_ATTR int boot = 0;
unsigned long epoch = 0;
int selectedIndex = 0;

const char* apiKey = "kl9Kk7HUHliXy2FEhAf8XQ81KudERdNr";
const char* location = "Jamshedpur";
const char* units = "metric";

// pong_start

void drawHollowRect(int x, int y, int w, int h, int border, uint16_t color) {
  for (int i = 0; i < border; i++) {
    tft.drawRect(x + i, y + i, w - 2 * i, h - 2 * i, color);
  }
}

void drawScore() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(String(score), tft.width() / 2, tft.height() / 2, 4);
}

void ball_tick() {
  tft.fillCircle(prevx, prevy, ball_rad, TFT_BLACK);
  newx = prevx + (xdir * velocity);
  newy = prevy + (ydir * velocity);
  if (newx > tft.width() - (court_x + border_thickness + ball_rad + 2)) {
    xdir = -1;
  }
  if (newx < court_x + border_thickness + ball_rad + 1) {
    xdir = 1;
    game_over = true;
  }
  if ((newx == (paddle_x + paddle_thickness + ball_rad + 1)) && (newy > paddle_y) && (newy < paddle_y + paddle_length) && (xdir == -1)) {
    xdir = 1;
    score++;
  }
  if (newy > tft.height() - (court_y + border_thickness + ball_rad + 2)) {
    ydir = -1;
  }
  if (newy < court_y + border_thickness + ball_rad + 1) {
    ydir = 1;
  }
  tft.fillCircle(newx, newy, ball_rad, TFT_WHITE);
  prevx = newx;
  prevy = newy;
}

void paddle_tick() {
  tft.drawRect(paddle_prev_x, paddle_prev_y, paddle_thickness, paddle_length, TFT_BLACK);
  tft.drawRect(paddle_x, paddle_y, paddle_thickness, paddle_length, TFT_WHITE);
  paddle_prev_y = paddle_y;
}

void get_input() {
  if ((digitalRead(UP) == 0) && (paddle_y > (court_y + 1))) {
    paddle_y -= paddle_velocity;
  }
  if ((digitalRead(DOWN) == 0) && (paddle_y < (tft.height() - (court_y + paddle_length + 1)))) {
    paddle_y += paddle_velocity;
  }
}

void pong() {
  tft.fillScreen(TFT_BLACK);
  drawHollowRect(court_x, court_y, tft.width() - 2 * court_x, tft.height() - 2 * court_y, border_thickness, TFT_WHITE);
  while (!game_over) {
    get_input();
    paddle_tick();
    ball_tick();

    drawScore();
    delay(10);
  }
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Game Over", tft.width() / 2, tft.height() / 2, 4);
  score = 0;
  delay(3000);
  tft.fillScreen(TFT_BLACK);
  game_over = false;
  menu_index = 0;
  last_menu_index = -1;
}


bool frame = false;

int x_origin = 16;
int y_origin = 16;
int x_dir = 3;
int prev_x_origin = 16;
int prev_y_origin = 16;

int playerX = 140;
const int playerY = 200;

int invader_tick_time = 250;
int level = 1;
bool invaders_gameover = false;

const int invaderSpacing = 28;
const int invaderCols = 7;
const int invaderRows = 3;

unsigned long last_update = 0;

bool bulletActive = false;
int bulletX = 0, bulletY = 0;
const int bulletWidth = 2;
const int bulletHeight = 4;

bool invaders[invaderCols][invaderRows];

const uint8_t invader1_frame1[8] = {
  0b00111100,
  0b01111110,
  0b11111111,
  0b11011011,
  0b11111111,
  0b00100100,
  0b01011010,
  0b10100101
};

const uint8_t invader1_frame2[8] = {
  0b00111100,
  0b01111110,
  0b11111111,
  0b11011011,
  0b11111111,
  0b00100100,
  0b10000001,
  0b01000010
};

const uint8_t playerSprite[8] = {
  0b00011000,
  0b00111100,
  0b01111110,
  0b11111111,
  0b11111111,
  0b00100100,
  0b01111110,
  0b01000010
};

void drawInvader(int x, int y, bool current_frame) {
  const uint8_t* bitmap = current_frame ? invader1_frame2 : invader1_frame1;

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      if (bitmap[row] & (1 << (7 - col))) {
        tft.fillRect(x + col * 2, y + row * 2, 2, 2, TFT_WHITE);
      } else {
        tft.fillRect(x + col * 2, y + row * 2, 2, 2, TFT_BLACK);
      }
    }
  }
}

void draw_grid(bool current_frame) {
  for (int i = 0; i < invaderCols; i++) {
    for (int j = 0; j < invaderRows; j++) {
      if (invaders[i][j]) {
        int x = x_origin + i * invaderSpacing;
        int y = y_origin + j * invaderSpacing;
        drawInvader(x, y, current_frame);
      }
    }
  }
}

void checkBulletCollision() {
  if (!bulletActive) return;

  for (int i = 0; i < invaderCols; i++) {
    for (int j = 0; j < invaderRows; j++) {
      if (!invaders[i][j]) continue;

      int invaderX = x_origin + i * invaderSpacing;
      int invaderY = y_origin + j * invaderSpacing;

      if (bulletX + bulletWidth >= invaderX && bulletX <= invaderX + 16 && bulletY + bulletHeight >= invaderY && bulletY <= invaderY + 16) {
        // Collision occurred
        invaders[i][j] = false;
        tft.fillRect(invaderX, invaderY, 16, 16, TFT_BLACK);                   // Clear invader
        bulletActive = false;                                                  // Deactivate bullet
        tft.fillRect(bulletX, bulletY, bulletWidth, bulletHeight, TFT_BLACK);  // Clear bullet
        return;
      }
    }
  }
}

void clearPlayer() {
  tft.fillRect(playerX, playerY, 16, 16, TFT_BLACK);
}

void drawPlayer() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      if (playerSprite[row] & (1 << (7 - col))) {
        tft.fillRect(playerX + col * 2, playerY + row * 2, 2, 2, TFT_GREEN);
      } else {
        tft.fillRect(playerX + col * 2, playerY + row * 2, 2, 2, TFT_BLACK);
      }
    }
  }
}

void clear_grid() {
  for (int i = 0; i < invaderCols; i++) {
    for (int j = 0; j < invaderRows; j++) {
      if (invaders[i][j]) {
        int x = x_origin + i * invaderSpacing;
        int y = y_origin + j * invaderSpacing;
        tft.fillRect(x, y, 16, 16, TFT_BLACK);
      }
    }
  }
}

void invader_tick() {
  for (int i = 0; i < invaderCols; i++) {
    for (int j = 0; j < invaderRows; j++) {
      int x = prev_x_origin + i * invaderSpacing;
      int y = prev_y_origin + j * invaderSpacing;
      tft.fillRect(x, y, 16, 16, TFT_BLACK);
    }
  }

  x_origin += x_dir;
  int rightEdge = x_origin + (invaderCols - 1) * invaderSpacing + 16;
  int leftEdge = x_origin;

  if (rightEdge >= tft.width() - 16 || leftEdge <= 0) {
    x_dir = -x_dir;
    y_origin += invaderSpacing / 2;
  }

  draw_grid(frame);
  frame = !frame;

  prev_x_origin = x_origin;
  prev_y_origin = y_origin;
}

void player_tick() {
  clearPlayer();
  if (!digitalRead(UP) && playerX > 0) {
    playerX -= 4;
  }
  if (!digitalRead(DOWN) && playerX < tft.width() - 16) {
    playerX += 4;
  }
  drawPlayer();
}

void clearBullet() {
  if (bulletActive) {
    tft.fillRect(bulletX, bulletY, bulletWidth, bulletHeight, TFT_BLACK);
  }
}

void drawBullet() {
  if (bulletActive) {
    tft.fillRect(bulletX, bulletY, bulletWidth, bulletHeight, TFT_RED);
  }
}

void shootBullet() {
  if (!bulletActive && !digitalRead(SEL)) {
    bulletX = playerX + 6;
    bulletY = playerY;
    bulletActive = true;
  }
}

void bullet_tick() {
  if (bulletActive) {
    clearBullet();
    bulletY -= 6;
    if (bulletY < 0) {
      bulletActive = false;
    } else {
      checkBulletCollision();
      drawBullet();
    }
  }
}

void checkGameOver() {
  for (int i = 0; i < invaderCols; i++) {
    for (int j = 0; j < invaderRows; j++) {
      if (!invaders[i][j]) continue;

      int invaderY = y_origin + j * invaderSpacing;
      if (invaderY + 16 >= playerY) {
        // Game Over
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("GAME OVER", 140, 120, 4);
        delay(2000);
        invaders_gameover = true;
        return;
      }
    }
  }
}

void resetGame() {
  // Reset invaders
  for (int i = 0; i < invaderCols; i++) {
    for (int j = 0; j < invaderRows; j++) {
      invaders[i][j] = true;
    }
  }

  // Reset positions
  x_origin = 16;
  y_origin = 16;
  prev_x_origin = 16;
  prev_y_origin = 16;
  x_dir = 3;

  playerX = 140;
  bulletActive = false;

  tft.fillScreen(TFT_BLACK);
  draw_grid(frame);
  drawPlayer();
}

void checkWinAndRestart() {
  for (int i = 0; i < invaderCols; i++) {
    for (int j = 0; j < invaderRows; j++) {
      if (invaders[i][j]) {
        return;  // At least one invader is still alive
      }
    }
  }

  // All invaders are dead – win
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("level " + String(level) + " cleared", 140, 120, 4);
  delay(2000);  // Wait before restarting
  level++;
  resetGame();
}

void invaders_game() {
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(SEL, INPUT_PULLUP);

  for (int i = 0; i < invaderCols; i++) {
    for (int j = 0; j < invaderRows; j++) {
      invaders[i][j] = true;
    }
  }

  draw_grid(frame);
  drawPlayer();

  while (!invaders_gameover) {
    unsigned long now = millis();
    if (now - last_update >= constrain((invader_tick_time - (10 * level)), 0, 250)) {
      invader_tick();
      last_update = now;
    }
    bullet_tick();
    shootBullet();
    player_tick();
    checkGameOver();
    checkWinAndRestart();
    if (invaders_gameover) {
      invaders_gameover = false;
      tft.setRotation(0);
      tft.fillScreen(TFT_BLACK);
      menu_index = 0;
      last_menu_index = -1;
      break;
    }
    if ((!digitalRead(UP)) && (!digitalRead(DOWN)) && (!digitalRead(SEL))) {
      resetGame();
      tft.setRotation(0);
      tft.fillScreen(TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Exiting", 120, 140, 4);
      delay(750);
      tft.fillScreen(TFT_BLACK);
      tft.setTextDatum(TL_DATUM);
      menu_index = 0;
      last_menu_index = -1;
      break;
    }
    delay(50);
  }
}

void stopWatch() {
  unsigned long stopWatchStart = 0;
  unsigned long pausedTime = 0;
  bool running = false;
  bool paused = false;
  char buffer[9];
  char prev_buffer[9] = "--------";  // Start with something definitely different

  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(&JetBrainsMono_ExtraBold16pt7b);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("00:00:00", 120, 140);

  while (true) {
    if (digitalRead(SEL) == LOW) {
      stopWatchStart = rtc.getEpoch();
      running = true;
      paused = false;
      delay(300);
    }

    if (digitalRead(DOWN) == LOW) {
      tft.fillScreen(TFT_BLACK);  // This line was never reached before
      delay(200);
      break;
    }

    while (running) {
      if (digitalRead(SEL) == LOW) {
        paused = !paused;
        if (paused) {
          pausedTime = rtc.getEpoch();
        } else {
          stopWatchStart += rtc.getEpoch() - pausedTime;
        }
        delay(300);
      }

      if (digitalRead(DOWN) == LOW) {
        running = false;
        stopWatchStart = 0;
        tft.fillScreen(TFT_BLACK);
        delay(300);
        return;
      }

      if (!paused) {
        unsigned long elapsed = rtc.getEpoch() - stopWatchStart;
        int hour = elapsed / 3600;
        int min = (elapsed % 3600) / 60;
        int sec = elapsed % 60;

        sprintf(buffer, "%02d:%02d:%02d", hour, min, sec);

        if (strcmp(buffer, prev_buffer) != 0) {
          tft.drawString(buffer, 120, 140);
          memcpy(prev_buffer, buffer, sizeof(buffer));
        }
      }

      delay(50);
    }
  }
}

void execMainMenuFunction(int item) {
  last_menu_index = -1;
  menu_index = 0;
  delay(250);
  if (item == 1) {
    drawMenu(wifi_menu_items, wifi_menu_len);
    delay(250);
  } else if (item == 2) {
    tft.fillScreen(TFT_BLACK);
    drawMenu(ble_menu_items, ble_menu_len);
  } else if (item == 3) {
    tft.fillScreen(TFT_WHITE);
    tft.setFreeFont(&JetBrainsMono_ExtraBold18pt7b);
    tft.setTextColor(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(String(map(brightness, 25, 250, 1, 10)), tft.width() / 2, tft.height() / 2);
    delay(200);
    while (1) {
      delay(150);
      if (digitalRead(SEL) == 0) {
        delay(250);
        tft.fillScreen(TFT_BLACK);
        menu_index = 0;
        last_menu_index = -1;
        break;
      }
      if ((digitalRead(UP) == 0) and (brightness + 25 < 255)) {
        brightness = brightness + 25;
        tft.fillScreen(TFT_WHITE);
        tft.drawString(String(map(brightness, 25, 250, 1, 10)), tft.width() / 2, tft.height() / 2);
      }
      if ((digitalRead(DOWN) == 0) and (brightness - 25 > 0)) {
        brightness = brightness - 25;
        tft.fillScreen(TFT_WHITE);
        tft.drawString(String(map(brightness, 25, 250, 1, 10)), tft.width() / 2, tft.height() / 2);
      }
      analogWrite(TFT_BL, brightness);
    }
  } else if (item == 4) {
    stopWatch();
  } else if (item == 5) {
    timeSync();
  } else {
    return;
  }
}

void drawMenu(const char* menu_items[], int menu_len) {
  delay(250);
  last_menu_index = -1;
  while (1) {
    delay(20);
    if (last_menu_index != menu_index) {
      tft.setFreeFont(&JetBrainsMono_ExtraBold18pt7b);
      tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
      tft.setCursor(10, 24);
      tft.setTextDatum(TL_DATUM);
      int i = 0;
      while ((tft.getCursorY() < tft.height()) and (menu_len > i)) {
        tft.setCursor(10, tft.getCursorY());
        if (menu_index == i) {
          tft.drawString("~", 10, tft.getCursorY());
        } else {
          tft.drawString("-", 10, tft.getCursorY());
        }
        tft.drawString(menu_items[i], 34, tft.getCursorY());
        tft.setCursor(10, tft.getCursorY() + 38);
        i++;
      }
    }
    last_menu_index = menu_index;
    if ((digitalRead(UP) == 0) and (menu_index > 0)) {
      menu_index = menu_index - 1;
      Serial.println(menu_index);
      delay(250);
    }
    if ((digitalRead(DOWN) == 0) and (menu_index < menu_len - 1)) {
      menu_index = menu_index + 1;
      Serial.println(menu_index);
      delay(250);
    }
    if ((digitalRead(SEL) == 0)) {
      if ((menu_index == 0) && (menu_items != ssid_list)) {
        delay(250);
        last_menu_index = -1;
        break;
      } else {
        if (menu_items == main_menu_items) {
          execMainMenuFunction(menu_index);
        } else if (menu_items == wifi_menu_items) {
          execWifiMenuFunction(menu_index);
        } else if (menu_items == ble_menu_items) {
          execBleMenuFunction(menu_index);
        } else if (menu_items == game_menu_items) {
          execGameMenuFunction(menu_index);
        } else if (menu_items == ssid_list) {
          ssid = ssid_list[menu_index];
          pass = pass_list[menu_index];
          delay(250);
          break;
        } else {
          continue;
        }
      }
    }
  }
}

void disable_wifi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  esp_wifi_deinit();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  return;
}

void wifi_ap() {
  disable_wifi();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  return;
}

void wifi_sta() {
  disable_wifi();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(1000);

  //Serial.println(readFile("/wpa_creds.txt"));

  WifiCredList wifiList = readCreds();

  for (int i = 0; i < wifiList.count; i++) {
    Serial.print("SSID: ");
    Serial.println(wifiList.creds[i].ssid);
    ssid_list[i] = wifiList.creds[i].ssid.c_str();
    Serial.print("Password: ");
    Serial.println(wifiList.creds[i].password);
    pass_list[i] = wifiList.creds[i].password.c_str();
  }
  tft.fillScreen(TFT_BLACK);
  menu_index = 0;
  drawMenu(ssid_list, wifiList.count);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(&JetBrainsMono_ExtraBold16pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Connecting", 120, 140);

  WiFi.begin(ssid, pass);
  Serial.println(ssid);
  Serial.println(pass);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  tft.fillScreen(TFT_BLACK);
  if (WiFi.status() == WL_CONNECTED) {
    tft.drawString("Connected!", 120, 140);
    fetchWeather();
  } else {
    tft.drawString("Failed", 120, 140);
    disable_wifi();
    WiFi.mode(WIFI_OFF);
    WiFi.disconnect(true);
  }
  delay(2000);
  tft.fillScreen(TFT_BLACK);
  menu_index = 0;
  last_menu_index = -1;
  return;
}

unsigned long getUnixEpochTime() {
  wifi_sta();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setFreeFont(&JetBrainsMono_ExtraBold16pt7b);
  tft.drawString("Syncing", tft.width() / 2, tft.height() / 2);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() - startTime > 10000) {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      return 0;
    }
  }
  timeClient.begin();
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  fetchWeather();
  disable_wifi();
  return epochTime;
}

void fetchWeather() {
  String url = "https://api.tomorrow.io/v4/weather/realtime?location=" + String(location) + "&units=" + units + "&apikey=" + apiKey;
  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      JsonObject values = doc["data"]["values"];
      float temp = values["temperature"];
      float feelsLike = values["temperatureApparent"];
      float windSpeed = values["windSpeed"];
      int windDir = values["windDirection"];
      int wcode = values["weatherCode"];
      const char* timestamp = doc["data"]["time"];

      String data = String(timestamp) + "\n";
      data += String(temp) + "\n";
      data += String(feelsLike) + "\n";
      data += String(windSpeed) + "\n";
      data += String(windDir) + "\n";
      data += String(wcode) + "\n";

      writeFile("/weather_cache.txt", data.c_str());
    }
  }
  return;
}

void timeSync() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setFreeFont(&JetBrainsMono_ExtraBold16pt7b);
  tft.drawString("Loading", tft.width() / 2, tft.height() / 2);
  delay(500);
  int i = 0;
  while (epoch < 1609459200) {
    epoch = getUnixEpochTime();
    if (i > 10) {
      break;
    }
    i++;
    delay(100);
  }

  if (epoch > 1609459200) {
    tft.drawString("Synced_", tft.width() / 2, tft.height() / 2);
    rtc.setTime(epoch);
    delay(500);
    tft.fillScreen(TFT_BLACK);
  } else {
    tft.drawString("Failure", tft.width() / 2, tft.height() / 2);
    delay(500);
    tft.fillScreen(TFT_BLACK);
  }
  return;
}

void charging() {
  tft.setFreeFont(&JetBrainsMono_ExtraBold18pt7b);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Charging", tft.width() / 2, tft.height() / 2);
  while (digitalRead(D1) == 1) {
    for (int i = 0; i <= 200; i++) {
      analogWrite(TFT_BL, i);
      delay(5);
    }
    for (int i = 255; i >= 50; i--) {
      analogWrite(TFT_BL, i);
      delay(5);
    }
  }
  return;
}

void drawFireflies(int count) {
  for (int i = 0; i < count; i++) {
    int x = random(0, tft.width());
    int y = random(0, tft.height());
    int r = random(4, 17);
    uint16_t color;
    int randVal = random(0, 4);
    if (randVal == 0)
      color = 0x1CF4;  // cyan
    else if (randVal == 1)
      color = 0xAD83;  // yellow
    else if (randVal == 2)
      color = 0x50F0;  // violet
    else
      color = 0x1D06;  // green
    tft.fillCircle(x, y, r, color);
  }
}

const uint16_t sunny[16] = {
  0b0000110000110000,
  0b0100011001100010,
  0b0010001001000100,
  0b0001011111101000,
  0b1000100000010001,
  0b1101001111001011,
  0b0111011111101110,
  0b0001011111101000,
  0b0001011111101000,
  0b0111011111101110,
  0b1101001111001011,
  0b1000100000010001,
  0b0001011111101000,
  0b0010001001000100,
  0b0100011001100010,
  0b0000110000110000
};

const uint16_t mostly_clear[16] = {
  0b0000000011000000,
  0b0000110111100000,
  0b0001111111110001,
  0b0001111111110011,
  0b0000111111100011,
  0b0000000000000001,
  0b0110000010110100,
  0b1111000100000010,
  0b1111101001111001,
  0b1111100011111100,
  0b1111001011111101,
  0b0000001011111101,
  0b0000000011111100,
  0b0000001001111001,
  0b0000000100000010,
  0b0000000010110100
};

const uint16_t partly_cloudy[16] = {
  0b0000000011000000,
  0b0000110111100000,
  0b0001111111110000,
  0b0001111111110000,
  0b0000111111100000,
  0b0000000000000000,
  0b0110000001001000,
  0b1111000100000010,
  0b1111100001111000,
  0b1111101010000101,
  0b1111000010110100,
  0b0000000010110100,
  0b0000001010000101,
  0b0000000001111000,
  0b0000000100000010,
  0b0000000001001000
};

const uint16_t mostly_cloudy[16] = {
  0b0000000011000000,
  0b0000110111100000,
  0b0001111111110000,
  0b0001111111110000,
  0b0000111111100000,
  0b0000000000000000,
  0b0110000001010000,
  0b1111000011111000,
  0b1111100001110000,
  0b1111100000000000,
  0b1111000001100000,
  0b0000000011110100,
  0b0000000011111110,
  0b0000000001111100,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t cloudy[16] = {
  0b0000000011000000,
  0b0000110111100000,
  0b0001111111110001,
  0b0001111111110011,
  0b0000111111100011,
  0b0000000000000001,
  0b0110000001010000,
  0b1111000011111000,
  0b1111100001110001,
  0b1111100000000011,
  0b1111000001100000,
  0b0000000011110100,
  0b0001100011111110,
  0b0011110001111100,
  0b0111110000000000,
  0b0011100000000000
};

const uint16_t fog[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b1001100110011001,
  0b0110011001100110,
  0b0000001000000100,
  0b0000000110011001,
  0b0000001001100110,
  0b0011001000000000,
  0b1100110000000000,
  0b0010000000000000,
  0b0001001100100011,
  0b0000110011001100,
  0b0000000000001000,
  0b0001100110010000,
  0b1110011001100000,
  0b0000000000000000
};

const uint16_t light_fog[16] = {
  0b0000000000000000,
  0b0000011100000000,
  0b0000000001100000,
  0b0000000000000000,
  0b0011000000000000,
  0b0000000110000001,
  0b0000000001100110,
  0b0000000000000000,
  0b1100110000000000,
  0b0000001100000000,
  0b0000000000100011,
  0b0000110001000000,
  0b0000000000000000,
  0b0001000110000010,
  0b0010000001100100,
  0b0000000000000000
};

const uint16_t drizzle[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001000010000,
  0b0011010000001000,
  0b0100100000000100,
  0b1000000000000010,
  0b1000000000000010,
  0b1000000000000010,
  0b0111111111111100,
  0b0000000000000000,
  0b0010000000010000,
  0b0000000000000000,
  0b0000000100000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t rain[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001000010000,
  0b0011010000001000,
  0b0100100000000100,
  0b1000000000000010,
  0b1000111111000010,
  0b1111111111111110,
  0b0111111111111100,
  0b0000000000000000,
  0b0000100001000100,
  0b0010000000000100,
  0b0010000100010000,
  0b0000000100010000,
  0b0000000000000000
};

const uint16_t light_rain[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001000010000,
  0b0011010000001000,
  0b0100100000000100,
  0b1000000000000010,
  0b1000000000000010,
  0b1000000000000010,
  0b0111111111111100,
  0b0000000000000000,
  0b0000100001000100,
  0b0010000000000100,
  0b0010000100010000,
  0b0000000100010000,
  0b0000000000000000
};

const uint16_t heavy_rain[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001000010000,
  0b0011010000001000,
  0b0100100000000100,
  0b1000000000000010,
  0b1000111111000010,
  0b1111111111111110,
  0b0111111111111100,
  0b0010010010010100,
  0b0010000010010100,
  0b0010010010010000,
  0b0000010010000000,
  0b0010000000010000,
  0b0010000000010000
};

const uint16_t snow[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001110111000,
  0b0011011011101000,
  0b0111111101011100,
  0b1111110110110110,
  0b1111111101011110,
  0b1111111011101110,
  0b0111111110111100,
  0b0000000000000000,
  0b0100000100000100,
  0b0000100000100000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t flurries[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001110111000,
  0b0011011011101000,
  0b0111111101011100,
  0b1111110110110110,
  0b1111111101011110,
  0b1111111011101110,
  0b0111111110111100,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t light_snow[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001110111000,
  0b0011011011101000,
  0b0111111101011100,
  0b1111110110110110,
  0b1111111101011110,
  0b1111111011101110,
  0b0111111110111100,
  0b0000000000000000,
  0b0101010101010100,
  0b0000000000000000,
  0b0000000000000000,
  0b0101010101010100,
  0b0000000000000000
};

const uint16_t heavy_snow[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001110111000,
  0b0011011011101000,
  0b0111111101011100,
  0b1111110110110110,
  0b1111111101011110,
  0b1111111011101110,
  0b0111111110111100,
  0b0000000000000000,
  0b0101010101010100,
  0b0010101010101000,
  0b0101010101010100,
  0b0010101010101000,
  0b0000000000000000
};

const uint16_t freezing_drizzle[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001000010000,
  0b0011010000001000,
  0b0100100000000100,
  0b1000000000000010,
  0b1000000000000010,
  0b1000000000000010,
  0b0111111111111100,
  0b0000000000000000,
  0b0010000000010000,
  0b0000000000000000,
  0b0000000100000000,
  0b0000000000000000,
  0b1111111111111111
};

const uint16_t freezing_rain[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001000010000,
  0b0011010000001000,
  0b0100100000000100,
  0b1000000000000010,
  0b1000111111000010,
  0b1111111111111110,
  0b0111111111111100,
  0b0000000000000000,
  0b0000100001000100,
  0b0010000000000100,
  0b0010000100010000,
  0b0000000100010000,
  0b1111111111111111
};

const uint16_t light_freezing_rain[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001000010000,
  0b0011010000001000,
  0b0100100000000100,
  0b1000000000000010,
  0b1000000000000010,
  0b1000000000000010,
  0b0111111111111100,
  0b0000000000000000,
  0b0000100001000100,
  0b0010000000000100,
  0b0010000100010000,
  0b0000000100010000,
  0b1111111111111111
};

const uint16_t heavy_freezing_rain[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001000010000,
  0b0011010000001000,
  0b0100100000000100,
  0b1000000000000010,
  0b1000111111000010,
  0b1111111111111110,
  0b0111111111111100,
  0b0010010010010100,
  0b0010000010010100,
  0b0010010010010000,
  0b0000010010000000,
  0b0010000000010000,
  0b1111111111111111
};

const uint16_t hail[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001110111000,
  0b0011011011101000,
  0b0111111101011100,
  0b1111110110110110,
  0b1111111101011110,
  0b1111111011101110,
  0b0111111110111100,
  0b0000000000000000,
  0b0110011001100110,
  0b0110011001100110,
  0b0000000000000000,
  0b0001100110011000,
  0b0001100110011000
};

const uint16_t light_hail[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001110111000,
  0b0011011011101000,
  0b0111111101011100,
  0b1111110110110110,
  0b1111111101011110,
  0b1111111011101110,
  0b0111111110111100,
  0b0000000000000000,
  0b0000000000000000,
  0b0000001100000000,
  0b0000001100000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t heavy_hail[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001110111000,
  0b0011011011101000,
  0b0111111101011100,
  0b1111110110110110,
  0b1111111101011110,
  0b1111111011101110,
  0b0111111110111100,
  0b0000000000000000,
  0b1100110011001100,
  0b1100110011001100,
  0b0000000000000000,
  0b0011001100110011,
  0b0011001100110011
};

const uint16_t thunderstorm[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000111100000,
  0b0000001100011000,
  0b0011011000111000,
  0b0111110000111100,
  0b1111100011111110,
  0b1111000000011110,
  0b1111110000111110,
  0b0111100011111100,
  0b0000011100000000,
  0b0000111000000000,
  0b0000110000000000,
  0b0001100000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t empty[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t arrow_bitmap_N[16] = {
  0b1111111111111111,
  0b0111111111111110,
  0b0011111111111100,
  0b0000000000000000,
  0b0000011111100000,
  0b0000001111000000,
  0b0000000000000000,
  0b0000000110000000,
  0b0000000110000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};
const uint16_t arrow_bitmap_NE[16] = {
  0b0000011111111111,
  0b0000001111111111,
  0b0000000111111111,
  0b0000000000000111,
  0b0000000111110111,
  0b0000000011110111,
  0b0000000000110111,
  0b0000000110110111,
  0b0000000110010111,
  0b0000000000000011,
  0b0000000000000001,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};
const uint16_t arrow_bitmap_E[16] = {
  0b0000000000000001,
  0b0000000000000011,
  0b0000000000000111,
  0b0000000000000111,
  0b0000000000000111,
  0b0000000000010111,
  0b0000000000110111,
  0b0000000110110111,
  0b0000000110110111,
  0b0000000000110111,
  0b0000000000010111,
  0b0000000000000111,
  0b0000000000000111,
  0b0000000000000111,
  0b0000000000000011,
  0b0000000000000001
};
const uint16_t arrow_bitmap_SE[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000001,
  0b0000000000000011,
  0b0000000110010111,
  0b0000000110110111,
  0b0000000000110111,
  0b0000000011110111,
  0b0000000111110111,
  0b0000000000000111,
  0b0000000111111111,
  0b0000001111111111,
  0b0000011111111111
};
const uint16_t arrow_bitmap_S[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000110000000,
  0b0000000110000000,
  0b0000000000000000,
  0b0000001111000000,
  0b0000011111100000,
  0b0000000000000000,
  0b0011111111111100,
  0b0111111111111110,
  0b1111111111111111
};
const uint16_t arrow_bitmap_SW[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b1000000000000000,
  0b1100000000000000,
  0b1110100110000000,
  0b1110110110000000,
  0b1110110000000000,
  0b1110111100000000,
  0b1110111110000000,
  0b1110000000000000,
  0b1111111110000000,
  0b1111111111000000,
  0b1111111111100000
};
const uint16_t arrow_bitmap_W[16] = {
  0b1000000000000000,
  0b1100000000000000,
  0b1110000000000000,
  0b1110000000000000,
  0b1110000000000000,
  0b1110100000000000,
  0b1110110000000000,
  0b1110110110000000,
  0b1110110110000000,
  0b1110110000000000,
  0b1110100000000000,
  0b1110000000000000,
  0b1110000000000000,
  0b1110000000000000,
  0b1100000000000000,
  0b1000000000000000
};
const uint16_t arrow_bitmap_NW[16] = {
  0b1111111111100000,
  0b1111111111000000,
  0b1111111110000000,
  0b1110000000000000,
  0b1110111110000000,
  0b1110111100000000,
  0b1110110000000000,
  0b1110110110000000,
  0b1110100110000000,
  0b1100000000000000,
  0b1000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t* arrow_bitmaps[8] = {
  arrow_bitmap_N,
  arrow_bitmap_NE,
  arrow_bitmap_E,
  arrow_bitmap_SE,
  arrow_bitmap_S,
  arrow_bitmap_SW,
  arrow_bitmap_W,
  arrow_bitmap_NW
};

const uint16_t battery_0[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b1111111111111110,
  0b1000000000000010,
  0b1011001100110010,
  0b1011001100110011,
  0b1011001100110011,
  0b1011001100110011,
  0b1000000000000011,
  0b1011001100110010,
  0b1000000000000010,
  0b1111111111111110,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t battery_1[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0111111111111100,
  0b1000000000000010,
  0b1011000000000010,
  0b1011000000000011,
  0b1011000000000011,
  0b1011000000000011,
  0b1011000000000011,
  0b1011000000000010,
  0b1000000000000010,
  0b0111111111111100,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t battery_2[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0111111111111100,
  0b1000000000000010,
  0b1011011000000010,
  0b1011011000000011,
  0b1011011000000011,
  0b1011011000000011,
  0b1011011000000011,
  0b1011011000000010,
  0b1000000000000010,
  0b0111111111111100,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t battery_3[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0111111111111100,
  0b1000000000000010,
  0b1011011011000010,
  0b1011011011000011,
  0b1011011011000011,
  0b1011011011000011,
  0b1011011011000011,
  0b1011011011000010,
  0b1000000000000010,
  0b0111111111111100,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t battery_4[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0111111111111100,
  0b1000000000000010,
  0b1011011011011010,
  0b1011011011011011,
  0b1011011011011011,
  0b1011011011011011,
  0b1011011011011011,
  0b1011011011011010,
  0b1000000000000010,
  0b0111111111111100,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t battery_5[16] = {
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0111111111111100,
  0b1000000000000010,
  0b1011111111111010,
  0b1011111111111011,
  0b1011111111111011,
  0b1011111111111011,
  0b1011111111111011,
  0b1011111111111010,
  0b1000000000000010,
  0b0111111111111100,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000
};

const uint16_t* battery_bitmaps[6] = {
  battery_0,
  battery_1,
  battery_2,
  battery_3,
  battery_4,
  battery_5,
};

const uint16_t* battery_manager() {
  int vbat = analogRead(D0);
  int vmax = readFile("/battery_stats").toInt();
  vbat = map(vbat, 1710, vmax, 0, 5);
  vbat = constrain(vbat, 0, 5);
  return battery_bitmaps[vbat];
}

const uint16_t* getWeatherBitmap(int code) {
  switch (code) {
    case 1000: return sunny;
    case 1100: return mostly_clear;
    case 1101: return partly_cloudy;
    case 1102: return mostly_cloudy;
    case 1001: return cloudy;
    case 2000: return fog;
    case 2100: return light_fog;
    case 4000: return drizzle;
    case 4001: return rain;
    case 4200: return light_rain;
    case 4201: return heavy_rain;
    case 5000: return snow;
    case 5001: return flurries;
    case 5100: return light_snow;
    case 5101: return heavy_snow;
    case 6000: return freezing_drizzle;
    case 6001: return freezing_rain;
    case 6200: return light_freezing_rain;
    case 6201: return heavy_freezing_rain;
    case 7000: return hail;
    case 7101: return heavy_hail;
    case 7102: return light_hail;
    case 8000: return thunderstorm;
    case 0: return empty;
    default: return empty;
  }
}

void draw_16_Bit_Bitmap(int x, int y, const uint16_t* bitmap, int width, int height, uint16_t color) {
  x = x - 24;
  y = y - 24;
  int bytesPerRow = (width + 15) / 16;  // since each uint16_t holds 16 pixels

  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      int index = row * bytesPerRow + col / 16;
      uint16_t word = bitmap[index];
      bool pixelOn = word & (0x8000 >> (col % 16));

      if (pixelOn) {
        // Draw 2x2 scaled pixel
        tft.fillRect(x + col * 3, y + row * 3, 3, 3, color);
      }
    }
  }
}

int windDirToArrowIndex(int degrees) {
  degrees = (degrees + 360) % 360;
  int index = (degrees + 22) / 45;
  return index % 8;
}

void watchface() {
  File file = SPIFFS.open("/weather_cache.txt", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
  }
  String tstamp = file.readStringUntil('\n');
  String temperature = file.readStringUntil('\n');
  String temperature_apparent = file.readStringUntil('.') + "C";
  file.readStringUntil('\n');
  String wind_speed = file.readStringUntil('\n');
  String wind_direction = file.readStringUntil('\n');
  String weather_code = file.readStringUntil('\n');
  wind_direction.trim();
  weather_code.trim();
  int weather_code_int = weather_code.toInt();
  int wind_direction_int = wind_direction.toInt();
  Serial.println(weather_code_int);
  file.close();

  tft.fillScreen(TFT_BLACK);
  drawFireflies(50);
  tft.setFreeFont(&JetBrainsMono_ExtraBold30pt7b);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(rtc.getTime("%R"), tft.width() / 2, 60);
  tft.setFreeFont(&JetBrainsMono_ExtraBold18pt7b);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(rtc.getTime("%a"), 40, 130);
  tft.setFreeFont(&JetBrainsMono_ExtraBold14pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(rtc.getTime("%b %d, %Y"), tft.width() / 2, 180);
  tft.setFreeFont(&JetBrainsMono_ExtraBold14pt7b);
  tft.setTextDatum(MC_DATUM);
  if(!digitalRead(DOWN)){
    tft.drawString(String(analogRead(D0)), 160, 130);
  } else {
    tft.drawString(temperature, 160, 130);
  }
  draw_16_Bit_Bitmap(64, 230, battery_manager(), 16, 16, TFT_WHITE);
  draw_16_Bit_Bitmap(176, 230, getWeatherBitmap(weather_code_int), 16, 16, TFT_WHITE);
  draw_16_Bit_Bitmap(120, 230, arrow_bitmaps[windDirToArrowIndex(wind_direction_int)], 16, 16, TFT_WHITE);
  return;
}

void host_cp() {
  tft.fillScreen(TFT_BLACK);
  drawFireflies(15);
  tft.setFreeFont(&JetBrainsMono_ExtraBold18pt7b);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("WS Active", 120, 80);
  if (WiFi.getMode() == 1) {
    tft.drawString(WiFi.localIP().toString(), 120, 160);
  } else if (WiFi.getMode() == 2) {
    tft.drawString(WiFi.softAPIP().toString(), 120, 160);
  } else {
    wifi_ap();
    tft.drawString(WiFi.softAPIP().toString(), 120, 160);
  }
  tft.drawString(WiFiModes[WiFi.getMode()], 120, 120);

  //ws-start

  server.on("/create", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", form_html);
  });

  // Handle file creation
  server.on("/submit", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("fileName") && request->hasParam("fileContent")) {
      String fileName = request->getParam("fileName")->value();
      String fileContent = request->getParam("fileContent")->value();

      if (!fileName.startsWith("/")) {
        fileName = "/" + fileName;
      }

      Serial.println("File Name: " + fileName);
      Serial.println("File Content: " + fileContent);

      writeFile(fileName.c_str(), fileContent.c_str());
      request->send(200, "text/plain", "File created successfully.");
    } else {
      request->send(400, "text/plain", "Missing parameters.");
    }
  });

  // Homepage: list files
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    String output = R"rawliteral(
    <html>
    <body>
    <head>
      <title>
        EspWatch
      </title>
    </head>
    <style>
        body {
            font-family: monospace;
            font-weight: bold;
            background: #c7c1c1;
        }

        h2 {
            color: #333;
            text-align: center;
            font-size: 2em;
            margin: 20px 10px;
        }

        ul {
            list-style-type: none;
            padding: 0;
            margin: 0;
        }

        li {
            font-size: 1.3em;
            background: #fff;
            border: 1px solid #ccc;
            padding: 10px;
            margin: 5px 0;
            border-radius: 5px;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .file-info {
            flex-grow: 1;
        }

        li a {
            text-decoration: none;
            color: #ffffff;
            margin-left: 10px;
            font-size: 1.2em;
            background: #e96e26;
            padding: 4px 10px;
            border-radius: 5px;
        }

        .create {
            background: #94b4d6;
            color: white;
            padding: 10px 20px;
            text-decoration: none;
            border-radius: 5px;
            font-size: 1.2em;
            display: inline-block;
            margin-top: 20px;
            margin-left: 10px;
        }

        .create:hover {
            background: #2684e9;
        }
    </style>

    <h2>SPIFFS File List</h2>
    <ul>)rawliteral";

    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
      String fileName = file.name();
      String fileSize = String(file.size());
      output += "<li><span class='file-info'>" + fileName + "   (" + fileSize + " bytes)</span>";
      output += "<div><a href='/download?fileName=" + fileName + "'>Download</a> ";
      output += "<a href='/delete?fileName=" + fileName + "'>Delete</a>";
      output += "<a href='/read?filePath=" + fileName + "'>Read</a></div>";
      output += "</li>";
      file = root.openNextFile();
    }
    output += "</ul><a class='create' href='/create'>Create New File</a></body></html>";
    request->send(200, "text/html", output);
  });

  // File deletion
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("fileName")) {
      String fileName = request->getParam("fileName")->value();
      if (!fileName.startsWith("/")) fileName = "/" + fileName;

      if (SPIFFS.exists(fileName)) {
        SPIFFS.remove(fileName);
        Serial.println("File deleted: " + fileName);
        request->redirect("/");
      } else {
        request->send(404, "text/plain", "File not found.");
      }
    } else {
      request->send(400, "text/plain", "Missing fileName parameter.");
    }
  });


  // File download
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("fileName")) {
      String fileName = request->getParam("fileName")->value();
      if (!fileName.startsWith("/")) fileName = "/" + fileName;

      if (SPIFFS.exists(fileName)) {
        AsyncWebServerResponse* response = request->beginResponse(SPIFFS, fileName, "application/octet-stream");
        response->addHeader("Content-Disposition", "attachment; filename=\"" + fileName.substring(1) + "\"");
        request->send(response);
      } else {
        request->send(404, "text/plain", "File not found.");
      }
    } else {
      request->send(400, "text/plain", "Missing fileName parameter.");
    }
  });


  // Read file content
  server.on("/read", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("filePath")) {
      String filePath = request->getParam("filePath")->value();
      if (!filePath.startsWith("/")) filePath = "/" + filePath;
      String content = readFile(filePath.c_str());
      request->send(200, "text/html", "<!DOCTYPE html><head><title>" + filePath + "</title></head><body style='font-family:monospace' ><pre>" + content + "</pre></body></html>");
    } else {
      request->send(400, "text/plain", "Missing 'filePath' parameter.");
    }
  });

  server.begin();

  //ws-end

  while (1) {
    if (!digitalRead(SEL)) {
      tft.fillScreen(TFT_BLACK);
      tft.drawString("Exiting", 120, 140);
      server.end();
      delay(2000);
      tft.fillScreen(TFT_BLACK);
      menu_index = 0;
      last_menu_index = -1;
      break;
    }
  }
}

void bt_off() {
  btStop();
  esp_bt_controller_disable();
}

void BLEMediaController() {
  btStart();
  bleKeyboard.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(&JetBrainsMono_ExtraBold24pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("x-x", 120, 140);
  while (!bleKeyboard.isConnected()) {
    if (!digitalRead(SEL)) {
      break;
    }
  }
  tft.drawString("^~^", 120, 140);
  while (bleKeyboard.isConnected()) {

    if (!digitalRead(UP)) {
      unsigned long pressStart = millis();
      while (!digitalRead(UP)) {}
      if ((millis() - pressStart) > 500) {
        bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
      } else {
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
      }
      delay(200);
    }
    if (!digitalRead(SEL)) {
      unsigned long pressStart = millis();
      while (!digitalRead(SEL)) {}
      if ((millis() - pressStart) > 500) {
        break;
      } else {
        bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
      }
      delay(200);
    }
    if (!digitalRead(DOWN)) {
      unsigned long pressStart = millis();
      while (!digitalRead(DOWN)) {}
      if ((millis() - pressStart) > 500) {
        bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
      } else {
        bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
      }
      delay(200);
    }
    delay(50);
  }
  tft.fillScreen(TFT_BLACK);
  bt_off();
  ESP.restart();
}

void BLERemote() {
  btStart();
  bleKeyboard.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(&JetBrainsMono_ExtraBold24pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("-_-", 120, 140);
  while (!bleKeyboard.isConnected()) {
    if (!digitalRead(SEL)) {
      break;
    }
  }
  tft.drawString(">~<", 120, 140);
  while (bleKeyboard.isConnected()) {

    if (!digitalRead(UP)) {
      bleKeyboard.press(KEY_LEFT_ARROW);
      while (!digitalRead(UP)) {}
      bleKeyboard.release(KEY_LEFT_ARROW);
      delay(200);
    }
    if (!digitalRead(SEL)) {
      unsigned long pressStart = millis();
      while (!digitalRead(SEL)) {}
      if ((millis() - pressStart) > 500) {
        break;
      } else {
        bleKeyboard.press(KEY_LEFT_GUI);
        bleKeyboard.press('l');
        delay(100);
        bleKeyboard.releaseAll();
      }
      delay(200);
    }
    if (!digitalRead(DOWN)) {
      bleKeyboard.press(KEY_RIGHT_ARROW);
      while (!digitalRead(DOWN)) {}
      bleKeyboard.release(KEY_RIGHT_ARROW);
      delay(200);
    }
    delay(50);
  }
  tft.fillScreen(TFT_BLACK);
  bt_off();
  ESP.restart();
}

void BLEkeeb() {
  int update = 0;
  btStart();
  bleKeyboard.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(&JetBrainsMono_ExtraBold24pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(";n;", 120, 140);
  while (!bleKeyboard.isConnected()) {
    if (!digitalRead(SEL)) {
      break;
    }
  }
  tft.drawString(">w<", 120, 140);
  while (bleKeyboard.isConnected()) {

    if (!digitalRead(UP)) {
      if (char_counter > 0) {
        char_counter--;
        delay(200);
        update = 1;
      }
    }
    if (!digitalRead(SEL)) {
      unsigned long pressStart = millis();
      while (!digitalRead(SEL)) {}
      if ((millis() - pressStart) > 500) {
        break;
      } else {
        bleKeyboard.press(char_list[char_counter]);
        while (!digitalRead(SEL)) {}
        bleKeyboard.release(char_list[char_counter]);
      }
      delay(200);
    }
    if (!digitalRead(DOWN)) {
      if (char_counter < char_list_length - 1) {
        char_counter++;
        delay(200);
        update = 1;
      }
    }
    if (update == 1) {
      tft.fillScreen(TFT_BLACK);
      tft.drawString(String(char_list[char_counter]), 120, 140);
    }
    delay(50);
  }
  tft.fillScreen(TFT_BLACK);
  bt_off();
  ESP.restart();
}

void execWifiMenuFunction(int item) {
  delay(250);
  if (item == 1) {
    tft.fillScreen(TFT_BLACK);
    tft.setFreeFont(&JetBrainsMono_ExtraBold16pt7b);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(WiFiModes[WiFi.getMode()], 20, 20);
    if (WiFi.getMode() == 1) {
      tft.setFreeFont(&JetBrainsMono_ExtraBold14pt7b);
      tft.drawString("IP:", 20, 60);
      tft.drawString(WiFi.localIP().toString(), 20, 90);
      tft.drawString("Net:", 20, 120);
      tft.drawString(String(WiFi.SSID()), 20, 150);
    } else if (WiFi.getMode() == 2) {
      tft.setFreeFont(&JetBrainsMono_ExtraBold14pt7b);
      tft.drawString("IP:", 20, 60);
      tft.drawString(WiFi.softAPIP().toString(), 20, 90);
      tft.drawString("Net:", 20, 120);
      tft.drawString(ap_ssid, 20, 150);
      tft.drawString("Password:", 20, 180);
      tft.drawString(ap_password, 20, 210);
    }
    while (1) {
      if (digitalRead(SEL) == 0) {
        delay(250);
        tft.fillScreen(TFT_BLACK);
        menu_index = 0;
        last_menu_index = -1;
        break;
      }
    }
  } else if (item == 2) {
    disable_wifi();
    Serial.println("disable");
    delay(250);
  } else if (item == 3) {
    wifi_ap();
    Serial.println("ap");
    delay(250);
  } else if (item == 4) {
    Serial.println("station");
    wifi_sta();
    delay(250);
  } else if (item == 5) {
    Serial.println("cp");
    host_cp();
    delay(250);
  } else {
    return;
  }
}

void execBleMenuFunction(int item) {
  if (item == 1) {
    BLERemote();
    delay(250);
  } else if (item == 2) {
    BLEMediaController();
    delay(250);
  } else if (item == 3) {
    BLEkeeb();
    delay(250);
  } else {
    return;
  }
}

void execGameMenuFunction(int item) {
  if (item == 1) {
    pong();
  } else if (item == 2) {
    invaders_game();
  } else {
    return;
  }
}

void setup() {
  pinMode(UP, INPUT_PULLUP);
  pinMode(D1, INPUT_PULLDOWN);
  pinMode(D0, INPUT);
  pinMode(SEL, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  esp_deep_sleep_enable_gpio_wakeup((1ULL << D3), ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_enable_gpio_wakeup((1ULL << D2), ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_enable_gpio_wakeup((1ULL << D1), ESP_GPIO_WAKEUP_GPIO_HIGH);
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  tft.init();
  analogWrite(TFT_BL, brightness);
  tft.fillScreen(TFT_BLACK);
  if (digitalRead(D1) == 1) {
    charging();
  } else if (digitalRead(SEL) == 0) {
    if ((boot == 0) or (rtc.getEpoch() < 1609459200)) {
      timeSync();
    }
    watchface();
    delay(8000);
  } else if (digitalRead(UP) == 0) {
    tft.fillScreen(TFT_BLACK);
    menu_index = 0;
    drawMenu(main_menu_items, main_menu_len);
  } else if (digitalRead(DOWN) == 0) {
    tft.fillScreen(TFT_BLACK);
    menu_index = 0;
    drawMenu(game_menu_items, game_menu_len);
  }
  boot++;
  tft.fillScreen(TFT_BLACK);
  digitalWrite(TFT_BL, LOW);
  Serial.println("I'm eepy");
  esp_deep_sleep_start();
}

void loop() {}