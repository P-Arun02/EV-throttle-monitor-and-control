// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP8266_HARDSERIAL_CLOUD                                    //esp8266 connected to arduino via hardserial

#include <RemoteXY.h>                                                              //including RemoteXY library
#include<Wire.h>                                                                   //including wire library to communicate with I2C devices
#include<LiquidCrystal_I2C.h>                                                      //including I2C LCD library to control LCD
#include<string.h>                                                                 //including strings library for string manipulation

// RemoteXY connection settings 
#define REMOTEXY_SERIAL Serial
#define REMOTEXY_SERIAL_SPEED 9600                                                 //baud rate
#define REMOTEXY_WIFI_SSID "ARUN-PAVILION"                                         //my  wifi name
#define REMOTEXY_WIFI_PASSWORD "thankyoutokitou"                                   //my wifi password
#define REMOTEXY_CLOUD_SERVER "cloud.remotexy.com"                                 //RemoteXY cloud server web address
#define REMOTEXY_CLOUD_PORT 6376                                                   //server port for cloud server
#define REMOTEXY_CLOUD_TOKEN "fea02a733d3f8c13f06993c062f0307d"                    //token to log in


// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =                                                          // 86 bytes
  { 255,1,0,26,0,79,0,16,180,0,68,17,28,32,68,28,8,36,4,0,
  9,31,6,28,2,26,129,0,3,6,92,6,38,84,72,82,79,84,84,76,
  69,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,68,73,
  82,69,67,84,73,79,78,0,67,1,6,15,28,5,32,26,11,67,1,63,
  15,32,5,32,180,11 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t slide;                                                                    // =0..100 slider position 

    // output variables
  float graph;
  char acc[11];                                                                    // string UTF8 end zero 
  char dir[11];                                                                    // string UTF8 end zero 

    // other variable
  uint8_t connect_flag;                                                            // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)


LiquidCrystal_I2C lcd(0x27,16,2);                                                  //defining lcd => I2C address = 0x27, 16 x 2 display

#define start_hall 179                                                             //value of ADC(hall sensor) with 0% throttle 
#define end_hall 880                                                               //value of ADC(hall sensor) with 100% throttle 
#define aver 3                                                                     //number of times hall sensor reading should be taken for a loop
#define tolerance 5                                                                //tolerance for middle values of throttle
#define tolerance2 10                                                              //tolerance for initial and final values throttle

int value_prev,value_pres,direct,control;                          
control = 0;                                                                       //variable specifing the online control                     
float percent;                                                                     //throttle percent 
void setup() 
{
  RemoteXY_Init ();                                                                //initialization of REMOTEXY module
  pinMode(8,INPUT_PULLUP);                                                         //8th pin set as digital input pin with iternal pullup
  pinMode(9,INPUT_PULLUP);                                                         //9th pin set as digital input pin with iternal pullup
  lcd.init();                                                                      //initialize the lcd
  lcd.backlight();                                                                 //open the backlight 
  lcd.setCursor(0,0);                                                              //set lcd cursor to row: 0 and col : 0
  lcd.print("THROTTLE :     %");
  lcd.setCursor(0,1);                                                              //set lcd cursor to row: 1 and col : 0
  lcd.print("DIRECTION : "); 
  
}

void loop() 
{ 
  RemoteXY_Handler ();                                                             //updating the RemoteXY structure variables to cloud in loop
    direct = digitalRead(8);                                                       //read direction pin (forward or reverse)
  lcd.setCursor(12,1);                                                             //set lcd cursor to row: 0 and col : 0
  if(direct == 0){                                                                 //if reverse
    lcd.print("REV");
    strcpy(d,"REVERSE");                  
    strcpy(RemoteXY.dir,"REVERSE");                                                //store the string "REVERSE" in RemoteXY structure variable - dir
  }
  else{
    lcd.print("FOR");             
    strcpy(RemoteXY.dir,"FORWARD");                                                //store the string "REVERSE" in RemoteXY structure variable - dir
  }
  
  control = digitalRead(9);                                                        //read online control pin (enable online control or not)                              
  if(control){                                                                     //if online control was not enabled
    value_pres = 0;
  for(int i = 0;i < aver;i++){                                                     //do read the analog pin A0(output of HALL sensor) for n number of times(n - aver)
      value_pres += analogRead(A0);                                                //make sum of hall sensor reading
    }
  value_pres /= aver;                                                              //make average of the HALL sensor reading

  
  if(abs(value_pres - hall_start) < tolerance2)                                    //if throttle is near 0% 
    value_pres = hall_start;                                                       //set throttle to 0%
  else if(abs(value_pres - hall_end) < tolerance2)                                 //if throttle is near 100% 
    value_pres = hall_end;                                                         //set throttle to 100%
  else if(abs(value_pres - value_prev) < tolerance)                                //if throttle change value is lesser than tolerace
    value_pres = value_prev;                                                       //don't change throttle value
  value_prev = value_pres;

  percent = ((value_pres - hall_start) * 100.0) / (hall_end - hall_start);         //ADC hall sensor value to percent
  }
  else                                                                             //if online control is enabled
    percent = (float)RemoteXY.slide;                                               //read slider value from cloud 
  
  dtostrf(percent , 5 , 1 , RemoteXY.acc);                                         //convert percent to string and send to cloud
  RemoteXY.graph = percent;                                                        //send percent value to cloud for graph plotting
  lcd.setCursor(10,0);                                                             //set lcd cursor to row: 0 and col : 10
  lcd.print(t); 
}
