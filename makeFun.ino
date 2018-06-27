#include <SPI.h>
#include <MFRC522.h>

#define BUZZER_PIN 2
#define MOTOR_L1_PIN 4
#define MOTOR_L2_PIN 5
#define MOTOR_R1_PIN 7
#define MOTOR_R2_PIN 6
#define RST_PIN 9           // Configurable, see typical pin layout above
#define SS_PIN  10          // Configurable, see typical pin layout above
#define LINE_FOLLOW_L A0
#define LINE_FOLLOW_R A1

int commandIndex = 0;
String COMMAND[100];
bool exec = false;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

//*****************************************************************************************//
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MOTOR_L1_PIN, OUTPUT);
  pinMode(MOTOR_L2_PIN, OUTPUT);
  pinMode(MOTOR_R1_PIN, OUTPUT);
  pinMode(MOTOR_R2_PIN, OUTPUT);
  pinMode(LINE_FOLLOW_L, INPUT);
  pinMode(LINE_FOLLOW_R, INPUT);
  
  Serial.begin(9600);                                           
  SPI.begin();                                                 
  
  mfrc522.PCD_Init();                                           
  Serial.println(F("Read personal data on a MIFARE PICC:"));    
  
  for(int i=0;i<100;i++) {
    COMMAND[i] = "";
  }
}

//*****************************************************************************************//
void loop() {
  readRFID();

  if(exec) {
    Serial.println("=====execute=====");
    for(int i=0;i<=commandIndex;i++) {
      delay(1500);
      actionCardContent(i);
    }
    commandIndex = 0;
    exec = false;
  }
//  int line = analogRead(LINE_FOLLOW_L);
//  Serial.println(line);
}

//*****************************************************************************************//
void readRFID() {
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //-------------------------------------------

  // mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

  //-------------------------------------------

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  String currentRead = "";
  //PRINT FIRST NAME
  for (uint8_t i = 0; i < 16; i++) {        
    if (buffer1[i] != 32) {
      char value = buffer1[i];
      if(buffer1[i] != 10 && buffer1[i] != 13) {
        currentRead += value;
        COMMAND[commandIndex] += value;    
      }
    }
  }
  // Serial.println(COMMAND[commandIndex]);
  commandIndex++;
  Serial.print("Command: ");
  Serial.println(currentRead);

  if(currentRead.equals("action")) {
    exec = true;
  }
 
  //----------------------------------------

  // delay(1000); //change value if you want to read cards faster
  tone(2, 1000, 100);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void actionCardContent(int index) {
  if(COMMAND[index] == "forward") {
    Serial.println("forward done");
    commandLineFollow();
  } else if(COMMAND[index] == "left") {
    Serial.println("left done");
    commandTurnLeft();
  } else if(COMMAND[index] == "right") {
    Serial.println("right done");
    commandTurnRight();
  } else if(COMMAND[index] == "repeat") {
    Serial.println("repeat done");
  } else if(COMMAND[index] == "break") {
    Serial.println("break done");
  } else if(COMMAND[index] == "2") {
    Serial.println("2 done");
  } else if(COMMAND[index] == "3") {
    Serial.println("3 done");
  } else if(COMMAND[index] == "4") {
    Serial.println("4 done");
  } else if(COMMAND[index] == "5") {
    Serial.println("5 done");
  } else if(COMMAND[index] == "action") {
    Serial.println("=====finish=====");
    Serial.println(F("Read personal data on a MIFARE PICC:"));  
  }

  if(COMMAND[index] != "action") {
    tone(2, 400, 500);    
  }
  COMMAND[index] = "";
}

void forward(int speed) {
  digitalWrite(MOTOR_L1_PIN, LOW);
  analogWrite(MOTOR_L2_PIN, speed);
  digitalWrite(MOTOR_R1_PIN, HIGH);
  analogWrite(MOTOR_R2_PIN, 255 - speed);
}

void turnLeft(int speed) {
  digitalWrite(MOTOR_L1_PIN, LOW);
  analogWrite(MOTOR_L2_PIN, speed - 20);
  digitalWrite(MOTOR_R1_PIN, HIGH);
  analogWrite(MOTOR_R2_PIN, 255 - speed);
}

void turnRight(int speed) {
  digitalWrite(MOTOR_L1_PIN, LOW);
  analogWrite(MOTOR_L2_PIN, speed);
  digitalWrite(MOTOR_R1_PIN, HIGH);
  analogWrite(MOTOR_R2_PIN, 255 - (speed - 20));
}

void lineFollow(int speed) {
  int leftLine = analogRead(LINE_FOLLOW_L);
  int rightLine = analogRead(LINE_FOLLOW_R);
  
  if(rightLine > 40) {
    turnRight(speed);
  } else if(rightLine <= 40 && rightLine > 35) {
    forward(speed); 
  } else {
    if(leftLine <= 35) {
      forward(0);
    } else {
      // delay(300);
      turnLeft(speed);    
    }
  }
}

void commandLineFollow() {
  int left = analogRead(LINE_FOLLOW_L);
  int right = analogRead(LINE_FOLLOW_R);
 
  while(left > 35 || right > 35) {
    left = analogRead(LINE_FOLLOW_L);
    right = analogRead(LINE_FOLLOW_R);
    lineFollow(55);
  }
  forward(55);
  delay(100);
  forward(0);
}

void commandTurnLeft() {
  int left = analogRead(LINE_FOLLOW_L);
  int right = analogRead(LINE_FOLLOW_R);

  while(left <= 45) {
    left = analogRead(LINE_FOLLOW_L);
    digitalWrite(MOTOR_L1_PIN, HIGH);
    analogWrite(MOTOR_L2_PIN, 200);
    digitalWrite(MOTOR_R1_PIN, HIGH);
    analogWrite(MOTOR_R2_PIN, 200);
  }
  delay(100);
  forward(0);
}

void commandTurnRight() {
  int left = analogRead(LINE_FOLLOW_L);
  int right = analogRead(LINE_FOLLOW_R);
 
  while(right <= 45) {
    right = analogRead(LINE_FOLLOW_R);
    digitalWrite(MOTOR_L1_PIN, LOW);
    analogWrite(MOTOR_L2_PIN, 55);
    digitalWrite(MOTOR_R1_PIN, LOW);
    analogWrite(MOTOR_R2_PIN, 55);
  }
  delay(100);
  forward(0);
}

