/*
   BMP180 altitude and pressure reading example
*/

#include <SFE_BMP180.h>
#include <Wire.h>

double baseline;

SFE_BMP180 pressure;

void setup() {
  // put your setup code here, to run once:
  if (pressure.begin()) {
    //success
  }
  //take a baseline measurement
  baseline = getPressure();
}

//pressure function
double getPressure()
{
  char status;
  double T, P, p0, a;
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
        delay(status);

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
        }

      }
    }
  }
}




void loop() {
  double a,P,T;
  // put your main code here, to run repeatedly:
  //a is altitude P is pressure
  P = getPressure();
  a = pressure.altitude(P,baseline);
  pressure.getTemperature(T);
}
