/*
   Main rocket progeam.
   This code is for putting on the rocket.
   No copy this code.
   command list also lives up here:
   to rocket:
    1:arm rocket
    2: disarm rocket
    3:turn on debug
    4:test deploy/deploy when not armed
    5:transferr all data
    6:crreate new file
    7:calibrate baseline pressure
*/
#include <DigitalIO.h>
//debug
#define debug true

#define chipSelect 4

#define nrf_ce 2
#define nrf_csn 3

//imports
#include "Wire.h"

//sd
#include <SD.h>

//mq135
#include <MQ135.h>

//rf
#include<RF24.h>

//servo
#include <Servo.h>

//bmp
#include <SFE_BMP180.h>

//variables and definitions and pins
//servo
#define servopin 8
#define servo_rot 90
#define servo_init 0

const char* comma = ",";

//rocket status vars
int done_setups = 0;
int arm_time;

//pressure
double baseline;

//mq135
#define mq135_pin A0

//sd
int filenumber = 0;
//sensor reading values
//data1 aka the sending data
struct data1 {
  float alt;
  float pres;
  int flight_time;
  int tem;
  float ppm;
  int current_stat; //0= not ready 1=ready 2=transfer data ongoing 3= file transfer done 4=setup done
};

//data2 aka receive data aka control data
struct data2 {
  int message;
  bool debugmode;
};

//initializing the structures


//initializing vars
RF24 radio(nrf_ce, nrf_csn);
SFE_BMP180 pressure;
Servo myservo;

//sensors
void takereadings(data1 *pdata) {
  double T;
  pdata->pres = getPressure();
  pdata->alt = pressure.altitude(pdata->pres, baseline);
  pressure.getTemperature(T);
  pdata->tem = T;
  MQ135 mq_135(mq135_pin);
  pdata->ppm = mq_135.getCorrectedPPM(T, pdata->tem);

  pdata->flight_time = millis() - arm_time;
}

//bmp
float getPressure()
{
  char status;
  double T, P;
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status); //suspicious

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P, T);
        if (status != 0)
        {
          return (P);
          //Serial.print(P*0.0295333727,2);
        }

      }
    }
  }
};

//servo
void deploy() {
  myservo.write(servo_rot);
  delay(200);
  myservo.write(servo_init);
}

//sd
String filename;
void createnewfile() {
  while (true) {
    filename = String(filenumber);
    Serial.println("New file creation func run");
    if (SD.exists(filename))
    {
      filenumber++;
    }
    else
    {
      File datafile = SD.open(filename, FILE_WRITE);
      Serial.print("New file created");
      Serial.println(filename);
      datafile.println("tm,pres,alt,tem,ppm");
      datafile.close();
      return;
    }
  };
}

void setup() {
  Serial.begin(9600);
  createnewfile();
  //gyro comment out serial stuff later
  myservo.attach(servopin);
  myservo.write(servo_init);
  // put your setup code here, to run once:
  if (pressure.begin()) {
    Serial.println("Succcess");
  }
  done_setups++;
  // get expected DMP packet size for later comparison

#ifdef debug
  Serial.println("Setup running");
#endif
  //rf init
  if (radio.begin()) {
    done_setups++;
#ifdef debug
    Serial.println("Radio success");
#endif
  }

  radio.openWritingPipe(0xffff02);
  radio.openReadingPipe(1, 0xffff01);
  radio.setPALevel(RF24_PA_MAX);
  //channel 35 has the least chatter on it
  radio.setChannel(35);
  radio.setDataRate(RF24_250KBPS);

  if (SD.begin(chipSelect)) {
    done_setups++;
#ifdef debug
    Serial.println("Sd card setup done");
#endif
    // don't do anything more:
  };

  if (pressure.begin())
    done_setups++;

}


void printFloat(File *datafile, float fl, bool fComma)
{
  datafile->print(String(fl));

  if (fComma)
    datafile->print(comma);
}

void printInt(File *datafile, int i)
{
  datafile->print(String(i));
  datafile->print(comma);
}

void loop() {
  data1 s_data;
  data2 r_data;
  bool armed = false;

  takereadings(&s_data);
  //send data only if armed is false because this clogs the data collection. when armed rocket will only listen and send nothing.
  if (armed == false) {
    radio.stopListening();
    radio.write(&s_data, sizeof(data1));
    radio.startListening();

    if (radio.available()) {

      radio.read(&r_data, sizeof(r_data));
      if (r_data.message == 1) {
        armed = true;
        arm_time = millis();
        r_data.message = 999; //done to avoid doing same process twice
      }

      if (r_data.message == 3) {
        deploy();
        r_data.message = 999;
      }
      if (r_data.message == 5) {

        r_data.message = 999;
        radio.stopListening();
        s_data.current_stat = 2;
        radio.write(&s_data, sizeof(s_data)); //to notify status of rocket
        File datafile = SD.open(filename);


        while (datafile.available()) {
          char file_content = datafile.read();
          radio.write(&file_content, sizeof(file_content));
        }
        s_data.current_stat = 3; //notify controler transfer done
      }
      if (r_data.message == 6) {
        createnewfile();
        r_data.message = 999;
      }
      if (r_data.message == 7) {
        baseline = getPressure();
        r_data.message = 999;
      }
    }
  }
  //make this int if too big.
  float prevalt;
  while (armed) {
    prevalt = s_data.pres;
    takereadings(&s_data);
    if (s_data.pres - prevalt > 0.5) {
      deploy();
    }
    
    File datafile = SD.open(filename, FILE_WRITE);
    printInt(&datafile, s_data.flight_time);
    printFloat(&datafile, s_data.pres, false);
    printFloat(&datafile, s_data.alt, false);
    printInt(&datafile, s_data.tem);
    printFloat(&datafile, s_data.ppm, true);
    datafile.close();
    radio.read(&r_data, sizeof(r_data));
    if (r_data.message == 3) {
      deploy();
      r_data.message = 999;
    }
    if (r_data.message == 2) {
      armed = false;
      r_data.message = 999;
    }
  }
}
