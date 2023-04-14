#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;

int x = 0;

void setup() {
  Serial.begin(19200);
  Serial2.begin(19400);
  SD.begin(chipSelect);
 
}

void loop() {
  if (Serial.readStringUntil('\n') == "A") {
    int x = 0;
    while (x<=2) {
      File file = SD.open("PIC0" + String(x) + ".jpg", FILE_READ);
      if (file) {
        Serial.println("file " + String(x) + " exist");
        Serial2.println("image/");
        while (file.available()) {
          byte data = file.read();
          //Serial.print(data);
          Serial2.print(data);
        }
      }
      Serial.println("Donner entièrement transmise");
      file.close();
      x++;
    }
    }
}
