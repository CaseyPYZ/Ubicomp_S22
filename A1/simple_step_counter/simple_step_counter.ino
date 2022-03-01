/*
  Simple step counter

  created Feb 2022
  by Casey Pan
*/

#include <Arduino_MKRIoTCarrier.h>
#include "visuals.h"
#include "pitches.h"

MKRIoTCarrier carrier;

/*
 * Global variables
 */
const uint32_t COLOR_GREEN = carrier.leds.Color(255, 0, 0);
const uint32_t COLOR_BLUE = carrier.leds.Color(0, 0, 255);
const uint32_t NO_COLOR = carrier.leds.Color(0, 0, 0);

const float THRES_ACC = 0.10;
const int THRES_GY = 100;
const int RIGHT_LED = 1;
const int LEFT_LED = 3;

// do re mi
int melody[] = {
  NOTE_C4, NOTE_D4, NOTE_E4
};


float xval[100] = {0};
float yval[100] = {0};
float zval[100] = {0};
float xavg, yavg, zavg;

int steps = 0;
int flag = 0;

/*
 * Calibrate accelerometer
 */
void calibrate() {
  float x, y, z;
  float xsum = 0;
  float ysum = 0;
  float zsum = 0;
  
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
  xavg = xsum / 100.0;
  yavg = ysum / 100.0;
  zavg = zsum / 100.0;
  
  Serial.print(xavg + '\t');
  Serial.print(yavg + '\t');
  Serial.println(zavg);
}

/*
 * Turning tones
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
 * Setup
 */
void setup() {
  CARRIER_CASE = false;
  carrier.begin();

  carrier.display.fillScreen(0x0000);
  carrier.display.setRotation(0);
  carrier.display.setTextWrap(true);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setTextSize(3);

  calibrate();
}


void loop() {
  float ax, ay, az;
  float gx, gy, gz;

  float totvect[100] = {0};
  float totave[100] = {0};
  float xaccl[100] = {0};
  float yaccl[100] = {0};
  float zaccl[100] = {0};
  
  for (int a = 0; a < 100; a++){
    carrier.IMUmodule.readGyroscope(gx, gy, gz);
    carrier.IMUmodule.readAcceleration(ax, ay, az);
    xaccl[a] = ax;
    yaccl[a] = ay;
    zaccl[a] = az;
        
    Serial.print("ax ");
    Serial.print(ax);
    Serial.print(" ay ");
    Serial.print(ay);
    Serial.print(" az ");
    Serial.println(az);
    
    /* 
     * Get the total acceleration vector by taking the square root of X, Y, and Z-axis values
     * then calculate the average of the maximum and minimum acceleration vector values
     */
    totvect[a] = sqrt(((xaccl[a] - xavg) * (xaccl[a] - xavg)) + ((yaccl[a] - yavg) * (yaccl[a] - yavg)) + ((zval[a] - zavg) * (zval[a] - zavg)));
    totave[a] = (totvect[a] + totvect[a - 1]) / 2 ;
    Serial.print("totave[a]\t");
    Serial.println(totave[a]);

    
    // Check if update step according to threshold
    // If acceleration vector greater than the threshold & flag is down, increase the step count and raise the flag
    if (totave[a] > THRES_ACC && flag == 0){
      steps = steps + 1;
      flag = 1;
    }
    // If less than the threshold & flag is up, increase the step count and raise the flag
    if (totave[a] < THRES_ACC && flag == 1) {
      flag = 0;
    }
    
    // Check if turning
    // LEFT
    if (gy > THRES_GY){
      carrier.leds.setPixelColor(LEFT_LED, COLOR_BLUE);
      carrier.leds.show();
      carrier.display.setCursor(80, 130);
      carrier.display.setTextSize(3);
      carrier.display.print(">>>");

      play_melody('L');
      
      carrier.leds.setPixelColor(LEFT_LED, NO_COLOR);
      carrier.leds.show();
    } 
    // RIGHT
    else if (gy < -THRES_GY) {
      carrier.leds.setPixelColor(RIGHT_LED, COLOR_BLUE);
      carrier.leds.show();
      carrier.display.setCursor(80, 130);
      carrier.display.setTextSize(3);
      carrier.display.print("<<<");

      play_melody('R');

      carrier.leds.setPixelColor(RIGHT_LED, NO_COLOR);
      carrier.leds.show();
    }
    
    /* 
     * Display step count
     */
    Serial.println('\n');
    Serial.print("steps: ");
    Serial.println(steps);

    carrier.display.fillScreen(0x0000);
    carrier.display.setCursor(50, 80);
    carrier.display.setTextColor(0xFFFF);
    carrier.display.setTextSize(3);
    carrier.display.print("Steps: ");
    carrier.display.println(steps);

    delay(100);
  }
}