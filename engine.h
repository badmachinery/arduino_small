//commands
#define STOP 0
#define FORWARD  100
#define BACKWARD  -100

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

int state = STOP;
int leftEngineState = STOP;
int rightEngineState = STOP;
int _speed = 0;

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

void Move(int power) {
      state = power > 0 ? FORWARD : BACKWARD;
      if(power == 0) state = STOP;
      _speed = abs(power);
      eMove(dEngine, state, 255 / 100.0 * abs(power));
}


