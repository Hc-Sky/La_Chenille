/* Exemple controlP5 bouton rotatif */
// déclaration bibliothèque Serial
import processing.serial.*;
import controlP5.*;  // Ajout de la librairie controlP5
PImage left,middle,right,left_d,middle_d,right_d;
ControlP5 cp5; // créer un objet (cp5) de type ControlP5
Knob knobPWM;  // créer un objet de type Knob
OutputStream output;

String pic[] ={"pic00.jpg","pic01.jpg","pic02.jpg","pic03.jpg","pic04.jpg","pic05.jpg"};
int PAD[] = {40,597,193,193};

int lf = 10;    // Linefeed in ASCII
int camx, camy, camx2, camy2, cpos_x = PAD[0] + (PAD[2]/2), cpos_y = PAD[1] + 166, n2 = 5, n = 0, vit, vit1,dis, etat_d = 0, sp = 10, pwm, dir, dir_t = -1,XT = 1340,YT = 637,envoi_pwm, pf, dir_v, et_b = -1, b_c = 0, imgb, rx;
int cursor[] = {cpos_x, cpos_y, 10};
String RX = null,BT = "P";
Serial mySerialPort;
int serialValue;
Boolean etat = false,trx = false, serial = true, act = true, read = false, grab = false;
String portStream="0";  // init variable reception val série
float obstacle = 314, res;
long timer;

void setup() {
  surface.setResizable(true);
  surface.setTitle("");
  size(1560, 900);  // Dimensions la fenetre d'affichage
  background(0,0,0); // couleur de fond noire si pas d'image de fond
  fill(255,255,255);
  printArray(Serial.list());
  textSize(50); // Taille du text en 26
  try {
  mySerialPort = new Serial(this, Serial.list()[0], 19200);
  mySerialPort.bufferUntil('\n');
  mySerialPort.clear();
  }
  catch (Exception e) {  
    read = true;
    etat_d = 1;
    n = 645;
    int n2 = n;
    timer = millis();
    windowResize(900,540);  // Dimensions la fenetre d'affichage
    background(0,0,0); // couleur de fond noire si pas d'image de fond
    fill(255,0,0);
    surface.setResizable(false);
    surface.setLocation(1920/2-450,1080/2-270);
    surface.setTitle("ERROR");
    println(">> WIRELESS CONNECTION SETUP FAILED");
    while (n2 <= n + 80){ 
      text(">> BLUETOOTH CONNECTION FAILED", 50, 50);
      text(">> CHECK WIRELESS MODULE CONNECTION", 50, 100);
      text(">> STARTING PREVIEW MODE", 50, 800);
      if (millis() - timer >= 1000) {
        timer = millis();
        n2 = n2 + 20;
        text(".", n2, 800);
        windowResize(900,540-1);
        windowResize(900,540);
      }
    }
    surface.setLocation(150,100);
    windowResize(1560,900);
  }
  surface.setTitle("IHM");
  textSize(26);
  surface.setResizable(false);
  PFont font = createFont("arial", 20); // Creation d'un objet font pour les box
  
  cp5 = new ControlP5(this); // Instanciation de l'objet cp5
  
  knobPWM = cp5.addKnob("vitesse")  // créé un bouton rotatif nommé PWM
    .setRange(0,255) // amplitude des valeurs du bouton
    .setValue(0) // Valeur par defaut
    .setPosition(4.15*(width/6), 4*(height/6) ) // position dans la fenetre
    .setFont(font) // utilisation de la font créée au dessus
    .setRadius(100) // Rayon en pixels du bouton
    .setNumberOfTickMarks(10) // Nombre de graduation
    .setTickMarkLength(4) // Longueur des tratis graduation
    .setColorForeground(color(0,0,0)) // Couleur des textes, graduations...
    .setColorBackground(color(120,120,120)) // Couleur de fond du bouton
    .setColorActive(color(0, 127, 255)) // Couleur de la partie active quand la souris est dessus
    .setDragDirection(Knob.HORIZONTAL ); //Sens d'utilisation avec la souris
    cp5.getController("vitesse").getCaptionLabel().hide();

}
  
void draw() {  // boucle principale 
  trame();
  affichage();
  interactions();
  pad();
  debug();
  comms();
}
void pad() {
  fill(0,0,0);
  rectMode(CORNER);
  rect(PAD[0],PAD[1],PAD[2],PAD[3]);
  if (grab == true && (mouseX > PAD[0]+25 && mouseY > PAD[1]+15 && mouseX < PAD[0]+PAD[2]-25 && mouseY < PAD[1]+PAD[3]-15)) {
    cpos_x = mouseX;
    cpos_y = mouseY;
    cursor[0] = cpos_x;
    cursor[1] = cpos_y;
    fill(255,0,0);
  }
  camx2 = camx;
  camy2 = camy;
  strokeWeight(1);
  stroke(255);
  fill(0);
  rectMode(CENTER);
  rect(cursor[0],cursor[1],50,30);
  circle(cursor[0],cursor[1],cursor[2]);
  camx = int(map(cursor[0],PAD[0]+10, PAD[0]+PAD[2]-10, -90, 90));
  camy = int(map(cursor[1],PAD[1]+10, PAD[1]+PAD[3]-10, 90, -10));
  textSize(10);
  stroke(0,127,255);
  line(PAD[0],cursor[1],PAD[0]+PAD[2],cursor[1]);
  line(cursor[0],PAD[1],cursor[0],PAD[1]+PAD[3]);
  stroke(255);
  strokeWeight(1);
  line(PAD[0],PAD[1]+166,PAD[0]+PAD[2],PAD[1]+166);
  line(PAD[0]+PAD[2]/2,PAD[1],PAD[0]+PAD[2]/2,PAD[1]+PAD[3]);
  text("Cam x = " + camx +"°", 50, 590);
  text("Cam y = " + camy +"°", 152, 590);
  if (etat_d == 1 && camx != camx2 && camy != camy2)
    println("Camera x = " + camx +" // y = "+ camy);
  if (read == false && camx != camx2 && camy != camy2) {
    mySerialPort.write("x");
    mySerialPort.write(str(camx));
    mySerialPort.write("y");
    mySerialPort.write(str(camy));
    mySerialPort.write('/');
    mySerialPort.write('\n');
  }
  noStroke();
  textSize(26);
  rectMode(CORNER);
}
void img() {
  if (n2 == 5) {
    fill(255);
    stroke(255,255,255);
    strokeWeight(5);
    rectMode(CENTER);
    rect(width/2,height/4.4-2,width/1.5,height/2.2);
    res = 4.475;
    middle = loadImage("pic00.jpg");
    right = loadImage("pic01.jpg");
    left = loadImage("pic02.jpg");
    middle_d = loadImage("pic04.jpg");
    right_d = loadImage("pic05.jpg");
    left_d = loadImage("pic03.jpg");
    pushMatrix();
    rotate(radians(180));
    image(left, -(width/2 - (width/res)/2 - 640/res*2.4)- width/res -1, -405, width/res, height/res);
    image(middle, -(width/2 - (width/res)/2 )- width/res -1, -405, width/res, height/res);
    image(right, -(width/2 - (width/res)/2 + 640/res*2.4)- width/res -1, -405, width/res, height/res);
    image(left_d, -(width/2 - (width/res)/2 - 640/res*2.4)- width/res -1, -205, width/res, height/res);
    image(middle_d, -(width/2 - (width/res)/2 )- width/res -1, -205, width/res, height/res);
    image(right_d, -(width/2 - (width/res)/2 + 640/res*2.4)- width/res -1, -205, width/res, height/res);
    popMatrix();
  }
  else if (n2 == 0) {
    res = 2.5;
    fill(255);
    stroke(255,255,255);
    strokeWeight(5);
    rectMode(CENTER);
    rect(width/2,height/4.4-2,width/2.5+6,height/2.2);
    middle = loadImage("pic00.jpg");
    pushMatrix();
    rotate(radians(180));
    image(middle, -(width/2 - (width/res)/2 )- width/res -1, -405, width/res, height/res);
    popMatrix();
  }
}

void interactions(){
  rectMode(CENTER);
   if (mouseX > 1330 && mouseX < 1370 && mouseY > 612 && mouseY < 782){
    if (etat_d == 1){
       noCursor();
    }
    else {
       cursor(HAND);
    }
    fill(100);
    stroke(0,127,255);
    strokeWeight(5);
    rect(1350, 698, 40, 175);
  }

  else if (etat_d == 0) {
    cursor(ARROW);
    fill(100);
    stroke(0,0,0);
    strokeWeight(5);
    rect(1350, 698, 40, 175);
  }
  else{
    fill(100);
    stroke(0);
    strokeWeight(5);
    rect(1350, 698, 40, 175);
  }
  
    fill(0,0,0);
    text("P", 1340, 637);
    text("D", 1340, 772);
    text("R", 1340, 704);
  
      
  if (mouseX > 1330 && mouseX < 1370 && mouseY > 612 && mouseY < 668){
    dir_t = -1;
    dir_v = -1;
  }
  else if (mouseX > 1330 && mouseX < 1370 && mouseY > 669 && mouseY < 725){
    dir_t = 0;
    dir_v = 0;
  }
  else if (mouseX > 1330 && mouseX < 1370 && mouseY > 726 && mouseY < 782){
    dir_v = 1;
    dir_t = 1;
  }
  
  if (et_b == -1){
    cp5.getController("vitesse").setValue(255);
    cp5.getController("vitesse").setColorActive(color(255,0,0));
    cp5.getController("vitesse").setColorForeground(color(255,0,0));
  }
  else {
    cp5.getController("vitesse").setColorActive(color(0, 127, 255));
    cp5.getController("vitesse").setColorForeground(color(0,0,0));
  }
  
  strokeWeight(5);
  rectMode(CENTER);
    
  if (mouseX > 261 && mouseX < 450 && mouseY > 600 && mouseY < 788){
    if (etat_d == 1){
       noCursor();
    }
    else {
       cursor(HAND);
    }
    stroke(0,127,255);
    rect(355, 693, 190, 190);
    fill(0, 127, 255);
  }
  else {
    stroke(0);
    rect(355, 693, 190, 190);
    fill(255);
  }
    
    noStroke();
    
  if (pf == 1){
    fill(200,200,0);
  }
      
      
    quad(380,625,355,625,330,680,350,680);
    quad(330,680,323,695,375,695,385,680); // éclair
    quad(385,680,360,680,330,770,385,680);

    
    
    fill(255,255,255);
    text(BT,XT,YT);
}

void affichage() {
  clear();
  background(0,0,0); // couleur de fond noire si pas d'image de fond
  if (read == true) {
    fill(255,0,0);
    text("PREVIEW MODE",55,30);
  }
  fill(0, 127, 255);
  rectMode(CENTER);
  rect(10,height/2,25,height+50);
  rect(width-10,height/2,25,height+50);
  
  fill(255, 255, 255);
  rect(20,height/2,10,height);
  rect(width-20,height/2,10,height);
  
  img();
  fill(255, 255, 255);
  stroke(0);
  quad(width/3+15, -10, (width/2)-200 , 45, width/2+200, 45, 2*(width/3)-15, -10);
  noStroke();
  ellipse(4.5*(width/6), 4*(height/6)+100, 250, 250);
  rect(5*(width/6), 4*(height/6)+100, 250, 250);
  quad(width-20, height, width-150 , height-85, width-150, height-325, width-20, height-325);
  quad(20, height, 181 , height-85, 181, height-325, 20, height-325);
  
  rect(width-135, height*0.275, 142 , 400);
  line(width-160, 388 - obstacle, width-140, 388 - obstacle);
  
  rectMode(CORNER);
  rect(180, 815, 300 , -240);
  rectMode(CENTER);
  
  fill(0,0,0);
  text("Echo", width-180, 440);
  if (dis <= 0){
    text("...", width-133, 90);
  }
  else {
    text(dis +" cm", width-133, 388 - obstacle);
  }
  noStroke();
  rect(width-160, height*0.26, 40 , 350 );
  text("Contrôle rover d'exploration", width/2 - textWidth("Contrôle rover d'exploration")/2, 25); // affiche le titre
  fill(255, 255, 255);
  rect(width-160, height*0.26, 35 , 340 );
  
  fill(130);
  strokeWeight(1);
  noStroke();
  
  if (dis > 29){
    fill(0, 127, 255);
  }
  else {
    fill(255,0,0);
  }
  rectMode(CORNERS);
  rect(width-147, 388 - obstacle, width-172, 404); // echo graph
}
  //[DEBUG]
void debug(){
  if (etat_d == 1) {
  noCursor();
  fill(255,0,0);
  text("DEBUG MODE", 1351, 30);
  text("X = " + str(mouseX), mouseX+20, mouseY - 30 ); // affiche la position X de la souris
  text("Y = " + str(mouseY), mouseX+20, mouseY - 5); // affiche la position Y de la souris
  strokeWeight(1);
  stroke(255,0,0,255);
  line(mouseX, mouseY +5, mouseX, mouseY -5);
  line(mouseX +5, mouseY, mouseX -5, mouseY);
  noStroke();
  }
  else{
  cursor();
  }
}

void controlEvent(ControlEvent theEvent) { // Fonction recuperant les evenement du controleur controlP5
  if (theEvent.getController().getName() == "vitesse"){ // Test du controleur qui a généré un evenement 
      vit = int(theEvent.getValue()); // recupere la valeur renvoyée par le controleur sous format int
      envoi_pwm = 1;
  }
}

void keyPressed(){
  if (key == 'r') {
    cpos_x = PAD[0] + (PAD[2]/2);
    cpos_y = PAD[1] + 166;
    cursor[0] = cpos_x;
    cursor[1] = cpos_y;
    camx = int(map(cursor[0],PAD[0]+10, PAD[0]+PAD[2]-10, -90, 90));
    camy = int(map(cursor[1],PAD[1]+10, PAD[1]+PAD[3]-10, 90, -10));
    if (etat_d == 1)
      println("Camera x = " + camx +" // y = "+ camy);
    if (read == false) {
      mySerialPort.write("x");
      mySerialPort.write(str(camx));
      mySerialPort.write("y");
      mySerialPort.write(str(camy));
      mySerialPort.write('/');
      mySerialPort.write('\n');
    }
  }
  if (keyCode == 32){
    pf = 1;
  }
  if (key == 'µ' && etat_d == 0) {
  etat_d = 1;
  println("Debug mode : ON");
  }
  else if (key == 'µ' && etat_d == 1) {
  etat_d = 0;
  println("Debug mode : OFF");
  }
  if (keyCode == 33){
   if (et_b == -1){
     cp5.getController("vitesse").setValue(0);
     vit = 0;
     vit1 = 0;
   }
   dir_t = 1 ;
   dir_v = 1;
   vit = vit1;
   et_b = 1;

  }
  else if (keyCode == 34){
    if (et_b == -1){
      cp5.getController("vitesse").setValue(0);
      vit = 0;
      vit1 = 0;
    }
    dir_t = 0 ;
    dir_v = 0;
    vit = vit1;
    et_b = 0;

  }
  else if (keyCode == SHIFT){
    dir_t = -1 ;
    dir_v = -1;
    vit1 = vit ;
    et_b = -1;
  }
  else if (keyCode == LEFT && dir_v >= 0) {
    dir_t = 2;
  }
  else if (keyCode == RIGHT&& dir_v >= 0){
    dir_t = 3;
  }
  else if (keyCode == LEFT && dir_v >= 1) {
    dir_t = 3;
  }
  else if (keyCode == RIGHT&& dir_v >= 1){
    dir_t = 2;
  }
  else if (keyCode == UP && dir_t != -1) {
    cp5.getController("vitesse").setValue(vit + 8);
  }
  else if (keyCode == DOWN && dir_t != -1){
    cp5.getController("vitesse").setValue(vit - 8);
  }
  if (dir_t == -1){
    BT = "P";
    XT = 1340;
    YT = 637;
    cp5.getController("vitesse").setColorActive(color(255,0,0));
    cp5.getController("vitesse").setColorForeground(color(255,0,0));
  }
  else if (dir_t == 0){
    BT = "R";
    XT = 1340;
    YT = 704;
    cp5.getController("vitesse").setColorActive(color(0, 127, 255));
    cp5.getController("vitesse").setColorForeground(color(0,0,0));
  }
  else if (dir_t == 1){
    BT = "D";
    XT = 1340;
    YT = 772;
    cp5.getController("vitesse").setColorActive(color(0, 127, 255));
    cp5.getController("vitesse").setColorForeground(color(0,0,0));
  }
}

void lecture_trame() {
  if (etat == false) {
    if (RX.indexOf("d") >= 0) {
      dis = int(RX.substring(RX.indexOf("d")+1, RX.indexOf('/')));
    }
    if (dis > -1){
      obstacle = map(dis, 0, 50, 0, 314);
    }
    else {
      obstacle = 314;
    }
    if (etat_d == 1){
      println("distance = " + obstacle);
      println("distance brut = " + dis);
    }
    if (RX.indexOf("image/") >= 0) {
      n2 = 5;
      etat = true;
    }
    if (RX.indexOf("image1/") >= 0) {
      etat = true;
      n2 = 0;
    }
  }
  if (etat == true) {
  lecture_image();
  }
  RX = "";
}

void lecture_image(){
  int stop = 0;
  int pre_stop = 0;
  while (n <= n2) {
    if (trx == false) {
      output = createOutput(pic[n]);
      println("image " + n);
      trx = true;
    }
    try {
      while (mySerialPort.available() > 0 || ( pre_stop != 255 && stop != 217)) {
        pre_stop = stop;
        int c = mySerialPort.read();
        trx = true;
        println(c);
        if (c != -1){
          stop = c;
          output.write(char(c));
        }
      }
    } catch (IOException e) {e.printStackTrace();}
    
    if (pre_stop == 255 && stop == 217 && trx == true) {
      try {
        output.flush();
        output.close();
        trx = false;
        println("fin image " + n);
        n++;
        stop = 0;
        pre_stop = 0;
      } catch (IOException e) {e.printStackTrace();}
    }
  }
  if (n == n2+1) {
    println("fin reception");
    etat = false;
    n = 0;
    act = true;
  }
}


void trame() {
  if (read == false) {
    if (mySerialPort.available() > 0){
      if (etat == true) {
        lecture_image();
      }
      rx = mySerialPort.read();
      if (etat == false) {
        RX = RX + char(rx);
        if (rx == 13) {
          lecture_trame();
        }
      }
    }
  }
}


void keyReleased(){
  pf = 0;
  if (keyCode == LEFT && dir_v >= 0) {
    dir_t = 0;
  }
  else if (keyCode == RIGHT&& dir_v >= 0){
    dir_t = 0;
  }
    if (keyCode == LEFT && dir_v >= 1) {
    dir_t = 1;
  }
  else if (keyCode == RIGHT&& dir_v >= 1){
    dir_t = 1;
  }
}

void mousePressed(){
  if (mouseX > PAD[0]+10 && mouseY > PAD[1]+10 && mouseX < PAD[0]+PAD[2]-10 && mouseY < PAD[1]+PAD[3]-10){
    grab = true;
  }
  if (dir_t == -1){
    BT = "P";
    XT = 1340;
    YT = 637;
    et_b = -1;
    cp5.getController("vitesse").setValue(0);
  }
  else if (dir_t == 0){
    if (et_b == -1){
     cp5.getController("vitesse").setValue(0);
     vit = 0;
    }
    BT = "R";
    XT = 1340;
    YT = 704;
    et_b = 0;
    
  }
  else if (dir_t == 1){
    if (et_b == -1){
     cp5.getController("vitesse").setValue(0);
     vit = 0;
    }
    BT = "D";
    XT = 1340;
    YT = 772;
    et_b = 1;
  }
  if (mouseX > 261 && mouseX < 450 && mouseY > 600 && mouseY < 788){
  pf = 1;
  mySerialPort.write("i1");
  }
}

void mouseReleased(){
  pf = 0;
  if (grab == true) {
    grab = false;
  }
}

void comms(){
  if (envoi_pwm == 1 && read == false){
      envoi_pwm = 0;
      if (et_b > -1){
      mySerialPort.write("v");
      mySerialPort.write(str(vit));
      mySerialPort.write('/');
      mySerialPort.write('\n');
      }
      else if (et_b == -1){
      mySerialPort.write("v");
      mySerialPort.write("0");
      mySerialPort.write('/');
      mySerialPort.write('\n');
      }
      
      if (etat_d == 1 && et_b > -1) {
        println("PWM = " + str(vit)); // Affiche dans la console que la valeur de la PWM transmise en RS232
      }
  }
  if (dir != dir_t && read == false){
    dir = dir_t;
    if (dir == -1){
      cp5.getController("vitesse").setValue(0);
      mySerialPort.write("v0/"+'\n');
      if (etat_d == 1){
        println("Direction = STOP");
      }
    }
    else{
      mySerialPort.write("d"+str(dir)+"/"+'\n');
      if (etat_d == 1){
        println("Direction = "+str(dir));
      }
    }
  }
}
