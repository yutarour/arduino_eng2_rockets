/*
   Controler side code by Yutaro
    1:arm rocket
    2: disarm rocket
    3:turn on debug
    4:test deploy/deploy when not armed
    5:transferr all data
    6:crreate new file
    7:calibrate baseline pressure
*/
#include <DigitalIO.h>
#include <SD.h>
#include<RF24.h>
#include <LiquidCrystal_I2C.h>

//deploy_pin,arm_pin,data_transfer_pin,makefile,baseline_pin
int buttons[] = {4, 5, 6, 10, 11};
LiquidCrystal_I2C lcd(0x27, 16, 2);

File backupfile;
RF24 radio(2, 3);

//rocket status vars
bool armed = false;

//data1 aka receiving data
struct data1 {
  float alt;
  float pres;
  double flight_time;
  int tem;
  float ppm;
  int current_stat; //0= not ready 1=ready 2=transfer data ongoing 3= file transfer done 4=setup done
};

//data2 aka sending data
struct data2 {
  int message;
  bool debugmode;
};

data1 r_data;
data2 s_data;

//funcs
void writedata() {
  radio.stopListening();
  radio.write(&s_data, sizeof(s_data));
  radio.startListening();
}

void setup() {
  // put your setup code here, to run once:
  pinMode(buttons[0], INPUT_PULLUP);
  pinMode(buttons[1], INPUT_PULLUP);
  pinMode(buttons[2], INPUT_PULLUP);
  pinMode(buttons[3], INPUT_PULLUP);
  pinMode(buttons[4], INPUT_PULLUP);
  pinMode(buttons[5], INPUT_PULLUP);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.home();
  delay(200);
  lcd.print("Initializing");
  delay(500);
  if (!radio.begin()) {
    //success
    lcd.clear();
    lcd.print("NRF24 init fail");
    delay(500);

  }
  else {
    lcd.clear();
    lcd.print("NRF24 init ok");
    delay(500);
  }

  radio.openWritingPipe(0xffff01);
  radio.openReadingPipe(1, 0xffff02);
  radio.setPALevel(RF24_PA_MAX);
  //channel 35 has the least chatter on it
  radio.setChannel(35);
  radio.setDataRate(RF24_250KBPS);

  //do pinmodes here for buttons and whatnot
}

int filenum = 0;
byte received_data;
int buttonstate[5];

void loop() {
  // put your main code here, to run repeatedly:
  buttonstate[0] = digitalRead(buttons[0]);
  buttonstate[1] = digitalRead(buttons[1]);
  buttonstate[2] = digitalRead(buttons[2]);
  buttonstate[3] = digitalRead(buttons[3]);
  buttonstate[4] = digitalRead(buttons[4]);
  buttonstate[5] = digitalRead(buttons[5]);
  Serial.println(digitalRead(7));
  //deploy
  if (buttonstate[0] == LOW) {
    radio.stopListening();
    delay(100);
    s_data.message = 3;
    Serial.println("HI");
    bool report = radio.write(&s_data, sizeof(s_data));
    if (!report) {
      Serial.println("Could not send");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Deploy fail");
      Serial.println(s_data.message);
    }
    else {
      Serial.println("Com sent");
      radio.startListening();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Deploy");
    }
    delay(500);
  }


  //arm and disarm
  if (buttonstate[1] == LOW) {
    radio.stopListening();
    s_data.message = 2;
    bool report = radio.write(&s_data, sizeof(s_data));

    radio.startListening();
    lcd.clear();
    lcd.setCursor(0, 0);
    if (!report) {
      lcd.print("Failed arm");
    }
    else {
      if (armed == true) {
        lcd.print("Armed");
        armed = false;
      }
      else if (armed == false) {
        lcd.print("Disarmed");
        armed = true;
      }

      delay(800);
    }
  }


  //data transfer pin
  //  if (buttonstate[2] == LOW) {
  //    String buff;
  //    s_data.message = 5;
  //    radio.startListening();
  //    radio.read(&r_data, sizeof(r_data));
  //    File datafile = SD.open("emergency_datalog.txt");
  //    lcd.clear();
  //    lcd.setCursor(0, 0);
  //    lcd.print("Listening for data");
  //    while (r_data.current_stat == 2) {
  //      radio.read(&buff, sizeof(buff));
  //      datafile.print(buff);
  //    }
  //    lcd.print("Receiving data");
  //    delay(500);
  //  }

  if (buttonstate[3] == LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Making new file");
    radio.stopListening();
    s_data.message = 6;
    bool report = radio.write(&s_data, sizeof(s_data));
    if (!report) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Newfile fail");
    }
    radio.startListening();
    delay(800);
  }
  else if (buttonstate[4] == LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Setting baseline");
    radio.stopListening();
    s_data.message = 7;
    bool report = radio.write(&s_data, sizeof(s_data));
    if (!report) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Baseline fail");
    }
    radio.startListening();
    delay(800);
  }

  int lasttime = 0;
  if (millis() - lasttime > 100) {
    radio.startListening();
    if (radio.available()) {
      radio.read(&r_data, sizeof(r_data));
      Serial.print("Pressure: "); Serial.println(r_data.pres);
      Serial.print("Flight_time: "); Serial.println(r_data.flight_time);
      Serial.print("Temp: "); Serial.println(r_data.tem);
      Serial.print("PPM: "); Serial.println(r_data.ppm);
    }
  }
}
