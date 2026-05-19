/*
 * CYD Two PulseSensor Playground A/B
 *
 * ESP32-2432S028 CYD experiment for comparing how two PulseSensor hardware
 * candidates work with the existing PulseSensorPlayground BPM, IBI, and
 * beat-event detector.
 *
 * Current hardware-tested mapping:
 *   PulseSensor A purple signal -> GPIO35
 *   PulseSensor B purple signal -> GPIO27
 *   Sensor power                -> 3.3V
 *   Sensor ground               -> GND
 */

#include <TFT_eSPI.h>
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

// ===== CYD PINS =====

#define PULSE_A_PIN 35
#define PULSE_B_PIN 27
#define BACKLIGHT_PIN 21

// ===== PULSESENSOR SETTINGS =====

#define PULSE_THRESHOLD 550
#define NO_BEAT_TIMEOUT 3000
#define MIN_QUALIFIED_BPM 40
#define MAX_QUALIFIED_BPM 180
#define MIN_QUALIFIED_IBI 333
#define MAX_QUALIFIED_IBI 1500
#define MIN_QUALIFIED_AMPLITUDE 20
#define MIN_LIVE_RANGE 80
#define LOCK_QUALIFIED_BEATS 4

// ===== APP VERSION =====

#define APP_TITLE "Two PulseSensor"
#define APP_VERSION "v0.3"
#define APP_DATE "2026-05-19"

// ===== SCREEN =====

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define HEADER_H 34
#define WAVE_X 8
#define WAVE_W 304
#define WAVE_A_Y 45
#define WAVE_B_Y 94
#define WAVE_H 34
#define VERDICT_Y 132
#define PANEL_Y 160
#define PANEL_H 72
#define PANEL_W 98
#define GRAPH_INTERVAL_MS 25
#define TEXT_INTERVAL_MS 250
#define SERIAL_INTERVAL_MS 40
#define SAMPLE_MAX_VALUE 1023

// ===== COLORS (RGB565) =====

#define COLOR_BG 0x0000
#define COLOR_PANEL 0x0841
#define COLOR_PANEL_DARK 0x0400
#define COLOR_GRID 0x18E3
#define COLOR_GRID_SOFT 0x10A2
#define COLOR_TEXT 0xFFFF
#define COLOR_DIM 0x8C71
#define COLOR_A 0x8EFF
#define COLOR_B 0xFFD4
#define COLOR_OVERLAP 0x9FF3
#define COLOR_GOOD 0x07E0
#define COLOR_WARN 0xFBE0
#define COLOR_BAD 0xF800

struct SensorChannel {
  const char* label;
  const char* pinLabel;
  uint8_t pin;
  uint8_t index;
  uint16_t color;
  int sample;
  int minValue;
  int maxValue;
  int range;
  int bpm;
  int ibi;
  int amplitude;
  int previousIbi;
  int quality;
  int qualifiedBeatStreak;
  int beatCount;
  int lastY;
  unsigned long lastBeatMs;
  unsigned long lastQualifiedBeatMs;
  unsigned long beatFlashUntilMs;
  unsigned long lastRangeDecayMs;
  bool beatNow;
  bool insideBeat;
  bool locked;
};

TFT_eSPI tft = TFT_eSPI();
PulseSensorPlayground pulseSensor(2);

SensorChannel channelA = {"A", "GPIO35", PULSE_A_PIN, 0, COLOR_A, 512, 512, 512, 0, 0, 0, 0, 0, 0, 0, 0, WAVE_A_Y + WAVE_H / 2, 0, 0, 0, 0, false, false, false};
SensorChannel channelB = {"B", "GPIO27", PULSE_B_PIN, 1, COLOR_B, 512, 512, 512, 0, 0, 0, 0, 0, 0, 0, 0, WAVE_B_Y + WAVE_H / 2, 0, 0, 0, 0, false, false, false};

int graphX = 0;
unsigned long lastGraphDraw = 0;
unsigned long lastTextDraw = 0;
unsigned long lastSerialTelemetry = 0;
bool pulseSensorReady = false;

void setup();
void loop();
void setupPulseSensor();
void updateSensor(SensorChannel& channel);
void updateLiveRange(SensorChannel& channel);
bool isQualifiedBeat(const SensorChannel& channel, int bpm, int ibi);
void updatePickupScore(SensorChannel& channel, bool qualified, int bpm, int ibi);
void decayPickupScore(SensorChannel& channel);
int sampleToY(const SensorChannel& channel, int waveY);
void drawStaticScreen();
void drawWaveFrame(int y, const SensorChannel& channel);
void drawWaveColumnBackground(int localX, int y);
void drawWaveforms();
void drawPanelFrames();
void drawMetrics();
void drawSensorPanel(int x, SensorChannel& channel);
void drawVerdictPanel();
void drawQualityBar(int x, int y, int w, int value);
void drawBeatGlyph(int x, int y, const SensorChannel& channel);
void drawCenteredText(const char* text, int x, int y, int w, int textSize, uint16_t color, uint16_t bg);
void printSerialTelemetry();
const char* winnerLabel();
int bpmDelta();

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("CYD Two PulseSensor Playground A/B");

  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  setupPulseSensor();
  drawStaticScreen();
}

void loop() {
  updateSensor(channelA);
  updateSensor(channelB);

  if (millis() - lastGraphDraw >= GRAPH_INTERVAL_MS) {
    lastGraphDraw = millis();
    drawWaveforms();
  }

  if (millis() - lastTextDraw >= TEXT_INTERVAL_MS) {
    lastTextDraw = millis();
    drawMetrics();
  }

  if (millis() - lastSerialTelemetry >= SERIAL_INTERVAL_MS) {
    lastSerialTelemetry = millis();
    printSerialTelemetry();
  }
}

void setupPulseSensor() {
  // PulseSensorPlayground's detector and ESP32 examples expect 10-bit samples.
  analogReadResolution(10);
  analogSetAttenuation(ADC_11db);
  pinMode(PULSE_A_PIN, INPUT);
  pinMode(PULSE_B_PIN, INPUT);

  pulseSensor.analogInput(PULSE_A_PIN, channelA.index);
  pulseSensor.analogInput(PULSE_B_PIN, channelB.index);
  pulseSensor.setThreshold(PULSE_THRESHOLD, channelA.index);
  pulseSensor.setThreshold(PULSE_THRESHOLD, channelB.index);

  pulseSensorReady = pulseSensor.begin();
  if (!pulseSensorReady) {
    Serial.println("PulseSensorPlayground initialization failed");
  }
}

void updateSensor(SensorChannel& channel) {
  channel.beatNow = false;
  channel.sample = pulseSensor.getLatestSample(channel.index);
  channel.amplitude = pulseSensor.getPulseAmplitude(channel.index);
  channel.insideBeat = pulseSensor.isInsideBeat(channel.index);
  updateLiveRange(channel);

  if (pulseSensor.sawStartOfBeat(channel.index)) {
    int bpm = pulseSensor.getBeatsPerMinute(channel.index);
    int ibi = pulseSensor.getInterBeatIntervalMs(channel.index);
    bool qualified = isQualifiedBeat(channel, bpm, ibi);

    channel.beatNow = true;
    channel.lastBeatMs = millis();
    channel.beatFlashUntilMs = millis() + 180;
    channel.beatCount++;

    if (qualified) {
      channel.bpm = bpm;
      channel.ibi = ibi;
      channel.lastQualifiedBeatMs = millis();
      if (channel.qualifiedBeatStreak < LOCK_QUALIFIED_BEATS) channel.qualifiedBeatStreak++;
      channel.locked = channel.qualifiedBeatStreak >= LOCK_QUALIFIED_BEATS;
    } else {
      channel.qualifiedBeatStreak = 0;
      channel.locked = false;
    }

    updatePickupScore(channel, qualified, bpm, ibi);
  }

  decayPickupScore(channel);
}

void updateLiveRange(SensorChannel& channel) {
  if (millis() - channel.lastRangeDecayMs >= 100) {
    channel.lastRangeDecayMs = millis();
    channel.minValue = min(channel.minValue + 4, channel.sample);
    channel.maxValue = max(channel.maxValue - 4, channel.sample);
  }

  if (channel.sample < channel.minValue) channel.minValue = channel.sample;
  if (channel.sample > channel.maxValue) channel.maxValue = channel.sample;

  if (channel.maxValue - channel.minValue < 70) {
    channel.minValue = constrain(channel.sample - 35, 0, SAMPLE_MAX_VALUE);
    channel.maxValue = constrain(channel.sample + 35, 0, SAMPLE_MAX_VALUE);
  }

  channel.range = channel.maxValue - channel.minValue;
}

bool isQualifiedBeat(const SensorChannel& channel, int bpm, int ibi) {
  if (bpm < MIN_QUALIFIED_BPM || bpm > MAX_QUALIFIED_BPM) return false;
  if (ibi < MIN_QUALIFIED_IBI || ibi > MAX_QUALIFIED_IBI) return false;
  if (channel.amplitude < MIN_QUALIFIED_AMPLITUDE) return false;
  if (channel.range < MIN_LIVE_RANGE) return false;
  return true;
}

void updatePickupScore(SensorChannel& channel, bool qualified, int bpm, int ibi) {
  if (!qualified) {
    channel.quality = max(channel.quality - 20, 0);
    return;
  }

  int score = 35 + channel.qualifiedBeatStreak * 10;
  int amplitudeScore = map(constrain(channel.amplitude, 0, 90), 0, 90, 0, 15);
  int rangeScore = map(constrain(channel.range, 0, 220), 0, 220, 0, 15);
  int stabilityScore = 0;

  if (channel.previousIbi > 0) {
    int ibiDelta = abs(ibi - channel.previousIbi);
    stabilityScore = map(constrain(ibiDelta, 0, 240), 240, 0, 0, 25);
  }

  channel.quality = constrain(score + amplitudeScore + rangeScore + stabilityScore, 0, 100);
  channel.previousIbi = ibi;
}

void decayPickupScore(SensorChannel& channel) {
  unsigned long now = millis();

  if (channel.lastQualifiedBeatMs > 0 && now - channel.lastQualifiedBeatMs > NO_BEAT_TIMEOUT) {
    channel.bpm = 0;
    channel.ibi = 0;
    channel.locked = false;
    channel.qualifiedBeatStreak = 0;
    channel.quality = max(channel.quality - 4, 0);
  }

  if (channel.lastBeatMs == 0 && channel.quality > 0) {
    channel.quality--;
  }
}

int sampleToY(const SensorChannel& channel, int waveY) {
  int lo = channel.minValue - 10;
  int hi = channel.maxValue + 10;
  if (hi <= lo) hi = lo + 1;

  int y = map(channel.sample, lo, hi, waveY + WAVE_H - 4, waveY + 4);
  return constrain(y, waveY + 3, waveY + WAVE_H - 3);
}

void drawStaticScreen() {
  tft.fillScreen(COLOR_BG);

  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setCursor(10, 7);
  tft.print(APP_TITLE);

  tft.setTextColor(COLOR_DIM, COLOR_BG);
  tft.setCursor(10, 22);
  tft.print("Playground BPM / IBI / beat A-B");

  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setCursor(234, 7);
  tft.print(APP_DATE);
  tft.setTextColor(COLOR_DIM, COLOR_BG);
  tft.setCursor(286, 22);
  tft.print(APP_VERSION);

  tft.drawFastHLine(0, HEADER_H - 1, SCREEN_WIDTH, COLOR_GRID);
  drawWaveFrame(WAVE_A_Y, channelA);
  drawWaveFrame(WAVE_B_Y, channelB);
  drawPanelFrames();
}

void drawWaveFrame(int y, const SensorChannel& channel) {
  tft.fillRect(WAVE_X - 1, y - 1, WAVE_W + 2, WAVE_H + 2, COLOR_BG);
  tft.drawRect(WAVE_X, y, WAVE_W, WAVE_H, COLOR_GRID);
  for (int x = 0; x <= WAVE_W; x += 38) {
    tft.drawFastVLine(WAVE_X + x, y, WAVE_H, COLOR_GRID_SOFT);
  }
  for (int x = WAVE_X + 2; x < WAVE_X + WAVE_W - 2; x += 6) {
    tft.drawPixel(x, y + WAVE_H / 2, COLOR_GRID_SOFT);
  }

  tft.setTextSize(1);
  tft.setTextColor(channel.color, COLOR_BG);
  tft.setCursor(WAVE_X + 5, y - 10);
  tft.print(channel.label);
  tft.print(" ");
  tft.print(channel.pinLabel);
  tft.setTextColor(COLOR_DIM, COLOR_BG);
  tft.setCursor(WAVE_X + 78, y - 10);
  tft.print("raw truth strip");
}

void drawWaveColumnBackground(int localX, int y) {
  int x = WAVE_X + localX;
  tft.drawFastVLine(x, y + 1, WAVE_H - 2, COLOR_BG);

  if (localX % 38 == 0) {
    tft.drawFastVLine(x, y + 1, WAVE_H - 2, COLOR_GRID_SOFT);
  }

  tft.drawPixel(x, y + WAVE_H / 2, COLOR_GRID_SOFT);
}

void drawWaveforms() {
  int x = WAVE_X + graphX;
  int prevX = WAVE_X + graphX - 1;
  int yA = sampleToY(channelA, WAVE_A_Y);
  int yB = sampleToY(channelB, WAVE_B_Y);

  drawWaveColumnBackground(graphX, WAVE_A_Y);
  drawWaveColumnBackground(graphX, WAVE_B_Y);
  drawWaveColumnBackground((graphX + 1) % WAVE_W, WAVE_A_Y);
  drawWaveColumnBackground((graphX + 1) % WAVE_W, WAVE_B_Y);

  if (graphX > 0) {
    tft.drawLine(prevX, channelA.lastY, x, yA, channelA.color);
    tft.drawLine(prevX, channelB.lastY, x, yB, channelB.color);
  }

  if (channelA.beatFlashUntilMs > millis()) {
    tft.fillCircle(x, yA, 3, COLOR_OVERLAP);
  }
  if (channelB.beatFlashUntilMs > millis()) {
    tft.fillCircle(x, yB, 3, COLOR_OVERLAP);
  }

  channelA.lastY = yA;
  channelB.lastY = yB;

  graphX++;
  if (graphX >= WAVE_W) {
    graphX = 0;
    channelA.lastY = yA;
    channelB.lastY = yB;
    drawWaveFrame(WAVE_A_Y, channelA);
    drawWaveFrame(WAVE_B_Y, channelB);
  }
}

void drawPanelFrames() {
  tft.fillRoundRect(8, PANEL_Y, PANEL_W, PANEL_H, 6, COLOR_PANEL);
  tft.drawRoundRect(8, PANEL_Y, PANEL_W, PANEL_H, 6, COLOR_A);
  tft.fillRoundRect(111, PANEL_Y, PANEL_W, PANEL_H, 6, COLOR_PANEL);
  tft.drawRoundRect(111, PANEL_Y, PANEL_W, PANEL_H, 6, COLOR_B);
  tft.fillRoundRect(214, PANEL_Y, PANEL_W, PANEL_H, 6, COLOR_PANEL);
  tft.drawRoundRect(214, PANEL_Y, PANEL_W, PANEL_H, 6, COLOR_GRID);
}

void drawMetrics() {
  drawSensorPanel(8, channelA);
  drawSensorPanel(111, channelB);
  drawVerdictPanel();
}

void drawSensorPanel(int x, SensorChannel& channel) {
  tft.setTextSize(1);
  tft.setTextColor(channel.color, COLOR_PANEL);
  tft.setCursor(x + 9, PANEL_Y + 8);
  tft.print(channel.label);
  tft.print(" ");
  tft.print(channel.pinLabel);

  tft.fillRect(x + 9, PANEL_Y + 20, 54, 26, COLOR_PANEL);
  tft.setTextSize(3);
  tft.setTextColor(channel.bpm > 0 ? COLOR_TEXT : COLOR_DIM, COLOR_PANEL);
  tft.setCursor(x + 9, PANEL_Y + 22);
  if (channel.bpm > 0) {
    tft.printf("%3d", channel.bpm);
  } else {
    tft.print("--");
  }

  tft.setTextSize(1);
  tft.setTextColor(COLOR_DIM, COLOR_PANEL);
  tft.setCursor(x + 66, PANEL_Y + 24);
  tft.print("BPM");
  tft.setCursor(x + 66, PANEL_Y + 36);
  if (channel.ibi > 0) {
    tft.printf("%4d", channel.ibi);
  } else {
    tft.print(" -- ");
  }

  drawBeatGlyph(x + 83, PANEL_Y + 12, channel);
  drawQualityBar(x + 9, PANEL_Y + 54, 78, channel.quality);
}

void drawVerdictPanel() {
  const char* winner = winnerLabel();
  int delta = bpmDelta();

  tft.setTextSize(1);
  tft.setTextColor(COLOR_DIM, COLOR_PANEL);
  tft.setCursor(223, PANEL_Y + 8);
  tft.print("BETTER PICKUP");

  tft.fillRect(223, PANEL_Y + 21, 78, 23, COLOR_PANEL);
  tft.setTextSize(2);
  uint16_t winnerColor = COLOR_OVERLAP;
  if (winner[0] == 'A') winnerColor = COLOR_A;
  if (winner[0] == 'B') winnerColor = COLOR_B;
  tft.setTextColor(winnerColor, COLOR_PANEL);
  tft.setCursor(223, PANEL_Y + 24);
  tft.print(winner);

  tft.fillRect(223, PANEL_Y + 48, 80, 13, COLOR_PANEL);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_DIM, COLOR_PANEL);
  tft.setCursor(223, PANEL_Y + 49);
  if (delta >= 0) {
    tft.printf("BPM diff %02d", delta);
  } else {
    tft.print("BPM diff --");
  }
  tft.setCursor(223, PANEL_Y + 61);
  tft.printf("beats %02d/%02d", channelA.beatCount % 100, channelB.beatCount % 100);
}

void drawQualityBar(int x, int y, int w, int value) {
  tft.fillRect(x, y, w, 9, COLOR_BG);
  int filled = map(constrain(value, 0, 100), 0, 100, 0, w);
  uint16_t color = COLOR_BAD;
  if (value >= 70) color = COLOR_GOOD;
  else if (value >= 40) color = COLOR_WARN;

  tft.fillRect(x, y, filled, 9, color);
  tft.drawRect(x, y, w, 9, COLOR_GRID);
}

void drawBeatGlyph(int x, int y, const SensorChannel& channel) {
  uint16_t color = millis() < channel.beatFlashUntilMs ? COLOR_OVERLAP : COLOR_GRID;
  if (channel.locked && color == COLOR_GRID) color = COLOR_GOOD;
  tft.fillCircle(x, y, 5, color);
}

void drawCenteredText(const char* text, int x, int y, int w, int textSize, uint16_t color, uint16_t bg) {
  int charW = 6 * textSize;
  int textW = strlen(text) * charW;
  int cursorX = x + max(0, (w - textW) / 2);
  tft.setTextSize(textSize);
  tft.setTextColor(color, bg);
  tft.setCursor(cursorX, y);
  tft.print(text);
}

void printSerialTelemetry() {
  unsigned long now = millis();
  int aBeatActive = channelA.beatFlashUntilMs > now ? 1 : 0;
  int bBeatActive = channelB.beatFlashUntilMs > now ? 1 : 0;

  Serial.printf("AB,%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
                now,
                channelA.sample, channelA.bpm, channelA.ibi, channelA.quality,
                channelA.beatCount, channelA.amplitude, channelA.range,
                channelA.locked ? 1 : 0, aBeatActive,
                channelB.sample, channelB.bpm, channelB.ibi, channelB.quality,
                channelB.beatCount, channelB.amplitude, channelB.range,
                channelB.locked ? 1 : 0, bBeatActive,
                bpmDelta(), winnerLabel());
}

const char* winnerLabel() {
  int diff = channelA.quality - channelB.quality;
  if (abs(diff) < 8) return "EVEN";
  return diff > 0 ? "A" : "B";
}

int bpmDelta() {
  if (channelA.bpm <= 0 || channelB.bpm <= 0) return -1;
  return abs(channelA.bpm - channelB.bpm);
}
