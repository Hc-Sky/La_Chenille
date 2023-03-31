#include <Servo.h>
#define Broche_Echo 52 // Broche Echo du HC-SR04 sur 52 //
#define Broche_Trigger 53 // Broche Trigger du HC-SR04 sur 53 //
// Definition des variables
int MesureMaxi = 50; // Distance maxi a mesurer //
int MesureMini = 7; // Distance mini a mesurer //
int vitesse;
long Duree;
long Distance, preDistance;
unsigned long tmes, tdebug, t1;
int pwmPinB = 11, pwmPinA = 3, BrkPinB = 8, BrkPinA = 9, directionPinB = 13, directionPinA = 12;
byte dir = 1, tour_mouv ;
bool BrkA, BrkB, directionA, directionB, presence_obstacle, prise_photo=0;
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
  Serial.begin(115200);
  Serial2.begin(115200);
  Servo_el.attach(5);
  Servo_az.attach(6);
  Servo_el.write(90);
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
  coeff = map(Distance, 15, 40, 0, 1);
}
void detec_obstacle() {
  if (millis() - tmes > 500) {
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
    // Verification si valeur mesuree dans la plage //
    Serial.println(String(Distance));
    tmes = millis();
  }

  if (Distance <= MesureMaxi && Distance >= MesureMini) {
    if (preDistance != Distance) {
      Serial2.println("d" + String(Distance) + "/");
      preDistance = Distance;
    }
    if (Distance <= 20 && !presence_obstacle) {
      presence_obstacle = 1;
      vitesse = 0;
      BrkA = 1;
      BrkB = 1;
      t1 = millis();
    }
  }
  else if (preDistance != Distance) {
    Serial2.println("d-1/");
    preDistance = Distance;
  }
  if (Distance > 20) {
    presence_obstacle = 0;
    tour_mouv = 0;
  }
  if (dir == 255 || presence_obstacle && tour_mouv < 3) {
    tourelle();
  }
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
  } if (prise_photo = 1) {
    BrkA = 0;
    BrkB = 0;
    directionA = 1;
    directionB = 0;
    vitesse = 255;
    delay(1000);
    prise_photo = 0;
  }
  digitalWrite(BrkPinA, BrkA);
  digitalWrite(BrkPinB, BrkB);
  digitalWrite(directionPinA, directionA);
  digitalWrite(directionPinB, directionB);
  analogWrite(pwmPinA, vitesse * coeff);
  analogWrite(pwmPinB, vitesse * coeff);
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

void tourelle() {
  //Serial.println("t1:" + String(t1));
  //Serial.println("millis():" + String(millis()));
  //Serial.print("tourelle");
  // First photo at 45 degrees

  if (millis() - t1 > 1000 && tour_mouv == 0) {
    Servo_az.write(0);
    Servo_el.write(45);
    tour_mouv = 1;
  }

  // Second photo at 0 degrees
  if (millis() - t1 > 2000 && tour_mouv == 1) {
    Servo_az.write(90);
    tour_mouv = 2;
  }

  // Third photo at 135 degrees
  if (millis() - t1 > 3000 && tour_mouv == 2) {
    Servo_az.write(135);
    tour_mouv = 3;
  }
  prise_photo = 1;
}
