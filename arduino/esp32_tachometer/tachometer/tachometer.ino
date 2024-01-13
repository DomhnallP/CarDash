#include <Arduino_GFX_Library.h>
#include <RotaryEncoder.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include "touch.h"
#include "background.h"

//----------Define Display Drivers----------
Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
  1 /* CS */, 46 /* SCK */, 0 /* SDA */,
  2 /* DE */, 42 /* VSYNC */, 3 /* HSYNC */, 45 /* PCLK */,
  11 /* R0 */, 15 /* R1 */, 12 /* R2 */, 16 /* R3 */, 21 /* R4 */,
  39 /* G0/P22 */, 7 /* G1/P23 */, 47 /* G2/P24 */, 8 /* G3/P25 */, 48 /* G4/P26 */, 9 /* G5 */,
  4 /* B0 */, 41 /* B1 */, 5 /* B2 */, 40 /* B3 */, 6 /* B4 */
);

Arduino_ST7701_RGBPanel *gfx = new Arduino_ST7701_RGBPanel(
  bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */,
  false /* IPS */, 480 /* width */, 480 /* height */,
  st7701_type5_init_operations, sizeof(st7701_type5_init_operations),
  true /* BGR */,
  10 /* hsync_front_porch */, 8 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
  10 /* vsync_front_porch */, 8 /* vsync_pulse_width */, 20 /* vsync_back_porch */);

//---------Define I/O Pins----------
#define PWM_CHANNEL 1
#define PWM_FREQ 5000  //Hz
#define pwm_resolution_bits 10
#define IO_PWM_PIN 38

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18
#define TOUCH_RST -1  // 38
#define TOUCH_IRQ -1  // 0

#define PIN_IN1 13
#define PIN_IN2 10
#define BUTTON 14

//----------Initialise Rotary Encoder----------
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

//----------Define Color Variables----------
#define red 0xE8E4
#define blue 0x10d8
#define yellow 0x9381
#define bck TFT_BLACK

//----------Define Dial and Needle Variables----------

#define CLOCK_R 480.0f / 2.0f  // Clock face radius (float type)
#define NEEDLE_LENGTH CLOCK_R / 1.2f
#define FACE_W CLOCK_R * 2 + 1
#define FACE_H CLOCK_R * 2 + 1
#define ANGLE_START -180
#define ANGLE_END 60
int needle_speed = 0;
int angle = -180;
short direction = 1;

//used to precalculate end positions for the needle
float xp = 0.0f, yp = 0.0f;
float needle_end_x[360];  //outer points of Speed gauges
float needle_end_y[360];

float indices_inner_x[360];  //outer points of Speed gauges
float indices_inner_y[360];

float indices_outer_x[360];  //outer points of Speed gauges
float indices_outer_y[360];



TFT_eSPI tft = TFT_eSPI();
TFT_eSprite gauge_background = TFT_eSprite(&tft);


void setup() {
  pinMode(IO_PWM_PIN, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, pwm_resolution_bits);
  ledcAttachPin(IO_PWM_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 840);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);



  for (int i = 0; i < 360; i++) {
    //precalculate needle positions
    calculate_needle_coords(CLOCK_R, CLOCK_R, &xp, &yp, NEEDLE_LENGTH, i);
    needle_end_x[i] = xp;
    needle_end_y[i] = yp;

    if (i % 30 == 0) {
      calculate_index_coords(210, 30, i, i);
    } else if (i % 15 == 0) {
      calculate_index_coords(220, 20, i, i);
    } else if (i % 3 == 0) {
      calculate_index_coords(225, 10, i, i);
    } else {
      indices_inner_x[i] = 0.0;  //outer points of Speed gauges
      indices_inner_y[i] = 0.0;
      indices_outer_x[i] = 0.0;  //outer points of Speed gauges
      indices_outer_y[i] = 0.0;
    }
  }


  //init gauge
  gauge_background.setColorDepth(16);
  gauge_background.setSwapBytes(true);
  gauge_background.createSprite(480, 480);

  //initialise graphics processor
  gfx->begin();
  draw(-180);
}

void loop() {

  readEncoder();

  if (direction == 1) {
    angle += needle_speed;
  } else {
    angle -= needle_speed;
  }

  if (angle <= ANGLE_START) {
    angle = ANGLE_START;
    direction = 1;
  } else if (angle >= ANGLE_END) {
    angle = ANGLE_END;
    direction = 0;
  }

  draw(angle);
}

void draw(int angle) {
  plot_gauge(angle);
  gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)gauge_background.getPointer(), 480, 480);
}

void plot_gauge(int angle) {
  gauge_background.fillCircle(240, 240, 250, WHITE);
  gauge_background.drawArc(240, 240, 241, 126, 210, 240, RED, TFT_TRANSPARENT, 1);
  gauge_background.fillSmoothCircle(240, 240, 127, BLACK, WHITE);

  for (int i = ANGLE_START; i <= ANGLE_END; i += 3) {
    if (i < 0) {
      gauge_background.drawWideLine(indices_inner_x[360 + i],
                                    indices_inner_y[360 + i],
                                    indices_outer_x[360 + i],
                                    indices_outer_y[360 + i],
                                    2.0f,
                                    BLACK);
    } else {
      gauge_background.drawWideLine(indices_inner_x[i],
                                    indices_inner_y[i],
                                    indices_outer_x[i],
                                    indices_outer_y[i],
                                    2.0f,
                                    BLACK);
    }
  }

  // gauge_background.pushImage(0, 0, 480, 480, background);

  gauge_background.setTextColor(YELLOW, TFT_TRANSPARENT);
  gauge_background.drawString(String(needle_speed), CLOCK_R, CLOCK_R * 0.75);
  if (angle < 0) {
    gauge_background.drawWideLine(CLOCK_R, CLOCK_R, needle_end_x[360 + angle], needle_end_y[360 + angle], 6.0f, BLUE);
  } else {
    gauge_background.drawWideLine(CLOCK_R, CLOCK_R, needle_end_x[angle], needle_end_y[angle], 6.0f, BLUE);
  }
}

#define DEG2RAD 0.0174532925
void calculate_needle_coords(int16_t x, int16_t y, float *xp, float *yp, int16_t r, float a) {
  float sx1 = cos((a - 90) * DEG2RAD);
  float sy1 = sin((a - 90) * DEG2RAD);
  *xp = sx1 * r + x;
  *yp = sy1 * r + y;
}

void calculate_index_coords(int16_t r, int16_t len, float a, int i) {
  indices_inner_x[i] = r * cos(DEG2RAD * (a - 90)) + CLOCK_R;
  indices_inner_y[i] = r * sin(DEG2RAD * (a - 90)) + CLOCK_R;
  indices_outer_x[i] = (r + len) * cos(DEG2RAD * (a - 90)) + CLOCK_R;  //outer points of Speed gauges
  indices_outer_y[i] = (r + len) * sin(DEG2RAD * (a - 90)) + CLOCK_R;
}

//TODO - add encoder logic in future - what do with it? ¯\_(ツ)_/¯
void readEncoder() {
  static int pos = 0;
  encoder.tick();

  int newPos = encoder.getPosition();
  USBSerial.println(newPos);
  if (pos != newPos) {

    if (newPos > pos)
      needle_speed++;
    if (newPos < pos)
      needle_speed--;

    pos = newPos;
  }
  if (needle_speed < 0)
    needle_speed = 0;

  if (needle_speed >= 10)
    needle_speed = 10;
}
