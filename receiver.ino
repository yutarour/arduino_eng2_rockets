#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

struct incoming_data {
  int temperature;
  int humidity;
  float ppm;
};

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  radio.begin();
  radio.setChannel(35);
  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_LOW);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();              //This sets the module as receiver
}
incoming_data A;
void loop()
{
  if (radio.available())              //Looking for the data.
  {
    radio.read(&A,sizeof(A));
    lcd.setCursor(0,0);
    lcd.print("temp:"+String(A.temperature)+"C");
    lcd.setCursor(9,0);
    lcd.print("hum:"+String(A.humidity)+"%");
    lcd.setCursor(0,1);
    lcd.print("ppm:"+String(A.ppm));
    Serial.println(A.temperature);
    Serial.println(A.humidity);
    Serial.println(A.ppm);
  }

}
