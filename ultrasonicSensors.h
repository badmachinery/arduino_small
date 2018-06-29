#define F_TRIG 3 
#define L_TRIG 4
#define R_TRIG 11

#define L_PIN A1
#define R_PIN A2
#define F_PIN A0

#define L_SENSOR 0 
#define R_SENSOR 1 
#define F_SENSOR 2 

int32_t F_distance = 0;
int32_t R_distance = 0;
int32_t L_distance = 0;

#define ultrasonicDataSize 30

int buf[3][ultrasonicDataSize];

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


void sort(int cur){
  int n = ultrasonicDataSize;
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

void updateSensorData(){
  for(int i = 0; i<ultrasonicDataSize; ++i){
    readData();
    buf[0][i] = L_distance;
    buf[1][i] = R_distance;
    buf[2][i] = F_distance; 
  }
  sort(0);
  sort(1);
  sort(2);

  L_distance = buf[0][ultrasonicDataSize/2];
  R_distance = buf[1][ultrasonicDataSize/2];
  F_distance = buf[2][ultrasonicDataSize/2];
}

void sendSonarData(){
  Serial.print("F");
  Serial.println(F_distance);
  
  Serial.print("L");
  Serial.println(L_distance);
  
  Serial.print("R");
  Serial.println(R_distance);
}

