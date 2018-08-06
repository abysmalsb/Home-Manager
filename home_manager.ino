#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050_tockn.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define LEFT_BUTTON   P1_1
#define ENTER_BUTTON  P1_2
#define RIGHT_BUTTON  P1_0
#define BACK_BUTTON   P1_3

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16


#define MAIN_MENU   0
#define SUB_MENU    1
#define APPLICATION 2

#define SAFETY      0
#define SPORT       1
#define HOME        2
#define ROBOT_ARM   3
#define GAMES       4
#define OTHER       5
char * mainMenu[] = {
  "Safety",
  "Sport",
  "Home",
  "Robot Arm",
  "Games",
  "Other"
};

#define CALL_FOR_HELP   0
#define FALL_DETECTION  1
#define SLEEPING        2
char * safetySubMenu[] = {
  "Call for help",
  "Fall detection",
  "Sleeping"
};

#define RUNNING     0
#define JUMPING     1
char * sportSubMenu[] = {
  "Running",
  "Jumping"
};

#define MY_DOOR_LOCK        0
#define WATERING_MIMOSA     1
#define KITCHEN_SINK_LIGHTS 2
char * homeSubMenu[] = {
  "My door lock",
  "Watering mimosa",
  "Kitchen sink lights"
};

#define CONTROL     0
#define VIEW_SENSOR 1
char * robotArmSubMenu[] = {
  "Control",
  "View sensor data"
};

#define CONFIGURE   0
#define ABOUT       1
char * otherSubMenu[] = {
  "Configure",
  "About"
};

int previousButtonState[] =  {LOW, LOW, LOW, LOW};

int currentLayer = MAIN_MENU;
int currentMenuPointInMainMenu = HOME;
int currentMenuPointInSubMenu = 0;

bool displayEnabled = true;

long timeOfLastClick;
int displaySleepIn = 30000; // millis

static const unsigned char PROGMEM hackster_logo_bmp[] =
{ B00000000, B00011111, B11100000, B00000000,
  B00000000, B01111111, B11111000, B00000000,
  B00000001, B11111000, B01111110, B00000000,
  B00000011, B11000000, B00001111, B00000000,
  B00000111, B00000000, B00000011, B10000000,
  B00001110, B00000000, B00000001, B11000000,
  B00011100, B00000000, B00000000, B11100000,
  B00111000, B00000000, B01110000, B01100000,
  B00111000, B00000000, B01110000, B01110000,
  B00110000, B00111000, B01110000, B00110000,
  B01110000, B00000000, B01110000, B00110000,
  B01110000, B00111000, B01110000, B00111000,
  B11100001, B11111000, B00000000, B00111000,
  B11100000, B00111111, B11110000, B00011000,
  B11100000, B00111111, B11110000, B00011000,
  B11100000, B00000000, B01111110, B00111000,
  B01110000, B00111000, B01110000, B00111000,
  B01110000, B00111000, B00000000, B00110000,
  B00110000, B00111000, B01110000, B00110000,
  B00111000, B00111000, B00000000, B01110000,
  B00111000, B00111000, B00000000, B01100000,
  B00011100, B00000000, B00000000, B11100000,
  B00001110, B00000000, B00000001, B11000000,
  B00000111, B00000000, B00000011, B10000000,
  B00000011, B11000000, B00001111, B00000000,
  B00000001, B11111000, B01111110, B00000000,
  B00000000, B01111111, B11111000, B00000000,
  B00000000, B00011111, B11100000, B00000000
};

static const unsigned char PROGMEM maxim_logo_bmp[] =
{ B00000000, B00000011, B11111100, B00000000,
  B00000000, B00001111, B11111111, B00000000,
  B00000000, B00111111, B11111111, B11000000,
  B00000000, B01111111, B11111111, B11100000,
  B00000000, B11111111, B11111111, B11110000,
  B00000001, B11111111, B11111111, B11111000,
  B00000011, B11111111, B11111111, B11111100,
  B00000011, B11000001, B11111000, B00111100,
  B00000111, B10000000, B11110000, B00111110,
  B00000111, B10000000, B01100000, B00111110,
  B00001111, B10001100, B01100011, B00111110,
  B00001111, B10001110, B01000111, B00111111,
  B00001111, B10001110, B11000111, B00111111,
  B00001111, B10001111, B10001111, B00111111,
  B00001111, B10001111, B10011111, B00111111,
  B00001111, B10001111, B00001111, B00111111,
  B00001111, B10001110, B00001111, B00111111,
  B00001111, B10001110, B00000111, B00111110,
  B00000111, B10001100, B01100011, B00111110,
  B00000111, B10001100, B01100011, B00111110,
  B00000111, B10001000, B11110001, B00111100,
  B00000011, B11111111, B11111111, B11111100,
  B00000001, B11111111, B11111111, B11111000,
  B00000000, B11111111, B11111111, B11110000,
  B00000000, B01111111, B11111111, B11100000,
  B00000000, B00111111, B11111111, B11000000,
  B00000000, B00001111, B11111111, B00000000,
  B00000000, B00000011, B11111100, B00000000
};

static const unsigned char PROGMEM left_arrow_bmp[] =
{ B00001100,
  B00011000,
  B00110000,
  B01100000,
  B11000000,
  B01100000,
  B00110000,
  B00011000,
  B00001100
};

static const unsigned char PROGMEM right_arrow_bmp[] =
{
  B00110000,
  B00011000,
  B00001100,
  B00000110,
  B00000011,
  B00000110,
  B00001100,
  B00011000,
  B00110000
};

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

MPU6050 mpu6050(Wire0);

void setup()   {
  Serial.begin(9600);

  pinMode(LEFT_BUTTON, INPUT);
  pinMode(ENTER_BUTTON, INPUT);
  pinMode(RIGHT_BUTTON, INPUT);
  pinMode(BACK_BUTTON, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.setRotation(2); // rotating the screen upside down

  // Showing the name of the device and the logos
  display.clearDisplay();
  drawSplashScreen();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  display.clearDisplay();

  //Draw the first layer of the menu
  refreshMenu();

  timeOfLastClick = millis();

  /* draw a single pixel
    display.drawPixel(10, 10, WHITE);
    // Show the display buffer on the hardware.
    // NOTE: You _must_ call display after making any drawing commands
    // to make them visible on the display hardware!
    display.display();
    delay(2000);
    display.clearDisplay();

    // draw many lines
    testdrawline();
    display.display();
    delay(2000);
    display.clearDisplay();

    // draw rectangles
    testdrawrect();
    display.display();
    delay(2000);
    display.clearDisplay();

    // draw multiple rectangles
    testfillrect();
    display.display();
    delay(2000);
    display.clearDisplay();

    // draw mulitple circles
    testdrawcircle();
    display.display();
    delay(2000);
    display.clearDisplay();

    // draw a white circle, 10 pixel radius
    display.fillCircle(display.width() / 2, display.height() / 2, 10, WHITE);
    display.display();
    delay(2000);
    display.clearDisplay();

    testdrawroundrect();
    delay(2000);
    display.clearDisplay();

    testfillroundrect();
    delay(2000);
    display.clearDisplay();

    testdrawtriangle();
    delay(2000);
    display.clearDisplay();

    testfilltriangle();
    delay(2000);
    display.clearDisplay();

    // draw the first ~12 characters in the font
    testdrawchar();
    display.display();
    delay(2000);
    display.clearDisplay();

    // draw scrolling text
    testscrolltext();
    delay(20000);
    display.clearDisplay();

    // text display tests
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Hello, world!");
    display.setTextColor(BLACK, WHITE); // 'inverted' text
    display.println(3.141592);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("0x"); display.println(0xDEADBEEF, HEX);
    display.display();
    delay(2000);
    display.clearDisplay();

    // miniature bitmap display
    display.drawBitmap(30, 16,  logo16_glcd_bmp, 16, 16, 1);
    display.display();
    delay(1);

    // invert the display
    display.invertDisplay(true);
    delay(1000);
    display.invertDisplay(false);
    delay(1000);
    display.clearDisplay();

    // draw a bitmap icon and 'animate' movement
    testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);*/
}


void loop() {
  if (buttonUpdate(LEFT_BUTTON, 0)) {
    timeOfLastClick = millis();
    if (!displayEnabled) {
      switchOnDisplay();
    }
    else if (currentLayer == MAIN_MENU && currentMenuPointInMainMenu > SAFETY) {
      currentMenuPointInMainMenu--;
      refreshMenu();
    }
    else if (currentLayer == SUB_MENU && currentMenuPointInSubMenu > 0) {
      currentMenuPointInSubMenu--;
      refreshMenu();
    }
  }
  else if (buttonUpdate(ENTER_BUTTON, 1)) {
    timeOfLastClick = millis();
    if (!displayEnabled) {
      switchOnDisplay();
    }
    else if (currentLayer < APPLICATION) {
      currentLayer++;
      refreshMenu();
    }
  }
  else if (buttonUpdate(RIGHT_BUTTON, 2)) {
    timeOfLastClick = millis();
    if (!displayEnabled) {
      switchOnDisplay();
    }
    else if (currentLayer == MAIN_MENU && currentMenuPointInMainMenu < OTHER) {
      currentMenuPointInMainMenu++;
      refreshMenu();
    }
    else if (currentLayer == SUB_MENU && currentMenuPointInSubMenu < getLastElementOfCurrentSubMenu()) {
      currentMenuPointInSubMenu++;
      refreshMenu();
    }
  }
  else if (buttonUpdate(BACK_BUTTON, 3)) {
    timeOfLastClick = millis();
    if (!displayEnabled) {
      switchOnDisplay();
    }
    else if (currentLayer > MAIN_MENU) {
      if (currentLayer == SUB_MENU) {
        currentMenuPointInSubMenu = 0;
      }
      currentLayer--;
      refreshMenu();
    }
  }

  if (displayEnabled && currentLayer == APPLICATION) {
    refreshMenu();
  }

  if (displayEnabled && millis() - timeOfLastClick > displaySleepIn) {
    switchOffDisplay();
  }
}

bool buttonUpdate(int buttonId, int previousStateId) {

  int currentButtonState = digitalRead(buttonId);
  bool needsUpdate = false;
  if (currentButtonState && !previousButtonState[previousStateId]) {
    delay(50);
    needsUpdate = true;
  }
  else if (!currentButtonState && previousButtonState[previousStateId]) {
    delay(50);
  }
  previousButtonState[previousStateId] = currentButtonState;
  return needsUpdate;
}

void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  uint8_t icons[NUMFLAKES][3];

  // initialize
  for (uint8_t f = 0; f < NUMFLAKES; f++) {
    icons[f][XPOS] = random(display.width());
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = random(3) + 1;

    Serial.print("x: ");
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(" y: ");
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(" dy: ");
    Serial.println(icons[f][DELTAY], DEC);
  }

  while (1) {
    // draw each icon
    digitalWrite(LED_BUILTIN, !digitalRead(BACK_BUTTON));
    for (uint8_t f = 0; f < NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, WHITE);
    }
    display.display();
    delay(50);

    // then erase it + move it
    for (uint8_t f = 0; f < NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
        icons[f][XPOS] = random(display.width());
        icons[f][YPOS] = 0;
        icons[f][DELTAY] = random(3) + 1;
      }
    }
  }
}


void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
  delay(1);
}

void testdrawcircle(void) {
  for (int16_t i = 0; i < display.height(); i += 2) {
    display.drawCircle(display.width() / 2, display.height() / 2, i, WHITE);
    display.display();
    delay(1);
  }
}

void fillHeader(void) {
  uint8_t color = 1;
  for (int16_t i = 0; i < display.height() / 2; i += 3) {
    // alternate colors
    display.fillRect(i, i, display.width() - i * 2, display.height() - i * 2, color % 2);
    display.display();
    delay(1);
    color++;
  }
}

void testdrawtriangle(void) {
  for (int16_t i = 0; i < min(display.width(), display.height()) / 2; i += 5) {
    display.drawTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, WHITE);
    display.display();
    delay(1);
  }
}

void testfilltriangle(void) {
  uint8_t color = WHITE;
  for (int16_t i = min(display.width(), display.height()) / 2; i > 0; i -= 5) {
    display.fillTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, WHITE);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
    delay(1);
  }
}

void testdrawroundrect(void) {
  for (int16_t i = 0; i < display.height() / 2 - 2; i += 2) {
    display.drawRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, WHITE);
    display.display();
    delay(1);
  }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for (int16_t i = 0; i < display.height() / 2 - 2; i += 2) {
    display.fillRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
    delay(1);
  }
}

void testdrawrect(void) {
  for (int16_t i = 0; i < display.height() / 2; i += 2) {
    display.drawRect(i, i, display.width() - 2 * i, display.height() - 2 * i, WHITE);
    display.display();
    delay(1);
  }
}

void testdrawline() {
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(0, 0, i, display.height() - 1, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i = 0; i < display.height(); i += 4) {
    display.drawLine(0, 0, display.width() - 1, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(0, display.height() - 1, i, 0, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4) {
    display.drawLine(0, display.height() - 1, display.width() - 1, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = display.width() - 1; i >= 0; i -= 4) {
    display.drawLine(display.width() - 1, display.height() - 1, i, 0, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4) {
    display.drawLine(display.width() - 1, display.height() - 1, 0, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.height(); i += 4) {
    display.drawLine(display.width() - 1, 0, 0, i, WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(display.width() - 1, 0, i, display.height() - 1, WHITE);
    display.display();
    delay(1);
  }
  delay(250);
}

void testscrolltext(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  display.setCursor(0, 0);
  display.clearDisplay();
  display.println("abcdefghijklmnoprqstuvwxyzabcdefghijklmnoprqstuvwxyz");
  display.display();
  delay(10000);

  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}

void drawSplashScreen(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(52, 6);
  display.println("Home");
  display.setCursor(45, 18);
  display.println("Manager");
  // drawing Hackster logo
  display.drawBitmap(0, 2,  hackster_logo_bmp, 32, 28, 1);
  // drawing Maxim logo
  display.drawBitmap(96, 2,  maxim_logo_bmp, 32, 28, 1);
  display.display();
}

void refreshMenu() {
  display.clearDisplay();

  if (currentLayer == MAIN_MENU) {
    refreshMainMenu();
  }
  else if (currentLayer == SUB_MENU) {
    refreshSubMenu();
  }
  else if (currentLayer == APPLICATION) {
    refreshApplication();
  }
  else {
    drawError();
  }

  display.display();
}

void drawError() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(16, 0);
  display.println("UNDEFINED");
  display.setCursor(34, 16);
  display.println("STATE");
}

void refreshMainMenu() {
  if (currentMenuPointInMainMenu > SAFETY)
    display.drawBitmap(0, 12,  left_arrow_bmp, 8, 9, 1);
  if (currentMenuPointInMainMenu < OTHER)
    display.drawBitmap(120, 12,  right_arrow_bmp, 8, 9, 1);

  int cursorOffsetX = strlen(mainMenu[currentMenuPointInMainMenu]) * 6;
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(64 - cursorOffsetX, 8);
  display.println(mainMenu[currentMenuPointInMainMenu]);
}

void refreshSubMenu() {
  switch (currentMenuPointInMainMenu) {
    case SAFETY:
      drawSubMenu(safetySubMenu);
      break;
    case SPORT:
      drawSubMenu(sportSubMenu);
      break;
    case HOME:
      drawSubMenu(homeSubMenu);
      break;
    case ROBOT_ARM:
      drawSubMenu(robotArmSubMenu);
      break;
    case OTHER:
      drawSubMenu(otherSubMenu);
      break;
    default:
      drawError();
      return;
  }
}

void drawSubMenu(char * subMenu[]) {
  int lastOption = getLastElementOfCurrentSubMenu();
  if (currentMenuPointInSubMenu > 0)
    display.drawBitmap(0, 22,  left_arrow_bmp, 8, 9, 1);
  if (currentMenuPointInSubMenu < lastOption)
    display.drawBitmap(120, 22,  right_arrow_bmp, 8, 9, 1);

  int cursorOffsetX = strlen(mainMenu[currentMenuPointInMainMenu]) * 6;
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(64 - cursorOffsetX, 0);
  display.println(mainMenu[currentMenuPointInMainMenu]);
  display.drawLine(0, display.height() / 2, display.width(), display.height() / 2, WHITE);

  cursorOffsetX = strlen(subMenu[currentMenuPointInSubMenu]) * 3;
  display.setTextSize(1);
  display.setCursor(64 - cursorOffsetX, 24);
  display.println(subMenu[currentMenuPointInSubMenu]);
}

int getLastElementOfCurrentSubMenu() {
  switch (currentMenuPointInMainMenu) {
    case SAFETY:
      return SLEEPING;
    case SPORT:
      return JUMPING;
    case HOME:
      return KITCHEN_SINK_LIGHTS;
    case ROBOT_ARM:
      return VIEW_SENSOR;
    case OTHER:
      return ABOUT;
    default:
      drawError();
      return 0;
  }
}

void refreshApplication() {
  switch (currentMenuPointInMainMenu) {
    case SAFETY:
      break;
    case SPORT:
      break;
    case HOME:
      break;
    case ROBOT_ARM:
      if (currentMenuPointInSubMenu == CONTROL) {

      }
      else if (currentMenuPointInSubMenu == VIEW_SENSOR) {
        drawViewSensorData();
      }
      break;
    case OTHER:
      if (currentMenuPointInSubMenu == CONFIGURE) {

      }
      else if (currentMenuPointInSubMenu == ABOUT) {
        drawAboutApp();
      }
      break;
    default:
      drawError();
  }
}

void switchOffDisplay() {
  displayEnabled = false;
  display.clearDisplay();
  display.display();
}

void switchOnDisplay() {
  displayEnabled = true;
  refreshMenu();
}

void drawViewSensorData() {
  mpu6050.update();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Accelerometer angles:");
  display.setCursor(0, 8);
  display.println("X:");
  display.setCursor(16, 8);
  display.println(correctGyroAngle(mpu6050.getAccAngleX()));
  display.setCursor(64, 8);
  display.println("Y:");
  display.setCursor(80, 8);
  display.println(correctGyroAngle(mpu6050.getAccAngleY()));
  display.setCursor(0, 16);
  display.println("Gyroscope angles:");
  display.setCursor(0, 24);
  display.println("X:");
  display.setCursor(16, 24);
  display.println(mpu6050.getGyroAngleX());
  display.setCursor(64, 24);
  display.println("Y:");
  display.setCursor(80, 24);
  display.println(mpu6050.getGyroAngleY());
}

// The sensor is upside down, the angles needs to be flipped
float correctGyroAngle(float sensorAngle) {
  if (sensorAngle > 0)
    sensorAngle -= 180;
  else
    sensorAngle += 180;
  return sensorAngle;
}

void drawAboutApp() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Creator:\nBalazs Simon\nWeb:\nhackster.io/Abysmal");
  display.drawLine(0, display.height() / 2, display.width(), display.height() / 2, WHITE);
}

