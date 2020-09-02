#include <Arduino.h>

#include <TMCStepper.h>

#include <Ticker.h>

#define EN_PIN D2           // Enable
#define DIR_PIN D1          // Direction
#define STEP_PIN D0         // Step
#define SERIAL_PORT Serial  // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS 0b00 // TMC2209 Driver address according to MS1 and MS2

#define R_SENSE 0.11f // Match to your driver                         \
                      // SilentStepStick series use 0.11              \
                      // UltiMachine Einsy and Archim2 boards use 0.2 \
                      // Panucatt BSD2660 uses 0.1                    \
                      // Watterott TMC5160 uses 0.075

// Select your stepper driver type

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);

#define USTEPS 256
#define FULL_STEPS_PER_REV 200 * 16 * USTEPS ///200 stepper motor steps * 16:1 gear reduction

Ticker step_pulse_gen;

bool shaft = false;
typedef enum
{
  POSEDGE,
  NEGEDGE
} edge_dir_t;

volatile edge_dir_t next_edge_dir = POSEDGE;
int step_period_us = 80;
int current_pos = 0;
int set_pos = 0;
int shaft_dir = 0;

void step()
{
  if (next_edge_dir == POSEDGE)
  {
    digitalWrite(STEP_PIN, HIGH);
    next_edge_dir = NEGEDGE;
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
}

void print_stats()
{
  Serial.printf("Set pos: %d Actual Pos: %d\n", set_pos, current_pos);
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

  step_pulse_gen.attach(0.001, update_motor_pos);
}

void loop()
{
  if (Serial.available() > 0)
  {
    int val;
    val = Serial.parseInt();
    Serial.printf("New set point is: %d\n", val);
  }
}