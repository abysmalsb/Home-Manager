#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050_tockn.h>
#include <MAX77650-Arduino-Library.h>
#include <MAX17055.h>

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
#define SLEEPING        2
char * safetySubMenu[] = {
  "Call for help",
  "Fall detection",
  "Sleeping"
};

#define RUNNING         0
#define JUMPING         1
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

#define CONTROL         0
#define VIEW_SENSOR     1
char * robotArmSubMenu[] = {
  "Control",
  "View sensor data"
};

#define BATTERY         0
#define ABOUT           1
char * otherSubMenu[] = {
  "Battery",
  "About"
};

int previousButtonState[] =  {LOW, LOW, LOW, LOW};

int currentLayer = MAIN_MENU;
int currentMenuPointInMainMenu = HOME;
int currentMenuPointInSubMenu = 0;

bool displayEnabled = true;

long timeOfLastClick;
int displaySleepIn = 60000; // millis

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
MAX17055 sensor;

int counter = 0;
boolean doTask = false;

void setup()   {
  // enabling power from battery
  pinMode(MAX77650_PHLD, OUTPUT);          //configure pin as output
  digitalWrite(MAX77650_PHLD, HIGH);       //set output to HIGH to hold the power-on state

  Serial.begin(115200);
  
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
  if (MAX77650_debug) Serial.print("Set Thermistor enable bit: ");
  if (MAX77650_setTHM_EN(true)) if (MAX77650_debug) Serial.println("okay"); else if (MAX77650_debug) Serial.println("failed");     //enable the thermistor monitoring
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
    }
  }

  if (currentLayer == APPLICATION) {
    refreshMenu();
  }

  if ((displayEnabled && millis() - timeOfLastClick > displaySleepIn && currentLayer != APPLICATION)
    || (displayEnabled && millis() - timeOfLastClick > displaySleepIn * 10 && currentLayer == APPLICATION)) {
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

void drawSplashScreen(void) {
  // drawing Hackster logo
  display.drawBitmap(0, 2,  hackster_logo_bmp, 32, 28, 1);
  // drawing Maxim logo
  display.drawBitmap(96, 2,  maxim_logo_bmp, 32, 28, 1);
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
      if (currentMenuPointInSubMenu == RUNNING) {

      }
      else if (currentMenuPointInSubMenu == JUMPING) {
        drawJumping();
      }
      break;
    case HOME:
      if (currentMenuPointInSubMenu == MY_DOOR_LOCK) {

      }
      else if (currentMenuPointInSubMenu == WATERING_MIMOSA) {

      }
      else if (currentMenuPointInSubMenu == KITCHEN_SINK_LIGHTS) {

      }
      break;
    case ROBOT_ARM:
      if (currentMenuPointInSubMenu == CONTROL) {

      }
      else if (currentMenuPointInSubMenu == VIEW_SENSOR) {
        drawViewSensorData();
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

void switchOffDisplay() {
  displayEnabled = false;
  display.clearDisplay();
  display.display();
}

void switchOnDisplay() {
  displayEnabled = true;
  refreshMenu();
}

void drawJumping() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Jumps");
  if (doTask) {
    display.setCursor(51, 24);
    display.println("Stop");
  }
  else {
    display.setCursor(49, 24);
    display.println("Start");
  }

  updateCounter();

  display.setTextSize(2);
  display.setCursor(64 - counterDigits() * 6, 8);
  display.println(counter);
  counter++;
}

void updateCounter() {
  mpu6050.update();
  Serial.println(abs(correctGyroAngle(mpu6050.getAccY())) + abs(correctGyroAngle(mpu6050.getAccX())) + abs(correctGyroAngle(mpu6050.getAccZ())));
}

int counterDigits() {
  int n = counter;
  int count = 0;
  while (n != 0)
  {
    // n = n/10
    n /= 10;
    ++count;
  }
  return count;
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

void drawBatteryInfo() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Battery:");
  display.setCursor(8, 8);
  display.println("Status (%):");
  display.setCursor(88, 8);
  display.println(sensor.getSOC());
  display.setCursor(8, 16);
  display.println("Hours remain:");
  display.setCursor(88, 16);
  display.println(sensor.getTimeToEmpty());
  display.setCursor(8, 24);
  display.println("Voltage:");
  display.setCursor(88, 24);
  display.println(sensor.getInstantaneousVoltage());
}

void drawAboutApp() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Creator:\nBalazs Simon\nWeb:\nhackster.io/Abysmal");
  display.drawLine(0, display.height() / 2, display.width(), display.height() / 2, WHITE);
}
