 /* ////////////////////////////////////////////////////////////////////////////
** File:      LowLevelSupervisor.ino
*/                                  
 unsigned char  Ver[] = "LinoLLS     1.0.0 Guiott 10-13"; // 30+1 char

/**
* \mainpage LowLevelSupervisor.ino
* \author    Guido Ottaviani-->guido@guiott.com<--
* \version 1.0.0
* \date 10/13
* \details 
 *
-------------------------------------------------------------------------------
\copyright 2012-2013 Guido Ottaviani
guido@guiott.com

    LowLevelSupervisor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LowLevelSupervisor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LowLevelSupervisor.  If not, see <http://www.gnu.org/licenses/>.

-------------------------------------------------------------------------------
*/
// Compiler options. Define and timeouts should be modified to perform an easier debug
//#define DEBUG_MODE // If defined the serial output is in ASCII for debug
// #define DEMO_MODE  // If defined it switches off at the end
unsigned long Timeout=50;      // timeout in ms once RX packet started. It will be adapted on the serial speed
unsigned long RxTimeout = 0;   // 500; // HLS communication timeout in ms: no communication at all. "0" means no timeout control 
unsigned long I2cTimeout = 0;   // 500; // Sonar board communication timeout in ms: no communication at all
unsigned long ShutdownHlsTimeout = 99000; // HLS shutdown replay timeout in ms
unsigned long StartupHlsTimeout = 255000;  // HLS startup  timeout in ms


#include <Wire.h>
#include <Metro.h>
#include <Time.h>

// Digital OUT
int Led = 13;             // standard yellow led            
int Batt_1_En = 46;       // Enable 14.4V on PowerControl 1
int Pwr_1_En  = 48;       // Enable 7V on PowerControl 1         
int Batt_2_En = 50;       // Enable 14.4V on PowerControl 2               
int Pwr_2_En  = 52;       // Enable 7V on PowerControl 2                
int Sw_Power_latch = 53;  // Relay self retention on LL board
int Buzzer = 51;          // Buzzer on LLS board
int Sw_Off_Btn = 44;      // Software Off Button
int Hls_Pwr_Off = 42;     // Hls Power check

// PWM
int Light_L = 2;          // Headlights intensity control
int Light_R = 3;


//Analog IN
int V7 = A4;              // Switching power supply on LLS board
int Pwr1_Vin = A3;        // Switching power supply on PwrCtrl 1 board
int Pwr2_Vin = A2;        // Switching power supply on PwrCtrl 2 board
int Batt2_Vin = A1;       // Battery 1 level
int Batt1_Vin = A0;       // Battery 2 level
int Temp1 = A5;           // Temperature sensor 1
int Temp2 = A6;           // Temperature sensor 2

//Analog values
int V7_Val;
int Pwr1_Vin_Val;
int Pwr2_Vin_Val;
int Batt1_Vin_Val;
int Batt2_Vin_Val;
int Temp1_Val;
int Temp2_Val;

//Summation values
unsigned long V7_Sum=0;
unsigned long Pwr1_Vin_Sum=0;
unsigned long Pwr2_Vin_Sum=0;
unsigned long Batt1_Vin_Sum=0;
unsigned long Batt2_Vin_Sum=0;
unsigned long Temp1_Sum=0;
unsigned long Temp2_Sum=0;

int AverageCount = 0;
int AveragePort = 0;

// Conversion and calibration factors
const float V7_K = 2.32;
const float Pwr1_Vin_K = 2.52;
const float Pwr2_Vin_K = 2.51;
const float Batt1_Vin_K = 2.422;
const float Batt2_Vin_K = 2.422;
const float Temp1_K = 5.35;
const float Temp2_K = 5.35;

unsigned long Elapsed = 0;

#define VBATT_THRESHOLD  1350 // Battery alert in Volt * 100
#define VBATT_THRESHOLD1 1300 // Battery alert in Volt * 100
#define PWR_THRESHOLD_MIN 550 // Power Supply alert in Volt * 100
#define PWR_THRESHOLD_MAX 850 // Power Supply alert in Volt * 100
#define TEMP_THRESHOLD 450   // Temeperature limit in degrees * 10
#define UP 1                  // Arrow direction
#define DN 2                  // Arrow direction
#define AVERAGE_MAX 16        // Number of iterations before averaging analog values
#define AVERAGE_SHIFT 4       // Divide by 16 bit shifting 4 positions
#define AVERAGE_CYCLE 50      // Time between cycles 
#define AVERAGE_PORT_MAX 6    // Analog port number
/* Each analog measure is done every AVERAGE_CYCLE milliseconds. 
   To complete a full ports scan it must cycle AVERAGE_PORT_MAX + 1.
   A full set of measure is for AVERAGE_MAX iterations before averaging.
   So, there is a new full set of measures every:
     AVERAGE_CYCLE * (AVERAGE_PORT_MAX + 1) * AVERAGE_MAX 
   in this case 50 * (6+1) * 16 = 5.6 seconds
*/

int ErrCode=0;                // Error code

#define BLINK_ON 200          // HeartBeat Blink On time
#define BLINK_OFF 800         // HeartBeat Blink Off time
#define BLINK_ALRT 100        // HeartBeat Blink Alert time

int LedStat = LOW;

#define I2C_DISP 0x22         // Display I2C address
#define I2C_SONAR 0x24        // Sonar board I2C address

// SW Off Button must be kept pressed SW_OFF_CYCLE * SW_OFF_MAX ms
// i.e.: 250 * 12 = 3s
#define SW_OFF_CYCLE 250      // Sofware Off Button Control cycle
#define SW_OFF_MAX 12         // Iterations before accepting SW shutdown 
int SwOffFlag = 0;            // Is the gracefull shutdown procedure started?
int SwOffCount = 0;           // Cycle counter

Metro BlinkCycle = Metro(BLINK_OFF,1);       // LED blink cycle
Metro AnalogCycle = Metro(AVERAGE_CYCLE,1);  // Display write cycle
Metro SwOffCycle = Metro(SW_OFF_CYCLE,1);    // Sofware Off Button Control cycle

unsigned long TimeElapsed = millis();

long Bps = 115200;          // serial port speed
const int MAX_BUFF = 256;   // buffer size
int RxPtrIn = 0;            // circular queue pointer read
int RxPtrOut = 0;           // outgoing bytes read by Rx function
int RxStatus = 0;           // index for command decoding FSM status
char RxBuff[MAX_BUFF];      // circular buffer
char TxBuff[MAX_BUFF];

int RX_HEADER_LEN = 3;	    // command string header length (byte)

unsigned long StartTime;    // the moment the packet starts receiving
int RxPtrStart = 0;         // message packet starting pointer in queue
int RxPtrEnd = 0;           // message packet ending pointer in queue
int RxPtrData = 0;          // pointer to first data in queue
int RxCmdLen = 0;

int ShutdownFlag = 0;  // software shutdown started?
unsigned long RxTime = millis();
unsigned long I2cTime = millis();
unsigned long ShutdownHlsTime = millis();
unsigned char Digit_T; // tens on display
unsigned char Digit_U; // units on display
    
//-----------------------------------------------------------------------------      

void setup()
{
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(Sw_Off_Btn, INPUT);
  pinMode(Hls_Pwr_Off, INPUT);
    
  pinMode(Led, OUTPUT);      // sets the digital pin as output
  pinMode(Batt_1_En, OUTPUT);
  pinMode(Pwr_1_En, OUTPUT);
  pinMode(Batt_2_En, OUTPUT);
  pinMode(Pwr_2_En, OUTPUT);
  pinMode(Sw_Power_latch, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Light_L, OUTPUT);
  pinMode(Light_R, OUTPUT);

 // initialize serial:
  Serial1.begin(Bps);    // communication with HLS
  Serial.begin(Bps);     // console
  // timeout in ms as a function of comm speed and buffer size
  Timeout = (int)((MAX_BUFF * 10000.0) / Bps); 
  setTime(11,26,0,10,8,2013); // just for test debug
  
  #ifdef DEBUG_MODE
    Timeout=2000; // need to increase timeout because output requires more time
    RxTimeout = 0; // no timeout on RX
  #endif

  Wire.begin();                // join i2c bus (as master)
  
  LLSstartup (); 
}

//-----------------------------------------------------------------------------      

void loop()
{  
 
  if (BlinkCycle.check() == 1) {HeartBeat();}     // Led blink
  if (AnalogCycle.check() == 1) {AnalogRead();}   // Read all analog values
  if (SwOffCycle.check() == 1) {SwOff();}         // Control SW Off button
  
  if (RxPtrIn != RxPtrOut) RxData(); // at least one character in circular queue
 
  if (ShutdownFlag == 1)
  {// shutdown procedure already started, waiting for HLS confirm
    Shutdown (10);  
  }
  
  if ((RxTimeout != 0) && ((millis()-RxTime) > RxTimeout))
  {// no receiving from HLS. Timeout = 0 means no timeout
    ShutdownHlsTimeout = 5; // if no data from HLS the shutdown is immediate
    Shutdown (10);  
  }
  
  if (RxStatus == 99)                
  {// the message packet is complete and verified
    Parser();      // decode ready message packet 
  }
  else if (RxStatus > 0) 
  {// a new message packet is coming
    if((millis()-StartTime) > Timeout)
    {// if the packet is not complete within Timeout ms -> error
      RxError(1);
    }
  }
}


/*==============================================================================*/
void serialEvent1() 
{/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
  while (Serial1.available()) 
  {
    // get the new byte:
    char inChar = Serial1.read(); 
    RxBuff[RxPtrIn] = inChar;
    RxPtrIn ++;	                      // fill-up the circular queue
    if (RxPtrIn>=MAX_BUFF) RxPtrIn=0; // reset circolar queue
  }
}

