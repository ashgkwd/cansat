// Guideline Articles URL:
// http://embedded-lab.com/blog/bmp180/
// https://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <BMP180.h>
#include <Wire.h>
#include <SPI.h>
#include "RF24.h"

BMP180 barometer;
float seaLevelPressure = 101000; // pascals

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/

byte addresses[][6] = {"1Node","2Node"};

// Used to control whether this node is sending (1) or receiving (0)
bool role = 1;

void setup() {
  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to transmit OR 'R' to receive"));
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_MAX);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  
  // Start the radio listening for data
  radio.startListening();

  // We start the I2C on the Arduino for communication with the BMP180 sensor.
   Wire.begin();
   // We create an instance of our BMP180 sensor.
   barometer = BMP180();
   // We check to see if we can connect to the sensor.
   if(barometer.EnsureConnected())
   {
   Serial.println("Connected to BMP180."); // Output we are connected to the computer.
   // When we have connected, we reset the device to ensure a clean start.
   barometer.SoftReset();
   // Now we initialize the sensor and pull the calibration data.
   barometer.Initialize();
   }
   else
   { 
   Serial.println("No BMP180 sensor found.");
   }
}

void loop() {
  
/****************** Ping Out Role ***************************/  
if (role == 1)  {
    
    radio.stopListening();                                    // First, stop listening so we can talk.


    if(barometer.IsConnected)
    {
      Serial.println(F("Now sending"));
      float start_time = 0;
      float start_time_p = 1;
      
      float currentPressureP = 0;
      float currentPressureP_p = 2;

      float altitudem = 0;
      float altitudem_p = 3;

      float currentTemperatureC = 0;
      float currentTemperatureC_p = 4;

      // Retrive the current pressure in Pascals.
      currentPressureP = barometer.GetPressure();
      Serial.print("Pressure: ");
      Serial.println(currentPressureP);
      // Retrive the current altitude (in meters). Current Sea Level Pressure is required for this.
      altitudem = barometer.GetAltitude(seaLevelPressure);
      Serial.print("Altitude: ");
      Serial.println(altitudem);
      // Retrive the current temperature in degrees celcius.
      currentTemperatureC = barometer.GetTemperature();
      Serial.print("Temperature: ");
      Serial.print(currentTemperatureC);
      Serial.write(176);

      Serial.println();

      start_time = micros();                             // Take the time, and send it.  This will block until complete
      if ( !radio.write( &start_time_p, sizeof(float)) && !radio.write( &start_time, sizeof(float) )){
       Serial.println(F("failed Time"));
      }

      if(!radio.write( &currentTemperatureC_p, sizeof(float)) && !radio.write( &currentPressureP, sizeof(float) )) {
      Serial.println(F("failed Pressure"));
      }

      if(!radio.write( &altitudem_p, sizeof(float)) && !radio.write( &altitudem, sizeof(float) )) {
      Serial.println(F("failed Altitude"));
      }

      if(!radio.write( &currentTemperatureC_p, sizeof(float)) && !radio.write( &currentTemperatureC, sizeof(float) )) {
      Serial.println(F("failed Temperature"));
      }
          
      radio.startListening();                                    // Now, continue listening
      
      unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
      boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
      
      while ( ! radio.available() ){                             // While nothing is received
        if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
            timeout = true;
            break;
        }      
      }
          
      if ( timeout ){                                             // Describe the results
          Serial.println(F("Failed, response timed out."));
      }else{
          unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew
          radio.read( &got_time, sizeof(unsigned long) );
          unsigned long end_time = micros();
          
          // Spew it
          Serial.print(F("Sent "));
          Serial.print(start_time);
          Serial.print(F(", Got response "));
          Serial.print(got_time);
          Serial.print(F(", Round-trip delay "));
          Serial.print(end_time-start_time);
          Serial.println(F(" microseconds"));
      }

      // Try again 1s later
      delay(1000);
    } // end barometer if
    
  } // end role==1 if



/****************** Pong Back Role ***************************/

  if ( role == 0 )
  {
    float got_resp;
    float got_time;
    boolean is_time = false;
    
    if( radio.available()){
                                                                    // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( &got_resp, sizeof(float) );             // Get the payload
        if(got_resp == 1) {
          is_time = true;
        }
        radio.read( &got_time, sizeof(float));
        Serial.print(F("Got timestamp "));
        Serial.println(got_time);
      }
     
      radio.stopListening();                                        // First, stop listening so we can talk   
      radio.write( &got_time, sizeof(float) );              // Send the final one back.      
      radio.startListening();                                       // Now, resume listening so we catch the next packets.     
      Serial.print(F("Got response "));
      Serial.println(got_resp);  
   }
 }




/****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ){      
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1;                  // Become the primary transmitter (ping out)
    
   }else
    if ( c == 'R' && role == 1 ){
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));      
       role = 0;                // Become the primary receiver (pong back)
       radio.startListening();
       
    }
  }


} // Loop
