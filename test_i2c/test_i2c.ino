#include <Wire.h>

#include <stdio.h>
#include <unistd.h>

void setup() {
  printf("%d",Wire.begin(19,18));          // I2C-Bus starten (Standardpins SDA, SCL)
}

void loop() {



  printf("Scanning... \n");
  // fflush(stdout);

  Wire.beginTransmission(0x3C);
  printf("%d",Wire.endTransmission());


  // int nDevices = 0; // Zähler für gefundene Geräte

  // for (byte address = 1; address < 127; address++) { // Adressbereich 0x01 bis 0x7F durchsuchen
  //   //Wire.beginTransmission(address);
  //   byte error = 0;//Wire.endTransmission();

  //   if (error == 0) {
  //     printf("I2C device found at address 0x");
  //     fflush(stdout);
  //     if (address < 16)
  //       printf("0"); // Adressen unter 0x10 korrekt formatieren
  //     printf("%d",address);
  //     printf(" !");
  //     fflush(stdout);

  //     nDevices++;
  //   } else if (error == 4) {
  //     printf("Unknown error at address 0x");
  //     fflush(stdout);
  //     if (address < 16)
  //       Serial.print("0");
  //     printf("%d",address);
  //     fflush(stdout);
  //   }
  // }

  // if (nDevices == 0){
  //   printf("No I2C devices found\n");
  //   fflush(stdout);
  // }
  // else{
  //   printf("done\n");
  //   fflush(stdout);
  // }

  // delay(1000); // 5 Sekunden warten, bevor erneut gescannt wird

}
