import processing.pdf.*;
import java.util.Calendar;
import oscP5.*;

boolean recordPDF = false;
float pupilArea;
float pupilDilation;
float red = 255;
float green = 0;
float blue = 0;
float centroidX;
float centroidY;
float x;
float y;
float sx; 
float sy; 
float cx; 
float cy; 
float startcol;
float alphaPOT = 0.95;
int circleResolution = 300;
String colorStates = "green";
String currentColor;

OscP5 oscP5;
OscP5 wekOscP5;

Blob blob = new Blob();

void setup(){
  size(960, 540);
  smooth();
  noFill();
  background(255);
  oscP5 = new OscP5(this, 12001);
  wekOscP5 = new OscP5(this, 12000);
  startcol = random(0,255);
  long leftLimit = 1L;
  long rightLimit = 10L;
  long generatedLong = leftLimit + (long) (Math.random() * (rightLimit - leftLimit));
  noiseSeed(generatedLong); 
  blob = new Blob();
}

void draw(){
  float radius = map(pupilArea, 3000, 10000, 0.2, 1);
  switch (colorStates) {
    case "red":
      if (red <= 255)
          red = red + 0.9;
      if (red >= 255)
          blue = blue - 0.7;
      if (blue <= 0)
          colorStates = "green";
      break;
    case "green":
      if (green <= 255)
          green = green + 0.7;
      if (green >= 255)
          red = red - 0.7;
      if(red <= 0)
          colorStates = "blue";
      break;
    case "blue":
      if (blue <= 255)
          blue = blue + 0.7;
      if (blue >= 255)
          green = green - 0.7;
      if(green <= 0)
          colorStates = "red";
      break;
    default:
      colorStates = "red";
      break;
    }
    
    beginShape();
    for (int i=0; i<circleResolution; i++){
      float angle = map(i, 0, circleResolution, 0, TWO_PI);
      float gx = frameCount * 2 - 200;
      float xx = 100 * cos(angle + gx / 10);
      float yy = 100 * sin(angle + gx / 10);
      PVector v = new PVector(xx, yy);
    
      cx = width/10;
      cy = height / 5 + sin(frameCount/500);
      xx = (xx + gx) / 150; yy = (yy + cy) / 150;
      
      v.mult(1 + 1.5 * noise(xx, yy));
      vertex(blob.location.x + cx + v.x*radius*0.9, blob.location.y + cy + v.y*radius*0.9);
      
      if(i == 0) {
        sx = blob.location.x + cx + v.x*radius*0.9;
        sy = blob.location.y + cy + v.y*radius*0.9; 
      }  
    }
    blob.update();
    blob.display();
    endShape();
}

void oscEvent(OscMessage theOscMessage) {
  if (theOscMessage.addrPattern().equals("/p2/inputs")) {
    pupilArea = theOscMessage.get(0).floatValue();
    centroidX = map(theOscMessage.get(1).floatValue(), 150, 400, 100, width-100);
    centroidY = map(theOscMessage.get(2).floatValue(), 100, 200, 0, height-60);
    pupilDilation = theOscMessage.get(3).floatValue();
  }
  // ****Wekinator Messaging****
  //else if (theOscMessage.addrPattern().equals("/wek/outputs")) {
  //  pupilDilation = map(theOscMessage.get(0).floatValue(), 0, 1, 0, 100);
  //}
}

class Blob {
  PVector location;
  PVector velocity;
  PVector acceleration;
  PVector centroid;
  float topspeed;

  Blob() {
    centroid = new PVector(width/2,height/2);
    location = new PVector(centroidX,centroidY);
    velocity = new PVector(0,0);
    topspeed = 1;
  }
  void update() {
    PVector centroid = new PVector(centroidX,centroidY);
    PVector dir = PVector.sub(centroid,location);
    dir.normalize();
    dir.mult(20);
    acceleration = dir;
    velocity.add(acceleration);
    velocity.limit(topspeed);
    location.add(velocity);
  }
  void display() {
    fill(255, 3);
    strokeWeight(1f);
    stroke(red, green, blue, 90);  
  }
}

void keyReleased(){
  if (key == DELETE || key == BACKSPACE) background(255);
  if (key=='s' || key=='S') saveFrame(timestamp()+"_##.png");

  // r to start, e to stop/save
  if (key =='r' || key =='R') {
    if (recordPDF == false) {
      beginRecord(PDF, timestamp()+".pdf");
      println("recording started");
      recordPDF = true;
      smooth();
      noFill();
      background(255);
    }
  } 
  else if (key == 'e' || key =='E') {
    if (recordPDF) {
      println("recording stopped");
      endRecord();
      recordPDF = false;
      background(255); 
    }
  }  
}

String timestamp() {
  Calendar now = Calendar.getInstance();
  return String.format("%1$ty%1$tm%1$td_%1$tH%1$tM%1$tS", now);
}