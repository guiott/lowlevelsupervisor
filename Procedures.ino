//-----------------------------------------------------------------------------      
void Shutdown (int ShTime)
{
/**
 *\brief Gracefull shutdown
  *
 */
  int i=0;
   int static ShtDwnTimeOld = ShutdownHlsTimeout/1000; 
   int ShtDwnTime = 0;
        
  if (ShutdownFlag == 0)  
  {// shutdown procedure just started
    ShutdownFlag = 1;
    hPwrOff = 1;
    lPwrOff = 1;
    
    delay(ShTime);
  
    // start switching down the system
    // Light dim to 0
    for(i=0; i<=255; i++)
    {
      analogWrite(Light_L,255-i);
      analogWrite(Light_R,255-i);
      delay(5);
    }
  
    #ifdef DEBUG_MODE
      DelayBar(2000);
      Serial.println("-------------Software shutdown started-------------");
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

    ShutdownHlsTime = millis();
  }
  else     
  {// check for a while if HLS is still alive before getting off power
    if ((digitalRead(Hls_Pwr_Off) == 0) || ((millis()-ShutdownHlsTime) > ShutdownHlsTimeout))
    {// HLS power really off or  HLS didn't switch off for too much time? Switch off everything
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
      #endif    
      Beep(1000);
      digitalWrite(Sw_Power_latch,LOW);
      while(1){}; //never return
    }
    else
    {
      ShtDwnTime = (ShutdownHlsTimeout - (millis()-ShutdownHlsTime))/1000;
      if (ShtDwnTime < ShtDwnTimeOld)
      {
        DispDigit(ShtDwnTime, 16);  // convert value in two digits base 16
        I2cDisplay ((BatteryLevel(Batt1_Vin_Val) | 0XF20), Digit_T, Digit_U, (BatteryLevel(Batt2_Vin_Val) | 0XF20), 0); //display timeout countdown
      
        #ifdef DEBUG_MODE
          Serial.print("      Waiting for HLS shutdown  ");
          Serial.println(ShtDwnTime);
        #endif  
        
        ShtDwnTimeOld = ShtDwnTime;
      }
    }
  }
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
 
        I2cDisplay (0x50, 0x20, 0x2F, 0x50, 0x33);
        
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
  
  if (ShutdownFlag == 1)
  {
    return;
  }
  
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
       Defcon3(51); // alert
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
       Defcon3(52); // alert
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
       Defcon2(5); // never returns because this procedure hangs the program
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
void Defcon1 (int Code)
{
/**
 *\brief do what is needed to manage a critical error:
 *Display error, wait, switch off EVERYTHING

   critical errors that requires complete switchoff:
          01 = LLS PWR supply not in range
          02 = Battery 1 voltage very low
 */
  ErrCode=Code;
  DisplayError();
  
  #ifdef DEBUG_MODE
    Serial.print("***** PROGRAM STOPPED ***** coming from Defcon 1 - Err #  ");
    Serial.println(ErrCode);
  #endif
  
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

  less critical errors that keep LLS on:
          03 = Battery 2 voltage very low
          04 = PWR1 supply not in range
          05 = PWR2 supply not in range
          06 = HLS board not responding
          07 = Temperature 1 too high
          08 = Temperature 2 too high
          09 = Sonar board not responding
 */
  ErrCode=Code;
  DisplayError();
  Beep(1000);
  
  digitalWrite(Batt_1_En,LOW);
  digitalWrite(Batt_2_En,LOW);
  digitalWrite(Pwr_1_En,LOW);
  digitalWrite(Pwr_2_En,LOW);

  #ifdef DEBUG_MODE
    Serial.print("***** PROGRAM STOPPED ***** coming from Defcon 2 - Err #  ");
    Serial.println(ErrCode);
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

 less critical errors that keep LLS on:
          51 = Battery 1 voltage  low
          52 = Battery 2 voltage  low
 */
  ErrCode=Code;
  
  #ifdef DEBUG_MODE
    Serial.print("***** PROGRAM STOPPED ***** coming from Defcon 3 - Err #  ");
    Serial.println(ErrCode);
  #endif
  
  DisplayError();
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

