#include <Arduino_GFX_Library.h>
#include <RotaryEncoder.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include "touch.h"
#include "background.h"

//----------Define Display Drivers----------
#define DISPLAY_DC GFX_NOT_DEFINED
#define DISPLAY_CS 1
#define DISPLAY_SCK 46
#define DISPLAY_MOSI 0
#define DISPLAY_MISO GFX_NOT_DEFINED

#define DISPLAY_DE 2
#define DISPLAY_VSYNC 42
#define DISPLAY_HSYNC 3
#define DISPLAY_PCLK 45
#define DISPLAY_R0 11
#define DISPLAY_R1 15
#define DISPLAY_R2 12
#define DISPLAY_R3 16
#define DISPLAY_R4 21
#define DISPLAY_G0 39
#define DISPLAY_G1 7
#define DISPLAY_G2 47
#define DISPLAY_G3 8
#define DISPLAY_G4 48
#define DISPLAY_G5 9
#define DISPLAY_B0 4
#define DISPLAY_B1 41
#define DISPLAY_B2 5
#define DISPLAY_B3 40
#define DISPLAY_B4 6
#define DISPLAY_HSYNC_POL 1
#define DISPLAY_VSYNC_POL 1
#define DISPLAY_HSYNC_FRONT_PORCH 10
#define DISPLAY_VSYNC_FRONT_PORCH 10
#define DISPLAY_HSYNC_BACK_PORCH 50
#define DISPLAY_VSYNC_BACK_PORCH 20
#define DISPLAY_HSYNC_PULSE_WIDTH 8
#define DISPLAY_VSYNC_PULSE_WIDTH 8

Arduino_DataBus *bus = new Arduino_HWSPI(
    DISPLAY_DC /* DC */, DISPLAY_CS /* CS */,
    DISPLAY_SCK /* SCK */, DISPLAY_MOSI /* MOSI */, DISPLAY_MISO /* MISO */);


Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    DISPLAY_DE, DISPLAY_VSYNC , DISPLAY_HSYNC, DISPLAY_PCLK,
    DISPLAY_B0, DISPLAY_B1, DISPLAY_B2, DISPLAY_B3, DISPLAY_B4,
    DISPLAY_G0, DISPLAY_G1, DISPLAY_G2, DISPLAY_G3, DISPLAY_G4, DISPLAY_G5,
    DISPLAY_R0, DISPLAY_R1, DISPLAY_R2, DISPLAY_R3, DISPLAY_R4,
    DISPLAY_HSYNC_POL, DISPLAY_HSYNC_FRONT_PORCH, DISPLAY_HSYNC_PULSE_WIDTH, DISPLAY_VSYNC_BACK_PORCH,
    DISPLAY_VSYNC_POL, DISPLAY_VSYNC_FRONT_PORCH, DISPLAY_VSYNC_PULSE_WIDTH, DISPLAY_VSYNC_BACK_PORCH);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    480 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */, true /* auto_flush */,
    bus, GFX_NOT_DEFINED /* RST */, st7701_type5_init_operations, sizeof(st7701_type5_init_operations));

//---------Define I/O Pins----------
#define PWM_CHANNEL 1
#define PWM_FREQ 5000  //Hz
#define pwm_resolution_bits 10
#define IO_PWM_PIN 38

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18
#define TOUCH_RST -1  // 38
#define TOUCH_IRQ -1  // 0

#define ENCODER_CLK 13
#define ENCODER_DT 10
#define BUTTON 14

//----------Initialise Rotary Encoder and Push Button variables----------

int State;
int old_State;
int move_flag = 0;
int button_flag = 0;
int flesh_flag = 1;
long previousMillis = 0;
bool shift_light_flash_state = true;

//----------Define Color Variables----------
#define red 0xE8E4
#define blue 0x10d8
#define yellow 0xFFE0
#define bck TFT_BLACK
#define wht TFT_WHITE

//----------Define Dial and Needle Variables----------

#define CLOCK_R 480.0f / 2.0f  // Clock face radius (float type)
#define NEEDLE_LENGTH CLOCK_R / 1.2f
#define FACE_W CLOCK_R * 2 + 1
#define FACE_H CLOCK_R * 2 + 1
#define ANGLE_START -180.0f
#define ANGLE_END 60.0f
#define RPM_MIN_STEP 0
#define RPM_MAX_STEP 500
#define RPM_MIN 0
#define RPM_MAX 10000
#define RPM_TRUNC_END 4000
#define ANGLE_TRUNC_END -120.0f
#define SHIFT_LIGHT_YELLOW 75.0f
#define SHIFT_LIGHT_RED 90.0f
#define SHIFT_LIGHT_FLASH 95.0f
#define SHIFT_LIGHT_MAX_THICKNESS 30
#define SHIFT_LIGHT_FLASH_INTERVAL 150


int rpm_iteration_step = 0;
int rpm = 8500;
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

  pin_init();


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
    rpm += rpm_iteration_step;
  } else {
    rpm -= rpm_iteration_step;
  }

  if (rpm <= RPM_MIN) {
    rpm = RPM_MIN;
    direction = 1;
  } else if (rpm >= RPM_MAX) {
    rpm = RPM_MAX;
    direction = 0;
  }

  draw(rpm);
}

void draw(int rpm) {
  //allows for the non linear nature of some tachometers. i.e. lfa indices go 0,2,4,5,6,7,8,9,10
  angle = compute_angle(rpm);
  plot_gauge(angle, rpm);
  gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)gauge_background.getPointer(), 480, 480);
}

int compute_angle(int rpm){

    if (rpm < RPM_TRUNC_END) {
    return map(rpm, RPM_MIN, RPM_TRUNC_END, ANGLE_START, ANGLE_TRUNC_END);
  } else {
    return map(rpm, RPM_TRUNC_END, RPM_MAX, ANGLE_TRUNC_END, ANGLE_END);
  }
}

void plot_gauge(int angle, int rpm) {

  gauge_background.pushImage(0, 0, 480, 480, background);
  gauge_background.setTextColor(YELLOW, TFT_TRANSPARENT);
  float rpm_percentage = ((float) rpm / (float) RPM_MAX) * 100.0f;

  //TODO - refactor this into it's own function (drawing shift light)
  if(rpm_percentage >= SHIFT_LIGHT_YELLOW) {
    if(rpm_percentage <= SHIFT_LIGHT_FLASH || shift_light_flash_state ){
      gauge_background.drawSmoothArc(CLOCK_R, CLOCK_R+1, 143, 0, 0, 360, (rpm_percentage <= SHIFT_LIGHT_RED ? yellow : red), wht, false);
      gauge_background.drawSmoothArc(CLOCK_R, CLOCK_R+1, (rpm_percentage <= SHIFT_LIGHT_RED ? map(rpm_percentage, SHIFT_LIGHT_YELLOW, SHIFT_LIGHT_RED, 143, 110) : 110), 0, 0, 360, bck, (rpm_percentage <= SHIFT_LIGHT_RED ? yellow : red), false);
    }
  }
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= SHIFT_LIGHT_FLASH_INTERVAL){
    shift_light_flash_state = !shift_light_flash_state;
    previousMillis = currentMillis;
  }

  //TODO - refactor this into it's own function (drawing the needle)
  int angle_int = (int)trunc((angle));
  if (angle < 0) {
    gauge_background.drawWideLine(CLOCK_R, CLOCK_R, needle_end_x[360 + angle_int], needle_end_y[360 + angle_int], 6.0f, BLUE);
  } else {
    gauge_background.drawWideLine(CLOCK_R, CLOCK_R, needle_end_x[angle_int], needle_end_y[angle_int], 6.0f, BLUE);
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

  State = digitalRead(ENCODER_CLK);
  if (State != old_State) {
    if (digitalRead(ENCODER_DT) == State) {
      rpm_iteration_step += 5;
      if (rpm_iteration_step > RPM_MAX_STEP)
        rpm_iteration_step = RPM_MAX_STEP;
    } else {
      rpm_iteration_step -= 5;
      if (rpm_iteration_step < RPM_MIN_STEP)
        rpm_iteration_step = RPM_MIN_STEP;
    }
  }
  old_State = State;  // the first position was changed
  move_flag = 1;
}

void pin_init() {
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(IO_PWM_PIN, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, pwm_resolution_bits);
  ledcAttachPin(IO_PWM_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 840);
  old_State = digitalRead(ENCODER_CLK);

  attachInterrupt(ENCODER_CLK, readEncoder, CHANGE);
}
