#include <SD.h>

/*
   sd card stuff
   this contains logging and new file creation functions for easy logging.
   data.txt is technically in a csv file format. Make sure to edit the headers to match
   what is being logged in the progream.
*/

int filenumber = 0;
const int chipSelect = 4;
File datafile;

void createnewfile() {
  while (SD.exists(String(filenumber))){
    filenumber++;
    }
   datafile = SD.open(String(filenumber), FILE_WRITE);
   datafile.println("Insert csv headers here");
}

void setup() {
  // put your setup code here, to run once:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
//insert data logging functions here.
}
