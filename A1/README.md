# Assignment 1: Wearable Project
> Wearable system for environmental sensing and feedback

> [PROMPT LINK](https://docs.google.com/document/d/1Xl1YSyWRIVDWvTWeaNu3gSM3fZbOMw0mk-BN9TmgWDI/edit?usp=sharing)


## Week 1

> The goal of this week is two-folded. The technical goal is to set up your microcontroller prototyping environment. The design goal is to sketch out your wearable device (what type of garments, where will the sensors be placed?) 
> ...

### **The Wearable "Sitting is Killing You" Charm**

This project is a wearable belt charm designed to tell people to stand up and move around once in a while, because sitting is killing you.

The device has a accelerometer detecting sitting to standing movements, which keeps track of the last time you stood up and moved your body around. If you've been sitting for too long, say, 2 hours in a row, it vibrates and buzzes like an alarm clock to remind you to stand up and get some exercise. Once you've done that, it quiets down, resets its timer, and waits for the next time it needs to buzz again.


### System Schematics

![System Schematics](./A1_schematics.png)


### Sketch

![Sketch](./A1_w1_sketch.png)


### Test - IMU & LED / Screen Feedback Device


https://user-images.githubusercontent.com/37056925/154743757-f4a42400-125d-4744-a43e-52d72bade187.mp4


https://user-images.githubusercontent.com/37056925/154743764-fa9acede-ac85-498e-b015-f871a4106215.mp4


<br>

## Week 2

> Time to start sewing. Let’s start with a step counter and a smidge of feedback. You will sew the devices into your wearable ecosystem, and record a video of you using the step counter.  Describe and demonstrate the feedback loop in the video.  Remember, this is an additive project, so make sure that you put this together such that everything else can be easily integrated.


### Function demo of step counter

> * Your screen should function as: once a step is detected, the screen displays the total number of steps. For example, when you just start to wear the device, the screen should display “0 steps”. Once you make a step, the screen should display “1 step”. 
> * There are five LEDs on the Opla. These LEDs should function as, when you turn left, the LEDs on the left side should blink. When you turn right, the LEDs on the right side should blink. 
> * Integrate another type of feedback device. 

#### **Functionality Check**

https://user-images.githubusercontent.com/37056925/156268892-28ebf381-d111-47fb-926b-ab48d4354b69.mp4

#### **Wearable Demo**


### A screen recording of the steps being counted on the Arduino serial monitor


https://user-images.githubusercontent.com/37056925/156268877-6811ac54-b0d0-4c50-80ca-f1c0078f6104.mp4


### Arduino code

Arduino code is uploaded to [this directory](./simple_step_counter/).


### A short write up

#### **Accelerometer >>> Steps**

This simple step counter collects data from the accelerometer and gyroscope in its IMU. In setup, a calibration method is called to calibrate the board according to its current position. The `calibrate()` method reads in 100 samples of acceleration data and stores a average value for each axis, which would serve as a reference to 'idle state' of board movements. Then, in each loop, the program reads in accelerometer values of the X, Y and Z-axis.

After getting the 3-axis values, calculate the total **acceleration vector** by taking the square root of the sum of X, Y, and Z-axis's squared value minus the axis's average value. 

$\sqrt{ (x_{acc} - x_{avg})^2 + (y_{acc} - y_{avg})^2 + (z_{acc} - z_{avg})^2}$

This acceleration vector reflects the board's movement. When the board movement is idle, the vector's value is below 0.1, thus I set the threshold of step detection to be **0.1**. Whenever the vector value is above 0.1, a step is detected and the total step count is incremented by 1.

#### **Gyroscope >>> Turning**

Gyroscope data is used to detect turning. With some testing, I found out that the Y-axis value from the gyroscope reflects turning movements horizontal to the ground. *(This is due to the fact that the Opla board is held up vertical to the ground in this design.)* Thus, the Y-axis value is used to detect turning. The threshold value in this case is set to **100** and **-100** in this case.

When a *RIGHT* turn is detected, the LED on the right blinks, and the board buzzer plays the melody "DO RE MI". Similarly when a *LEFT* turn is detected, the LED on the left blinks and the board buzzer plays a "MI RE DO" melody. The notes are specified in the `pitches.h` header file.


### References

* Acceleration detection code used in this project is inspired by [this blog post](https://circuitdigest.com/microcontroller-projects/diy-arduino-pedometer-counting-steps-using-arduino-and-accelerometer) by Ashish Choudhary.




