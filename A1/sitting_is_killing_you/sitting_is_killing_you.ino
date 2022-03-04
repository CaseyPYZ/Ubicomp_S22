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
const uint32_t COLOR_GREEN = carrier.leds.Color(255, 0, 0);
const uint32_t COLOR_BLUE = carrier.leds.Color(0, 0, 255);
const uint32_t NO_COLOR = carrier.leds.Color(0, 0, 0);

const uint32_t COLOR_BLACK = 0x0000;
const uint32_t COLOR_WHITE = 0xFFFF;
uint32_t bkgd_color = COLOR_BLACK;
uint32_t text_color = COLOR_WHITE;
  
const float THRES_ACC = 1.;
const int THRES_GY = 100;
const int RIGHT_LED = 1;
const int LEFT_LED = 3;

// do re mi
int melody[] = {
  NOTE_C4, NOTE_D4, NOTE_E4
};

/*
 * Store Accelerometer & Gyroscope values
 */
float ax, ay, az;
float gx, gy, gz;

float totvect[100] = {0};
float totave[100] = {0};
float xaccl[100] = {0};
float yaccl[100] = {0};
float zaccl[100] = {0};

float ax_avg, ay_avg, az_avg;

/*
 * Timer interval (seconds) and flag
 */
const int INTERVAL = 30;
int last_movement, current_time, sit_duration;
bool should_move;

/*
 * The is_moving flag
 */
bool is_moving;
char main_msg[50];


void setup() {
  CARRIER_CASE = false;
  carrier.begin();

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
  update_timer();
  
  carrier.display.fillScreen(bkgd_color);
  carrier.display.setCursor(50, 80);

  // Update feedbacks
  if (should_move && !is_moving){
    // Buzz!
    strcpy(main_msg, "MOVE UR BODY!");
    carrier.display.println(main_msg);
    play_melody('R');
  } else {
    strcpy(main_msg, "Sitting...");
    carrier.display.println(main_msg);
  }
  
  carrier.display.println(sit_duration);
  delay(100);
}

/*
 * Update timer, check if we need to buzz
 */
void update_timer(){
  current_time = now();
  sit_duration  = current_time - last_movement;

  if (sit_duration >= INTERVAL && !is_moving){
    // Buzz!
    should_move = true;
  } else {
    should_move = false;
  }
}

/*
 * Get accelerometer updates
 */
void get_accl_update(){
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
    Serial.print("totave[a]\t");
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
    // carrier.IMUmodule.readGyroscope(x, y, z);
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
      carrier.Buzzer.sound(melody[note]);
      delay(note_duration);
      carrier.Buzzer.noSound();
    }
  } else {
    for (int note = 2; note > -1; note--){
      carrier.Buzzer.sound(melody[note]);
      delay(note_duration);
      carrier.Buzzer.noSound();
    }
  }
}

/*
 * Display text
 */
void display_text(char msg[]){
  carrier.display.fillScreen(bkgd_color);
  carrier.display.setCursor(50, 80);
  carrier.display.setTextColor(text_color);
  carrier.display.setTextSize(3);
  carrier.display.println(msg);
  delay(100);
}
