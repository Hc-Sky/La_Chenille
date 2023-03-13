/* Controle de la PWM sur Arduino par liaison série */
import controlP5.*;  // Ajout au projet de la librairie pour l'utilisation de controlP5
import processing.serial.*;// Ajout au projet de la librairie pour l'utilisation de la liason série

ControlP5 cp5;  // créer un objet (cp5) de type ControlP5
Slider sliderPWM;  // créer un objet de type Slider
Knob knobPWM; // créer un objet de type Knob

PImage fond;  // créer un objet de type PImage
Serial monPort;  // créer un objet monPort de la classe Serial

int slider, valeur, pwm, prepwm; // Variables entières pour le programme
Boolean event = false, verif = false; // Variable boolenne contenantl'etat de la presence d'un evenement

void setup() {

  size(800, 600);  // Dimensions la fenetre d'affichage
  background(0);  // couleur de fond noire
  fond = loadImage("fond.jpg");  // Chargement du fond d'ecran en memoire

  cp5 = new ControlP5(this);  // Instanciation de l'objet cp5
  // Initialisation de la liason série
  println(Serial.list()); // Affiche les ports série du PC sous format de tableau indexé
  monPort = new Serial(this, Serial.list()[3], 9600); // Configure le 4eme port COM 
  monPort.bufferUntil('\n');  // Récupère les données du buffer jusqu'au caractere \n
  textSize(26);  // Taille du text en 26
  PFont font = createFont("arial", 20);  // font pour cp5

  cp5.addTextfield("saisie")// créé un slider ou bouton ou saisie de texte
    .setPosition(350, 100) //Position de la fenetre de saisie
    .setSize(100, 40) // Taille de la fenetre de saisie
    .setFont(font)  // utilisation de la font créée au dessus
    .setAutoClear(true); // efface le champs quand on appuie sur ENTRER */

  knobPWM = cp5.addKnob("bouton") // créé un bouton rotatif nommé PWM
    .setRange(0, 255) // amplitude des valeurs du bouton
    .setValue(0) // Valeur par defaut
    .setPosition(350, 200) // position dans la fenetre
    .setFont(font) // utilisation de la font créée au dessus
    .setRadius(50) // Rayon en pixels du bouton
    .setNumberOfTickMarks(10) // Nombre de graduation
    .setTickMarkLength(4) // Longueur des tratis graduation
    .setColorForeground(color(255)) // Couleur des textes, graduations...
    .setColorBackground(color(0, 160, 100)) // Couleur de fond du bouton
    .setColorActive(color(255, 255, 0)) // Couleur de la partie active quand la souris est dessus
    .setDragDirection(Knob.VERTICAL); //Sens d'utilisation avec la souris 

  sliderPWM = cp5.addSlider("sliderH") // créé un slider horizontal
    .setRange(0, 255) // amplitude des valeurs
    .setPosition(285, 400) // position dans la fenetre
    .setSize(300, 40)  // Taille du slider
    .setFont(font); // utilisation de la font créée au dessus
}
void draw() {  // boucle principale
  affichage();
  knobPWM.setValue(pwm);
  sliderPWM.setValue(pwm);
}

void affichage() {
  clear();  // Effacer la fenetre
  background(fond);  // Affichage du fond d'ecran
  text("Contrôle Moteur PWM", width/2 - textWidth("Contrôle Moteur PWM")/2, 30);  // Affiche mon titre
  text("Pos souris X : " + str(mouseX), 10, 530); // affiche la position X de la souris
  text("Pos souris Y : " + str(mouseY), 10, 555); // affiche la position Y de la souris
  text("Valeur h récupérée : " + valeur, 500, 500); // Affiche la valeur récupérée par controlP5 
  text("PWM récupérée : " + pwm, 500, 550); // affiche la valeur de la PWM
}

void controlEvent(ControlEvent theEvent) { // Fonction de gestion des evennement produits par controlP5
  if (theEvent.getController().getName() == "bouton") { // Recupere la valeur renvoyee par le controller P5 slider, bouton ou saisie
    valeur = int(theEvent.getValue());// recupere la valeur renvoyée par le controleur sous format int
    event = true;  // Flag a true pour dire qu'un evenement à été reçu
  }
  if (theEvent.getController().getName() == "sliderH") {
    valeur = int(theEvent.getValue());// recupere la valeur renvoyée par le controleur sous format int
    event = true;  // Flag a true pour dire qu'un evenement à été reçu
  }
  if (theEvent.getController().getName() == "saisie") { // Test de l'id du controleur qui a généré un evenement 
    valeur = int(theEvent.getStringValue()); // recupere la valeur renvoyée par le controleur sour format int
    event = true;
  }
  if (event) { // Si un evenement à été reçu
    if (valeur >= 0 && valeur <= 255) { // si la valeur saisie est dans la plage 0 à 255
      pwm = valeur; // Affecte a pwm valeur  
      envoiPWM(); //Appel de la fonction envoiPWM
    } else {
      valeur=pwm; // sinon valeur garde la valeur de pwm
      println("PWM hors valeur"); // Affiche dans la console que la valeur est hors champ
    }
    event = false;  // Flag remis à false
  }
}

void envoiPWM() {
  monPort.write(str(valeur));  // Envoie valeur sous forme d'une chaine de caractères
  monPort.write('\n');  // envoie nouvelle ligne 
  println("PWM = " + str(pwm)); // Affiche dans la console que la valeur de la PWM transmise en RS232
}
