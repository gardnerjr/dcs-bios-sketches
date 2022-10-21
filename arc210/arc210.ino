#define DCSBIOS_IRQ_SERIAL
#include "DcsBios.h"

#include <Arduino.h>
#include <SPI.h>
#include <stdarg.h>
#include <RotaryEncoder.h>

#include "draw.h"

// All the mcufriend.com UNO shields have the same pinout.
// i.e. control pins A0-A4.  Data D2-D9.  microSD D10-D13.
// Touchscreens are normally A1, A2, D7, D6 but the order varies
//
// This demo should work with most Adafruit TFT libraries
// If you are not using a shield,  use a full Adafruit constructor()
// e.g. Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define LCD_CS A3    // Chip Select goes to Analog 3
#define LCD_CD A2    // Command/Data goes to Analog 2
#define LCD_WR A1    // LCD Write goes to Analog 1
#define LCD_RD A0    // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#include "Adafruit_GFX.h" // Hardware-specific library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

// top
RotaryEncoder radioChooser(23, 39, RotaryEncoder::LatchMode::FOUR3); // TWO03);

// middle, from right to left (Least significant digit to most)
RotaryEncoder digit4Chooser(44, 28, RotaryEncoder::LatchMode::FOUR3); // TWO03);
RotaryEncoder digit3Chooser(42, 26, RotaryEncoder::LatchMode::FOUR3); // TWO03); // pins of 1, 2 swapped on plug
RotaryEncoder digit2Chooser(43, 27, RotaryEncoder::LatchMode::FOUR3); // TWO03);
RotaryEncoder digit1Chooser(41, 25, RotaryEncoder::LatchMode::FOUR3); // TWO03);
RotaryEncoder digit0Chooser(40, 24, RotaryEncoder::LatchMode::FOUR3); // TWO03);

// bottom row
// rightmost
RotaryEncoder modeChooser(32, 48, RotaryEncoder::LatchMode::FOUR3); // TWO03);

// middle
RotaryEncoder middleDial(33, 49, RotaryEncoder::LatchMode::FOUR3); // TWO03);
// middle button,for now hooked up to lamp test to test it
//  DcsBios::Switch2Pos lampTestBtn("LAMP_TEST_BTN", 38);
const int buttonPin = 38;
bool dialIsVolume = false; // indicates if center dial is volume or channel

// leftmost
RotaryEncoder radioStateChooser(34, 50, RotaryEncoder::LatchMode::FOUR3); // TWO03);

char debugBuf[64];
// #define DEBUG 1

const int RADIO_UHF = 0;
const int RADIO_VHF_AM = 1;
const int RADIO_VHF_FM = 2;
const int RADIO_TACAN = 3;
const int RADIO_ILS = 4;

const int RADIO_COUNT = 5;
const char *RADIO_NAMES[] = {"UHF", "VHF-AM", "VHF-FM", "TACAN", "ILS"};

const int FREQ_DIGITS = 5;

int activeRadio = 0;

#define DEBUG 0

int supportedAircraft = 0;

void onAcftNameChange(char *newValue)
{
  sprintf(debugBuf, "aircraft change: %s", newValue);
  DEBUG_PRINTLN(debugBuf);

  // if this is a supported aircraft, let drawing work
  // otherwise, blank unless in debug mode
  if (DEBUG || !strcmp(newValue, "A-10C_2") || !strcmp(newValue, "A-10C") || !strcmp(newValue, "A-10A"))
  {
    supportedAircraft = 1;
    RedrawRadio();
  }
  else
  {
    DrawUnsupported(newValue);
  }
}
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, onAcftNameChange);

const char *radioFrequencies[RADIO_COUNT] =
#if DEBUG
    {"251.000", "124.000", "30.000", "000X ", "108.95 "};
#else
    {"", "", "", "", ""};
#endif

const char *radioFreqMode[RADIO_COUNT] =
#if DEBUG
    {"MNL", "PRESET", "GRD", "", ""};
#else
    {"", "", "", "", ""};
#endif

const char *radioStates[RADIO_COUNT] =
#if DEBUG
    {"BOTH", "TR", "TR", "A/A REC", "OFF"};
#else
    {"", "", "", "", ""};
#endif

const char *radioPresets[RADIO_COUNT] = {"1", "1", "1", "", ""};

unsigned int radioVolumes[RADIO_COUNT] = {0, 0, 0, 0, 0};

void IfActiveRedrawRadio(int radio)
{
  if (radio == activeRadio)
  {
    RedrawRadio();
  }
  else
  {
    DrawRadioSideState(radio);
  }
}

void IfActiveRedrawFreq(int radio)
{
  if (radio == activeRadio)
  {
    RedrawRadioFreq();
  }
}

//=========================================================================
// UHF RADIO Stuff

void onUhfFunctionChange(unsigned int newValue)
{
  const char *state = "?";
  switch (newValue)
  {
  case 0:
    state = "OFF";
    break;
  case 1:
    state = "MAIN";
    break;
  case 2:
    state = "BOTH";
    break;
  case 3:
    state = "ADF";
    break;
  }
  radioStates[RADIO_UHF] = state;
  IfActiveRedrawRadio(RADIO_UHF);
}

void onUhfFrequencyChange(char *newValue)
{
  radioFrequencies[RADIO_UHF] = newValue;
  IfActiveRedrawFreq(RADIO_UHF);
}

void onUhfPresetChange(char *newValue)
{
  radioPresets[RADIO_UHF] = newValue;
  RedrawPreset();
}

void onUhfVolChange(unsigned int newValue)
{
  radioVolumes[RADIO_UHF] = newValue;
  sprintf(debugBuf, "uhf vol %u", newValue);
  DEBUG_PRINTLN(debugBuf);
}

void onUhfModeChange(unsigned int newValue)
{
  const char *state = "?";
  switch (newValue)
  {
  case 0:
    state = "MNL";
    break;
  case 1:
    state = "PRESET";
    break;
  case 2:
    state = "GRD";
    break;
  }
  radioFreqMode[RADIO_UHF] = state;
  IfActiveRedrawRadio(RADIO_UHF);
}

DcsBios::IntegerBuffer uhfModeBuffer(0x117c, 0x0003, 0, onUhfModeChange);
DcsBios::IntegerBuffer uhfFunctionBuffer(0x117c, 0x000c, 2, onUhfFunctionChange);
DcsBios::StringBuffer<2> uhfPresetBuffer(0x1188, onUhfPresetChange);
DcsBios::StringBuffer<7> uhfFrequencyBuffer(0x1180, onUhfFrequencyChange);
DcsBios::IntegerBuffer uhfVolBuffer(0x117e, 0xffff, 0, onUhfVolChange);

//=========================================================================
// VHF AM RADIO Stuff
void onVhfamModeChange(unsigned int newValue)
{
  const char *state = "?";
  switch (newValue)
  {
  case 0:
    state = "OFF";
    break;
  case 1:
    state = "TR";
    break;
  case 2:
    state = "DF";
    break;
  }
  radioStates[RADIO_VHF_AM] = state;
  IfActiveRedrawRadio(RADIO_VHF_AM);
}

DcsBios::IntegerBuffer vhfamModeBuffer(0x1186, 0x0300, 8, onVhfamModeChange);
void onVhfAmFrequencySChange(char *newValue)
{
  radioFrequencies[RADIO_VHF_AM] = newValue;
  IfActiveRedrawFreq(RADIO_VHF_AM);
}
DcsBios::StringBuffer<7> vhfAmFrequencySBuffer(0x12de, onVhfAmFrequencySChange);

void onVhfamVolChange(unsigned int newValue)
{
  radioVolumes[RADIO_VHF_AM] = newValue;
  sprintf(debugBuf, "vhfam vol %u", newValue);
  DEBUG_PRINTLN(debugBuf);
}
DcsBios::IntegerBuffer vhfamVolBuffer(0x118c, 0xffff, 0, onVhfamVolChange);

void onVhfamPresetChange(char *newValue)
{
  radioPresets[RADIO_VHF_AM] = newValue;
  RedrawPreset();
}
DcsBios::StringBuffer<2> vhfamPresetBuffer(0x118a, onVhfamPresetChange);

void onVhfamFreqemerChange(unsigned int newValue)
{
  const char *state = "?";
  switch (newValue)
  {
  case 0:
    state = "FM";
    break;
  case 1:
    state = "AM";
    break;
  case 2:
    state = "MAN";
    break;
  case 3:
    state = "PRE";
    break;
  }
  radioFreqMode[RADIO_VHF_AM] = state;
  IfActiveRedrawRadio(RADIO_VHF_AM);
}

DcsBios::IntegerBuffer vhfamFreqemerBuffer(0x1186, 0x0c00, 10, onVhfamFreqemerChange);
//=========================================================================
// VHF FM RADIO Stuff
void onVhffmModeChange(unsigned int newValue)
{
  const char *state = "?";
  switch (newValue)
  {
  case 0:
    state = "OFF";
    break;
  case 1:
    state = "TR";
    break;
  case 2:
    state = "DF";
    break;
  }
  radioStates[RADIO_VHF_FM] = state;
  IfActiveRedrawRadio(RADIO_VHF_FM);
}

DcsBios::IntegerBuffer vhffmModeBuffer(0x1194, 0x0060, 5, onVhffmModeChange);

void onVhfFmFrequencySChange(char *newValue)
{
  radioFrequencies[RADIO_VHF_FM] = newValue;
  IfActiveRedrawFreq(RADIO_VHF_FM);
}
DcsBios::StringBuffer<7> vhfFmFrequencySBuffer(0x12e6, onVhfFmFrequencySChange);

void onVhffmVolChange(unsigned int newValue)
{
  radioVolumes[RADIO_VHF_FM] = newValue;
  sprintf(debugBuf, "vhf fm vol %u", newValue);
  DEBUG_PRINTLN(debugBuf);
}
DcsBios::IntegerBuffer vhffmVolBuffer(0x1198, 0xffff, 0, onVhffmVolChange);

void onVhffmPresetChange(char *newValue)
{
  radioPresets[RADIO_VHF_FM] = newValue;
  RedrawPreset();
}
DcsBios::StringBuffer<2> vhffmPresetBuffer(0x1196, onVhffmPresetChange);

void onVhffmFreqemerChange(unsigned int newValue)
{
  const char *state = "?";
  switch (newValue)
  {
  case 0:
    state = "FM";
    break;
  case 1:
    state = "AM";
    break;
  case 2:
    state = "MAN";
    break;
  case 3:
    state = "PRE";
    break;
  }
  radioFreqMode[RADIO_VHF_FM] = state;
  IfActiveRedrawRadio(RADIO_VHF_FM);
}
DcsBios::IntegerBuffer vhffmFreqemerBuffer(0x1194, 0x0180, 7, onVhffmFreqemerChange);

//=========================================================================
// TACAN Stuff
void onTacanModeChange(unsigned int newValue)
{
  const char *state = "?";
  switch (newValue)
  {
  case 0:
    state = "OFF";
    break;
  case 1:
    state = "REC";
    break;
  case 2:
    state = "T/R";
    break;
  case 3:
    state = "A/A REC";
    break;
  case 4:
    state = "A/A T/R";
    break;
  }
  radioStates[RADIO_TACAN] = state;
  IfActiveRedrawRadio(RADIO_TACAN);
}

DcsBios::IntegerBuffer tacanModeBuffer(0x1168, 0x000e, 1, onTacanModeChange);

void onTacanChannelChange(char *newValue)
{
  radioFrequencies[RADIO_TACAN] = newValue;
  IfActiveRedrawFreq(RADIO_TACAN);
}
DcsBios::StringBuffer<4> tacanChannelBuffer(0x1162, onTacanChannelChange);

void onTacanVolChange(unsigned int newValue)
{
  radioVolumes[RADIO_TACAN] = newValue;
  sprintf(debugBuf, "tacan vol %u", newValue);
  DEBUG_PRINTLN(debugBuf);
}
DcsBios::IntegerBuffer tacanVolBuffer(0x1166, 0xffff, 0, onTacanVolChange);

//=========================================================================
// ILS Stuff
void onIlsPwrChange(unsigned int newValue)
{
  const char *state = "?";
  switch (newValue)
  {
  case 0:
    state = "OFF";
    break;
  case 1:
    state = "PWR";
    break;
  }
  radioStates[RADIO_ILS] = state;
  IfActiveRedrawRadio(RADIO_ILS);
}

DcsBios::IntegerBuffer ilsPwrBuffer(0x1168, 0x0010, 4, onIlsPwrChange);

void onIlsFrequencySChange(char *newValue)
{
  radioFrequencies[RADIO_ILS] = newValue;
  IfActiveRedrawFreq(RADIO_ILS);
}
DcsBios::StringBuffer<6> ilsFrequencySBuffer(0x12d8, onIlsFrequencySChange);

void onIlsVolChange(unsigned int newValue)
{
  radioVolumes[RADIO_ILS] = newValue;
  sprintf(debugBuf, "ils vol %u", newValue);
  DEBUG_PRINTLN(debugBuf);
}

DcsBios::IntegerBuffer ilsVolBuffer(0x1174, 0xffff, 0, onIlsVolChange);

//===========================================================================
// things that call sendDcsBiosMessage
void changeActiveRadioMode(bool increase)
{
  const char *msg = "";
  switch (activeRadio)
  {
  case RADIO_UHF:
    msg = "UHF_FUNCTION";
    break;
  case RADIO_VHF_AM:
    msg = "VHFAM_MODE";
    break;
  case RADIO_VHF_FM:
    msg = "VHFFM_MODE";
    break;
  case RADIO_TACAN:
    msg = "TACAN_MODE";
    break;
  case RADIO_ILS:
    msg = "ILS_PWR";
    break;
  }
  sendDcsBiosMessage(msg, increase ? "INC" : "DEC");
}

void changeActiveRadioFrequencyMode(bool increase)
{
  const char *msg = "";
  switch (activeRadio)
  {
  case RADIO_UHF:
    msg = "UHF_MODE";
    break;
  case RADIO_VHF_AM:
    msg = "VHFAM_FREQEMER";
    break;
  case RADIO_VHF_FM:
    msg = "VHFFM_FREQEMER";
    break;
  }
  if (strlen(msg) > 0)
  {
    sendDcsBiosMessage(msg, increase ? "INC" : "DEC");
  }
}

void volumeChange(bool increase)
{
  const char *msg = "";

  switch (activeRadio)
  {
  case RADIO_UHF:
    msg = "UHF_VOL";
    break;
  case RADIO_VHF_AM:
    msg = "VHFAM_VOL";
    break;
  case RADIO_VHF_FM:
    msg = "VHFFM_VOL";
    break;
  case RADIO_TACAN:
    msg = "TACAN_VOL";
    break;
  case RADIO_ILS:
    msg = "ILS_VOL";
    break;
  }
  sendDcsBiosMessage(msg, increase ? "+3200" : "-3200");
}

void changeActiveRadioPreset(bool increase)
{
  const char *msg = "";
  switch (activeRadio)
  {
  case RADIO_UHF:
    msg = "UHF_PRESET_SEL";
    break;
  case RADIO_VHF_AM:
    msg = "VHFAM_PRESET";
    break;
  case RADIO_VHF_FM:
    msg = "VHFFM_PRESET";
    break;
  }
  if (strlen(msg) > 0)
  {
    sendDcsBiosMessage(msg, increase ? "INC" : "DEC");
  }
}

const char *FREQ_SELECTORS[RADIO_COUNT][FREQ_DIGITS] = {
    // UHF frequency selectors
  { "UHF_POINT25_SEL", "UHF_POINT1MHZ_SEL", "UHF_1MHZ_SEL", "UHF_10MHZ_SEL", "UHF_100MHZ_SEL" }, 
    // VHF AM
  { "VHFAM_FREQ4", "VHFAM_FREQ3", "VHFAM_FREQ2", "VHFAM_FREQ1", "", }, 
    // VHF FM
  { "VHFFM_FREQ4", "VHFFM_FREQ3", "VHFFM_FREQ2", "VHFFM_FREQ1", "", }, 
    // TACAN
  { "TACAN_XY", "TACAN_1", "TACAN_10", "", "", },
    // ILS
  { "ILS_KHZ", "ILS_MHZ", "", "", "", },
};

void frequencyChange(int digit, bool increase)
{
  // digit is 0-5, 0=LSB, 5 = MSB
  // not all radios use all 5
  const char *msg = FREQ_SELECTORS[activeRadio][digit];
  if (strlen(msg) > 0)
  {
    sendDcsBiosMessage(msg, increase ? "INC" : "DEC");
  }
}

void checkRadioChange()
{
  static int lastRadioPos = 0;
  radioChooser.tick();
  int newRadioPos = radioChooser.getPosition();

  if (newRadioPos != lastRadioPos)
  {
    lastRadioPos = newRadioPos;
    activeRadio = abs(lastRadioPos) % RADIO_COUNT;
    // DBG_STAT("new radio rotation now %d, radio is %d (%s)\n", newRadioPos, activeRadio, RADIO_NAMES[activeRadio]);
    DEBUG_PRINTLN("radio change");
    RedrawRadio();
  }
}

void checkRadioStateChange()
{
  static int lastRadioState = 0;
  radioStateChooser.tick();
  int newRadioState = radioStateChooser.getPosition();

  if (newRadioState != lastRadioState)
  {
    changeActiveRadioMode(newRadioState > lastRadioState);
    lastRadioState = newRadioState;
    DEBUG_PRINTLN("radio state change");
    RedrawRadio();
  }
}

void checkMiddleDialChange()
{
  static int lastValue = 0;

  middleDial.tick();
  int newValue = middleDial.getPosition();

  if (newValue != lastValue)
  {
    if (dialIsVolume)
    {
      volumeChange(newValue > lastValue);
      DEBUG_PRINTLN("volume change");
    }
    else
    {
      changeActiveRadioPreset(newValue > lastValue);
      DEBUG_PRINTLN("preset change");
    }
    lastValue = newValue;
  }
}

void checkModeStateChange()
{
  static int lastValue = 0;
  modeChooser.tick();
  int newValue = modeChooser.getPosition();

  if (newValue != lastValue)
  {
    changeActiveRadioFrequencyMode(newValue > lastValue);
    lastValue = newValue;
    DEBUG_PRINTLN("radio frequency mode change");
    RedrawRadio();
  }
}

int lastDigitValue[FREQ_DIGITS] = {0, 0, 0, 0, 0};

void checkDigitChange(int index, RotaryEncoder *encoder)
{
  encoder->tick();
  int lastDigit = lastDigitValue[index];
  int newDigit = encoder->getPosition();

  if (newDigit != lastDigit)
  {
    frequencyChange(index, newDigit > lastDigit);
    lastDigitValue[index] = newDigit;
    sprintf(debugBuf, "digit %d changed %s", index, newDigit > lastDigit ? "+" : "-");
    DEBUG_PRINTLN(debugBuf);
    // technically, this doesn't need to redraw the freq, since the SIM will change
    // the frequency if frequencyChange did anything in the sim itself (rotating
    // the knob might have had no actual effect in the sim if the knob was already maxed)
    // RedrawRadioFreq();
  }
}

// button toggle state
// Variables will change:
int buttonState;            // the current reading from the input pin
int lastButtonState = HIGH; // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50;   // the debounce time; increase if the output flickers

void checkDialToggle()
{
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState)
  {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState)
    {
      buttonState = reading;
      if (buttonState == LOW)
      {
        dialIsVolume = !dialIsVolume;
        sprintf(debugBuf, "toggle mode to %s", dialIsVolume ? "volume" : "presets");
        DEBUG_PRINTLN(debugBuf);
      }
    }
  }
  lastButtonState = reading;
}

//===========================================================================
void setup()
{
  DcsBios::setup();

  // setup button
  pinMode(buttonPin, INPUT_PULLUP);

  uint16_t ID = tft.readID(); //
  tft.begin(ID);
  tft.setRotation(1);
  DrawWaiting();
}

void loop()
{
  DcsBios::loop();

  // check to see if the button is pushed
  checkDialToggle();

  // see if radio rotary has been changed
  checkRadioChange();

  // see if radio state has been changed
  checkRadioStateChange();

  // check if the frequency mode changed
  checkModeStateChange();

  // see if middle dial has changed
  checkMiddleDialChange();

  // see if any frequencies changed
  checkDigitChange(0, &digit0Chooser);
  checkDigitChange(1, &digit1Chooser);
  checkDigitChange(2, &digit2Chooser);
  checkDigitChange(3, &digit3Chooser);
  checkDigitChange(4, &digit4Chooser);
}
