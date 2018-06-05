#include <Wire.h>

//constants
int l_speed = 10;
int l_plus = 9;
int l_minus = 8;

int r_speed = 5;
int r_plus = 7;
int r_minus = 6;

int left_engine = 0; 
int right_engine = 1;
int dEngine = 2;

//commands
int STOP = 0;
int FORWARD = 42;
int BACKWARD = 41;

int state = STOP;

int readVal(char &check){
  int tmp = Serial.read();
  while(tmp!='s' && tmp!='r' && tmp!='b')
    tmp = Serial.read();
  check = tmp;
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
  if(state != FORWARD) {
    engine(_engine, FORWARD);
    state = FORWARD;
  }
  else {
    engine(_engine, BACKWARD);
    state = BACKWARD;
  }
  digitalWrite(l_speed,val);
  digitalWrite(r_speed,val);
}

void command_handler(char check, int tmp){
  if(tmp!=-1){
      if(check == 's'){
        if (tmp > 0)
          eMove(dEngine, FORWARD, 255 / 100.0 * tmp);
        else
          eMove(dEngine, BACKWARD, 255 / 100.0 * tmp * -1);
      } else if(check == 'r') {
        if (tmp > 0){
          eMove(left_engine, FORWARD, 255 / 100.0 * tmp);
          eMove(right_engine, BACKWARD, 255 / 100.0 * tmp);
        } else {
          eMove(left_engine, BACKWARD, 255 / 100.0 * tmp * -1);
          eMove(right_engine, FORWARD, 255 / 100.0 * tmp * -1);
        }
      }
    }
}

void setup() {
  initEngines();
  Wire.begin();
  Serial.begin(115200);
}

void loop() {
  if(Serial.available() > 0){
    char check;
    int tmp = readVal(check);
    Serial.println(String(check) + " " + String(tmp));
    command_handler(check, tmp);
  }
}
