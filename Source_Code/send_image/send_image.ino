void envoi.image() {
  int x = 0;
  while (1) {
    File file = SD.open("pic0" + String(x) + ".jpg", FILE_READ);
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
    x++;

    if (Serial2.readStringUntil('\n') == "ACK"){
      break;
    }
  }
}
