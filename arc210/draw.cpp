#include <Arduino.h>
#include "Adafruit_GFX.h" // Hardware-specific library
#include <MCUFRIEND_kbv.h>

extern MCUFRIEND_kbv tft;

const int RADIO_COUNT = 5; // for some reason even if declared extern, it doesn't see the one in the main file?
extern const char *RADIO_NAMES[];
extern const int FREQ_DIGITS;
extern int activeRadio;

extern const char *radioFrequencies[];
extern const char *radioStates[];
extern const char *radioPresets[];
extern const char *radioFreqMode[];

extern char debugBuf[64];

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x5555

// the left edge of the screen is covered, so this is really the zero
#define ZERO_OFFSET_X 20
#define ZERO_OFFSET_Y 5
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define SIDE_WIDTH 90

int GetRadioStateColor(const char *radioState)
{
  if (strcmp("OFF", radioState) == 0)
  {
    return GRAY;
  }
  else if (radioState[0] == 'A' && radioState[2] == 'A')
  {
    return YELLOW;
  }
  else
  {
    return GREEN;
  }
}

#define DEBUG_LINE_X ZERO_OFFSET_X + 5
#define DEBUG_LINE_Y 190 - ZERO_OFFSET_Y
#define DEBUG_LINE_WIDTH SCREEN_WIDTH - (DEBUG_LINE_X + SIDE_WIDTH + 20) - ZERO_OFFSET_X

void DEBUG_PRINTLN(const char *msg)
{
  tft.setTextColor(GRAY);
  tft.fillRect(DEBUG_LINE_X, DEBUG_LINE_Y, DEBUG_LINE_WIDTH, 40, BLACK);
  tft.setCursor(DEBUG_LINE_X, DEBUG_LINE_Y);
  tft.setTextSize(2);
  tft.println(msg);
}

void DrawWaiting()
{
  tft.fillScreen(BLACK);
  tft.setTextSize(8);
  int x = 80;
  int y = 102;
  const char *logo = "SLK-210";
  tft.setTextColor(WHITE);
  tft.setCursor(x - 1, y - 1);
  tft.println(logo);
  tft.setCursor(x - 1, y + 1);
  tft.println(logo);
  tft.setCursor(x + 1, y - 1);
  tft.println(logo);
  tft.setCursor(x + 1, y + 1);
  tft.println(logo);
  tft.setTextColor(GREEN);
  tft.setCursor(x, y);
  tft.println(logo);

  tft.setTextColor(GRAY);
  DEBUG_PRINTLN("waiting for data");
}

void DrawUnsupported(char *aircraft)
{
  tft.fillScreen(BLACK);
  sprintf(debugBuf, "unsupported: %s", aircraft);
  DEBUG_PRINTLN(debugBuf);
}

#define RADIO_Y (ZERO_OFFSET_Y + 2)
#define SIZE_7_CH_HEIGHT 60
#define SIZE_8_CH_HEIGHT 64
#define RADIO_STATE_Y 100
#define RADIO_STATE_WIDTH 290

void RedrawRadioName()
{
  const char *radioName = RADIO_NAMES[activeRadio];
  int bg = GetRadioStateColor(radioStates[activeRadio]);

  tft.setTextSize(8);
  tft.fillRect(ZERO_OFFSET_X, RADIO_Y, RADIO_STATE_WIDTH, SIZE_8_CH_HEIGHT, BLACK);
  tft.setTextColor(bg);
  tft.setCursor(ZERO_OFFSET_X + 5, RADIO_Y + 5);
  tft.print(radioName);
}

#define SIDE_X SCREEN_WIDTH - SIDE_WIDTH - ZERO_OFFSET_X
#define SIDE_ITEM_HEIGHT 32
#define SIDE_SPACE 6

void DrawRadioSideState(int radio)
{
  int y = ZERO_OFFSET_Y + radio * (SIDE_ITEM_HEIGHT + SIDE_SPACE);
  // clear there area where this radio would be, including the padding between items
  tft.fillRect(SIDE_X, y, SIDE_WIDTH, SIDE_ITEM_HEIGHT + SIDE_SPACE, BLACK);

  const char *radioName = RADIO_NAMES[radio];
  const char *radioState = radioStates[radio];
  int color = GetRadioStateColor(radioState);

  tft.drawRect(SIDE_X, y, SIDE_WIDTH, SIDE_ITEM_HEIGHT, color);
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setCursor(SIDE_X + 5, y + 5);
  tft.print(radioName);
}

void RedrawRadioState()
{
  const char *radioState = radioStates[activeRadio];
  const char *freqMode = radioFreqMode[activeRadio];
  int bg = GetRadioStateColor(radioState);
  int textColor = BLACK;

  tft.drawRect(ZERO_OFFSET_X, RADIO_Y - 5, RADIO_STATE_WIDTH, 2 * SIZE_8_CH_HEIGHT, bg);
  tft.fillRect(ZERO_OFFSET_X, RADIO_STATE_Y, RADIO_STATE_WIDTH, SIZE_8_CH_HEIGHT + 5, bg);
  int len = strlen(radioState);
  tft.setTextSize(6);
  tft.setTextColor(textColor);
  tft.setCursor(ZERO_OFFSET_X + 10, RADIO_STATE_Y + 13);
  tft.print(radioState);

  len = strlen(freqMode);
  if (len > 0)
  {
    tft.setTextSize(3);
    tft.setCursor(ZERO_OFFSET_X + RADIO_STATE_WIDTH - (len * 20), RADIO_STATE_Y + 35);
    tft.print(freqMode);
  }
  DrawRadioSideState(activeRadio);
}

// draws rectangles, outlined and text in color depending on radio state
void RedrawSideStates()
{
  for (int i = 0; i < RADIO_COUNT; i++)
  {
    DrawRadioSideState(i);
  }
}

void RedrawRadioConnector()
{
  const char *radioState = radioStates[activeRadio];
  int bg = GetRadioStateColor(radioState);
  int leftx = ZERO_OFFSET_X + RADIO_STATE_WIDTH;
  int rightx = SIDE_X;
  int lefttopy = RADIO_Y - 5;
  int leftbottomy = RADIO_STATE_Y + SIZE_8_CH_HEIGHT + 5;

  tft.fillRect(leftx, 0, rightx - leftx, (SIDE_ITEM_HEIGHT + SIDE_SPACE) * 5, BLACK);

  int righttopy = ZERO_OFFSET_Y + activeRadio * (SIDE_ITEM_HEIGHT + SIDE_SPACE);
  int rightbottomy = righttopy + SIDE_ITEM_HEIGHT;

  tft.drawLine(leftx, lefttopy, rightx, righttopy, bg);
  tft.drawLine(leftx, leftbottomy, rightx, rightbottomy, bg);
}

#define SIZE_8_CH_WIDTH 48

#define RADIO_FREQ_Y SCREEN_HEIGHT - SIZE_8_CH_HEIGHT - 20
#define RADIO_FREQ_WIDTH SCREEN_WIDTH - ZERO_OFFSET_X
#define PRESET_WIDTH 80

void RedrawRadioFreq()
{
  // for clearing space, clear the whole rect where freq would be
  tft.fillRect(ZERO_OFFSET_X + PRESET_WIDTH, RADIO_FREQ_Y, SCREEN_WIDTH, SIZE_8_CH_HEIGHT, BLACK);

  // draw current text
  int bg = GetRadioStateColor(radioStates[activeRadio]);
  const char *radioFreq = radioFrequencies[activeRadio];
  int len = strlen(radioFreq);
  int w = len * SIZE_8_CH_WIDTH;
  int x = SCREEN_WIDTH - w - 5;

  tft.setTextSize(8);
  tft.setTextColor(bg);
  tft.setCursor(x, RADIO_FREQ_Y);
  tft.print(radioFreq);
}

#define PRESET_HEIGHT 64
#define PRESET_Y RADIO_FREQ_Y

void RedrawPreset()
{
  // clear the preset area
  tft.fillRect(ZERO_OFFSET_X, PRESET_Y, PRESET_WIDTH, PRESET_HEIGHT, BLACK);
  // if there IS a preset for this radio, draw it, but always gray
  const char *preset = radioPresets[activeRadio];
  if (strlen(preset) > 0)
  {
    tft.drawRect(ZERO_OFFSET_X, PRESET_Y, PRESET_WIDTH, PRESET_HEIGHT, GRAY);
    tft.setTextColor(GRAY);
    tft.setTextSize(2);
    tft.setCursor(ZERO_OFFSET_X + 5, PRESET_Y + 5);
    tft.print("PRESET");
    tft.setTextSize(4);
    tft.setCursor(ZERO_OFFSET_X + 25, PRESET_Y + 25);
    tft.print(preset);
  }
}

void RedrawRadio()
{
  RedrawRadioName();
  RedrawRadioState();
  RedrawRadioFreq();
  RedrawSideStates();
  RedrawRadioConnector();
  RedrawPreset();
}
