#include <Wire.h>
#include <MIDI.h>
#include "Parameters.h"
#include <CircularBuffer.h>
#include "HWControls.h"
#include "EepromMgr.h"
#include "Settings.h"
#include <ShiftRegister74HC595.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <MCP4922.h>
#include <SPI.h>
MCP4922 DAC(2,1,0,9);    // (MOSI,SCK,CS,LDAC) define Connections for MEGA_board, 


#define SETTINGS 0       //Settings page
#define SETTINGSVALUE 1  //Settings page
#define PARAMETER 0

#define MIDI1 0
#define MIDI2 1
#define MIDI4 2
#define MIDI8 3
#define FILTERLINLOG 4
#define AMPLINLOG 5
#define FLOOPBIT1 6
#define FLOOPBIT0 7

#define NOTEPRIORITYA0 8
#define NOTEPRIORITYA2 9
#define ALOOPBIT1 10
#define ALOOPBIT0 11
#define EXTCLOCK 12
#define MIDICLOCK 13
#define AFTERTOUCH_A 14
#define AFTERTOUCH_B 15

#define AFTERTOUCH_C 16

const char* VERSION = "V1.1";

unsigned int state = SETTINGS;

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin  
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long buttonDebounce = 0;

char * currentSettingsOption = "";
char * currentSettingsValue = "";
int currentSettingsPart = SETTINGS;
byte oldfilterLogLin, oldampLogLin, oldmidiChannel, oldAfterTouchDest, oldNotePriority, oldFilterLoop, oldAmpLoop, oldClockSource;

unsigned long timer = 0;
ShiftRegister74HC595<3> sr(12, 13, 14);

void show() { // Often used sequence - Function to simplify code
  display.display(); delay(2000); display.fillScreen(SSD1306_BLACK);
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // OLED I2C Address, may need to change for different device,
  setUpSettings();

  setupHardware();
  renderBootUpPage();
  //DAC.Set(2048,2048);
  
  //Read MIDI Channel from EEPROM

  midiChannel = getMIDIChannel();
  if (midiChannel < 0 || midiChannel > 16) {
    storeMidiChannel(0);
  }
  oldmidiChannel = midiChannel;
  switch (midiChannel) {
    case 0:
      sr.set(MIDI1, LOW);
      sr.set(MIDI2, LOW);
      sr.set(MIDI4, LOW);
      sr.set(MIDI8, LOW);
      break;

    case 1:
      sr.set(MIDI1, HIGH);
      sr.set(MIDI2, LOW);
      sr.set(MIDI4, LOW);
      sr.set(MIDI8, LOW);
      break;

    case 2:
      sr.set(MIDI1, LOW);
      sr.set(MIDI2, HIGH);
      sr.set(MIDI4, LOW);
      sr.set(MIDI8, LOW);
      break;

    case 3:
      sr.set(MIDI1, HIGH);
      sr.set(MIDI2, HIGH);
      sr.set(MIDI4, LOW);
      sr.set(MIDI8, LOW);
      break;

    case 4:
      sr.set(MIDI1, LOW);
      sr.set(MIDI2, LOW);
      sr.set(MIDI4, HIGH);
      sr.set(MIDI8, LOW);
      break;

    case 5:
      sr.set(MIDI1, HIGH);
      sr.set(MIDI2, LOW);
      sr.set(MIDI4, HIGH);
      sr.set(MIDI8, LOW);
      break;

    case 6:
      sr.set(MIDI1, LOW);
      sr.set(MIDI2, HIGH);
      sr.set(MIDI4, HIGH);
      sr.set(MIDI8, LOW);
      break;

    case 7:
      sr.set(MIDI1, HIGH);
      sr.set(MIDI2, HIGH);
      sr.set(MIDI4, HIGH);
      sr.set(MIDI8, LOW);
      break;

    case 8:
      sr.set(MIDI1, LOW);
      sr.set(MIDI2, LOW);
      sr.set(MIDI4, LOW);
      sr.set(MIDI8, HIGH);
      break;

    case 9:
      sr.set(MIDI1, HIGH);
      sr.set(MIDI2, LOW);
      sr.set(MIDI4, LOW);
      sr.set(MIDI8, HIGH);
      break;

    case 10:
      sr.set(MIDI1, LOW);
      sr.set(MIDI2, HIGH);
      sr.set(MIDI4, LOW);
      sr.set(MIDI8, HIGH);
      break;

    case 11:
      sr.set(MIDI1, HIGH);
      sr.set(MIDI2, HIGH);
      sr.set(MIDI4, LOW);
      sr.set(MIDI8, HIGH);
      break;

    case 12:
      sr.set(MIDI1, LOW);
      sr.set(MIDI2, LOW);
      sr.set(MIDI4, HIGH);
      sr.set(MIDI8, HIGH);
      break;

    case 13:
      sr.set(MIDI1, HIGH);
      sr.set(MIDI2, LOW);
      sr.set(MIDI4, HIGH);
      sr.set(MIDI8, HIGH);
      break;

    case 14:
      sr.set(MIDI1, LOW);
      sr.set(MIDI2, HIGH);
      sr.set(MIDI4, HIGH);
      sr.set(MIDI8, HIGH);
      break;

    case 15:
      sr.set(MIDI1, HIGH);
      sr.set(MIDI2, HIGH);
      sr.set(MIDI4, HIGH);
      sr.set(MIDI8, HIGH);
      break;

  }

  // Read aftertouch destination from EEPROM

  AfterTouchDest = getAfterTouch();
  if (AfterTouchDest < 0 || AfterTouchDest > 3) {
    storeAfterTouch(0);
  }
  oldAfterTouchDest = AfterTouchDest;
  switch (AfterTouchDest) {
    case 0:
      sr.set(AFTERTOUCH_A, LOW);
      sr.set(AFTERTOUCH_B, LOW);
      sr.set(AFTERTOUCH_C, LOW);
      break;

    case 1:
      sr.set(AFTERTOUCH_A, HIGH);
      sr.set(AFTERTOUCH_B, LOW);
      sr.set(AFTERTOUCH_C, LOW);
      break;

    case 2:
      sr.set(AFTERTOUCH_A, LOW);
      sr.set(AFTERTOUCH_B, HIGH);
      sr.set(AFTERTOUCH_C, LOW);
      break;

    case 3:
      sr.set(AFTERTOUCH_A, HIGH);
      sr.set(AFTERTOUCH_B, HIGH);
      sr.set(AFTERTOUCH_C, LOW);
      break;
  }

  // Read NotePriority from EEPROM

  NotePriority = getNotePriority();
  if (NotePriority < 0 || NotePriority > 2) {
    storeNotePriority(1);
  }
  oldNotePriority = NotePriority;
  switch (NotePriority) {
    case 0:
      sr.set(NOTEPRIORITYA0, HIGH);
      sr.set(NOTEPRIORITYA2, LOW);
      break;

    case 1:
      sr.set(NOTEPRIORITYA0, LOW);
      sr.set(NOTEPRIORITYA2, LOW);
      break;

    case 2:
      sr.set(NOTEPRIORITYA0, LOW);
      sr.set(NOTEPRIORITYA2, HIGH);
      break;
  }

  // Read FilterLoop from EEPROM

  FilterLoop = getFilterLoop();
  if (FilterLoop < 0 || FilterLoop > 2) {
    storeFilterLoop(0);
  }
  oldFilterLoop = FilterLoop;
  switch (FilterLoop) {
    case 0:
      sr.set(FLOOPBIT0, LOW);
      sr.set(FLOOPBIT1, LOW);
      break;

    case 1:
      sr.set(FLOOPBIT0, HIGH);
      sr.set(FLOOPBIT1, LOW);
      break;

    case 2:
      sr.set(FLOOPBIT0, HIGH);
      sr.set(FLOOPBIT1, HIGH);
      break;
  }

  // Read FilterLoop from EEPROM

  AmpLoop = getAmpLoop();
  if (AmpLoop < 0 || AmpLoop > 2) {
    storeAmpLoop(0);
  }
  oldAmpLoop = AmpLoop;
  switch (AmpLoop) {
    case 0:
      sr.set(ALOOPBIT0, LOW);
      sr.set(ALOOPBIT1, LOW);
      break;

    case 1:
      sr.set(ALOOPBIT0, HIGH);
      sr.set(ALOOPBIT1, LOW);
      break;

    case 2:
      sr.set(ALOOPBIT0, HIGH);
      sr.set(ALOOPBIT1, HIGH);
      break;
  }

  // Read ClockSource from EEPROM

  ClockSource = getClockSource();
  if (ClockSource < 0 || ClockSource > 2) {
    storeClockSource(0);
  }
  oldClockSource = ClockSource;
  switch (ClockSource) {
    case 0:
      sr.set(EXTCLOCK, LOW);
      sr.set(MIDICLOCK, LOW);
      break;

    case 1:
      sr.set(EXTCLOCK, HIGH);
      sr.set(MIDICLOCK, LOW);
      break;

    case 2:
      sr.set(EXTCLOCK, LOW);
      sr.set(MIDICLOCK, HIGH);
      break;
  }

  //
  //  //Read Pitch Bend Range from EEPROM
  //  //pitchBendRange = getPitchBendRange();
  //
  //  //Read Mod Wheel Depth from EEPROM
  modWheelDepth = getModWheelDepth();
  if (modWheelDepth < 1 || modWheelDepth > 10) {
    storeModWheelDepth(10);
  }

  //Read Encoder Direction from EEPROM
  //encCW = getEncoderDir();

  filterLogLin = getFilterEnv();
  if (filterLogLin < 0 || filterLogLin > 1) {
    storeFilterEnv(0);
  }
  oldfilterLogLin = filterLogLin;

  if (filterLogLin == 1) {
    sr.set(FILTERLINLOG, HIGH);
  }
  else {
    sr.set(FILTERLINLOG, LOW);
  }

  ampLogLin = getAmpEnv();
  if (ampLogLin < 0 || ampLogLin > 1) {
    storeAmpEnv(0);
  }
  oldampLogLin = ampLogLin;

  if (ampLogLin == 1) {
    sr.set(AMPLINLOG, HIGH);
  }
  else {
    sr.set(AMPLINLOG, LOW);
  }

  showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
}

void checkSwitches() {

  //Encoder switch
  recallButton.update();
  if (recallButton.risingEdge()) {
    switch (state) {
      case SETTINGS:
        //Choose this option and allow value choice
        settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
        showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGSVALUE);
        state = SETTINGSVALUE;
        break;
      case SETTINGSVALUE:
        //Store current settings item and go back to options
        settingsHandler(settingsOptions.first().value[settingsValueIndex], settingsOptions.first().handler);
        showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
        state = SETTINGS;
        break;
    }
  }
}

void checkEncoder() {

  EncAButton.update();
  if (EncAButton.risingEdge()) {
    switch (state) {
      case SETTINGS:
        settingsOptions.push(settingsOptions.shift());
        settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
        showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
        break;
      case SETTINGSVALUE:
        if (settingsOptions.first().value[settingsValueIndex + 1] != '\0')
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[++settingsValueIndex], SETTINGSVALUE);
        break;
    }
  }

  EncBButton.update();
  if (EncBButton.risingEdge()) {
    switch (state) {

      case SETTINGS:
        settingsOptions.unshift(settingsOptions.pop());
        settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
        showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
        break;
      case SETTINGSVALUE:
        if (settingsValueIndex > 0)
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[--settingsValueIndex], SETTINGSVALUE);
        break;
    }
  }
}


void loop() {
  checkSwitches();
  checkEncoder();
  checkForChanges();
}

void checkForChanges() {

  if (filterLogLin != oldfilterLogLin) {
    if (filterLogLin == 1) {
      sr.set(FILTERLINLOG, HIGH);
    }
    else {
      sr.set(FILTERLINLOG, LOW);
    }
    oldfilterLogLin = filterLogLin;
  }

  if (ampLogLin != oldampLogLin) {
    if (ampLogLin == 1) {
      sr.set(AMPLINLOG, HIGH);
    }
    else {
      sr.set(AMPLINLOG, LOW);
    }
    oldampLogLin = ampLogLin;
  }

  if (midiChannel != oldmidiChannel ) {
    switch (midiChannel) {
      case 0:
        sr.set(MIDI1, LOW);
        sr.set(MIDI2, LOW);
        sr.set(MIDI4, LOW);
        sr.set(MIDI8, LOW);
        break;

      case 1:
        sr.set(MIDI1, HIGH);
        sr.set(MIDI2, LOW);
        sr.set(MIDI4, LOW);
        sr.set(MIDI8, LOW);
        break;

      case 2:
        sr.set(MIDI1, LOW);
        sr.set(MIDI2, HIGH);
        sr.set(MIDI4, LOW);
        sr.set(MIDI8, LOW);
        break;

      case 3:
        sr.set(MIDI1, HIGH);
        sr.set(MIDI2, HIGH);
        sr.set(MIDI4, LOW);
        sr.set(MIDI8, LOW);
        break;

      case 4:
        sr.set(MIDI1, LOW);
        sr.set(MIDI2, LOW);
        sr.set(MIDI4, HIGH);
        sr.set(MIDI8, LOW);
        break;

      case 5:
        sr.set(MIDI1, HIGH);
        sr.set(MIDI2, LOW);
        sr.set(MIDI4, HIGH);
        sr.set(MIDI8, LOW);
        break;

      case 6:
        sr.set(MIDI1, LOW);
        sr.set(MIDI2, HIGH);
        sr.set(MIDI4, HIGH);
        sr.set(MIDI8, LOW);
        break;

      case 7:
        sr.set(MIDI1, HIGH);
        sr.set(MIDI2, HIGH);
        sr.set(MIDI4, HIGH);
        sr.set(MIDI8, LOW);
        break;

      case 8:
        sr.set(MIDI1, LOW);
        sr.set(MIDI2, LOW);
        sr.set(MIDI4, LOW);
        sr.set(MIDI8, HIGH);
        break;

      case 9:
        sr.set(MIDI1, HIGH);
        sr.set(MIDI2, LOW);
        sr.set(MIDI4, LOW);
        sr.set(MIDI8, HIGH);
        break;

      case 10:
        sr.set(MIDI1, LOW);
        sr.set(MIDI2, HIGH);
        sr.set(MIDI4, LOW);
        sr.set(MIDI8, HIGH);
        break;

      case 11:
        sr.set(MIDI1, HIGH);
        sr.set(MIDI2, HIGH);
        sr.set(MIDI4, LOW);
        sr.set(MIDI8, HIGH);
        break;

      case 12:
        sr.set(MIDI1, LOW);
        sr.set(MIDI2, LOW);
        sr.set(MIDI4, HIGH);
        sr.set(MIDI8, HIGH);
        break;

      case 13:
        sr.set(MIDI1, HIGH);
        sr.set(MIDI2, LOW);
        sr.set(MIDI4, HIGH);
        sr.set(MIDI8, HIGH);
        break;

      case 14:
        sr.set(MIDI1, LOW);
        sr.set(MIDI2, HIGH);
        sr.set(MIDI4, HIGH);
        sr.set(MIDI8, HIGH);
        break;

      case 15:
        sr.set(MIDI1, HIGH);
        sr.set(MIDI2, HIGH);
        sr.set(MIDI4, HIGH);
        sr.set(MIDI8, HIGH);
        break;
    }
    oldmidiChannel = midiChannel;
  }

  if (AfterTouchDest != oldAfterTouchDest) {
    switch (AfterTouchDest) {
      case 0:
        sr.set(AFTERTOUCH_A, LOW);
        sr.set(AFTERTOUCH_B, LOW);
        sr.set(AFTERTOUCH_C, LOW);
        break;

      case 1:
        sr.set(AFTERTOUCH_A, HIGH);
        sr.set(AFTERTOUCH_B, LOW);
        sr.set(AFTERTOUCH_C, LOW);
        break;

      case 2:
        sr.set(AFTERTOUCH_A, LOW);
        sr.set(AFTERTOUCH_B, HIGH);
        sr.set(AFTERTOUCH_C, LOW);
        break;

      case 3:
        sr.set(AFTERTOUCH_A, HIGH);
        sr.set(AFTERTOUCH_B, HIGH);
        sr.set(AFTERTOUCH_C, LOW);
        break;
    }
    oldAfterTouchDest = AfterTouchDest;
  }

  if (NotePriority != oldNotePriority) {
    switch (NotePriority) {
      case 0:
        sr.set(NOTEPRIORITYA0, HIGH);
        sr.set(NOTEPRIORITYA2, LOW);
        break;

      case 1:
        sr.set(NOTEPRIORITYA0, LOW);
        sr.set(NOTEPRIORITYA2, LOW);
        break;

      case 2:
        sr.set(NOTEPRIORITYA0, LOW);
        sr.set(NOTEPRIORITYA2, HIGH);
        break;
    }
    oldNotePriority = NotePriority;
  }

  if (FilterLoop != oldFilterLoop) {
    switch (FilterLoop) {
      case 0:
        sr.set(FLOOPBIT0, LOW);
        sr.set(FLOOPBIT1, LOW);
        break;

      case 1:
        sr.set(FLOOPBIT0, HIGH);
        sr.set(FLOOPBIT1, LOW);
        break;

      case 2:
        sr.set(FLOOPBIT0, HIGH);
        sr.set(FLOOPBIT1, HIGH);
        break;
    }
    oldFilterLoop = FilterLoop;
  }

  // Read FilterLoop from EEPROM

  if (AmpLoop != oldAmpLoop) {
    switch (AmpLoop) {
      case 0:
        sr.set(ALOOPBIT0, LOW);
        sr.set(ALOOPBIT1, LOW);
        break;

      case 1:
        sr.set(ALOOPBIT0, HIGH);
        sr.set(ALOOPBIT1, LOW);
        break;

      case 2:
        sr.set(ALOOPBIT0, HIGH);
        sr.set(ALOOPBIT1, HIGH);
        break;
    }
    oldAmpLoop = AmpLoop;
  }

  if (ClockSource != oldClockSource) {
    switch (ClockSource) {
      case 0:
        sr.set(EXTCLOCK, LOW);
        sr.set(MIDICLOCK, LOW);
        break;

      case 1:
        sr.set(EXTCLOCK, HIGH);
        sr.set(MIDICLOCK, LOW);
        break;

      case 2:
        sr.set(EXTCLOCK, LOW);
        sr.set(MIDICLOCK, HIGH);
        break;
    }
    oldClockSource = ClockSource;
  }
}

void showSettingsPage(char *  option, char * value, int settingsPart)
{
  currentSettingsOption = option;
  currentSettingsValue = value;
  currentSettingsPart = settingsPart;
  renderSettingsPage();
}

void renderSettingsPage()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.print(currentSettingsOption);
  display.display();
  if (currentSettingsPart == SETTINGS) renderUpDown(100, 10, WHITE);
  display.drawFastHLine(10, 31, display.width() - 20, WHITE);
  display.setTextColor(WHITE);
  display.setCursor(10, 45);
  display.println(currentSettingsValue);
  display.display();
  if (currentSettingsPart == SETTINGSVALUE) renderUpDown(100, 45, WHITE);
}

void renderUpDown(uint16_t  x, uint16_t  y, uint16_t  colour)
{
  //Produces up/down indicator glyph at x,y
  display.setCursor(x, y);
  display.fillTriangle(x, y, x + 8, y - 8, x + 16, y, colour);
  display.fillTriangle(x, y + 4, x + 8, y + 12, x + 16, y + 4, colour);
  display.display();
}

void renderBootUpPage()
{
  display.clearDisplay();
  display.setTextColor(WHITE, BLACK);
  display.drawRect(5, 30, 46, 11, WHITE);
  display.fillRect(51, 30, 76, 11, WHITE);
  display.setCursor(10, 32);
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.print("HYBRID");
  display.setCursor(55, 32);
  display.setTextColor(BLACK, WHITE);
  display.print("SYNTHESIZER");
  display.setTextColor(WHITE, BLACK);
  display.drawRect(5, 45, 68, 11, WHITE);
  display.fillRect(73, 45, 54, 11, WHITE);
  display.setCursor(10, 47);
  display.setTextSize(1);
  display.print("Seeed Rack");
  display.setCursor(84, 47);
  display.setTextColor(BLACK, WHITE);
  display.print(VERSION);
  show();
}
