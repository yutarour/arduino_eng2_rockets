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

#include <SD.h>
#include<RF24.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

File backupfile;
RF24 radio(10, 9);

//rocket status vars
bool armed = false;

//data1 aka receiving data
struct data1 {
  int ypr[3];
  float alt;
  float pres;
  int flight_time;
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
  lcd.init();
  lcd.home();
  delay(200);
  if (radio.begin()) {
    //success
    lcd.print("NRF24 init success");
  }
  else {
    lcd.print("NRF24 init fail");
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
void loop() {
  // put your main code here, to run repeatedly:
if (button == HIGH){
  radio.write(&s_data,sizeof(s_data));
  }
}
