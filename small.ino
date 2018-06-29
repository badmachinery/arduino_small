#include <Wire.h>
#include "ultrasonicSensors.h"
#include "engine.h"

#define battery A4

// EGORCHIK
#define CONTROL_MANUAL 0
#define CONTROL_SCRIPT 1

int control_state = CONTROL_MANUAL;

//global variables
float batteries = 0;

extern int32_t F_distance;
extern int32_t R_distance;
extern int32_t L_distance;

extern int state;
extern int leftEngineState;
extern int rightEngineState;
extern int _speed;

void updateBatteryCapacity(){
   batteries = analogRead(battery)*2.54*0.004882;
}

int readVal(char &command){
  int tmp = Serial.read();
  while(tmp!='s' && tmp!='r' && tmp != 'u' && tmp != 'w' && tmp != 'c')
    tmp = Serial.read();
  command = tmp;
  tmp = Serial.parseInt();
  return tmp;
}

void  handleCommand(char, int);

void readCommands(){
  if(Serial.available() > 0){
    char command;
    int value = readVal(command);
    handleCommand(command, value);
  }
}

bool measurementsWhileRotate(int32_t &sensorMinValue, int32_t &attachedSensorDistance, int delayTime, int speedRotation){
   Rotation(speedRotation);
   delay(delayTime);
   //Move(STOP);
   updateSensorData();
   if((sensorMinValue > attachedSensorDistance) && (attachedSensorDistance != -1)){
     sensorMinValue = attachedSensorDistance;
     return true;
   }
   return false;
}

void correctPosition(int32_t &attachedSensorDistance, int speedRotation = 70, int delayTime = 3){
   int32_t sensorMinValue = 1000;
   int trashSensorValueCount = 0;
   int metricSensorCount = 0;
   char dimensionSide = 'F';
   int dimensionVal = 0;
   int rLim = 55;
   int lLim = 48;
   for(int i=0; i<rLim; ++i){
      bool res = measurementsWhileRotate(sensorMinValue, attachedSensorDistance, delayTime, speedRotation);
      Move(STOP); ///***detailed attention ***///
      ++metricSensorCount;
      if(attachedSensorDistance == -1)
         ++trashSensorValueCount; 
      if(res){
         dimensionSide = 'R';
         dimensionVal = i;
      }    
   }
   for(int i=lLim; i>0; --i){
      //state = STOP;
      bool res = measurementsWhileRotate(sensorMinValue, attachedSensorDistance, delayTime, -speedRotation);
      Move(STOP);
      ++metricSensorCount;
      if(attachedSensorDistance == -1)
         ++trashSensorValueCount; 
      if(res){
         //dimensionSide = 'R';
         //dimensionVal = i;
      } 
   }
   for(int i=0; i>-lLim; --i){
      //state = STOP;
      bool res = measurementsWhileRotate(sensorMinValue, attachedSensorDistance, delayTime, -speedRotation);
      Move(STOP);
      ++metricSensorCount;
      if(attachedSensorDistance == -1)
         ++trashSensorValueCount; 
      if(res){
         dimensionSide = 'L';
         dimensionVal = abs(i);
      } 
   }
   for(int i=-rLim; i<0; ++i){
      //state = STOP;
      bool res = measurementsWhileRotate(sensorMinValue, attachedSensorDistance, delayTime, speedRotation);
      Move(STOP);
      ++metricSensorCount;
      if(attachedSensorDistance == -1)
        ++trashSensorValueCount; 
      if(res){
         //dimensionSide = 'L';
         //dimensionVal = abs(i);
      } 
   }   
   int rotateInc = dimensionSide == 'L' ? -1 : 1;
   for(int i = dimensionVal; i!=0; --i){
      Rotation(speedRotation*rotateInc);
      delay(delayTime);
   }
   //Serial.println(dimensionVal);
   Move(STOP);
}

void moveToHand(){
  updateSensorData();
  sendSonarData();
  while (control_state == CONTROL_SCRIPT){
    if(F_distance < 15 && F_distance!=-1){
      Move(100);
      delay(10);
      Move(STOP);
    }
    updateSensorData();
    readCommands();
  }
}



void twoSensorsRotate(int32_t &attachedSensorA, int32_t &attachedSensorB, int speedRotate = 80, int accuracy = 5){
  updateSensorData();
  while((attachedSensorA == -1 || attachedSensorB == -1) || abs(attachedSensorA - attachedSensorB) > accuracy){
    updateSensorData();
    Rotation(speedRotate);
  }
  Move(STOP);
}

void upgradedScript2(int32_t &attachedSensorA, int32_t &attachedSensorB, int sensorsDelta = 10){
  while(true){
    readCommands();
    if(F_distance < 30 && F_distance!=-1){
        Move(STOP);
        control_state = CONTROL_MANUAL;
    }
    if (control_state != CONTROL_SCRIPT)
      return;
    updateSensorData();
    if(attachedSensorB != -1 && attachedSensorA != -1){
        int speedRotate = attachedSensorB < attachedSensorA ? -80 : 80;
        twoSensorsRotate(attachedSensorA, attachedSensorB, speedRotate, 5);
        /*if(abs(attachedSensorA < attachedSensorB) > sensorsDelta){
          twoSensorsRotate(attachedSensorA, attachedSensorB, speedRotate, 2);
          while((attachedSensorA == -1 || attachedSensorB == -1))
            updateSensorData();
          if(attachedSensorA < 50){
            Rotation(80);
            delay(2000);
            Move(STOP);
            Move(BACKWARD);
            delay(200);
          } else if (attachedSensorB < 50){
            Rotation(80);
            delay(2000);
            Move(STOP);
            Move(BACKWARD);
            delay(200);
          }*/
          //twoSensorsRotate(attachedSensorA, attachedSensorB, speedRotate);
        //}
    }   
    Move(FORWARD);
  }
}

#define ROTATION 2
#define MOVE 3

void upgradedScript(int32_t &attachedSensorDistance, int controlSensor = L_SENSOR, int timerDelay = 1000, int distanceWall = 50){
  updateSensorData();
  bool moveStatus = MOVE;

  int32_t prev_F_distance = F_distance;
  int32_t prev_R_distance = R_distance;
  int32_t prev_L_distance = L_distance;
  int32_t prev_attached_distance = attachedSensorDistance;
  
  uint32_t statusTime = millis();
  Move(FORWARD);
  while(true){
    readCommands();
    if(F_distance < 30 && F_distance!=-1){
        Move(STOP);
        moveStatus = STOP;
    }
   
    if (control_state != CONTROL_SCRIPT)
      return;
      
    updateSensorData();
    if(moveStatus == ROTATION){
        if(millis() - statusTime > timerDelay){
            if( (abs(distanceWall - attachedSensorDistance) > 20) && (attachedSensorDistance != -1)){
                int rotationVal = controlSensor == L_SENSOR ? -100 : 100;
                Move(FORWARD);
                Rotation(rotationVal);
            } else if((abs(distanceWall - attachedSensorDistance) <= 10)){
                moveStatus = MOVE;
                statusTime = millis();
                Move(FORWARD);
            }
        }
    } else if (moveStatus == MOVE){
        if(millis() - statusTime > timerDelay){
             if((abs(distanceWall - attachedSensorDistance) > 20) && (attachedSensorDistance != -1)){
                int rotationVal = controlSensor == L_SENSOR ? -100 : 100;
                Move(FORWARD);
                Rotation(rotationVal);
                moveStatus = ROTATION;
                statusTime = millis();
             }
        }
    }
  }
}

void handleCommand(char command, int value){
  if (command == 'c'){
    updateBatteryCapacity();     
    Serial.println(analogRead(battery)); 
    Serial.println("c" + String(batteries)); 
  }
  if (control_state == CONTROL_MANUAL){
    if(command == 's'){
        Move(value);
    } else if(command == 'r'){ 
        Rotation(value);
    } else if(command == 'u'){
        updateSensorData();
        sendSonarData();   
    } else if(command == 'w'){
      control_state = CONTROL_SCRIPT;
      //moveToHand();
      //correctPosition(F_distance);
      //upgradedScript(L_distance);
      //twoSensorsRotate(R_distance, L_distance);
      upgradedScript2(L_distance, R_distance);
    }
  } else {
    if(command == 'w'){
      control_state = CONTROL_MANUAL;
    }
  }
}

void setup() {
  initEngines();
  Wire.begin();
  
  prepareSensor(F_TRIG);
  prepareSensor(L_TRIG);
  prepareSensor(R_TRIG);
  
  Serial.begin(115200);
}

int reg = 0;
uint32_t _time = millis();

void loop() {
  readCommands();
}
