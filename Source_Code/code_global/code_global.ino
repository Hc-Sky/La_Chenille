#include <Servo.h>
#include <SPI.h>
#include <arduino.h>
#include <SD.h>

// Pins for the ultrasonic sensor
#define Broche_Echo 34 // Echo pin of HC-SR04 on pin 52
#define Broche_Trigger 35 // Trigger pin of HC-SR04 on pin 53

#define PIC_PKT_LEN    128                  // Data length of each read; don't set this too big because RAM is limited
#define PIC_FMT_VGA    7
//#define PIC_FMT_CIF    5
//#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define CAM_SERIAL     Serial

#define PIC_FMT        PIC_FMT_VGA

File myFile;

// Camera variables
const byte cameraAddr = (CAM_ADDR << 5);
unsigned long picTotalLen = 0;
int picNameNum = 0;

// Variables
int MesureMaxi = 50; // Maximum distance to measure
int MesureMini = 7; // Minimum distance to measure
int vitesse;
byte angle_depart = 103, angle = 33;
long Duree;
long Distance, preDistance;
unsigned long tmes, tdebug, t1, tenvoi;
int pwmPinB = 11, pwmPinA = 3, BrkPinB = 8, BrkPinA = 9, directionPinB = 13, directionPinA = 12, n = 0, xco, yco;
byte dir = 1, tour_mouv, nb_dist = 0, presence_obstacle;
bool BrkA, BrkB, directionA, directionB, prise_photo = 0;
float coeff;
Servo Servo_el;
Servo Servo_az;

// Initialize the program
void setup()
{
  // Ultrasonic sensor pins setup
  pinMode(Broche_Trigger, OUTPUT);
  pinMode(Broche_Echo, INPUT);

  // DC motor pins setup
  pinMode(directionPinA, OUTPUT);
  pinMode(directionPinB, OUTPUT);
  pinMode(BrkPinA, OUTPUT);
  pinMode(BrkPinB, OUTPUT);
  pinMode(pwmPinA, OUTPUT);
  pinMode(pwmPinB, OUTPUT);

  // Serial communication setup
  Serial.begin(19200); // Serial monitor
  Serial2.begin(19200); // Xbee
  Serial1.begin(115200);  // Camera

  // SD card setup
  pinMode(4, OUTPUT); // CS pin of SD card shield
  Serial.println("Initializing SD card....");
  if (!SD.begin(4))
  {
    Serial.print("//sd init failed");
    while (1);
  }
  Serial.println("sd init done.");

  initialize(); // Call the initialize function

  // Servomotors setup
  Servo_el.attach(5);
  Servo_az.attach(6);
  Servo_el.write(0);
  Servo_az.write(103);
}


void loop() {
  decodage_trame();
  detec_obstacle();
  commande_moteur();
  commande_camera();
}

void decodage_trame() {
  if (Serial2.available() > 0 && !presence_obstacle) {
    String trame = Serial2.readStringUntil('\n');
    Serial.println(trame);
    if (trame.indexOf("v") > -1) {
      vitesse = trame.substring(trame.indexOf("v") + 1, trame.indexOf("/")).toInt();
    }
    if (trame.indexOf("d") > -1) {
      dir = trame.substring(trame.indexOf("d") + 1, trame.indexOf("/")).toInt();
    }
    //Serial.println(String(vitesse) + String(dir));

    if (trame.indexOf("x") > -1) {
      xco = trame.substring(trame.indexOf("x") + 1, trame.indexOf("/")).toInt();
    }
    if (trame.indexOf("y") > -1) {
      yco = trame.substring(trame.indexOf("y") + 1, trame.indexOf("/")).toInt();
    }
    if (trame.indexOf("i") > -1) {
      man_pic();
    }
  }
}
void detec_obstacle() {
  if (millis() - tmes > 200) {
    // Debut de la mesure avec un signal de 10 µS applique sur TRIG //
    digitalWrite(Broche_Trigger, LOW); // On efface l'etat logique de TRIG //
    delayMicroseconds(2);
    digitalWrite(Broche_Trigger, HIGH); // On met la broche TRIG a "1" pendant 10µS //
    delayMicroseconds(10);
    digitalWrite(Broche_Trigger, LOW); // On remet la broche TRIG a "0" //
    // On mesure combien de temps le niveau logique haut est actif sur ECHO //
    Duree = pulseIn(Broche_Echo, HIGH);
    // Calcul de la distance grace au temps mesure //
    Distance = Duree * 0.034 / 2; // *** voir explications apres l'exemple de code *** //

    if (Distance <= MesureMaxi && Distance >= MesureMini) {
      Serial2.println("d" + String(Distance) + "/");
      Serial.println("d" + String(Distance) + "/");
      if (Distance <= 29) {
        presence_obstacle = presence_obstacle + 1;
        vitesse = 0;
        BrkA = 1;
        BrkB = 1;

        if (presence_obstacle >= 3 && !prise_photo) {
          picNameNum = 0;
          n = 0;
          tour_mouv = 0;
          prise_photo = 1;
          tourelle();
        }
      }
    }
    if (Distance >= MesureMaxi || Distance <= MesureMini) {
      nb_dist = nb_dist + 1;
      if (nb_dist == 3) {
        Serial2.println("d-1/");
        //Serial.println("d" + String(Distance) + "/");
        nb_dist = 0;
      }
      if (Distance > 30) {
        Servo_el.write(0);
        Servo_az.write(angle_depart);
        presence_obstacle = 0;
        prise_photo = 0;
      }


    }
    tmes = millis();

    /*if (dir == 255 || presence_obstacle && prise_photo == 0) {
      tourelle();
      }*/

  }
  //Verif Debug de la distance dans le moniteur série toutes les 500 ms
  /* if (millis() - tdebug > 500) {
     Serial.println("dist:" + String(Distance));
     Serial.println("obs:" + String(presence_obstacle));
     //Serial.println(String(Dist));
     tdebug = millis();
    }*/
}

void tourelle() {
  //Serial.println("t1:" + String(t1));
  //Serial.println("millis():" + String(millis()));
  Serial2.println("//obstacle");
  // First photo at 45 degrees

  if (tour_mouv == 0) {
    Servo_az.write(angle_depart);
    Servo_el.write(90);
    delay(500);
    if (n == 0) preCapture();
    Capture();
    GetData();
    //Serial2.print("//Taking pictures success ,number : ");
    //Serial2.println(String(n) + "\\");
    Serial.print("//Taking pictures success ,number : ");
    Serial.println(String(n) + "\\");
    tour_mouv = 1;
    n++ ;
  }

  //if (SD.exists(picName) && tour_mouv == 1) {
  // Second photo at 0 degrees
  if (tour_mouv == 1) {
    Servo_az.write(angle_depart - angle);
    delay(500);
    if (n == 1) preCapture(); {
      Capture();
      GetData();
    }
    //Serial2.print("//Taking pictures success ,number : ");
    //Serial2.println(String(n) + "\\");
    Serial.print("//Taking pictures success ,number : ");
    Serial.println(String(n) + "\\");
    tour_mouv = 2;
    n++ ;
  }

  //if (SD.exists(picName) && tour_mouv == 2) {
  // Third photo at 135 degrees
  if (tour_mouv == 2) {
    Servo_az.write(angle_depart + angle);
    delay(500);
    if (n == 2) preCapture();
    Capture();
    GetData();
    //Serial2.print("//Taking pictures success ,number : ");
    //Serial2.println(String(n) + "\\");
    Serial.print("//Taking pictures success ,number : ");
    Serial.println(String(n) + "\\");
    tour_mouv = 3;
    n++ ;
  }
  if (tour_mouv == 3) {
    Servo_az.write(angle_depart + angle - 1);
    Servo_el.write(64);
    delay(500);
    if (n == 3) preCapture();
    Capture();
    GetData();
    //Serial2.print("//Taking pictures success ,number : ");
    //Serial2.println(String(n) + "\\");
    Serial.print("//Taking pictures success ,number : ");
    Serial.println(String(n) + "\\");
    tour_mouv = 4;
    n++ ;
  }
  if (tour_mouv == 4) {
    Servo_az.write(angle_depart);
    delay(500);
    if (n == 3) preCapture();
    Capture();
    GetData();
    //Serial2.print("//Taking pictures success ,number : ");
    //Serial2.println(String(n) + "\\");
    Serial.print("//Taking pictures success ,number : ");
    Serial.println(String(n) + "\\");
    tour_mouv = 5;
    n++ ;
  }
  if (tour_mouv == 5) {
    Servo_az.write(angle_depart - angle + 1);
    delay(500);
    if (n == 3) preCapture();
    Capture();
    GetData();
    //Serial2.print("//Taking pictures success ,number : ");
    //Serial2.println(String(n) + "\\");
    Serial.print("//Taking pictures success ,number : ");
    Serial.println(String(n) + "\\");
    tour_mouv = 6;
    n++ ;
  }
  if (tour_mouv == 6) {
    Servo_el.write(0);
    Servo_az.write(angle_depart);
    envoi_image();
    //recul();
  }
}

void envoi_image() {
  Serial2.print("image/");
  Serial2.print('\r');
  Serial.println("envoi de l'image");
  int x = 0;
  while (x < 6) {
    File file = SD.open("pic0" + String(x) + ".jpg", FILE_READ);
    if (file) {
      Serial.println("file exist");
      while (file.available()) {
        Serial2.write(file.read());
      }
      Serial.println("Données entièrement transmises");
      file.close();
      delay(500);
    }
    x++;
  }
  Serial.print("fin de transmission");
  recul();
}

void commande_moteur() {
  if (!presence_obstacle) {
    BrkA = 0;
    BrkB = 0;
    if (dir == 1) {
      directionA = 0;
      directionB = 1;
    }
    if (dir == 3) {
      directionA = 1;
      directionB = 1;
    }
    if (dir == 2) {
      directionA = 0;
      directionB = 0;
    }
    if (dir == 0) {
      directionA = 1;
      directionB = 0;
    }
  }
  digitalWrite(BrkPinA, BrkA);
  digitalWrite(BrkPinB, BrkB);
  digitalWrite(directionPinA, directionA);
  digitalWrite(directionPinB, directionB);
  analogWrite(pwmPinA, vitesse);
  analogWrite(pwmPinB, vitesse);
}

/*if (millis() - tdebug > 1000) {
  Serial.println("vitesse :" + String(vitesse));
  Serial.println("presence obstacle :" + String(presence_obstacle));
  Serial.println("dirB :" + String(directionB));
  Serial.println("dirA :" + String(directionA));
  Serial.println("BrB :" + String(BrkB));
  Serial.println("BrA :" + String(BrkA));
  tdebug = millis();
  }*/


void recul() {
  digitalWrite(BrkPinA, 0);
  digitalWrite(BrkPinB, 0);
  digitalWrite(directionPinA, 1);
  digitalWrite(directionPinB, 0);
  analogWrite(pwmPinA, 100);
  analogWrite(pwmPinB, 100);
  delay(2500);
  analogWrite(pwmPinA, 0);
  analogWrite(pwmPinB, 0);
  vitesse = 0;
  Serial2.println("//recul terminer \\");
}

void commande_camera() {
  if (Distance > 30) {
    Servo_az.write(angle_depart - xco);
    Servo_el.write(90 - yco);
  }
}

void man_pic() {
  digitalWrite(BrkPinA, 1);
  digitalWrite(BrkPinB, 1);
  analogWrite(pwmPinA, 0);
  analogWrite(pwmPinB, 0);
  vitesse = 0;
  picNameNum = 6;
  preCapture();
  Capture();
  GetData();
  delay(1000);
  bool x = 0;
  Serial2.print("image1/");
  Serial2.print('\r');
  while (!x) {
    File file = SD.open("pic06.jpg", FILE_READ);
    if (file) {
      Serial.println("file exist");
      while (file.available()) {
        Serial2.write(file.read());
      }
      Serial.println("Données entièrement transmises");
      file.close();
      delay(1000);
    }
    x++;
  }
}
void clearRxBuf()
{
  while (Serial1.available())
  {
    Serial1.read();
  }
}
/*********************************************************************/
void sendCmd(char cmd[], int cmd_len)
{
  for (char i = 0; i < cmd_len; i++) Serial1.print(cmd[i]);
}
/*********************************************************************/
void initialize()
{
  char cmd[] = {0xaa, 0x0d | cameraAddr, 0x00, 0x00, 0x00, 0x00} ;
  unsigned char resp[6];

  Serial1.setTimeout(500);
  while (1)
  {
    //clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial1.readBytes((char *)resp, 6) != 6)
    {
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0)
    {
      if (Serial1.readBytes((char *)resp, 6) != 6) continue;
      if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0) break;
    }
  }
  cmd[1] = 0x0e | cameraAddr;
  cmd[2] = 0x0d;
  sendCmd(cmd, 6);
  Serial.println("\nCamera initialization done.");
}
/*********************************************************************/
void preCapture()
{
  char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };
  unsigned char resp[6];

  Serial1.setTimeout(100);
  while (1)
  {
    //Serial2.println("boucle precapture");
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial1.readBytes((char *)resp, 6) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break;
  }
}
void Capture()
{
  char cmd[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN >> 8) & 0xff , 0};
  unsigned char resp[6];

  Serial1.setTimeout(100);
  while (1)
  {
    //Serial2.println("boucle capture 1");
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial1.readBytes((char *)resp, 6) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x05 | cameraAddr;
  cmd[2] = 0;
  cmd[3] = 0;
  cmd[4] = 0;
  cmd[5] = 0;
  while (1)
  {
    //Serial2.println("boucle capture 2");
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial1.readBytes((char *)resp, 6) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x04 | cameraAddr;
  cmd[2] = 0x1;
  while (1)
  {
    //Serial2.println("boucle capture 3");
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial1.readBytes((char *)resp, 6) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
    {
      Serial1.setTimeout(1000);
      if (Serial1.readBytes((char *)resp, 6) != 6)
      {
        continue;
      }
      if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
      {
        picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16);
        Serial1.print("picTotalLen:");
        Serial1.println(picTotalLen);
        break;
      }
    }
  }

}
/*********************************************************************/
void GetData()
{
  unsigned int pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6);
  if ((picTotalLen % (PIC_PKT_LEN - 6)) != 0) pktCnt += 1;

  char cmd[] = { 0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00 };
  unsigned char pkt[PIC_PKT_LEN];

  char picName[] = "pic00.jpg";
  picName[3] = picNameNum / 10 + '0';
  picName[4] = picNameNum % 10 + '0';

  if (SD.exists(picName))
  {
    SD.remove(picName);
  }

  myFile = SD.open(picName, FILE_WRITE);
  if (!myFile) {
    Serial.println("myFile open fail...");
  }
  else {
    Serial1.setTimeout(1000);
    for (unsigned int i = 0; i < pktCnt; i++)
    {
      cmd[4] = i & 0xff;
      cmd[5] = (i >> 8) & 0xff;

      int retry_cnt = 0;
retry:
      delay(10);
      clearRxBuf();
      sendCmd(cmd, 6);
      uint16_t cnt = Serial1.readBytes((char *)pkt, PIC_PKT_LEN);

      unsigned char sum = 0;
      for (int y = 0; y < cnt - 2; y++)
      {
        sum += pkt[y];
      }
      if (sum != pkt[cnt - 2])
      {
        if (++retry_cnt < 100) goto retry;
        else break;
      }

      myFile.write((const uint8_t *)&pkt[4], cnt - 6);
      //if (cnt != PIC_PKT_LEN) break;
    }
    cmd[4] = 0xf0;
    cmd[5] = 0xf0;
    sendCmd(cmd, 6);
  }
  myFile.close();
  picNameNum ++;
}
