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
String COMMAND[50];
bool exec = false;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

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
  
  for(int i = 0; i < 50; i++) {
    COMMAND[i] = ""; 
  }
}

void loop() {  
  readRFID();

  if(exec) {
    int commandLength = 0;
    if(searchIndexOf(COMMAND, "repeat") > -1) {
      commandLength = 0;
    } else {
      commandLength = commandIndex;
    }

    Serial.println("=====execute=====");
    String finalArray[50];
    while(searchIndexOf(COMMAND, "repeat") > -1) {
      Serial.println("=====process=====");
      int repeatIndex = searchIndexOf(COMMAND, "repeat");
      int numberIndex = repeatIndex + 1;
      int breakIndex = searchIndexOf(COMMAND, "break");
  
      // String finalArray[repeatIndex + (COMMAND[numberIndex].toInt() * (breakIndex - numberIndex - 1)) + (commandIndex - breakIndex - 1)];
      int tempIndex4Repeat = repeatIndex;

      Serial.println("Step 1: get prefix");
      for(int i = 0; i < repeatIndex; i++) {
        finalArray[i] = COMMAND[i];
        Serial.print(finalArray[i]);
        Serial.print(", ");
      }
      Serial.println("");

      Serial.println("Step 2: get repeat command");
      for(int i = 0; i < COMMAND[numberIndex].toInt(); i++) {
        for(int j = numberIndex + 1; j < breakIndex; j++) {
          finalArray[tempIndex4Repeat] = COMMAND[j];
          Serial.print(finalArray[tempIndex4Repeat]);
          Serial.print(", ");
          tempIndex4Repeat++;
        }
      }
      Serial.println("");

      Serial.println("Step 3: get remain");
      for(int i = breakIndex + 1; i < commandIndex; i++) {
        finalArray[tempIndex4Repeat] = COMMAND[i];
        Serial.print(finalArray[tempIndex4Repeat]);
        Serial.print(", ");
        tempIndex4Repeat++;
      }
      Serial.println("");

      Serial.println("Step 4: clear COMMAND");
      for(int i = 0; i < commandIndex; i++) {
        COMMAND[i] = "";
      }
      
      Serial.println("Step 5: print Final");
      for(int i = 0; i < tempIndex4Repeat; i++) {
        Serial.print(finalArray[i]);
        Serial.print(", ");
      }
      Serial.println("");

      commandLength = tempIndex4Repeat;
      for(int i = 0; i < tempIndex4Repeat; i++) {
        COMMAND[i] = finalArray[i];
      }
    }
    Serial.print("Result(");
    Serial.print(commandLength);
    Serial.println("): ");
    for(int i = 0; i < commandLength; i++) {
      Serial.print(COMMAND[i]);
      Serial.print(", ");
    }
    Serial.println("");
    Serial.println("====terminate====");
    actionCardContent(COMMAND, commandLength);
  }
}

void readRFID() {
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  byte block;
  byte len;
  MFRC522::StatusCode status;

  if (! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

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
  for (uint8_t i = 0; i < 16; i++) {        
    if (buffer1[i] != 32) {
      char value = buffer1[i];
      if(buffer1[i] != 10 && buffer1[i] != 13) {
        currentRead += value;
        COMMAND[commandIndex] += value;
      }
    }
  }
  
  commandIndex++;
  Serial.print("Command: ");
  Serial.println(currentRead);

  if(currentRead.equals("action")) {
    exec = true;
  }
 
  tone(2, 1000, 100);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void actionCardContent(String command[], int commandLength) {
  for(int i = 0; i < commandLength; i++) {
    // Serial.println(command[i]);
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

  for(int i = 0; i < commandLength; i++) {
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

void move(int leftSpeed, int rightSpeed) {
  if(leftSpeed >= 0) {
    digitalWrite(MOTOR_L1_PIN, LOW);
    analogWrite(MOTOR_L2_PIN, leftSpeed);
  } else {
    digitalWrite(MOTOR_L1_PIN, HIGH);
    analogWrite(MOTOR_L2_PIN, 255 - leftSpeed * -1);
  }

  if(rightSpeed >= 0) {
    digitalWrite(MOTOR_R1_PIN, HIGH);
    analogWrite(MOTOR_R2_PIN, 255 - rightSpeed);
  } else {
    digitalWrite(MOTOR_R1_PIN, LOW);
    analogWrite(MOTOR_R2_PIN, rightSpeed * -1);
  }
}

bool lineFollow(int speed) {
  if(getLineFollow() == 0) {
    forward(speed);
  }

  if(getLineFollow() == 1) {
    turnLeft(speed);
  }

  if(getLineFollow() == 2) {
    turnRight(speed);
  }

  if(getLineFollow() == 3) {
    forward(0);
    return false;
  }
  
  return true;
}

void commandLineFollow() {
  while(lineFollow(55)) {}
  move(55, 55);
  delay(100);
  move(0, 0);
}

void commandTurnLeft() {
  while(getLineFollow() != 0) {
    move(0, 50);
  }
  move(0, 0);
}

void commandTurnRight() {
  while(getLineFollow() != 0) {
    move(50, 0);
  }
  move(0, 0);
}

int getLineFollow() {
  int leftLine = analogRead(LINE_FOLLOW_L);
  int rightLine = analogRead(LINE_FOLLOW_R);
  int threshold = 200;

  if(leftLine >= threshold && rightLine >= threshold) {
    return 0;
  } else if(leftLine >= threshold && rightLine < threshold) {
    return 1;
  } else if(leftLine < threshold && rightLine >= threshold) {
    return 2;
  } else if(leftLine < threshold && rightLine < threshold) {
    return 3;
  }
}

int searchIndexOf(String arr[], String str) {
  int idx = -1;
  for(int i = 0; i < commandIndex ; i++) {
    if(arr[i] == str) {
      idx = i;
      break;
    }
  }

  return idx;
}

