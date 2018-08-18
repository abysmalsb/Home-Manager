#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050_tockn.h>
#include <MAX77650-Arduino-Library.h>
#include <MAX17055.h>
#include <ArduinoJson.h>

#define ESP_SERIAL      Serial0

#define OLED_RESET      4
Adafruit_SSD1306 display(OLED_RESET);

#define LEFT_BUTTON     P1_1
#define ENTER_BUTTON    P1_2
#define RIGHT_BUTTON    P1_0
#define BACK_BUTTON     P1_3

#define MAX77650_debug  true
#define MAX77650_PHLD   P2_2   //Pin 18 -> connected to MAX77650 power hold input pin (A1)

#define MAIN_MENU       0
#define SUB_MENU        1
#define APPLICATION     2

#define SAFETY          0
#define SPORT           1
#define HOME            2
#define ROBOT_ARM       3
#define GAMES           4
#define OTHER           5
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
char * safetySubMenu[] = {
  "Call for help",
  "Fall detection"
};

#define PEDOMETER         0
#define JUMPING         1
char * sportSubMenu[] = {
  "Pedometer",
  "Jumping"
};

#define MY_DOOR_LOCK        0
#define IRRIGATING_MIMOSA     1
#define KITCHEN_SINK_LIGHTS 2
char * homeSubMenu[] = {
  "My door lock",
  "Irrigating mimosa",
  "Kitchen sink lights"
};

#define CONTROL         0
#define VIEW_SENSOR     1
char * robotArmSubMenu[] = {
  "Control",
  "View sensor data"
};

#define DINOSAUR        0
char * gamesSubMenu[] = {
  "Dinosaur"
};

#define BATTERY         0
#define ABOUT           1
char * otherSubMenu[] = {
  "Battery",
  "About"
};

#define BASE            0
#define LOWER_JOINT     1
#define UPPER_JOINT     2
#define GRIPPER         3
char * robotArmSegments[] = {
  "Base",
  "JntL",
  "JntU",
  "Grip"
};
int selectedRobotArmSegment = BASE;

#define DOOR_LOCK_OPEN        3000                              // the door stays open for 3 seconds
#define EMAIL_CONTACT         "test@example.com"        // e-mail address of your emergency contact
#define EMERGENCY_MSG         "I need help, please come ASAP!"  // emergency email message
#define FALL_MSG              "I fell, please help!"            // automatic message in case of the owner fell

#define FALLING_THRESHOLD         539.8f

#define PEDOMETER_THRESHOLD_HIGH  0.65f
#define PEDOMETER_THRESHOLD_LOW   -0.65f
#define BURNT_CALORIES_PER_STEP   0.05f
#define JUMPING_THRESHOLD_HIGH    0.6f
#define JUMPING_THRESHOLD_LOW     -0.8f
float avgAccSum = 500.0f;
bool overThreshold = false;

#define MQTT_TOPIC_MY_DOOR_LOCK         "my_door_lock"
#define MQTT_TOPIC_IRRIGATING_MIMOSA    "irrigating_mimosa"
#define MQTT_TOPIC_KITCHEN_SINK_LIGHTS  "kitchen_sink_lights"
#define MQTT_TOPIC_ROBOT_CONTROL        "robot_control"

float robotBaseAngle = 90.0f;
float robotLowerJointAngle = 90.0f;
float robotUpperJointAngle = 90.0f;
float robotGripper = 90.0f;

int previousButtonState[] =  {LOW, LOW, LOW, LOW};

int currentLayer = MAIN_MENU;
int currentMenuPointInMainMenu = HOME;
int currentMenuPointInSubMenu = 0;

bool displayEnabled = true;

long timeOfLastClick = 0;
int displaySleepIn = 60000; // millis

float mimosaSoilHumidity = 0;

// Dinosaur game config
#define GAME_SPEED      150
#define MAX_GAME_SPEED  15
#define RUNNING_SPEED   50
unsigned long scoreUpdate = 0;
int speedingUp = 0;
bool leftLeg = false;
long runningTimePassed = 0;
#define JUMP_SPEED      10.0f
#define GRAVITY         0.5f
float verticalSpeed = 0;
int dinoPosX = 8;
int dinoPosy = 17;
int dirtContainer[10];
int cactus1PosX;
int cactus2PosX;
int cactus1bmp;
int cactus2bmp;
bool gameover = false;

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

static const unsigned char PROGMEM dinosaur_body_bmp[] =
{
  B00000000, B00111110,
  B00000000, B01011111,
  B00000000, B01111111,
  B00000000, B01110000,
  B01000000, B01111110,
  B01000000, B11110000,
  B01100001, B11110000,
  B01110011, B11111100,
  B00111111, B11110100,
  B00111111, B11110000,
  B00011111, B11110000,
  B00000111, B11100000
};

static const unsigned char PROGMEM dinosaur_legs_right_bmp[] =
{
  B01100100,
  B00110100,
  B00000110
};

static const unsigned char PROGMEM dinosaur_legs_left_bmp[] =
{
  B01001100,
  B01000110,
  B01100000
};

static const unsigned char PROGMEM dinosaur_legs_jump_bmp[] =
{
  B01000100,
  B01000100,
  B01100110
};

static const unsigned char PROGMEM dirt1_bmp[] =
{
  B11100000, B00000001,
  B01000001, B00000000,
  B00000000, B00000100
};

static const unsigned char PROGMEM dirt2_bmp[] =
{
  B00000001, B00000000,
  B10000000, B00000000,
  B00000000, B00000001
};

static const unsigned char PROGMEM dirt3_bmp[] =
{
  B10000000, B00000000,
  B00000000, B00000011,
  B00001000, B00000000
};

static const unsigned char PROGMEM cactus1_bmp[] =
{
  B00000001, B00000000,
  B00000011, B10000000,
  B00000011, B10011000,
  B00010011, B10011000,
  B00111011, B10011000,
  B00111011, B10111000,
  B00111011, B11110000,
  B00011111, B11100000,
  B00001111, B10000000,
  B00000111, B10000000,
  B00000011, B10000000,
  B00000011, B10000000,
  B00000011, B10000000,
  B00000011, B10000000
};

static const unsigned char PROGMEM cactus2_bmp[] =
{
  B00111000,
  B00111000,
  B00111011,
  B10111011,
  B10111011,
  B10111011,
  B01111011,
  B00111111,
  B00111110,
  B00111000,
  B00111000,
  B00111000,
  B00111000
};

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

MPU6050 mpu6050(Wire0);
MAX17055 sensor;

int counter = 0;
boolean doTask = false;
boolean previousDoTask = false;


void setup()   {
  // enabling power from battery
  pinMode(MAX77650_PHLD, OUTPUT);          //configure pin as output
  digitalWrite(MAX77650_PHLD, HIGH);       //set output to HIGH to hold the power-on state

  Serial.begin(115200);
  ESP_SERIAL.begin(115200);
  
  enableCharging();

  Wire2.begin();
  sensor.setCapacity(260);

  pinMode(LEFT_BUTTON, INPUT);
  pinMode(ENTER_BUTTON, INPUT);
  pinMode(RIGHT_BUTTON, INPUT);
  pinMode(BACK_BUTTON, INPUT);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.setRotation(2); // rotating the screen upside down

  // Showing the name of the device and the logos
  display.clearDisplay();
  drawSplashScreen();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(52, 6);
  display.println("Home");
  display.setCursor(45, 18);
  display.println("Manager");
  display.display();

  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  delay(10);

  display.clearDisplay();

  //Draw the first layer of the menu
  refreshMenu();

  timeOfLastClick = millis();
}

void drawSplashScreen(void) {
  // drawing Hackster logo
  display.drawBitmap(0, 2, hackster_logo_bmp, 32, 28, 1);
  // drawing Maxim logo
  display.drawBitmap(96, 2, maxim_logo_bmp, 32, 28, 1);
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
    else if(currentLayer == APPLICATION){
      if (currentMenuPointInSubMenu == CONTROL && !doTask) {
        selectedRobotArmSegment = selectedRobotArmSegment != BASE ? selectedRobotArmSegment - 1 : GRIPPER;
      }
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
    else if (currentLayer == APPLICATION) {
      doTask = !doTask;
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
    else if(currentLayer == APPLICATION){
      if (currentMenuPointInSubMenu == CONTROL && !doTask) {
        selectedRobotArmSegment = selectedRobotArmSegment != GRIPPER ? selectedRobotArmSegment + 1 : BASE;
      }
      if (currentMenuPointInSubMenu == DINOSAUR && doTask && verticalSpeed == 0.0f) {
        // 0.0f will happen when you set it by hand
        verticalSpeed = JUMP_SPEED;
      }
    }
  }
  else if (buttonUpdate(BACK_BUTTON, 3)) {
    timeOfLastClick = millis();
    if (!displayEnabled) {
      switchOnDisplay();
    }
    else if (currentLayer == SUB_MENU) {
      currentMenuPointInSubMenu = 0;
      currentLayer--;
      refreshMenu();
    }
    else if (currentLayer == APPLICATION) {
      currentLayer--;
      refreshMenu();
      counter = 0;
      doTask = false;
      previousDoTask = false;
    }
  }

  if (currentLayer == APPLICATION) {
    refreshMenu();
  }

  if ((displayEnabled && millis() - timeOfLastClick > displaySleepIn && currentLayer != APPLICATION)
    || (displayEnabled && millis() - timeOfLastClick > displaySleepIn * 10 && currentLayer == APPLICATION)) {
    switchOffDisplay();
  }

  if (ESP_SERIAL.available()) {
    DynamicJsonBuffer jsonBuffer;
    String message = ESP_SERIAL.readStringUntil('\n');
    Serial.println(message);
    JsonObject& root = jsonBuffer.parseObject(message);

    String topic = root["topic"];
    if(topic.equals("mimosa_humidity_level")){
      mimosaSoilHumidity = atof(root["message"]);
    }
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

bool buttonUpdate(int buttonId, int previousStateId) {

  int currentButtonState = digitalRead(buttonId);
  bool needsUpdate = false;
  if (currentButtonState && !previousButtonState[previousStateId]) {
    delay(25);
    needsUpdate = true;
  }
  else if (!currentButtonState && previousButtonState[previousStateId]) {
    delay(25);
  }
  previousButtonState[previousStateId] = currentButtonState;
  return needsUpdate;
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
    case GAMES:
      drawSubMenu(gamesSubMenu);
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
      return FALL_DETECTION;
    case SPORT:
      return JUMPING;
    case HOME:
      return KITCHEN_SINK_LIGHTS;
    case ROBOT_ARM:
      return VIEW_SENSOR;
    case GAMES:
      return DINOSAUR;
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
      if (currentMenuPointInSubMenu == CALL_FOR_HELP) {
        drawCallForHelp();
      }
      else if (currentMenuPointInSubMenu == FALL_DETECTION) {
        drawFallDetection();
      }
      break;
    case SPORT:
      if (currentMenuPointInSubMenu == PEDOMETER) {
        drawPedometer();
      }
      else if (currentMenuPointInSubMenu == JUMPING) {
        drawJumping();
      }
      break;
    case HOME:
      if (currentMenuPointInSubMenu == MY_DOOR_LOCK) {
        drawMyDoorLock();
      }
      else if (currentMenuPointInSubMenu == IRRIGATING_MIMOSA) {
        drawIrrigatingMimosa();
      }
      else if (currentMenuPointInSubMenu == KITCHEN_SINK_LIGHTS) {
        drawKitchenSinkLights();
      }
      break;
    case ROBOT_ARM:
      if (currentMenuPointInSubMenu == CONTROL) {
        drawControl();
      }
      else if (currentMenuPointInSubMenu == VIEW_SENSOR) {
        drawViewSensorData();
      }
      break;
    case GAMES:
      if (currentMenuPointInSubMenu == DINOSAUR) {
        drawDinosaur();
      }
      break;
    case OTHER:
      if (currentMenuPointInSubMenu == BATTERY) {
        drawBatteryInfo();
      }
      else if (currentMenuPointInSubMenu == ABOUT) {
        drawAboutApp();
      }
      break;
    default:
      drawError();
  }
}

void drawCallForHelp(){
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(28, 0);
  display.println("Call for help");
  display.setTextColor(WHITE);
  display.setCursor(0, 8);
  display.println("Contact:");
  display.setCursor(0, 16);
  display.setTextWrap(false);
  display.println(EMAIL_CONTACT);
  display.setTextWrap(true);
  if (doTask){
    if(!previousDoTask){
      previousDoTask = true;
      ESP_SERIAL.print("{\"type\":\"email\",\"address\":\"");
      ESP_SERIAL.print(EMAIL_CONTACT);
      ESP_SERIAL.print("\",\"subject\":\"I NEED HELP!!!\",\"message\":\"");
      ESP_SERIAL.print(EMERGENCY_MSG);
      ESP_SERIAL.println("\"}");
    }
    if(millis() - timeOfLastClick > DOOR_LOCK_OPEN){
      doTask = !doTask;
    }
    display.setCursor(39, 24);
    display.println("Contacted");
  }
  else{
    previousDoTask = false,
    display.setCursor(28, 24);
    display.println("Ask for help");
  }
}

void drawFallDetection() {
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(22, 0);
  display.println("Fall detection");
  display.setTextColor(WHITE);
  display.setCursor(0, 12);
  display.println("Status:");
  if (doTask) {
    mpu6050.update();
    float currAccSum = abs(correctGyroAngle(mpu6050.getAccY())) + abs(correctGyroAngle(mpu6050.getAccX())) + abs(correctGyroAngle(mpu6050.getAccZ()));
    if (!previousDoTask && currAccSum > FALLING_THRESHOLD) {
      previousDoTask = true;
      ESP_SERIAL.print("{\"type\":\"email\",\"address\":\"");
      ESP_SERIAL.print(EMAIL_CONTACT);
      ESP_SERIAL.print("\",\"subject\":\"I FELL PLEASE HELP!!!\",\"message\":\"");
      ESP_SERIAL.print(FALL_MSG);
      ESP_SERIAL.println("\"}");
    }
    display.setCursor(46, 12);
    display.println("On");
    display.setCursor(43, 24);
    display.println("Disable");
  }
  else {
    previousDoTask = false;
    display.setCursor(46, 12);
    display.println("Off");
    display.setCursor(46, 24);
    display.println("Enable");
  }
}

void drawPedometer() {
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(37, 0);
  display.println("Pedometer");
  display.setTextColor(WHITE);
  if (doTask) {
    if (!previousDoTask){
      counter = 0;
      previousDoTask = true;
    }
    display.setCursor(52, 25);
    display.println("Stop");
    updateCounter(true);
  }
  else {
    previousDoTask = false;
    display.setCursor(49, 25);
    display.println("Start");
  }

  display.setCursor(0, 8);
  display.println("Steps:");
  display.setCursor(60, 8);
  display.println(counter);
  display.setCursor(0, 16);
  display.println("Calories:");
  display.setCursor(60, 16);
  display.println(counter * BURNT_CALORIES_PER_STEP);
}

void drawJumping() {
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(49, 0);
  display.println("Jumps");
  display.setTextColor(WHITE);
  if (doTask) {
    if (!previousDoTask){
      counter = 0;
      previousDoTask = true;
    }
    display.setCursor(52, 25);
    display.println("Stop");
    updateCounter(false);
  }
  else {
    previousDoTask = false;
    display.setCursor(49, 25);
    display.println("Start");
  }

  display.setTextSize(2);
  display.setCursor(64 - counterDigits() * 6, 9);
  display.println(counter / 2);
}

void updateCounter(bool pedometer) {
  mpu6050.update();

  float thresholdHigh = pedometer ? PEDOMETER_THRESHOLD_HIGH : JUMPING_THRESHOLD_HIGH;
  float thresholdLow = pedometer ? PEDOMETER_THRESHOLD_LOW : JUMPING_THRESHOLD_LOW;

  float currAccSum = abs(correctGyroAngle(mpu6050.getAccY())) + abs(correctGyroAngle(mpu6050.getAccX())) + abs(correctGyroAngle(mpu6050.getAccZ()));
  avgAccSum = avgAccSum * 0.95f + currAccSum * 0.05;
  
  if(currAccSum - avgAccSum > thresholdHigh && !overThreshold){
    overThreshold = true;
    counter++;
    delay(50);
  }
  else if(currAccSum - avgAccSum < thresholdLow && overThreshold){
    overThreshold = false;
    delay(50);
  }
  
  Serial.print(avgAccSum);
  Serial.print(";");
  Serial.println(currAccSum);
}

int counterDigits() {
  int n = counter / 2;
  int count = 0;
  while (n != 0)
  {
    n /= 10;
    ++count;
  }
  return count != 0 ? count : 1;
}


void drawMyDoorLock(){
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(28, 0);
  display.println("My door lock");
  display.setTextColor(WHITE);
  display.setCursor(0, 12);
  display.println("Status:");
  if (doTask){
    if(!previousDoTask){
      previousDoTask = true;
      ESP_SERIAL.print("{\"type\":\"mqtt\",\"topic\":\"");
      ESP_SERIAL.print(MQTT_TOPIC_MY_DOOR_LOCK);
      ESP_SERIAL.println("\",\"message\":1}");
    }
    if(millis() - timeOfLastClick > DOOR_LOCK_OPEN){
      doTask = !doTask;
    }
    display.setCursor(46, 12);
    display.println("Unlocked");
  }
  else{
    previousDoTask = false;
    display.setCursor(46, 12);
    display.println("Locked");
    display.setCursor(52, 24);
    display.println("Open");
  }
}

void drawIrrigatingMimosa(){
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(13, 0);
  display.println("Irrigating mimosa");
  display.setTextColor(WHITE);
  display.setCursor(0, 12);
  display.println("Soil moisture:");
  display.setCursor(86, 12);
  display.println(mimosaSoilHumidity);
  if (doTask){
    if(!previousDoTask){
      previousDoTask = true;
      ESP_SERIAL.print("{\"type\":\"mqtt\",\"topic\":\"");
      ESP_SERIAL.print(MQTT_TOPIC_IRRIGATING_MIMOSA);
      ESP_SERIAL.println("\",\"message\":1}");
    }
    if(millis() - timeOfLastClick > DOOR_LOCK_OPEN){
      doTask = !doTask;
    }
    display.setCursor(25, 24);
    display.println("Irrigating...");
  }
  else{
    previousDoTask = false;
    display.setCursor(25, 24);
    display.println("Do irrigation");
  }
}

void drawKitchenSinkLights(){
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(7, 0);
  display.println("Kitchen sink lights");
  display.setTextColor(WHITE);
  display.setCursor(0, 12);
  display.println("Status:");
  if (doTask){
    if(!previousDoTask){
      previousDoTask = true;
      ESP_SERIAL.print("{\"type\":\"mqtt\",\"topic\":\"");
      ESP_SERIAL.print(MQTT_TOPIC_KITCHEN_SINK_LIGHTS);
      ESP_SERIAL.println("\",\"message\":1}");
    }
    display.setCursor(46, 12);
    display.println("On");
    display.setCursor(40, 24);
    display.println("Turn off");
  }
  else{
    if(previousDoTask){
      previousDoTask = false;
      ESP_SERIAL.print("{\"type\":\"mqtt\",\"topic\":\"");
      ESP_SERIAL.print(MQTT_TOPIC_KITCHEN_SINK_LIGHTS);
      ESP_SERIAL.println("\",\"message\":0}");
    }
    display.setCursor(46, 12);
    display.println("Off");
    display.setCursor(43, 24);
    display.println("Turn on");
  }
}

void drawControl(){
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(43, 0);
  display.println("Control");

  if(doTask){
    mpu6050.update();
  }

  switch(selectedRobotArmSegment){
    case BASE:
      if(doTask && selectedRobotArmSegment == BASE){
        robotBaseAngle = robotBaseAngle * 0.95 + correctGyroAngle(mpu6050.getAccAngleX()) * 0.05;
        display.fillRect(0, 9, 32, 14, WHITE);
      } else
        display.drawRect(0, 9, 32, 14, WHITE);
      break;
    case LOWER_JOINT:
      if(doTask && selectedRobotArmSegment == LOWER_JOINT){
        robotLowerJointAngle = robotLowerJointAngle * 0.95 + correctGyroAngle(mpu6050.getAccAngleX()) * 0.05;
        display.fillRect(32, 9, 32, 14, WHITE);
      } else
        display.drawRect(32, 9, 32, 14, WHITE);
      break;
    case UPPER_JOINT:
      if(doTask && selectedRobotArmSegment == UPPER_JOINT){
        robotUpperJointAngle = robotUpperJointAngle * 0.95 + correctGyroAngle(mpu6050.getAccAngleX()) * 0.05;
        display.fillRect(64, 9, 32, 14, WHITE);
      } else
        display.drawRect(64, 9, 32, 14, WHITE);
      break;
    case GRIPPER:
      if(doTask && selectedRobotArmSegment == GRIPPER){
        robotGripper = robotGripper * 0.95 + correctGyroAngle(mpu6050.getAccAngleX())  * 0.05;
        display.fillRect(96, 9, 32, 14, WHITE);
      } else
        display.drawRect(96, 9, 32, 14, WHITE);
      break;
  }
  
  if(doTask){
    previousDoTask = false;
    ESP_SERIAL.print("{\"type\":\"mqtt\",\"topic\":\"");
    ESP_SERIAL.print(MQTT_TOPIC_ROBOT_CONTROL);
    ESP_SERIAL.print("\",\"message\":\"");
    ESP_SERIAL.print(20);
    ESP_SERIAL.print(" ");
    ESP_SERIAL.print(robotBaseAngle);
    ESP_SERIAL.print(" ");
    ESP_SERIAL.print(robotLowerJointAngle);
    ESP_SERIAL.print(" ");
    ESP_SERIAL.print(robotUpperJointAngle);
    ESP_SERIAL.print(" ");
    ESP_SERIAL.print(robotGripper > 0);
    ESP_SERIAL.println("\"}");
  }
  
  display.setTextColor(doTask && selectedRobotArmSegment == BASE ? BLACK : WHITE);
  display.setCursor(4, 12);
  display.println(robotArmSegments[BASE]);
  
  display.setTextColor(doTask && selectedRobotArmSegment == LOWER_JOINT ? BLACK : WHITE);
  display.setCursor(36, 12);
  display.println(robotArmSegments[LOWER_JOINT]);
  
  display.setTextColor(doTask && selectedRobotArmSegment == UPPER_JOINT ? BLACK : WHITE);
  display.setCursor(68, 12);
  display.println(robotArmSegments[UPPER_JOINT]);
  
  display.setTextColor(doTask && selectedRobotArmSegment == GRIPPER ? BLACK : WHITE);
  display.setCursor(100, 12);
  display.println(robotArmSegments[GRIPPER]);
  
  display.setTextColor(WHITE);
  if (doTask){
    display.setCursor(52, 24);
    display.println("Stop");
  }
  else{
    display.setCursor(52, 24);
    display.println("Move");
  }
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

void drawDinosaur() {
  delay(1);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(58, 0);
  display.println("Score:");
  display.setCursor(96, 0);
  display.println(getGameScore());
  if (doTask) {
    if (!previousDoTask){
      counter = 0;
      previousDoTask = true;
      speedingUp = 0;
      randomSeed(millis());
      dinoPosy = 17;
      verticalSpeed = 0;
      for(int i = 0; i < 10; i++){
        dirtContainer[i] = random(128) - 8;
      }
      cactus1bmp = random(4);
      cactus2bmp = random(4);
      cactus1PosX = 256;
      cactus2PosX = -20;
      gameover = false;
    }

    if (!gameover){
      for(int i = 0; i < 10; i++){
        dirtContainer[i] -= 2;
        if(dirtContainer[i] < -8)
          dirtContainer[i] = 128;
      }

      cactus1PosX -= 2;
      cactus2PosX -= 2;
      if(cactus1PosX < random(25) - 15){
        cactus2PosX = cactus1PosX;
        cactus2bmp = cactus1bmp,
        cactus1PosX = 128;
        cactus1bmp = random(4);
      }
      
      if (millis() - scoreUpdate > GAME_SPEED - speedingUp){
        scoreUpdate = millis();
        if (counter < 99999){
          counter++;
        }
        if(GAME_SPEED - speedingUp > MAX_GAME_SPEED){
          speedingUp = counter / 10;
        }
      }
  
      if (dinoPosy < 17 || verticalSpeed > 0){
        verticalSpeed -= GRAVITY;
        dinoPosy -= verticalSpeed / 10;
      }
      else{
        verticalSpeed = 0.0f;
        dinoPosy = 17;
      }

      if (dinoPosy > 10 && (abs(cactus1PosX - dinoPosX - 8) < 6 || abs(cactus2PosX - dinoPosX - 8) < 6 ))
        gameover = true;
    }
    else {
      display.setTextSize(1);
      display.setTextColor(BLACK, WHITE);
      display.setCursor(52, 8);
      display.println("GAME");
      display.setCursor(52, 16);
      display.println("OVER");
      display.setTextColor(WHITE);
    }

    display.drawLine(0, 26, display.width(), 26, WHITE);

    drawDino();
    drawDirt();
    drawCactuses();
  }
  else {
    previousDoTask = false;
    display.setCursor(49, 25);
    display.println("Start");
  }
}

void drawDino() {
  if (millis() - runningTimePassed > RUNNING_SPEED){
    runningTimePassed = millis();
    if (!gameover)
      leftLeg = !leftLeg;
  }
  display.drawBitmap(dinoPosX, dinoPosy,  dinosaur_body_bmp, 16, 12, 1);
  if(dinoPosy < 17){
    display.drawBitmap(dinoPosX + 4, dinoPosy + 12,  dinosaur_legs_jump_bmp, 8, 3, 1);
  }
  else if(leftLeg){
    display.drawBitmap(dinoPosX + 4, dinoPosy + 12,  dinosaur_legs_right_bmp, 8, 3, 1);
  }
  else{
    display.drawBitmap(dinoPosX + 4, dinoPosy + 12,  dinosaur_legs_left_bmp, 8, 3, 1);
  }
}

void drawDirt(){
  for(int i = 0; i < 10; i++){
    if(i % 3 == 0)
      display.drawBitmap(dirtContainer[i], 28,  dirt1_bmp, 16, 3, 1);
    else if(i % 3 == 1)
      display.drawBitmap(dirtContainer[i], 28,  dirt2_bmp, 8, 3, 1);
    else 
      display.drawBitmap(dirtContainer[i], 28,  dirt3_bmp, 16, 3, 1);
  }
}

void drawCactuses(){
  switch(cactus1bmp){
    case 0:
      display.drawBitmap(cactus1PosX, 18, cactus1_bmp, 16, 14, 1);
      break;
    case 1:
      display.drawBitmap(cactus1PosX, 18, cactus2_bmp, 8, 13, 1);
      break;
    case 2:
      display.drawBitmap(cactus1PosX - 5, 18, cactus1_bmp, 16, 14, 1);
      display.drawBitmap(cactus1PosX + 6, 18, cactus2_bmp, 8, 13, 1);
      break;
    case 3:
      display.drawBitmap(cactus1PosX - 5, 17, cactus2_bmp, 8, 13, 1);
      display.drawBitmap(cactus1PosX + 6, 18, cactus2_bmp, 8, 13, 1);
      break;
  }
  
  switch(cactus2bmp){
    case 0:
      display.drawBitmap(cactus2PosX, 18, cactus1_bmp, 16, 14, 1);
      break;
    case 1:
      display.drawBitmap(cactus2PosX, 18, cactus2_bmp, 8, 13, 1);
      break;
    case 2:
      display.drawBitmap(cactus2PosX - 5, 18, cactus1_bmp, 16, 14, 1);
      display.drawBitmap(cactus2PosX + 6, 18, cactus2_bmp, 8, 13, 1);
      break;
    case 3:
      display.drawBitmap(cactus2PosX - 5, 17, cactus2_bmp, 8, 13, 1);
      display.drawBitmap(cactus2PosX + 6, 18, cactus2_bmp, 8, 13, 1);
      break;
  }  
}

String getGameScore(){
  String score = "00000";
  score[4] = '0' + counter % 10;
  score[3] = '0' + counter % 100 / 10;
  score[2] = '0' + counter % 1000 / 100;
  score[1] = '0' + counter % 10000 / 1000;
  score[0] = '0' + counter % 100000 / 10000;
  return score;
}

void drawBatteryInfo() {
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(43, 0);
  display.println("Battery");
  display.setTextColor(WHITE);
  display.setCursor(0, 8);
  display.println("Status (%):");
  display.setCursor(80, 8);
  display.println(sensor.getSOC());
  display.setCursor(0, 16);
  display.println("Hours remain:");
  display.setCursor(80, 16);
  display.println(sensor.getTimeToEmpty());
  display.setCursor(0, 24);
  display.println("Voltage:");
  display.setCursor(80, 24);
  display.println(sensor.getInstantaneousVoltage());
}

void drawAboutApp() {
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(49, 0);
  display.println("About");
  display.setTextColor(WHITE);
  display.setCursor(0, 8);
  display.println("Creator: Balazs Simon\nWeb:\nhackster.io/Abysmal");
}

void enableCharging(){//Configure the Power-Management (Power-Hold)
  MAX77650_init();

  //Baseline Initialization following rules printed in MAX77650 Programmres Guide Chapter 4 Page 5
  if (MAX77650_debug) Serial.print("Set Main Bias to normal Mode: ");
  if (MAX77650_setSBIA_LPM(false)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");    //Set Main Bias to normal Mode
  if (MAX77650_debug) Serial.print("Set On/Off-Button to push-button-mode: ");
  if (MAX77650_setnEN_MODE(false)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");    //set on/off-button to push-button
  if (MAX77650_debug) Serial.print("Set nEN input debounce time to 30ms: ");
  if (MAX77650_setDBEN_nEN(true)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");    //set on/off-button to push-button
  if (MAX77650_debug) Serial.print("Comparing part-numbers: ");
  if (MAX77650_getDIDM() == PMIC_partnumber) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");     //checking partnumbers
  if (MAX77650_debug) Serial.print("Checking OTP options: ");
  if (MAX77650_getCID() != MAX77650_CID) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");     //checking OTP options
  //Values for NTC beta=3800K; Battery-values are for 1s 303759 with 600mAh
  if (MAX77650_debug) Serial.print("Set the VCOLD JEITA Temperature Threshold to 0°C: ");
  if (MAX77650_setTHM_COLD(2)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");   //0°C
  if (MAX77650_debug) Serial.print("Set the VCOOL JEITA Temperature Threshold to 15°C: ");
  if (MAX77650_setTHM_COOL(3)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");   //15°C
  if (MAX77650_debug) Serial.print("Set the VWARM JEITA Temperature Threshold to 45°C: ");
  if (MAX77650_setTHM_WARM(2)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");   //45°C
  if (MAX77650_debug) Serial.print("Set the VHOT JEITA Temperature Threshold to 60°C: ");
  if (MAX77650_setTHM_HOT(3)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");    //60°C
  if (MAX77650_debug) Serial.print("Set CHGIN regulation voltage to 4.00V: ");
  if (MAX77650_setVCHGIN_MIN(0)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed"); //
  if (MAX77650_debug) Serial.print("Set CHGIN Input Current Limit to 300mA: ");
  if (MAX77650_setICHGIN_LIM(0)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed"); //300mA
  if (MAX77650_debug) Serial.print("Set the prequalification charge current to 10%: ");
  if (MAX77650_setI_PQ(false)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");   //10%
  if (MAX77650_debug) Serial.print("Set Battery prequalification voltage threshold to 3.0V: ");
  if (MAX77650_setCHG_PQ(7)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");        //3.0V
  if (MAX77650_debug) Serial.print("Set Charger Termination Current to 15% of of fast charge current: ");
  if (MAX77650_setI_TERM(3)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");        //15%
  if (MAX77650_debug) Serial.print("Set Topoff timer value to 0 minutes: ");
  if (MAX77650_setT_TOPOFF(0)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");      //0 minutes
  if (MAX77650_debug) Serial.print("Set the die junction temperature regulation point to 60°C: ");
  if (MAX77650_setTJ_REG(0)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");        //60°C Tj
  if (MAX77650_debug) Serial.print("Set System voltage regulation to 4.50V: ");
  if (MAX77650_setVSYS_REG(0x10)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");   //Vsys=4.5V
  if (MAX77650_debug) Serial.print("Set the fast-charge constant current value to 300mA: ");
  if (MAX77650_setCHG_CC(0x3f)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");     //300mA
  if (MAX77650_debug) Serial.print("Set the fast-charge safety timer to 3h: ");
  if (MAX77650_setT_FAST_CHG(1)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");    //3h
  if (MAX77650_debug) Serial.print("Set IFAST-CHG_JEITA to 300mA: ");
  if (MAX77650_setCHG_CC_JEITA(0x3f)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed"); //300mA
  if (MAX77650_debug) Serial.print("Set fast-charge battery regulation voltage to 4.20V: ");
  if (MAX77650_setCHG_CV(0x18)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");     //4.20V
  if (MAX77650_debug) Serial.print("Set USB not in power down: ");
  if (MAX77650_setUSBS(false)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");      //USBS not suspended
  if (MAX77650_debug) Serial.print("Set the modified VFAST-CHG to 4.00V: ");
  if (MAX77650_setCHG_CV_JEITA(0x10)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed"); //4.0V
  if (MAX77650_debug) Serial.print("Selects the battery discharge current full-scale current value to 300mA: ");
  if (MAX77650_setIMON_DISCHG_SCALE(0x0A)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed"); //300mA
  if (MAX77650_debug) Serial.print("Disable the analog MUX output: ");
  if (MAX77650_setMUX_SEL(0)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");    //AMUX=off
  if (MAX77650_debug) Serial.print("Set the Charger to Enable: ");
  if (MAX77650_setCHG_EN(true)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");  //enable the Charger
  if (MAX77650_debug) Serial.print("Disable SIMO Buck-Boost Channel 0 Active-Discharge: ");
  if (MAX77650_setADE_SBB0(false)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");
  if (MAX77650_debug) Serial.print("Disable SIMO Buck-Boost Channel 1 Active-Discharge: ");
  if (MAX77650_setADE_SBB1(false)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");
  if (MAX77650_debug) Serial.print("Disable SIMO Buck-Boost Channel 2 Active-Discharge: ");
  if (MAX77650_setADE_SBB1(false)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");
  if (MAX77650_debug) Serial.print("Set SIMO Buck-Boost to maximum drive strength: ");
  if (MAX77650_setDRV_SBB(0b00)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");
  if (MAX77650_debug) Serial.print("Set SIMO Buck-Boost Channel 0 Peak Current Limit to 500mA: ");
  if (MAX77650_setIP_SBB0(0b00)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");
  if (MAX77650_debug) Serial.print("Set SIMO Buck-Boost Channel 1 Peak Current Limit to 500mA: ");
  if (MAX77650_setIP_SBB1(0b00)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");
  if (MAX77650_debug) Serial.print("Set SIMO Buck-Boost Channel 2 Peak Current Limit to 500mA: ");
  if (MAX77650_setIP_SBB2(0b00)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");
  if (MAX77650_debug) Serial.print("Set SIMO Buck-Boost Channel 2 to on while in stand-by-mode: ");
  if (MAX77650_setEN_SBB2(0b110)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");
  if (MAX77650_debug) Serial.print("Initialize Global Interrupt Mask Register: ");
  if (MAX77650_setINT_M_GLBL(0x0)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");
  if (MAX77650_debug) Serial.print("Initialize Charger Interrupt Mask Register: ");
  if (MAX77650_setINT_M_CHG(0x0)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");

  //Read and clear Interrupt Registers
  MAX77650_getINT_GLBL();
  MAX77650_getINT_CHG();
  MAX77650_getERCFLAG();

  if (MAX77650_debug) Serial.println("End Initialisation of MAX77650");
}
