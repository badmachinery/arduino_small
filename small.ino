#include <Wire.h>

#define F_TRIG 3 
#define L_TRIG 4
#define R_TRIG 11

#define L_PIN A1
#define R_PIN A2
#define F_PIN A0

int32_t F_distance = 0;
int32_t R_distance = 0;
int32_t L_distance = 0;

//constants
#define l_speed  10
#define l_plus  9
#define l_minus  8

#define r_speed   5
#define r_plus  7
#define r_minus  6

#define left_engine  0 
#define right_engine  1
#define dEngine  2

//commands
#define STOP 0
#define FORWARD  42
#define BACKWARD  41

int state = STOP;
int leftEngineState = STOP;
int rightEngineState = STOP;
int _speed = 0;
int buf[3][20];

// EGORCHIK
#define CONTROL_MANUAL 0
#define CONTROL_SCRIPT 1

int control_state = CONTROL_MANUAL;



int readVal(char &command){
  int tmp = Serial.read();
  while(tmp!='s' && tmp!='r' && tmp != 'u' && tmp != 'w')
    tmp = Serial.read();
  command = tmp;
  tmp = Serial.parseInt();
  return tmp;
}

void initEngines(){
  digitalWrite(l_speed, OUTPUT);
  digitalWrite(l_plus, OUTPUT);
  digitalWrite(l_minus, OUTPUT);
  
  digitalWrite(r_speed, OUTPUT);
  digitalWrite(r_plus, OUTPUT);
  digitalWrite(r_minus, OUTPUT);
}


void engine(int _engine, int type = FORWARD){
  int engine_plus = l_plus;
  int engine_minus = l_minus;
  if(_engine == left_engine){
    engine_plus = l_plus;
    engine_minus = l_minus;
  } else {
    engine_plus = r_plus;
    engine_minus = r_minus;
  }
  
  if(type == FORWARD){
    digitalWrite(engine_plus, LOW);
    digitalWrite(engine_minus,HIGH);
    
  } else {
    digitalWrite(engine_plus, HIGH);
    digitalWrite(engine_minus, LOW);
    
  }
  if(_engine == dEngine)
    engine(left_engine, type);
}

void eMove(int _engine = dEngine, int type = FORWARD,  int val = 0){
  engine(_engine, type);
  if(_engine != left_engine) analogWrite(r_speed,val);
  if((_engine == left_engine) || (_engine == dEngine)) analogWrite(l_speed,val);
}


void Rotation(int val){
  int leftPow = _speed, rightPow = _speed;
  int leftState = state, rightState = state;
  
  if(state == BACKWARD){
    if(val > 0)
      rightPow = _speed / 100.0 * (100 - val);
    else if (val < 0) 
      leftPow = _speed / 100.0 * (100 + val);
      
  } else if (state == FORWARD){
     if(val > 0)
       rightPow = _speed / 100.0 * (100 - val);
     else if (val < 0) 
       leftPow = _speed / 100.0 * (100 + val);
           
  } else if(state == STOP){
    leftPow = abs(val);
    rightPow = abs(val);
    if(val > 0) {
      leftState = FORWARD;
      rightState = BACKWARD;
    } else {
      rightState = FORWARD;
      leftState = BACKWARD;
    }
  }  
  
  eMove(left_engine, leftState, 255 / 100.0 * leftPow);
  eMove(right_engine, rightState, 255 / 100.0 * rightPow);
}

void Move(int power)
{
      state = power > 0 ? FORWARD : BACKWARD;
      if(power == 0) state = STOP;
      _speed = abs(power);
      
      eMove(dEngine, state, 255 / 100.0 * abs(power));
}

void handleCommand(char command, int value)
{
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
      script();
    }
  } else {
    if(command == 'w'){
      control_state = CONTROL_MANUAL;
    }
  }
}

void prepareSensor(int sensorPin){
  pinMode(sensorPin,OUTPUT);        
  digitalWrite(sensorPin,HIGH);                 
}

void setStateUndoIn(int sensorPin){
  digitalWrite(sensorPin, LOW);
  digitalWrite(sensorPin, HIGH);  
}

void readData(){ 
  setStateUndoIn(F_TRIG);                
  F_distance = analogRead(F_PIN); 
  
  setStateUndoIn(L_TRIG);                 
  L_distance = analogRead(L_PIN); 
  
  setStateUndoIn(R_TRIG);                 
  R_distance = analogRead(R_PIN); 
  
  F_distance = F_distance <=10 ? -1 : F_distance * 0.718;
  L_distance = L_distance <=10 ? -1 : L_distance * 0.718;
  R_distance = R_distance <=10 ? -1 : R_distance * 0.718;
}

void updateSensorData(){
  for(int i = 0; i<20; ++i){
    readData();
    buf[0][i] = L_distance;
    buf[1][i] = R_distance;
    buf[2][i] = F_distance; 
  }
  sort(0);
  sort(1);
  sort(2);

  L_distance = buf[0][11];
  R_distance = buf[1][11];
  F_distance = buf[2][11];
}

void sort(int cur){
  int n = 20;
  for(int i = 0; i<n; ++i){
    for(int k = 0; k<n-i-1; ++k){
      if(buf[cur][k] > buf[cur][k+1]){
        int _t = buf[cur][k];
        buf[cur][k] = buf[cur][k+1];
        buf[cur][k+1] = _t;
      }
    }
  }
}

void sendSonarData(){
  Serial.print("F");
  Serial.println(F_distance);
  
  Serial.print("L");
  Serial.println(L_distance);
  
  Serial.print("R");
  Serial.println(R_distance);
}

void readCommands()
{
  if(Serial.available() > 0)
  {
    char command;
    int value = readVal(command);
    handleCommand(command, value);
  }
}

void rotateRight(uint32_t milliseconds)
{
    uint32_t timer = millis();

    while (millis() - timer < milliseconds)
      Rotation(100);
}

void rotateLeft(uint32_t milliseconds)
{
    uint32_t timer = millis();

    while (millis() - timer < milliseconds)
      Rotation(-100);
}

void moveTime(int power, uint32_t milliseconds)
{
    uint32_t timer = millis();

    while (millis() - timer < milliseconds)
      Move(100);
}

void Pause()
{
    uint32_t timer = millis();

    while (millis() - timer < 500)
      Move(0);
}

void script()
{
  while(1)
  {
    readCommands();
    if (control_state != CONTROL_SCRIPT)
      return;

    updateSensorData();
    if (F_distance < 30 and F_distance != -1) 
    {
      control_state = CONTROL_MANUAL;
      return;
    }

    if (L_distance < 30 and L_distance != -1)
    {
      /*
      Pause();
      rotateRight(350);
      Pause();
      moveTime(100, 400);
      Pause();
      rotateLeft(350);
      Pause();
      */
      Move(100);
      Rotation(80);
    } else 
        if (R_distance < 30 and R_distance != -1)
        {
          /*
          Pause();
          rotateLeft(350);
          Pause();          moveTime(100, 400);
          Pause();
          rotateRight(350);
          Pause();
          */
          Move(100);
          Rotation(-80);
        }
        else
          Move(100);
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

void loop() {
  readCommands();
}+
