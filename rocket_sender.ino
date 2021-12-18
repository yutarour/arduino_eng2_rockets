

/*
   All code is By Yutaro Urata
   Built for Arduino Uno
   All code is shared under the MIT license.
   You are free to modify, share this content
   Please provide attribution.

   notes
   include auto leveling
*/

//importing libraries
#include <Wire.h>
#include <RF24.h>
#include <SFE_BMP180.h>
#include <nRF24L01.h>
#include <DHT.h>
#include <MPU6050.h>
#include <SD.h>
#include <MQ135.h>
#include <Servo.h>
#include <SPI.h>

//defining variables
//pins
#define mq135_pin 0
#define nrf_ce 0
#define nrf_cs 0
#define sd_cs 0
#define servopin 0
//variables
int16_t mpuGx, mpuGy, mpuGz;
const byte adresses[][6] = {"00001", "00002"};
int temperature;
int baseline_pressure;

//failsafe control variables
int baseline = 0;


//initializing the sensors
SFE_BMP180 bmp;
MPU6050 mpu;
File sdFile;
Servo servo;
RF24 radio(nrf_ce, nrf_cs);


struct send_data {
  int gyrox;
  int gyroy;
  int gyroz;
  int alt;
  float pressure;
  int message;//numbers are 0,1,2,3,4,5,6 all assigned to error messages
  /*
     Define the numbers
     0: Baseline temp setting is required
     1:parachute deployed
     2:Sd card begin failed.

  */
  int finished_processes; //var for storing all finished tasks as integers
  float flighttime;
  float temp;
  bool debug;
};

struct receive_data {
  int message;
  /*
     These are codes.
     0: Take a baseline
     1:Test parachute/deploy parachute
  */
  bool debugmode;
};

//create the structs
send_data content;
receive_data received;

void setup() {
  // put your setup code here, to run once:
  //initializing sensors
  bmp.begin();
  Wire.begin();
  mpu.initialize();
  servo.attach(servopin);


  //radio stuff
  radio.begin();
  radio.openWritingPipe(adresses[1]);
  radio.openReadingPipe(1, adresses[0]);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(35);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  //changeing pinmode
  pinMode(sd_cs, OUTPUT);
  if (!SD.begin()) {
    //handling sd card not existant error
    content.message = 2;
    while (1);
  }

}

//functions
void takereadings() {
  double P, a;
  P = readPressure();
  temperature = readTemp();
  content.pressure = P;
  //resolution
  a = bmp.altitude(P, baseline);
  content.alt = (a, 1);
  temperature = ((9.0 / 5.0) * temperature + 32.0, 2);
  content.temp = temperature;
}

void deploy() {
  servo.write(90);
  content.finished_processes = 1;
}

double readPressure()
{
  char status;
  double T, P, p0, a;
  bmp.startTemperature();
  status = bmp.getTemperature(T);
  if (status != 0)
  {
    status = bmp.startPressure(3);
    if (status != 0)
    {
      delay(status);
      status = bmp.getPressure(P, T);
      if (status != 0)
      {
        return (P);
      }
    }
  }
}

double readTemp()
{
  char status;
  double T;
  bmp.startTemperature();
  status = bmp.getTemperature(T);
  if (status != 0)
  {
    return (T);
  }
}

void loop() {
  //processing baseline error
  if (baseline == 0) {
    content.message = 0;
  }
  //send current data
  radio.stopListening();
  radio.write(&content, sizeof(content));
  //receive data if radio is available

  radio.startListening();
  if (radio.available()) {
    radio.read(&received, sizeof(received));
  }

  //handling take baseline command
  if (received.message == 0) {
    baseline_pressure = readPressure();
    content.finished_processes = 0;
  }

  //handling debug
  if (received.debugmode == true) {
    content.debug = true;
    if (received.message == 1) {
      deploy();
      content.finished_processes = 1;
    }
  }

}
