import processing.serial.*;
import java.awt.event.KeyEvent;
import java.io.IOException;

Serial myPort;

// initialise
String data="";
float roll, pitch, xAcc, yAcc, zAcc, xGyro, yGyro, zGyro, dt;
float time=0;

String fileName;
Table table;

PImage sky, astronaut, bubble, bubble2, stacc;

int rectX, rectY, rectSize, random;
boolean start, reset;

void setup() {
  size(1200,675,P3D);
  
  rectX = width/2-50;
  rectY = height;
  rectSize = 80;
  start = false;
  reset = false;
  
  // import images for graphics
  sky = loadImage("sky.jpeg");
  astronaut = loadImage("astronaut.png");
  bubble = loadImage("bubble.png");
  bubble2 = loadImage("bubble2.png");
  stacc = loadImage("stacc.png");
  
  // read serial output from Arduino
  myPort = new Serial(this, "/dev/cu.usbmodem14401", 9600); // starts the serial communication
  myPort.bufferUntil('\n');
  
  // set-up table for csv data
  table = new Table();

  table.addColumn("#"); // columns
  table.addColumn("roll");
  table.addColumn("pitch");
  table.addColumn("xAcc");
  table.addColumn("yAcc");
  table.addColumn("zAcc");
  table.addColumn("xGyro");
  table.addColumn("yGyro");
  table.addColumn("zGyro");
  table.addColumn("dt");
  }

void draw() {
  
  background(sky);
  startbutton();
  resetbutton();

  translate(width/2, height/2, 0);
  
  // add graphics
  image(astronaut,-580,50);
  image(stacc,-100,-320);
  
  fill(255,255,255,50);
  rect(280, -170, 300, 150);
  
  textSize(30);
  fill(0, 0, 0);
  text("Roll: " + int(roll), 300, -130);
  textSize(30);
  text("Pitch: " + int(pitch), 300, -90);
  textSize(30);
  text("Time: " + int(time/60) + " m " + int(time%60) + " s", 300, -50);
  
  animation();

}

// Read data from the Serial Port
void serialEvent (Serial myPort) { 
  // reads the data from the Serial Port up to the character '.' and puts it into the String variable "data".
  data = myPort.readStringUntil('\n');

  // if you got any bytes other than the linefeed:
  if (data != null) {
    data = trim(data);
    // split the string at "/"
    String items[] = split(data, '\t');
    if (items.length > 1) {

      //--- Roll,Pitch in degrees
      roll = float(items[0]);
      pitch = float(items[1]);
  
      //--- xAcc, yAcc, zAcc
      xAcc = float(items[2]);
      yAcc = float(items[3]);
      zAcc = float(items[4]);
      
      //--- xGyro, yGyro, zGyro
      xGyro = float(items[5]);
      yGyro = float(items[6]);
      zGyro = float(items[7]);
      
      //--- dt
      dt = float(items[8]);
      
      //--- timer
         if (start) {
            time += dt; // starts adding time if 'start' is pressed
          } else {
            pitch = 0;
            roll = 0;
          }
      
      //--- if condition capsense activated
      if (xGyro!=10000 && yGyro!=10000 && zGyro!=10000){
        TableRow newRow = table.addRow(); //add a row for this new reading
        newRow.setInt("#", table.lastRowIndex());//record a unique identifier (the row's index)
        
        //record sensor information. Customize the names so they match your sensor column names. 
        newRow.setFloat("roll", roll);
        newRow.setFloat("pitch", pitch);
        newRow.setFloat("xAcc", xAcc);
        newRow.setFloat("yAcc", yAcc);
        newRow.setFloat("zAcc", zAcc);
        newRow.setFloat("xGyro", xGyro);
        newRow.setFloat("yGyro", yGyro);
        newRow.setFloat("zGyro", zGyro);
        newRow.setFloat("dt", dt);
      }
    }
    }
}

//============ functions ============//

//-------- start button function --------//
void startbutton() {
  // draw button
  fill(0, 255, 0);
  rect(rectX-rectSize/2, rectY-rectSize, rectSize, rectSize/2);
  fill(255, 0, 0);
  textSize(24);
  
  // start/stop text conditions
  if (!start) {    
    text("Start", rectX-rectSize/3, rectY-rectSize+rectSize/3);
  } else if (start) {
    text("Stop", rectX-rectSize/3, rectY-rectSize+rectSize/3);
  } else if (!start) {
    text("Start", rectX-rectSize/3, rectY-rectSize+rectSize/3);
  }
  
  // pressed button conditions
  if (mousePressed) {
    if (mouseX > rectX-rectSize/2 && mouseX < rectX-rectSize/2 + rectSize &&
      mouseY > rectY-rectSize && mouseY < rectY-rectSize + rectSize/2) {
      if (start) {
        start = false;
      } else {
        start = true;
        reset = true;
      }
    }
  }
}

//-------- reset button --------//
void resetbutton() {
  // draw button
  fill(0, 255, 0);
  rect(rectX+100-rectSize/2, rectY-rectSize, rectSize+10, rectSize/2);
  fill(255, 0, 0);
  textSize(24);
  text("Reset", rectX+100-rectSize/3, rectY-rectSize+rectSize/3);
  
  // press reset button
  if (mousePressed) {
    if (mouseX > rectX+100-rectSize/2 && mouseX < rectX+100-rectSize/2 + rectSize &&
      mouseY > rectY-rectSize && mouseY < rectY-rectSize + rectSize/2) {
      time = 0; // reset time
      if (reset){
      reset = false;
      } else {
        reset = true;
      }
    }
  }
}


//-------- animation --------//
void animation(){
  if(start){     // display animation when 'start'
  
    image(bubble,-580,-200); // speech bubble
    
      //--- motivation!
    if (abs(pitch)>10 || abs(roll)>10){ // threshold 10 degrees
        textSize(45);
        fill(0,0,0);
        text("Keep it level!", -550, -90);
       }
    else if (abs(pitch)<10 && abs(roll)<10) {
        textSize(45);
        fill(0,0,0);
        text("Well done! ", -530, -90);
    }

    // Rotate the object
    rotateX(radians(roll));
    rotateZ(radians(-pitch));
    
    // 3D 0bject
    textSize(30);  
    
    fill(2, 7, 93);
    box (386, 100, 200); // Draw box
    
    textSize(25);
    fill(255, 255, 255);
    text("LEFT", -183, 10, 101);
    text("RIGHT", 110, 10, 101);
    
    fill(128,128,128,40); // Transparent
    sphere(222); // Draw sphere
    
  } else {
    image(bubble2,-350,-210);
  }
  
  //--- Instructions
  if (!start && !reset){    // before 'start'
    textSize(50);
    text("Press start and \nlaunch the rocket!",-270,-90);
  } else if (reset){
    textSize(35);
    text("Wow! You have reached \nthe moon in " + int(time/60) + " m " + int(time%60) + " s! \nCheck your scores!",-260,-100);
  
    //  //saves the table as a csv in the same folder as the sketch every numReadings. 
  //  //if (readingCounter % numReadings ==0)//The % is a modulus, a math operator that signifies remainder after division. The if statement checks if readingCounter is a multiple of numReadings (the remainder of readingCounter/numReadings is 0)
  //  else if (data == null) {
      //fileName = str(year()) + str(month()) + str(day()) + str(table.lastRowIndex()); //this filename is of the form year+month+day+readingCounter
      saveTable(table, "hcard.csv"); //Woo! save it to your computer. It is ready for all your spreadsheet dreams. 
  
}
  
}
