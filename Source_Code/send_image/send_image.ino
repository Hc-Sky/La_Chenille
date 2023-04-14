#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

const int chipSelect = 4;


void setup() {
  Serial.begin(9600);
  Serial2.begin(19200);
  SD.begin(chipSelect);

}

void loop() {
  File file = SD.open("pic00.jpg", FILE_READ);

  if (file) {
    Serial.println("file exist");
    Serial2.print("image");
    while (file.available()) {
      byte data = file.read();
      Serial.println("encore des donnée a envoyer");
      Serial2.print(data);
    }
  }
  Serial.println("Donner entièrement transmise");
  file.close();
}
}
