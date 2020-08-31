#include <Arduino.h>
  
#include <TMCStepper.h>

#define EN_PIN           D2 // Enable
#define DIR_PIN          D1 // Direction
#define STEP_PIN         D0 // Step
#define SERIAL_PORT Serial // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS 0b00 // TMC2209 Driver address according to MS1 and MS2

#define R_SENSE 0.11f // Match to your driver
                      // SilentStepStick series use 0.11
                      // UltiMachine Einsy and Archim2 boards use 0.2
                      // Panucatt BSD2660 uses 0.1
                      // Watterott TMC5160 uses 0.075

// Select your stepper driver type

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);

#define FULL_STEPS_PER_REV 200 * 16 ///200 stepper motor steps * 16:1 gear reduction

void setup() {
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);      // Enable driver in hardware

  SERIAL_PORT.begin(115200);      // HW UART drivers

  driver.begin();
                                  
  driver.toff(5);                 // Enables driver in software
  driver.rms_current(800);        // Set motor RMS current
      

  driver.en_spreadCycle(true);   // Toggle spreadCycle on TMC2208/2209/2224
  driver.pwm_autoscale(true);     // Needed for stealthChop

  
  
}

bool shaft = false;
int microsteps[] = {1, 16, 32, 64, 128, 256};
void loop() {
  for (size_t ustep = 0; ustep < 6; ustep++)
  {
    driver.microsteps(microsteps[ustep]);    
    

    int step_period_us = 80;

    for (int i = FULL_STEPS_PER_REV * microsteps[ustep]; i>0; i--) {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(step_period_us);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(step_period_us);

      yield(); //avoid the WDT
      if (i % 1000 == 0)
      {
        Serial.printf("uSteps: %d ", microsteps[ustep]);
        Serial.printf("i = %d out of %d\n", i, FULL_STEPS_PER_REV * microsteps[ustep]);
      }
      
    }
    shaft = !shaft;
    //Serial.print("Shaft switched ");
    //Serial.println(shaft);
    driver.shaft(shaft);
    //digitalWrite(DIR_PIN, shaft);
    digitalWrite(LED_BUILTIN, shaft);
  }

  

}