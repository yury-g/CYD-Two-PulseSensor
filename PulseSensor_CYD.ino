/*
 * CYD Two PulseSensor Comparator
 *
 * Minimal ESP32-2432S028 CYD experiment for comparing two PulseSensor signal
 * candidates side by side. This intentionally reads raw ADC values instead of
 * using PulseSensorPlayground so we can judge wiring and signal quality first.
 *
 * Current hardware-tested mapping:
 *   PulseSensor A purple signal -> GPIO35
 *   PulseSensor B purple signal -> GPIO27
 *   Sensor power                -> 3.3V
 *   Sensor ground               -> GND
 */

#include <TFT_eSPI.h>
#include <math.h>

// ===== CYD PINS =====

#define PULSE_A_PIN 35
#define PULSE_B_PIN 27
#define BACKLIGHT_PIN 21

// ===== SCREEN =====

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define HEADER_H 36
#define GRAPH_X 8
#define GRAPH_W 304
#define GRAPH_A_Y 45
#define GRAPH_B_Y 116
#define GRAPH_H 58
#define METRIC_Y 184
#define ADC_MAX_VALUE 4095
#define HISTORY_SIZE 64

// ===== COLORS (RGB565) =====

#define COLOR_BG 0x0000
#define COLOR_PANEL 0x0841
#define COLOR_GRID 0x18E3
#define COLOR_TEXT 0xFFFF
#define COLOR_DIM 0x8C71
#define COLOR_A 0x07FF
#define COLOR_B 0xFBE0
#define COLOR_GREEN 0x07E0
#define COLOR_WARN 0xF800

struct Channel {
  const char* name;
  uint8_t pin;
  uint16_t color;
  int raw;
  int minValue;
  int maxValue;
  int range;
  int quality;
  int lastY;
};

TFT_eSPI tft = TFT_eSPI();

Channel channelA = {"GPIO35", PULSE_A_PIN, COLOR_A, 2048, ADC_MAX_VALUE, 0, 0, 0, GRAPH_A_Y + GRAPH_H / 2};
Channel channelB = {"GPIO27", PULSE_B_PIN, COLOR_B, 2048, ADC_MAX_VALUE, 0, 0, 0, GRAPH_B_Y + GRAPH_H / 2};

int graphX = 0;
int historyA[HISTORY_SIZE];
int historyB[HISTORY_SIZE];
int historyIndex = 0;
bool historyFilled = false;

unsigned long lastDraw = 0;
unsigned long lastSerial = 0;

void setup();
void loop();
void setupAdc();
int readAverage(uint8_t pin);
void updateChannel(Channel& channel);
void updateHistory();
int scoreChannel(const Channel& channel);
int signalToY(const Channel& channel, int graphY);
void drawStaticScreen();
void drawGraphFrame(int y, const char* title, uint16_t color);
void drawGraphColumn();
void drawMetrics();
void drawChannelMetric(int x, const Channel& channel, bool best);
void drawCenteredText(const char* text, int x, int y, int w, uint16_t color, uint16_t bg);
int correlationPercent();
const char* relationLabel(int corr);
bool isRailed(int value);

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("CYD Two PulseSensor Comparator");

  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);

  setupAdc();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);
  drawStaticScreen();
}

void loop() {
  updateChannel(channelA);
  updateChannel(channelB);
  updateHistory();

  if (millis() - lastDraw >= 25) {
    lastDraw = millis();
    drawGraphColumn();
    drawMetrics();
  }

  if (millis() - lastSerial >= 500) {
    lastSerial = millis();
    Serial.printf("GPIO35 raw=%4d range=%4d q=%3d  GPIO27 raw=%4d range=%4d q=%3d  corr=%d%%\n",
                  channelA.raw, channelA.range, channelA.quality,
                  channelB.raw, channelB.range, channelB.quality,
                  correlationPercent());
  }
}

void setupAdc() {
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(PULSE_A_PIN, INPUT);
  pinMode(PULSE_B_PIN, INPUT);

  for (int i = 0; i < HISTORY_SIZE; i++) {
    historyA[i] = 0;
    historyB[i] = 0;
  }
}

int readAverage(uint8_t pin) {
  long total = 0;
  for (int i = 0; i < 8; i++) {
    total += analogRead(pin);
    delayMicroseconds(120);
  }
  return total / 8;
}

void updateChannel(Channel& channel) {
  channel.raw = readAverage(channel.pin);

  if (channel.raw < channel.minValue) channel.minValue = channel.raw;
  if (channel.raw > channel.maxValue) channel.maxValue = channel.raw;

  channel.range = channel.maxValue - channel.minValue;
  channel.quality = scoreChannel(channel);

  // Relax the rolling min/max so old movement fades after a few seconds.
  if (channel.minValue < channel.raw) channel.minValue += 2;
  if (channel.maxValue > channel.raw) channel.maxValue -= 2;
}

void updateHistory() {
  historyA[historyIndex] = channelA.raw;
  historyB[historyIndex] = channelB.raw;
  historyIndex++;
  if (historyIndex >= HISTORY_SIZE) {
    historyIndex = 0;
    historyFilled = true;
  }
}

int scoreChannel(const Channel& channel) {
  int score = constrain(channel.range, 0, 600);
  if (isRailed(channel.raw)) score -= 250;
  if (channel.range < 20) score -= 80;
  return constrain(score, 0, 600);
}

int signalToY(const Channel& channel, int graphY) {
  int lo = channel.minValue - 20;
  int hi = channel.maxValue + 20;
  if (hi - lo < 120) {
    int center = (hi + lo) / 2;
    lo = center - 60;
    hi = center + 60;
  }
  lo = constrain(lo, 0, ADC_MAX_VALUE);
  hi = constrain(hi, 0, ADC_MAX_VALUE);
  if (hi <= lo) hi = lo + 1;

  int y = map(channel.raw, lo, hi, graphY + GRAPH_H - 3, graphY + 3);
  return constrain(y, graphY + 2, graphY + GRAPH_H - 2);
}

void drawStaticScreen() {
  tft.fillScreen(COLOR_BG);

  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setCursor(10, 8);
  tft.print("Two PulseSensor Comparator");

  tft.setTextColor(COLOR_DIM, COLOR_BG);
  tft.setCursor(10, 23);
  tft.print("GPIO35 vs GPIO27 raw signal, quality, relation");

  tft.drawFastHLine(0, HEADER_H, SCREEN_WIDTH, COLOR_GRID);
  drawGraphFrame(GRAPH_A_Y, "A GPIO35", COLOR_A);
  drawGraphFrame(GRAPH_B_Y, "B GPIO27", COLOR_B);
  tft.drawFastHLine(0, METRIC_Y - 7, SCREEN_WIDTH, COLOR_GRID);
}

void drawGraphFrame(int y, const char* title, uint16_t color) {
  tft.drawRect(GRAPH_X, y, GRAPH_W, GRAPH_H, COLOR_GRID);
  tft.drawFastHLine(GRAPH_X + 1, y + GRAPH_H / 2, GRAPH_W - 2, COLOR_GRID);
  tft.setTextSize(1);
  tft.setTextColor(color, COLOR_BG);
  tft.setCursor(GRAPH_X + 4, y - 11);
  tft.print(title);
}

void drawGraphColumn() {
  int x = GRAPH_X + 1 + graphX;
  int prevX = GRAPH_X + 1 + ((graphX + GRAPH_W - 3) % (GRAPH_W - 2));
  int yA = signalToY(channelA, GRAPH_A_Y);
  int yB = signalToY(channelB, GRAPH_B_Y);

  tft.drawFastVLine(x, GRAPH_A_Y + 1, GRAPH_H - 2, COLOR_BG);
  tft.drawFastVLine(x, GRAPH_B_Y + 1, GRAPH_H - 2, COLOR_BG);
  if (graphX % 24 == 0) {
    tft.drawFastVLine(x, GRAPH_A_Y + 1, GRAPH_H - 2, COLOR_GRID);
    tft.drawFastVLine(x, GRAPH_B_Y + 1, GRAPH_H - 2, COLOR_GRID);
  }
  tft.drawPixel(x, GRAPH_A_Y + GRAPH_H / 2, COLOR_GRID);
  tft.drawPixel(x, GRAPH_B_Y + GRAPH_H / 2, COLOR_GRID);

  if (graphX > 0) {
    tft.drawLine(prevX, channelA.lastY, x, yA, channelA.color);
    tft.drawLine(prevX, channelB.lastY, x, yB, channelB.color);
  }

  channelA.lastY = yA;
  channelB.lastY = yB;
  graphX++;
  if (graphX >= GRAPH_W - 2) {
    graphX = 0;
    tft.fillRect(GRAPH_X + 1, GRAPH_A_Y + 1, GRAPH_W - 2, GRAPH_H - 2, COLOR_BG);
    tft.fillRect(GRAPH_X + 1, GRAPH_B_Y + 1, GRAPH_W - 2, GRAPH_H - 2, COLOR_BG);
  }
}

void drawMetrics() {
  bool aBest = channelA.quality >= channelB.quality;
  int corr = correlationPercent();

  tft.fillRect(0, METRIC_Y, SCREEN_WIDTH, SCREEN_HEIGHT - METRIC_Y, COLOR_BG);
  drawChannelMetric(8, channelA, aBest);
  drawChannelMetric(112, channelB, !aBest);

  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setCursor(222, METRIC_Y + 5);
  tft.print("REL");
  tft.setCursor(222, METRIC_Y + 22);
  tft.printf("%+d%%", corr);
  tft.setTextColor(COLOR_DIM, COLOR_BG);
  tft.setCursor(222, METRIC_Y + 39);
  tft.print(relationLabel(corr));
}

void drawChannelMetric(int x, const Channel& channel, bool best) {
  tft.drawRect(x, METRIC_Y + 2, 94, 49, best ? COLOR_GREEN : COLOR_GRID);
  tft.setTextSize(1);
  tft.setTextColor(channel.color, COLOR_BG);
  tft.setCursor(x + 5, METRIC_Y + 7);
  tft.print(channel.name);

  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setCursor(x + 5, METRIC_Y + 20);
  tft.printf("raw %4d", channel.raw);
  tft.setCursor(x + 5, METRIC_Y + 33);
  tft.printf("rng %4d", channel.range);

  tft.setTextColor(best ? COLOR_GREEN : COLOR_DIM, COLOR_BG);
  tft.setCursor(x + 62, METRIC_Y + 7);
  tft.print(best ? "BEST" : "    ");
}

int correlationPercent() {
  int count = historyFilled ? HISTORY_SIZE : historyIndex;
  if (count < 12) return 0;

  long sumA = 0;
  long sumB = 0;
  for (int i = 0; i < count; i++) {
    sumA += historyA[i];
    sumB += historyB[i];
  }
  int meanA = sumA / count;
  int meanB = sumB / count;

  long numerator = 0;
  long energyA = 0;
  long energyB = 0;
  for (int i = 0; i < count; i++) {
    long a = historyA[i] - meanA;
    long b = historyB[i] - meanB;
    numerator += a * b;
    energyA += a * a;
    energyB += b * b;
  }

  if (energyA < 100 || energyB < 100) return 0;

  float corr = (float)numerator / sqrt((float)energyA * (float)energyB);
  return constrain((int)(corr * 100.0f), -100, 100);
}

const char* relationLabel(int corr) {
  int absCorr = abs(corr);
  if (absCorr >= 75) return corr > 0 ? "same" : "inverse";
  if (absCorr >= 40) return "related";
  return "separate";
}

bool isRailed(int value) {
  return value < 20 || value > ADC_MAX_VALUE - 20;
}
