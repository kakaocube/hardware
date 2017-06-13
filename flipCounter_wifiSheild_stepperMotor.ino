#include <SPI.h>
#include <Phpoc.h>
#include <Stepper.h>

////////////////////////////////////////////////////////// wifi shield
bool stop = false;
char server[] = "143.248.250.17";
//String dataLocation = "/unread_count HTTP/1.1";

char cdata;
int dataLength = 0;
int leftMotorData10 = 0;  // 10 : left motor
int rightMotorData01 = 0;  // 1 : right motor
int leftPrevData = 0;
int rightPrevData = 0;

PhpocClient client;
//////////////////////////////////////////////////////////

////////////////////////////////////////////////////////// motor
const int stepsPerRevolution = 180;
int leftPosition = 0;
int rightPosition = 0;

// initialize the stepper library on pins
Stepper leftStepper(stepsPerRevolution, 6, 7, 8, 9);
Stepper rightStepper(stepsPerRevolution, 2, 3, 4, 5);
//////////////////////////////////////////////////////////

////////////////////////////////////////////////////////// wifi shield
void connectToServer(String dataLocation) {
  Serial.begin(9600);
  while(!Serial)
    ;

  //Serial.println("Sending GET request to web server");
    
  //Phpoc.begin(PF_LOG_SPI | PF_LOG_NET);
  Phpoc.begin();
  
  if (client.connect(server, 8811)) {
    //Serial.println("making HTTP request...");
     
    // make HTTP GET request to dataLocation:
    client.println("GET " + dataLocation);
    //client.println("Host: 143.248.250.17");
    client.println();

    Serial.println();
    Serial.print("connected to ");
    Serial.println(server + dataLocation);
  }
  else
    Serial.println("connection failed");
}

char getData() {
  char c;
  if(client.available()) {
    c = client.read();
    //Serial.print(c);
  }
  return c;
}

void getNumber() {
  Serial.print("start to get numbers");
  
  stop = false;
  while(!stop) {
    cdata = getData();
    if(cdata == 'L') {
      for(int i=0; i<8; i++) {
        cdata = getData();
        dataLength = (int)cdata - 48;
      }
      
      //Serial.println();
      //Serial.print("lenght = ");
      //Serial.println(dataLength);

      // throw away useless data
      for(int i=0; i<4; i++) {
        cdata = getData();
      }

      if(dataLength == 1) {
        leftPrevData = leftMotorData10;
        leftMotorData10 = 0;
        cdata = getData();
        rightPrevData = rightMotorData01;
        rightMotorData01 = (int)cdata - 48;
      } else if(dataLength == 2) {
        cdata = getData();
        leftPrevData = leftMotorData10;
        leftMotorData10 = (int)cdata - 48;
        //Serial.print("motorData01 = ");
        //Serial.println(cdata);
        cdata = getData();
        rightPrevData = rightMotorData01;
        rightMotorData01 = (int)cdata - 48;
        //Serial.print("motorData02 = ");
        //Serial.println(cdata);
      }
      stop = true;
    }
  }
  
  Serial.println("---> done");
}

void disconnectToServer() {
  client.stop();
  Serial.println("disconnected");
  Serial.println();
}

////////////////////////////////////////////////////////// motor
void initCards() {
  getNumber();
  int lStep = 10 - leftMotorData10;
  int rStep = 10 - rightMotorData01;
  stepMotor(lStep, rStep);
}

void setupMotor() {
  leftStepper.setSpeed(60);
  rightStepper.setSpeed(60);
}

void stepMotor(int leftDataInterval, int rightDataInterval) {
  //int leftDataInterval = leftMotorData10 - leftPrevData;
  //int rightDataInterval = rightMotorData01 - rightPrevData;

  if(leftDataInterval < 0) {
    leftDataInterval += 10;
  }

  if(rightDataInterval < 0) {
    rightDataInterval += 10;
  }

  //Serial.print("leftDataInterval = ");
  //Serial.println(leftDataInterval);
  //Serial.print("rightDataInterval = ");
  //Serial.println(rightDataInterval);

  // step one revolution  in one direction:
  // clockwise
  rightStepper.step(-stepsPerRevolution*rightDataInterval);
  leftStepper.step(stepsPerRevolution*leftDataInterval);
}
//////////////////////////////////////////////////////////
void setup() {
  setupMotor();
  connectToServer("/init HTTP/1.1");
  initCards();
  disconnectToServer();
}

void loop() {
  //String dataLocation = "/unread_count HTTP/1.1";
  connectToServer("/unread_count HTTP/1.1");
  getNumber();
  disconnectToServer();
  int lStep = leftMotorData10 - leftPrevData;
  int rStep = rightMotorData01 - rightPrevData;
  stepMotor(lStep, rStep);
  delay(1000);
}
