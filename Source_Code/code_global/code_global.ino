#include <Servo.h>
#include <SPI.h>
#include <arduino.h>
#include <SD.h>

#define Broche_Echo 52 // Broche Echo du HC-SR04 sur 52 //
#define Broche_Trigger 53 // Broche Trigger du HC-SR04 sur 53 //

#define PIC_PKT_LEN    128                  //data length of each read, dont set this too big because ram is limited
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define CAM_SERIAL     Serial

#define PIC_FMT        PIC_FMT_VGA

File myFile;


const byte cameraAddr = (CAM_ADDR << 5);  // addr
const int buttonPin = 2;                 // the number of the pushbutton pin
unsigned long picTotalLen = 0;            // picture length
int picNameNum = 0;

// Definition des variables
int MesureMaxi = 50; // Distance maxi a mesurer //
int MesureMini = 7; // Distance mini a mesurer //
int vitesse;
long Duree;
long Distance, preDistance;
unsigned long tmes, tdebug, t1;
int pwmPinB = 11, pwmPinA = 3, BrkPinB = 8, BrkPinA = 9, directionPinB = 13, directionPinA = 12, n = 0;
byte dir = 1, tour_mouv, nb_dist = 0, presence_obstacle;
bool BrkA, BrkB, directionA, directionB, prise_photo = 0;
float coeff;
Servo Servo_el;
Servo Servo_az;

void setup()
{
  pinMode(Broche_Trigger, OUTPUT); // Broche Trigger en sortie //
  pinMode(Broche_Echo, INPUT); // Broche Echo en entree //
  pinMode(directionPinA, OUTPUT);
  pinMode(directionPinB, OUTPUT);
  pinMode(BrkPinA, OUTPUT);
  pinMode(BrkPinB, OUTPUT);
  pinMode(pwmPinA, OUTPUT);
  pinMode(pwmPinB, OUTPUT);
  Serial.begin(19200); // Moniteur Serie
  Serial2.begin(18518); //Xbee
  Serial1.begin(115200);  //cam

  Serial.println("Initializing SD card....");
  pinMode(4, OUTPUT);         // CS pin of SD Card Shield

  if (!SD.begin(4))
  {
    Serial.print("sd init failed");
    return;
  }
  Serial.println("sd init done.");
  initialize();

  Servo_el.attach(5);
  Servo_az.attach(6);
  Servo_el.write(0);
  Servo_az.write(103);
}

void loop() {
  if (Serial2.available() > 0 && !presence_obstacle) {
    decodage_trame();
  }
  Vitesse();
  detec_obstacle();

  commande_moteur();
}

void decodage_trame() {
  String trame = Serial2.readStringUntil('\n');
  Serial.println(trame);
  if (trame.indexOf("v") > -1) {
    vitesse = trame.substring(trame.indexOf("v") + 1, trame.indexOf("/")).toInt();
  }
  if (trame.indexOf("d") > -1) {
    dir = trame.substring(trame.indexOf("d") + 1, trame.indexOf("/")).toInt();
  }
  Serial.println(String(vitesse) + String(dir));
}
void Vitesse() {
  if (Distance <= MesureMaxi && Distance >= MesureMini) {
    coeff = map(constrain(Distance, 30, 40), 30, 40, 0, 1);
  } else {
    coeff = 1;
  }
}
void detec_obstacle() {

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
    if (preDistance != Distance) {
      Serial2.println("d" + String(Distance) + "/");
      preDistance = Distance;
    }
    if (Distance <= 30) {
      presence_obstacle = presence_obstacle + 1;
      vitesse = 0;
      BrkA = 1;
      BrkB = 1;
      
      if (presence_obstacle >= 3) {
        tourelle();
      }
    }
  }
  if (Distance > 30) {
    Servo_el.write(0);
    Servo_az.write(103);
    n = 0;
    presence_obstacle = 0;
    tour_mouv = 0;
  }
  /*if (dir == 255 || presence_obstacle && prise_photo == 0) {
    tourelle();
    }*/
  if (Distance >= MesureMaxi || Distance <= MesureMini) {
    nb_dist = nb_dist + 1;
    if (nb_dist == 3) {
      Serial2.println("d-1/");
      nb_dist = 0;
    }
  }
  //Verif Debug de la distance dans le moniteur série toutes les 500 ms
  /*if (millis() - tmes > 500) {
    Serial.println("dist:" + String(Distance));
    Serial.println("obs:" + String(presence_obstacle));
    //Serial.println(String(Dist));
    tmes = millis();
    }*/
}

void tourelle() {
  //Serial.println("t1:" + String(t1));
  //Serial.println("millis():" + String(millis()));
  //Serial.print("tourelle");
  // First photo at 45 degrees

  if (millis() - t1 > 1000 && tour_mouv == 0) {
    Servo_az.write(103);
    Servo_el.write(90);
    tour_mouv = 1;
    if (n == 0) preCapture();
    Capture();
    GetData();
    Serial.print("\r\nTaking pictures success ,number : ");
    Serial.println(n);
    n++ ;
  }

  // Second photo at 0 degrees
  if (millis() - t1 > 2000 && tour_mouv == 1) {
    Servo_az.write(58);
    tour_mouv = 2;
    if (n == 0) preCapture();
    Capture();
    GetData();
    Serial.print("\r\nTaking pictures success ,number : ");
    Serial.println(n);
    n++ ;
  }

  // Third photo at 135 degrees
  if (millis() - t1 > 3000 && tour_mouv == 2) {
    Servo_az.write(148);
    tour_mouv = 3;
    if (n == 0) preCapture();
    Capture();
    GetData();
    Serial.print("\r\nTaking pictures success ,number : ");
    Serial.println(n);
    n++ ;
  }
  prise_photo = 1;
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
  if (coeff <= 1) {
    analogWrite(pwmPinA, vitesse * coeff);
    analogWrite(pwmPinB, vitesse * coeff);
  } else {
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
}

void recul() {
  BrkA = 0;
  BrkB = 0;
  directionA = 1;
  directionB = 0;
  vitesse = 255;
  delay(1000);
  vitesse = 0;
  prise_photo = 0;
}


void clearRxBuf()
{
  while (Serial1.available())
  {
    Serial.read();
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
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial1.readBytes((char *)resp, 6) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x04 | cameraAddr;
  cmd[2] = 0x1;
  while (1)
  {
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
