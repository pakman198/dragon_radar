// Dragon Radar for the M5Stack StopWatch (round 466x466 AMOLED).
// BtnA: cycle zoom (plays zoom sound)   BtnB: mute / unmute   Both held: power off
// Needs: M5Stack board package >= 3.3.7 (board "M5StopWatch"), M5Unified >= 0.2.15, M5GFX >= 0.2.21.

#include <M5Unified.h>
#include <math.h>
#include "ping_sound.h"
#include "zoom_sound.h"

M5Canvas canvas(&M5.Display);

static const int SIZE = 466;
static const int CX = SIZE / 2, CY = SIZE / 2;
static const int RADIUS = 212;   // green screen
static const int BEZEL = 233;    // outer edge of the pale bezel ring
static const int GRID = 34;      // grid spacing in pixels

// Radar ranges in "meters"; BtnA cycles through them.
static const float RANGES[] = {500, 1000, 2000, 4000};
static const int   NUM_RANGES = sizeof(RANGES) / sizeof(RANGES[0]);
static const float MAX_DIST = 3500;  // the fake balls wander between 150 and this

// Fake dragon balls at fixed spots: star count, distance in meters, angle in
// radians (0 = right, positive = clockwise, so -PI/2 is straight up).
struct Ball { int stars; float dist, angle; };
static const Ball BALLS[] = {
  {4, 1500, -0.9f},  // up and to the right
  {1, 2300,  2.6f},  // down and to the left
  {1, 2000,  1.7f},  // below
};
static const int NUM_BALLS = sizeof(BALLS) / sizeof(BALLS[0]);

int      zoomIdx = 3;
bool     muted = false;
uint32_t lastPing = 0;
uint32_t blipFlashUntil = 0;

void zoomSound() {
  if (muted) return;
  M5.Speaker.playRaw(ZOOM_DATA, ZOOM_LEN, ZOOM_RATE, false, 1, -1, true);
}

void pingSound() {
  if (muted) return;
  M5.Speaker.playRaw(PING_DATA, PING_LEN, PING_RATE, false, 1, -1, true);
}

void exitApp() {
  M5.Display.fillScreen(0);
  M5.Speaker.tone(600, 150);
  delay(200);
  M5.Power.powerOff();  // untested on the StopWatch; fall back to a restart if it returns
  delay(500);
  ESP.restart();
}

void drawGrid(uint16_t line) {
  // Square grid clipped to the circular screen, centered on the player.
  for (int d = -(RADIUS / GRID) * GRID; d <= RADIUS; d += GRID) {
    int half = (int)sqrtf((float)(RADIUS * RADIUS - d * d));
    canvas.drawFastHLine(CX - half, CY + d, 2 * half + 1, line);
    canvas.drawFastVLine(CX + d, CY - half, 2 * half + 1, line);
  }
}

void drawBall(int x, int y, int stars, bool flash, uint16_t glow, uint16_t core) {
  canvas.fillCircle(x, y, flash ? 18 : 13, glow);   // soft yellow halo
  canvas.fillCircle(x, y, flash ? 12 : 9, core);    // bright core
  canvas.setTextColor(core);
  canvas.setTextDatum(middle_left);
  canvas.setTextSize(3);
  char s[4];
  snprintf(s, sizeof(s), "%d", stars);
  canvas.drawString(s, x + 22, y);
}

void drawRadar() {
  uint16_t bezel = canvas.color565(200, 205, 190);
  uint16_t screen = canvas.color565(30, 150, 60);
  uint16_t grid = canvas.color565(8, 70, 28);
  uint16_t glow = canvas.color565(255, 200, 0);
  uint16_t core = canvas.color565(255, 240, 60);
  uint16_t red = canvas.color565(230, 30, 30);

  canvas.fillScreen(0);
  canvas.fillCircle(CX, CY, BEZEL, bezel);
  canvas.fillCircle(CX, CY, RADIUS, screen);
  drawGrid(grid);

  // Balls in range; out-of-range ones simply don't show, like on the real radar.
  float range = RANGES[zoomIdx];
  bool flash = millis() < blipFlashUntil;
  for (int i = 0; i < NUM_BALLS; i++) {
    float r = BALLS[i].dist / range;
    if (r > 1.0f) continue;
    float a = BALLS[i].angle;
    int bx = CX + cosf(a) * r * (RADIUS - 30);
    int by = CY + sinf(a) * r * (RADIUS - 30);
    drawBall(bx, by, BALLS[i].stars, flash, glow, core);
  }

  // Player: red triangle pointing up
  canvas.fillTriangle(CX, CY - 12, CX - 11, CY + 9, CX + 11, CY + 9, red);

  // Range label
  canvas.setTextColor(grid);
  canvas.setTextDatum(middle_center);
  canvas.setTextSize(2);
  char buf[24];
  snprintf(buf, sizeof(buf), "%.0f m%s", range, muted ? " (mute)" : "");
  canvas.drawString(buf, CX, CY + RADIUS - 22);

  canvas.pushSprite(0, 0);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Speaker.setVolume(200);
  canvas.setPsram(true);
  canvas.setColorDepth(16);
  canvas.createSprite(SIZE, SIZE);
}

void loop() {
  M5.update();

  // Both buttons held for 500 ms = exit. Single presses act on release so the
  // chord doesn't also zoom or mute.
  static bool chord = false;
  static uint32_t chordStart = 0;
  bool a = M5.BtnA.isPressed(), b = M5.BtnB.isPressed();
  if (a && b) {
    if (!chord) { chord = true; chordStart = millis(); }
    else if (millis() - chordStart >= 500) exitApp();
  }
  if (M5.BtnA.wasReleased() && !chord) {
    zoomIdx = (zoomIdx + 1) % NUM_RANGES;
    zoomSound();
  }
  if (M5.BtnB.wasReleased() && !chord) muted = !muted;
  if (!a && !b) chord = false;

  // Beep interval is set by the nearest ball's distance (150 ms .. 1200 ms).
  float nearest = MAX_DIST;
  for (int i = 0; i < NUM_BALLS; i++) nearest = fminf(nearest, BALLS[i].dist);
  uint32_t interval = 150 + (uint32_t)(1050.0f * nearest / MAX_DIST);
  if (millis() - lastPing >= interval) {
    lastPing = millis();
    blipFlashUntil = millis() + 80;
    pingSound();
  }

  drawRadar();
}
