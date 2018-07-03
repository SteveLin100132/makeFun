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
  /*
  Serial.print(analogRead(A0));
  Serial.print("\t");
  Serial.println(analogRead(A1));
  */
  readRFID();

  if(exec) {
    Serial.println("=====execute=====");
    int repeatIndex = searchIndexOf(COMMAND, "repeat");
    int numberIndex = repeatIndex + 1;
    int breakIndex = searchIndexOf(COMMAND, "break");

    String finalArray[repeatIndex + (COMMAND[numberIndex].toInt() * (breakIndex - numberIndex - 1)) + (commandIndex - breakIndex - 1)];
    int tempIndex4Repeat = repeatIndex;

    if(breakIndex != 0) {
      for(int i = 0; i < repeatIndex; i++) {
        finalArray[i] = COMMAND[i];
      }

      for(int i = 0; i < COMMAND[numberIndex].toInt(); i++) {
        for(int j = numberIndex + 1; j < breakIndex; j++) {
          finalArray[tempIndex4Repeat] = COMMAND[j];
          tempIndex4Repeat++;
        }
      }
  
      for(int i = breakIndex + 1; i < commandIndex; i++) {
        finalArray[tempIndex4Repeat] = COMMAND[i];
        tempIndex4Repeat++;
      }

      for(int i = 0; i < tempIndex4Repeat; i++) {
        Serial.print(finalArray[i]);
        Serial.print(", ");
      }
      Serial.println("");
      actionCardContent(finalArray, tempIndex4Repeat);
    } else {
      actionCardContent(COMMAND, commandIndex);
    }

    for(int i = 0; i < sizeof(finalArray)/sizeof(finalArray[0]) + 1; i++) {
      Serial.println(COMMAND[i]);
    }
  }
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
  if (! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (! mfrc522.PICC_ReadCardSerial()) {
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

void actionCardContent(String command[], int commandLength) {
//  Serial.println(commandLength);
  for(int i = 0; i < commandLength; i++) {
    Serial.println(command[i]);
    if(command[i] == "forward") {
      Serial.println("forward done");
      commandLineFollow();
    } else if(command[i] == "left") {
      Serial.println("left done");
      commandTurnLeft();
    } else if(command[i] == "right") {
      Serial.println("right done");
      commandTurnRight();
    } else if(command[i] == "action") {
      Serial.println("=====finish=====");
      Serial.println(F("Read personal data on a MIFARE PICC:"));  
    }
  
    if(command[i] != "action") {
      tone(2, 400, 500);    
    }

    delay(1000);
  }

  for(int i = 0; i < commandIndex; i++) {
    COMMAND[i] = "";
  }
  commandIndex = 0;
  exec = false;
}

void forward(int speed) {
  digitalWrite(MOTOR_L1_PIN, LOW);
  analogWrite(MOTOR_L2_PIN, speed);
  digitalWrite(MOTOR_R1_PIN, HIGH);
  analogWrite(MOTOR_R2_PIN, 255 - speed);
}

void backward(int speed) {
  digitalWrite(MOTOR_L1_PIN, HIGH);
  analogWrite(MOTOR_L2_PIN, 255 - speed);
  digitalWrite(MOTOR_R1_PIN, LOW);
  analogWrite(MOTOR_R2_PIN, speed);
}

void turnLeft(int speed) {
  digitalWrite(MOTOR_L1_PIN, LOW);
  analogWrite(MOTOR_L2_PIN, speed - 30);
  digitalWrite(MOTOR_R1_PIN, HIGH);
  analogWrite(MOTOR_R2_PIN, 255 - speed);
}

void turnRight(int speed) {
  digitalWrite(MOTOR_L1_PIN, LOW);
  analogWrite(MOTOR_L2_PIN, speed);
  digitalWrite(MOTOR_R1_PIN, HIGH);
  analogWrite(MOTOR_R2_PIN, 255 - (speed - 30));
}

bool lineFollow(int speed) {
  int leftLine = analogRead(LINE_FOLLOW_L);
  int rightLine = analogRead(LINE_FOLLOW_R);

  if(leftLine <= 26 && rightLine <= 26) {
    forward(0);
    return false;
  } else if(rightLine <= 28) {
    turnLeft(speed);
  } else if(leftLine <= 28) {
    turnRight(speed);
  } else {
    forward(speed);
  }

  return true;
}

void commandLineFollow() {
  int left = analogRead(LINE_FOLLOW_L);
  int right = analogRead(LINE_FOLLOW_R);

  while(lineFollow(55)) {
    left = analogRead(LINE_FOLLOW_L);
    right = analogRead(LINE_FOLLOW_R);
  }
  forward(55);
  delay(100);  
  forward(0);
}

void commandTurnLeft() {
  int left = analogRead(LINE_FOLLOW_L);
  int right = analogRead(LINE_FOLLOW_R);

  backward(55);
  delay(100);
  forward(0);
  
  while(left <= 35) {
    left = analogRead(LINE_FOLLOW_L);
    right = analogRead(LINE_FOLLOW_R);

    if(left > 35) break;
    
    digitalWrite(MOTOR_L1_PIN, HIGH);
    analogWrite(MOTOR_L2_PIN, 190);
    digitalWrite(MOTOR_R1_PIN, HIGH);
    analogWrite(MOTOR_R2_PIN, 190);

    delay(25);
    forward(0);
    delay(25);
  }
  forward(0);

  while(right <= 35) {
    left = analogRead(LINE_FOLLOW_L);
    right = analogRead(LINE_FOLLOW_R);

    if(left > 35) break;
    
    digitalWrite(MOTOR_L1_PIN, HIGH);
    analogWrite(MOTOR_L2_PIN, 190);
    digitalWrite(MOTOR_R1_PIN, HIGH);
    analogWrite(MOTOR_R2_PIN, 190);

    delay(25);
    forward(0);
    delay(25);
  }
  forward(0);

  while(left > 30) {
    left = analogRead(LINE_FOLLOW_L);
    right = analogRead(LINE_FOLLOW_R);

    if(left <= 30) break;
    
    digitalWrite(MOTOR_L1_PIN, HIGH);
    analogWrite(MOTOR_L2_PIN, 190);
    digitalWrite(MOTOR_R1_PIN, HIGH);
    analogWrite(MOTOR_R2_PIN, 190);

    delay(25);
    forward(0);
    delay(25);
  }
  forward(0);

  forward(55);
  delay(200);
  forward(0);

  /*commandLineFollow();
  forward(55);
  delay(50);
  forward(0);*/
}

void commandTurnRight() {
  int left = analogRead(LINE_FOLLOW_L);
  int right = analogRead(LINE_FOLLOW_R);

  backward(55);
  delay(100);
  forward(0);
 
  while(right <= 35) {
    left = analogRead(LINE_FOLLOW_L);
    right = analogRead(LINE_FOLLOW_R);

    if(right > 35) break;
    
    digitalWrite(MOTOR_L1_PIN, LOW);
    analogWrite(MOTOR_L2_PIN, 65);
    digitalWrite(MOTOR_R1_PIN, LOW);
    analogWrite(MOTOR_R2_PIN, 65);
    
    delay(25);
    forward(0);
    delay(25);
  }
  forward(0);

  while(left <= 35) {
    left = analogRead(LINE_FOLLOW_L);
    right = analogRead(LINE_FOLLOW_R);

    if(right > 35) break;
    
    digitalWrite(MOTOR_L1_PIN, LOW);
    analogWrite(MOTOR_L2_PIN, 65);
    digitalWrite(MOTOR_R1_PIN, LOW);
    analogWrite(MOTOR_R2_PIN, 65);

    delay(25);
    forward(0);
    delay(25);
  }
  forward(0);

  while(right > 30) {
    left = analogRead(LINE_FOLLOW_L);
    right = analogRead(LINE_FOLLOW_R);

    if(right <= 30) break;
    
    digitalWrite(MOTOR_L1_PIN, LOW);
    analogWrite(MOTOR_L2_PIN, 65);
    digitalWrite(MOTOR_R1_PIN, LOW);
    analogWrite(MOTOR_R2_PIN, 65);

    delay(25);
    forward(0);
    delay(25);
  }
  forward(0);
  
  forward(55);
  delay(200);
  forward(0);

  /*commandLineFollow();
  forward(55);
  delay(50);
  forward(0);*/
}

int searchIndexOf(String arr[], String str) {
  int idx = 0;
  for(int i = 0; i < commandIndex ; i++) {
    if(arr[i] == str) {
      idx = i;
      break;
    }
  }

  return idx;
}

