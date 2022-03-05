/*
  Simple step counter

  created Feb 2022
  by Casey Pan
*/

#include <Arduino_MKRIoTCarrier.h>
#include <TimeLib.h>
#include "visuals.h"
#include "pitches.h"

MKRIoTCarrier carrier;

/*
 * Global variables
 */
// const uint32_t COLOR_GREEN = carrier.leds.Color(255, 0, 0);
// const uint32_t COLOR_BLUE = carrier.leds.Color(0, 0, 255);
const uint32_t NO_COLOR = carrier.leds.Color(0, 0, 0);
uint32_t RAND_COLOR;

const uint32_t COLOR_BLACK = 0x0000;
const uint32_t COLOR_WHITE = 0xFFFF;
const uint32_t COLOR_GRAY = 0xEEEE;
uint32_t bkgd_color = COLOR_GRAY;
uint32_t text_color = COLOR_BLACK;
  
const float THRES_ACC = 0.5;
const int THRES_GY = 100;
const int RIGHT_LED = 1;
const int LEFT_LED = 3;

// do re mi
int do_re_mi[] = {
  NOTE_C4, NOTE_D4, NOTE_E4
};

int canon[] = {
  NOTE_E5, NOTE_F5, NOTE_G5
};

/*
 * Store Accelerometer & Gyroscope values
 */
float ax, ay, az;
float gx, gy, gz;
float ax_avg, ay_avg, az_avg;
float totvect[100] = {0};
float totave[100] = {0};
float xaccl[100] = {0};
float yaccl[100] = {0};
float zaccl[100] = {0};

/*
 * Light value
 */
int _, light;
int THRES_LIGHT = 30;
bool auto_mode_switch = true;
bool dark_mode = false;

/*
 * Timer interval (seconds) and flag
 */
const int INTERVAL = 10;
int last_movement, current_time, sit_duration;
bool should_move;

/*
 * The is_moving flag
 */
bool is_moving;
char main_msg[50], temp_msg[50];


void setup() {
  Serial.begin(9600);
  while (!Serial);

  CARRIER_CASE = false;
  if (!carrier.begin()){
    Serial.println("Error initializing carrier!");  
    while(1);  
  };

  if (!carrier.IMUmodule.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  carrier.display.setRotation(0);
  carrier.display.fillScreen(bkgd_color);
  carrier.display.setTextColor(text_color);
  carrier.display.setTextWrap(true);
  carrier.display.setCursor(50, 80);
  carrier.display.setTextSize(3);

  // Calibrate accelerometer
  calibrate();
  
  // Initialize timer & flags
  current_time = now();
  last_movement = current_time;
  should_move = false;
  is_moving = false;
  strcpy(main_msg, "Sitting...");
}


void loop() {
  // Update step and turning detection
  get_accl_update();
  get_timer_update();
  get_button_update();
  get_light_update();
  update_color_mode();
  
  carrier.display.fillScreen(bkgd_color);
  carrier.display.setTextColor(text_color);
  carrier.display.setCursor(50, 70);

  // Update feedbacks
  if (should_move && !is_moving){
    // Buzz!
    strcpy(main_msg, "MOVE UR\n\n      BODY!!");
    carrier.display.println(main_msg);
    light_alarm(true);
    // Only play melody when it's not dark mode
    if(!dark_mode){
      play_melody('A');
    }
  } else {
    strcpy(main_msg, "Sitting...");
    carrier.display.println(main_msg);
    light_alarm(false);
  }
  
  carrier.display.setCursor(110, 160);
  carrier.display.println(sit_duration);
  delay(100);
}

/*
 * Update timer, check if we need to buzz
 */
void get_timer_update(){
  current_time = now();
  sit_duration  = current_time - last_movement;

  // Should move?
  if (sit_duration >= INTERVAL){
    should_move = true;
  } else {
    should_move = false;
  }
}

/*
 * Update colors according to light/dark mode if needed
 */
void update_color_mode(){
  if (dark_mode){
    bkgd_color = COLOR_BLACK;
    text_color = COLOR_WHITE;
  } else {
    bkgd_color = COLOR_GRAY;
    text_color = COLOR_BLACK;
  }
}

/*
 * Update buttons
 */
void get_button_update(){
  carrier.Buttons.update();
  
  // Button 01 - turn auto mode switch ON/OFF
  if (carrier.Buttons.onTouchDown(TOUCH1)){
    auto_mode_switch = !auto_mode_switch;
    if (auto_mode_switch){
      flash_text("AUTO MODE\n\n     ON");
    } else {
      flash_text("AUTO MODE\n\n     OFF");
    }
  }

  // Button 03 - mannually switch light/dark mode
  if (carrier.Buttons.onTouchDown(TOUCH3)){
    dark_mode = !dark_mode;
  }
}

/*
 * Update ambient light, check if we need to switch mode
 */
void get_light_update(){
  // Only read light & auto change mode if auto_mode_switch is ON
  if(carrier.Light.colorAvailable() && auto_mode_switch){
    carrier.Light.readColor(_, _, _, light);
    Serial.println(light);

    // Switch dark mode ON/OFF accordingly & update color scheme
    if (light < THRES_LIGHT){
      dark_mode = true;
    } else {
      dark_mode = false;
    }
  } else {
    // Serial.println("APDS9960 - Light (color) unavailable!");    
  }
}

/*
 * Get accelerometer updates
 */
void get_accl_update(){
  if(carrier.IMUmodule.accelerationAvailable()){
    /*
    * Collect 100 samples
    */
    for (int a = 0; a < 100; a++){
      carrier.IMUmodule.readAcceleration(ax, ay, az);
      // carrier.IMUmodule.readGyroscope(gx, gy, gz);
      xaccl[a] = ax;
      yaccl[a] = ay;
      zaccl[a] = az;

      /* 
      * Get the total acceleration vector by taking the square root of X, Y, and Z-axis values (of the accelerometer)
      * then calculate the average of the maximum and minimum acceleration vector values
      */
      totvect[a] = sqrt(((xaccl[a] - ax_avg) * (xaccl[a] - ax_avg)) + ((yaccl[a] - ay_avg) * (yaccl[a] - ay_avg)) + ((zaccl[a] - az_avg) * (zaccl[a] - az_avg)));
      totave[a] = (totvect[a] + totvect[a - 1]) / 2 ;
      // Serial.print("totave[a]\t");
      Serial.println(totave[a]);
      
      // Check if update step according to threshold
      // If acceleration vector greater than the threshold & flag is down, increase the step count and raise the flag
      if (totave[a] > THRES_ACC && is_moving == false){
        is_moving = true;
        last_movement = current_time;
      }
      // If less than the threshold & flag is up, increase the step count and raise the flag
      if (totave[a] < THRES_ACC && is_moving == true) {
        is_moving = false;
      }
    }
  } else {
    Serial.println("IMU Unavailable!");
    while(1);
  }
}

/*
 * Calibrate accelerometer
 */
void calibrate() {
  float x, y, z;
  float xsum = 0;
  float ysum = 0;
  float zsum = 0;
  float xval[100] = {0};
  float yval[100] = {0};
  float zval[100] = {0};
  
  for (int i = 0; i < 100; i++) {
    carrier.IMUmodule.readAcceleration(x, y, z);
    xval[i] = x;
    yval[i] = y;
    zval[i] = z;
    xsum = xval[i] + xsum;
    ysum = yval[i] + ysum;
    zsum = zval[i] + zsum;
  }
  delay(100);
  
  /*
   * Store avg values to global variable
   */
  ax_avg = xsum / 100.0;
  ay_avg = ysum / 100.0;
  az_avg = zsum / 100.0;
  
  Serial.print(ax_avg + '\t');
  Serial.print(ay_avg + '\t');
  Serial.println(az_avg);
}

/*
 * Play a melody
 */
void play_melody(char dir){
  int note_duration = 150;

  if (dir == 'R'){
    for (int note = 0; note < 3; note++){
      carrier.Buzzer.sound(do_re_mi[note]);
      delay(note_duration);
      carrier.Buzzer.noSound();
    }
  } else if (dir == 'L') {
    for (int note = 2; note > -1; note--){
      carrier.Buzzer.sound(do_re_mi[note]);
      delay(note_duration);
      carrier.Buzzer.noSound();
    }
  } else {
    // Canon?
    for (int note = 0; note < 3; note++){
      carrier.Buzzer.sound(canon[note]);
      delay(note_duration);
      carrier.Buzzer.noSound();
    }
  }
}

/*
 * Turn LED light alarm ON/OFF
 */
void light_alarm(bool on){
  if(on){
    int rand_pin = int(random(5));
    int r = int(random(256));
    int g = int(random(256));
    int b = int(random(256));

    RAND_COLOR = carrier.leds.Color(r, g, b);
    carrier.leds.setPixelColor(rand_pin, RAND_COLOR);
  } else {
    carrier.leds.fill(NO_COLOR, 0, 5);
  }

  carrier.leds.show();
}

/*
 * Flash text message
 */
void flash_text(char msg[]){
  strcpy(temp_msg, main_msg);
  strcpy(main_msg, msg);
  carrier.display.fillScreen(bkgd_color);
  carrier.display.setTextColor(text_color);
  carrier.display.setCursor(50, 70);
  carrier.display.println(main_msg);

  delay(200);

  strcpy(main_msg, temp_msg);
}