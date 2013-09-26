  /*  ???????????  To Be Done  ????????????????????
  Send a message to High Level Supervisor to order a shutdown procedure (the last byte of data packet)
        Wait for ACK
        Proceed with Shutdown() function
  
  I2C procedure with QuadSonar to read obstacles distances 
        #define I2C_SONAR 0x24  // QuadSonar I2C address
        I2cRegTx[0] // LL
        I2cRegTx[1] // LC
        I2cRegTx[2] // CL
        I2cRegTx[3] // CC
        I2cRegTx[4] // CR
        I2cRegTx[5] // RC
        I2cRegTx[6] // RR
        
  Comm procedure to send to HLS obstacles (1 byte, 255cm MAX), Vbat (0-5 * 20), Temp
  Comm procedure to receive and decode commands (request for parameters or initiate shutdown procedure)
  On the same packet receive shutdown order
  */
//-----------------------------------------------------------------------------      
void Shutdown (int ShTime)
{
/**
 *\brief Gracefull shutdown
  *
 */
  int i=0;

    delay(ShTime);
    
    // Light dim to 0
    for(i=0; i<=255; i++)
    {
      analogWrite(Light_L,255-i);
      analogWrite(Light_R,255-i);
      delay(5);
    }

    #ifdef DEBUG_MODE
      DelayBar(2000);
      DelayBar(2000);
      Serial.println("Switching off Battery 1");
      DelayBar(2000);
    #else 
      delay(500);
    #endif
    digitalWrite(Batt_1_En,LOW);
    
    #ifdef DEBUG_MODE
      Serial.println("Switching off Battery 2");
      DelayBar(2000);
    #else 
      delay(500);
    #endif
    digitalWrite(Batt_2_En,LOW);
    
    #ifdef DEBUG_MODE
      Serial.println("Switching off Power Supply 1");
      DelayBar(2000);
    #else 
      delay(500);
    #endif
    digitalWrite(Pwr_1_En,LOW);

    #ifdef DEBUG_MODE
      Serial.println("Switching off Power Supply 2");
      DelayBar(2000);
    #else 
      delay(500);
    #endif
    digitalWrite(Pwr_2_En,LOW);
    
    #ifdef DEBUG_MODE
      Serial.println("SWITCHING OFF MYSELF");
      DelayBar(4000);
      Serial.println("BYE");
    #else 
      delay(500);
    #endif    Beep(1000);
    digitalWrite(Sw_Power_latch,LOW);
    while(1){}; //never return
}

//-----------------------------------------------------------------------------      
void SwOff (void)
{
/**
 *\brief Control the Software Off Button to proceed with gracefull shutdown
  *
 */
  if (!SwOffFlag)
  {
    if (!digitalRead(Sw_Off_Btn))  // Control SW Off button
    {
      SwOffCount ++;  // Measure how long the button is pressed
      if (SwOffCount >= SW_OFF_MAX)
      {
        SwOffCount = 0;
        SwOffFlag = 1;
        
        Wire.beginTransmission(I2C_DISP);
          Wire.write(0x00);    //write on first register 
          Wire.write(0x50);    // LEDs bar left CCW animation
          Wire.write(0x20);    // first cipher 
          Wire.write(0x2F);    // second cipher
          Wire.write(0x50);    // LEDs bar right CCW animation
          Wire.write(0x33);    // Arrows 
        Wire.endTransmission();
        
        delay(2000);
      }
    }
    else
    {
      SwOffCount = 0;
    }
  }
  else
  {
     SwOffCount ++;  // Wait some time if button pressed
     if (SwOffCount >= SW_OFF_MAX)
     {// if the button is not pressed again abort shutdown
       SwOffCount = 0;
       SwOffFlag = 0;
     }
     else
     {
       if (!digitalRead(Sw_Off_Btn))  // Control SW Off button
        {  
          Shutdown(100);
        }   
     } 
  }
}

//-----------------------------------------------------------------------------      
void AnalogRead (void)
{
/**
 *\brief read and cumulate the analog port to compute average value
  *
 */
  
  /* Switching in advance to the next port to measure, to allow the voltage
      to stabilize on the ADC Sample/Hold
  */
  int Dummy; 
  
 // Summation of N values
  switch (AveragePort)
  { 
    case 0:
      Batt1_Vin_Sum += analogRead(Batt1_Vin);
      #ifdef DEBUG_MODE
        Dummy = analogRead(Batt1_Vin); 
        Serial.print("***Batt1_Vin = ");
        Serial.print(Dummy);
      #endif      
      Dummy = analogRead(Batt2_Vin); 
      break;
      
    case 1:
      Batt2_Vin_Sum += analogRead(Batt2_Vin);
      #ifdef DEBUG_MODE
        Dummy = analogRead(Batt2_Vin); 
        Serial.print("--Batt2_Vin = ");
        Serial.print(Dummy);
      #endif  
      Dummy = analogRead(Pwr1_Vin); 
      break;
      
    case 2:
      Pwr1_Vin_Sum += analogRead(Pwr1_Vin); 
      #ifdef DEBUG_MODE
        Dummy = analogRead(Pwr1_Vin); 
        Serial.print("--Pwr1_Vin = ");
        Serial.print(Dummy);
      #endif      
      Dummy = analogRead(Pwr2_Vin); 
      break;   
      
    case 3:
      Pwr2_Vin_Sum += analogRead(Pwr2_Vin); 
      #ifdef DEBUG_MODE
        Dummy = analogRead(Pwr2_Vin); 
        Serial.print(" Pwr2_Vin = ");
        Serial.print(Dummy);
      #endif
      Dummy = analogRead(V7); 
      break;
      
    case 4:
      V7_Sum += analogRead(V7); 
      #ifdef DEBUG_MODE
        Dummy = analogRead(V7); 
        Serial.print("--V7 = ");
        Serial.print(Dummy);
      #endif
      Dummy = analogRead(Temp1); 
      break;
   
    case 5:
      Temp1_Sum += analogRead(Temp1); 
      #ifdef DEBUG_MODE
        Dummy = analogRead(Temp1); 
        Serial.print("--Temp1 = ");
        Serial.print(Dummy);
      #endif
      Dummy = analogRead(Temp2); 
      break;  

    case 6:
      Temp2_Sum += analogRead(Temp2);
      #ifdef DEBUG_MODE
        Dummy = analogRead(Temp2); 
        Serial.print("--Temp2 = ");
        Serial.print(Dummy);
        Serial.print("--Time = ");
        Serial.println(millis()-Elapsed);
        Elapsed=millis();
      #endif
      Dummy = analogRead(Batt1_Vin); 
      break;
      
    default:
      break;
  }
  
  AveragePort++;
  if(AveragePort > AVERAGE_PORT_MAX)
  {// when all analog ports are scanned, returns to the first one
      AveragePort = 0;
      AverageCount ++;
  }
         
  if(AverageCount >= AVERAGE_MAX)
  {// after N cycles compute the mean values
      AverageCount = 0;
      
      // ---------------------------------------LLS PWR supply
      V7_Val = V7_Sum >> AVERAGE_SHIFT;           // average dividing by 64,
      V7_Sum = 0;                                 // restart for another cycle
      V7_Val = int((float(V7_Val) * V7_K)+0.5);   // multiplying by K to obtain the final value int * 100
      
      if((V7_Val < PWR_THRESHOLD_MIN) || (V7_Val > PWR_THRESHOLD_MAX))
      {// range test
        Defcon1(1); // never returns because this procedure switches off LLS
      }
      
  
      // --------------------------------------- Batt1
      Batt1_Vin_Val = Batt1_Vin_Sum >> AVERAGE_SHIFT;    // average dividing by 64,
      Batt1_Vin_Sum = 0;                                 // restart for another cycle
      Batt1_Vin_Val = int((float(Batt1_Vin_Val) * Batt1_Vin_K)+0.5);// multiplying by K to obtain the final value int * 100
                   
      if(Batt1_Vin_Val < VBATT_THRESHOLD1) 
      {
       Defcon1(2); // never returns because this procedure switches off LLS
      }
      else if(Batt1_Vin_Val < VBATT_THRESHOLD) 
      {
       Defcon2(51); // never returns because this procedure hangs the program
      } 
    
    
      // --------------------------------------- Batt2
      Batt2_Vin_Val = Batt2_Vin_Sum >> AVERAGE_SHIFT;    // average dividing by 64,
      Batt2_Vin_Sum = 0;                                 // restart for another cycle
      Batt2_Vin_Val = int((float(Batt2_Vin_Val) * Batt2_Vin_K)+0.5);// multiplying by K to obtain the final value int * 100
      if(Batt2_Vin_Val < VBATT_THRESHOLD1) 
      {
       Defcon1(3); // never returns because this procedure switches off LLS
      }
      else if(Batt2_Vin_Val < VBATT_THRESHOLD) 
      {
       Defcon2(52); // never returns because this procedure hangs the program
      }   
  
      // --------------------------------------- Power Supply 1
      Pwr1_Vin_Val = Pwr1_Vin_Sum >> AVERAGE_SHIFT;   // average dividing by 64,
      Pwr1_Vin_Sum = 0;                               // restart for another cycle
      Pwr1_Vin_Val = int((float(Pwr1_Vin_Val) * Pwr1_Vin_K)+0.5);// multiplying by K to obtain the final value int * 100
          
      if((Pwr1_Vin_Val < PWR_THRESHOLD_MIN) || (Pwr1_Vin_Val > PWR_THRESHOLD_MAX))
      {
       Defcon2(4); // never returns because this procedure switches off LLS
      }
      
      // --------------------------------------- Power Supply 2
      Pwr2_Vin_Val = Pwr2_Vin_Sum >> AVERAGE_SHIFT;   // average dividing by 64,
      Pwr2_Vin_Sum = 0;                               // restart for another cycle
      Pwr2_Vin_Val = int((float(Pwr2_Vin_Val) * Pwr2_Vin_K)+0.5);// multiplying by K to obtain the final value int * 100
          
      if((Pwr2_Vin_Val < PWR_THRESHOLD_MIN) || (Pwr2_Vin_Val > PWR_THRESHOLD_MAX))
      {
       Defcon2(5); // never returns because this procedure switches off LLS
      }  
    
    
      // --------------------------------------- Temperature 1
      Temp1_Val = Temp1_Sum >> AVERAGE_SHIFT;      // average dividing by 64,
      Temp1_Sum = 0;                               // restart for another cycle
      Temp1_Val = int((float(Temp1_Val) * Temp1_K)+0.5);// multiplying by K to obtain the final value int * 100
          
      if((Temp1_Val > TEMP_THRESHOLD))
      {
        Defcon2(7); // never returns because this procedure hangs the program
      }    
    
       // --------------------------------------- Temperature 2
      Temp2_Val = Temp2_Sum >> AVERAGE_SHIFT;      // average dividing by 64,
      Temp2_Sum = 0;                               // restart for another cycle
      Temp2_Val = int((float(Temp2_Val) * Temp2_K)+0.5);// multiplying by K to obtain the final value int * 100
          
      if((Temp2_Val > TEMP_THRESHOLD))
      {
        Defcon2(8); // never returns because this procedure hangs the program
      } 
   
     if (!SwOffFlag)
     {// update the display with the new values only if shutdown procedure is not started 
       Display();    
     }
  }  
}

//-----------------------------------------------------------------------------      
void Display (void)
{
/**
 *\brief write numbers on display via I2C bus
  *
 */
    unsigned char Arrow;
    int Temp_Val;
    int Temp_Val_T;
    int Temp_Val_U;
    static int TempFlag = 0;
    
    
    //Temperatures
    if(TempFlag == 0)
    {// alternating Temp 1 and Temp 2
      Temp_Val = Temp1_Val;
      TempFlag = 1;
      Arrow = DN; //DOWN arrow
    }
    else    
    {
      Temp_Val = Temp2_Val;
      TempFlag = 0;
      Arrow = UP; //UP arrow
    }
    Temp_Val_T=Temp_Val/100;
    Temp_Val_U=(Temp_Val-(Temp_Val_T*100))/10;

    if(!ErrCode)
    { 
      Wire.beginTransmission(I2C_DISP);
        Wire.write(0x00); //write on first register 
        Wire.write(BatteryLevel(Batt1_Vin_Val));    // LEDs bar left 
        Wire.write(Temp_Val_T);                     // Degrees units 
        Wire.write(Temp_Val_U);                     // Degrees tens
        Wire.write(BatteryLevel(Batt2_Vin_Val));    // LEDs bar right 
        Wire.write(Arrow);                          // Arrows, DN means Temp 1 
      Wire.endTransmission();
    }
    else
    {
      DisplayError();
    } 
}

//-----------------------------------------------------------------------------       
void DisplayError(void)
{
  /**
 *\brief Display the error code
  *
 */
 
   Wire.beginTransmission(I2C_DISP);
      Wire.write(0x00); //write on first register 
      Wire.write(BatteryLevel(Batt1_Vin_Val));  // LEDs bar left 
      Wire.write(ErrCode/10 | 0XF30);           // Error Code tens, fast blinking
      Wire.write(ErrCode    | 0XF30);           // Error Code units,fast blinking 
      Wire.write(BatteryLevel(Batt2_Vin_Val));  // LEDs bar right 
      Wire.write(0);                            // Switch off Arrows 
   Wire.endTransmission();
}    
    
//-----------------------------------------------------------------------------       
unsigned char BatteryLevel (int Value)
{
  /**
 *\brief transform the Battery level value to LEDs bar segments
  *
 */
  if (Value >= 1600)
  {
    return(5);    // full charge
  }
  else if (Value < 1600 && Value >= 1500)
  {
    return(4);    // 75% charge
  }
  else if (Value < 1500 && Value >= 1450)
  {
    return(3);    // 50% charge
  }
  else if (Value < 1450 && Value >= 1350)
  {
    return(2);    // 25% charge
  }
  else if (Value < 1350 && Value >= 1300)
  {
    return(1); // alarm, blinking 
  }
  else if (Value < 1300)
  {
    return(0X31);    // cutoff
  }
}


//-----------------------------------------------------------------------------      
void Beep (int Duration)
{
/**
 *\brief perform a beep on the LLS board of the given duration in milliseconds
  *
 */
  digitalWrite(Buzzer,HIGH);
  delay(Duration);
  digitalWrite(Buzzer,LOW); 
}

//-----------------------------------------------------------------------------      
void I2cBeep (int Duration)
{
/**
 *\brief perform a beep on display of the given duration in milliseconds
  *
 */
    Wire.beginTransmission(I2C_DISP);
    Wire.write(0x05); //write on sixth register: buzzer
    Wire.write(0X00); //beep ON 
    Wire.endTransmission();
    delay(Duration);
    Wire.beginTransmission(I2C_DISP);
    Wire.write(0x05); //write on sixth register: buzzer
    Wire.write(0X10); //beep OFF
    Wire.endTransmission();   
}

//-----------------------------------------------------------------------------      
void Defcon1 (int Code)
{
/**
 *\brief do what is needed to manage a critical error:
 *Display error, wait, switch off
  *
 */
  ErrCode=Code;
  DisplayError();
  Beep(1000);
  digitalWrite(Batt_1_En,LOW);
  digitalWrite(Batt_2_En,LOW);
  digitalWrite(Pwr_1_En,LOW);
  digitalWrite(Pwr_2_En,LOW);
  I2cBeep(1000);
  delay(3000);
  digitalWrite(Sw_Power_latch,LOW);
  while(1){}; //never return
}

//-----------------------------------------------------------------------------      
void Defcon2 (int Code)
{
/**
 *\brief do what is needed to manage a serious error:
 *Display error, wait for the operator
  *
 */
  ErrCode=Code;
  DisplayError();
  Beep(1000);
  
  digitalWrite(Batt_1_En,LOW);
  digitalWrite(Batt_2_En,LOW);
  digitalWrite(Pwr_1_En,LOW);
  digitalWrite(Pwr_2_En,LOW);

  #ifdef DEBUG_MODE
    Serial.println("***** PROGRAM STOPPED *****");
  #endif
  
  I2cBeep(1000);

  while(1); // halt
}

//-----------------------------------------------------------------------------      
void Defcon3 (int Code)
{
/**
 *\brief do what is needed to manage a warning:
 *change display from normal to allarm (blinking)
  *
 */
  ErrCode=Code;
  DisplayError();
}

//-----------------------------------------------------------------------------      
void DelayBar (unsigned int Duration)
{
/**
 *\brief perform a delay of given duration in milliseconds, sending a
  *dotted progression bar on Serial port
  *just for debug
 */
  unsigned int MsStep;
  int J;
  
  MsStep = Duration / 50;
  for(J=0; J<=50; J++)
  {
    delay(MsStep);
    Serial.print('.');
  }
   Serial.println(' ');
}

//-----------------------------------------------------------------------------      
void HeartBeat (void)
{ // regular blink as heart beat signal
  if (LedStat == LOW)
  {
    LedStat = HIGH;
    BlinkCycle.interval(BLINK_ON);  // change to ON period
  }
  else
  {
    LedStat = LOW;
    if (ErrCode != 0) 
    {// if battery low blink faster [5]
      BlinkCycle.interval(BLINK_ALRT);
    }
    else
    {
      BlinkCycle.interval(BLINK_OFF);  // change to OFF period
    }
  }
  
  digitalWrite(Led,LedStat);
}

