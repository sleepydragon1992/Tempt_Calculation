#include <Wire.h>
#include <math.h>
/*******************************************************************/
  //constant declaration
double Sensor_Tempt_Min = -20;
double Sensor_Tempt_Max = 85;
double Obj_Tempt_Min = 0;
double Obj_Tempt_Max = 100;


/**********************************************************************/
//variable initialization
int reading = 0;
long int reading_2 = 0;

long unsigned int ADC_obj;
long unsigned int ADC_sen;

float TC;
float T_Ref;
float k4_comp;
float k3_comp;
float k2_comp;
float k1_comp;
float k0_comp;
float k4_obj;
float k3_obj;
float k2_obj;
float k1_obj;
float k0_obj;
long unsigned int i;
long unsigned int j;
long unsigned int k;



/*******************************************************************/
  //function declaration

  //converting function from I2C input to readable unit
union {
  long unsigned int i;
  float f;
}MyUnion;

long unsigned int combine_hex (unsigned int H,unsigned int L){
  long unsigned SUM = H * pow(2,16) + L;
  return SUM;
}

float floatify(long unsigned x){
  MyUnion.i = x;
  return MyUnion.f;
}


long unsigned int read_EEPROM (int mybyte){
  reading = 0;
  reading_2 = 0;
  Wire.beginTransmission(0x00);   //start a serial communication with device address 0 
    Wire.write(byte(mybyte));
    Wire.endTransmission();
    delay(70);
    Wire.requestFrom(0x00,4);
    delay(50);
    for (int i = 0; i <= 2; i++){
      reading=Wire.read();
      if(i == 1) reading_2 = reading;
      if(i == 2) reading_2 = reading_2 * pow(2,8) + reading;  
    }
    delay(50);
    return reading_2;
}

float read_IEEE754 (int mybyte){
  i = read_EEPROM(byte(mybyte));
  j = read_EEPROM(byte(mybyte+1));
  k = combine_hex(i,j);
  return floatify(k);
}

void read_ADC(){
  
  Wire.beginTransmission(0x00);
  Wire.write(byte(0xAF));
  Wire.endTransmission();
  delay(70);
  Wire.requestFrom(0x00,8);
  delay(50);
      for (int i = 0; i <= 6; i++){
      reading=Wire.read();
      if(i == 1) ADC_obj = reading;
      if(i == 2) ADC_obj = ADC_obj * pow(2,8) + reading;  
      if(i == 3) ADC_obj = ADC_obj * pow(2,8) + reading;
      if(i == 4) ADC_sen = reading;
      if(i == 5) ADC_sen = ADC_sen * pow(2,8) + reading;
      if(i == 6) ADC_sen = ADC_sen * pow(2,8) + reading;
    }
    delay (50);
}

/*************************************************************/
//Math function
double Calculate_Sensor_Tempt (long unsigned int ADC_a, double Tmax, double Tmin){
    return (ADC_a/pow(2,24))*(Tmax-Tmin)+Tmin;
}

double Calculate_TCF(double T_sen, double T_ref, double Tempt_Coef){
  return 1 + (T_sen-T_ref)*Tempt_Coef;
}

double Calculate_Offset_TC(double k4, double k3, double k2, double k1, double k0,double Sensor_Tempt,double TCF){
  double offset = k4* pow(Sensor_Tempt,4) +k3 * pow(Sensor_Tempt,3) + k2 * pow(Sensor_Tempt,2) + k1 * pow(Sensor_Tempt,1) + k0;
  return offset * TCF;
}

double Calculate_Obj_Tempt(double k4, double k3, double k2, double k1, double k0, double ADC_obj, double TCF, double Offset_TC){
  double ADC_Comp = Offset_TC + ADC_obj - pow(2,23);
  double ADC_Comp_TC = ADC_Comp/TCF;
  return k4*pow(ADC_Comp_TC,4) + k3*pow(ADC_Comp_TC,3) + k2*pow(ADC_Comp_TC,2) + k1*pow(ADC_Comp_TC,1) + k0;
}

/****************************************************************/
//Set up initialization

void setup() { 
  Wire.begin();
  Serial.begin (9600);
  TC = read_IEEE754(0x1E);
  T_Ref = read_IEEE754(0x20);
  k4_comp = read_IEEE754(0x22);
  k3_comp = read_IEEE754(0x24);
  k2_comp = read_IEEE754(0x26);
  k1_comp = read_IEEE754(0x28);
  k0_comp = read_IEEE754(0x2A);
  k4_obj =  read_IEEE754(0x2E);
  k3_obj =  read_IEEE754(0x30);
  k2_obj =  read_IEEE754(0x32);
  k1_obj =  read_IEEE754(0x34);
  k0_obj =  read_IEEE754(0x36);
  
  delay(1000);
}

void loop() {

  read_ADC();
  delay(100);
  unsigned int y = ADC_sen;
  double Sensor_Tempt = Calculate_Sensor_Tempt(ADC_sen, Sensor_Tempt_Max, Sensor_Tempt_Min);
  double TCF = Calculate_TCF(Sensor_Tempt,T_Ref,TC);
  double Offset_TC = Calculate_Offset_TC(k4_comp,k3_comp,k2_comp,k1_comp,k0_comp,Sensor_Tempt,TCF);
  double Obj_Tempt = Calculate_Obj_Tempt(k4_obj,k3_obj,k2_obj,k1_obj,k0_obj,ADC_obj,TCF,Offset_TC);

  Serial.println("******");
  Serial.println("Sensor temperature is: ");
  Serial.println(Sensor_Tempt);
  Serial.println("");
  Serial.println("Object temperature is: ");
  Serial.println(Obj_Tempt);
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  delay(1000);
  }


  

