#include <Arduino.h>

#include <TMCStepper.h>

#include <Ticker.h>

#include "web_ui.h"

#define EN_PIN D2           // Enable
#define DIR_PIN D1          // Direction
#define STEP_PIN D0         // Step
#define SERIAL_PORT Serial  // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS 0b00 // TMC2209 Driver address according to MS1 and MS2

#define R_SENSE 0.11f // Match to your driver

// Select your stepper driver type

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);

#define USTEPS 256
#define FULL_STEPS_PER_REV 200 * 16 * USTEPS //200 stepper motor steps * 16:1 gear reduction

Ticker step_pulse_gen;
Ticker stat_man;
web_ui * ui;

const float MY_ARCSEC_PER_STEP = float(360 * 60 * 60) / float(FULL_STEPS_PER_REV);

bool shaft = false;
typedef enum
{
  POSEDGE,
  NEGEDGE
} edge_dir_t;

volatile edge_dir_t next_edge_dir = POSEDGE;
int current_pos = 0;
int set_pos = 0;
int shaft_dir = 0;
float step_period = 0;
int prev_step_time_us = 0;
int current_step_time_us = 0;
int actual_step_period_us;

float time_between_steps(float arcsec_per_step, float arsec_per_sec_v){
  return (1/(arsec_per_sec_v)) * arcsec_per_step;
}

//rotation of the earth is 15 arcsec per sec
float set_motor_period_s = time_between_steps(MY_ARCSEC_PER_STEP, float(15));

void step()
{
  
  if (next_edge_dir == POSEDGE)
  {
    digitalWrite(STEP_PIN, HIGH);
    next_edge_dir = NEGEDGE;

    current_step_time_us = micros();
    actual_step_period_us = current_step_time_us - prev_step_time_us;
    prev_step_time_us = current_step_time_us;
  }
  else
  {
    digitalWrite(STEP_PIN, LOW);
    next_edge_dir = POSEDGE;
  }
}

void update_motor_pos()
{
  if (current_pos != set_pos) //do a step
  {
    int shaft_dir = current_pos < set_pos ? 1 : 0;
    driver.shaft(shaft_dir);
    step();
    if (shaft_dir)
    {
      current_pos++;
    }
    else
    {
      current_pos--;
    }
  }
  //make it chase its own tail for now
  set_pos++;
}



void print_stats()
{
  Serial.printf("Set pos: %d Actual Pos: %d\n", set_pos, current_pos);
  Serial.printf("Set step period: %f Actual step period us: %d\n", set_motor_period_s, actual_step_period_us);
}

void setup()
{
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW); // Enable driver in hardware

  SERIAL_PORT.begin(115200); // HW UART drivers

  driver.begin();

  driver.toff(5);           // Enables driver in software
  driver.rms_current(1000); // Set motor RMS current

  driver.en_spreadCycle(true); // Toggle spreadCycle on TMC2208/2209/2224
  driver.pwm_autoscale(true);  // Needed for stealthChop
  driver.microsteps(USTEPS);

  //each pulse is one edge, so call func twice as fast as period (DDR step gen? :P )
  step_pulse_gen.attach(set_motor_period_s/2, update_motor_pos);
  stat_man.attach(0.1, print_stats);
}

void loop()
{
  /*
  if (Serial.available() > 0)
  {
    int val;
    val = Serial.parseInt();
    Serial.printf("New set point is: %d\n", val);
    set_pos = val;
  }
  */
}