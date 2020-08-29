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
//TMC2130Stepper driver(CS_PIN, R_SENSE);                           // Hardware SPI
//TMC2130Stepper driver(CS_PIN, R_SENSE, SW_MOSI, SW_MISO, SW_SCK); // Software SPI
//TMC2660Stepper driver(CS_PIN, R_SENSE);                           // Hardware SPI
//TMC2660Stepper driver(CS_PIN, R_SENSE, SW_MOSI, SW_MISO, SW_SCK);
//TMC5160Stepper driver(CS_PIN, R_SENSE);
//TMC5160Stepper driver(CS_PIN, R_SENSE, SW_MOSI, SW_MISO, SW_SCK);

//TMC2208Stepper driver(&SERIAL_PORT, R_SENSE);                     // Hardware Serial
//TMC2208Stepper driver(SW_RX, SW_TX, R_SENSE);                     // Software serial
TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
//TMC2209Stepper driver(SW_RX, SW_TX, R_SENSE, DRIVER_ADDRESS);

void setup() {
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);      // Enable driver in hardware

  SERIAL_PORT.begin(115200);      // HW UART drivers

  driver.begin();                 //  SPI: Init CS pins and possible SW SPI pins
                                  // UART: Init SW UART (if selected) with default 115200 baudrate
  driver.toff(5);                 // Enables driver in software
  driver.rms_current(600);        // Set motor RMS current
      

  //driver.en_spreadCycle(false);   // Toggle spreadCycle on TMC2208/2209/2224
  driver.pwm_autoscale(true);     // Needed for stealthChop
}

bool shaft = false;
int microsteps[] = {16, 32, 64, 128, 256};
void loop() {
  for (size_t i = 0; i < 5; i++)
  {
    driver.microsteps(microsteps[i]);    
    //Serial.print("Sel ustep val ");
    //Serial.println(i);
    // Run 5000 steps and switch direction in software
    for (uint16_t i = 10000; i>0; i--) {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(160);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(160);

      yield(); //avoid the WDT
    }
    shaft = !shaft;
    //Serial.print("Shaft switched ");
    //Serial.println(shaft);
    driver.shaft(shaft);
    //digitalWrite(DIR_PIN, shaft);
    digitalWrite(LED_BUILTIN, shaft);
    }
  
  
}