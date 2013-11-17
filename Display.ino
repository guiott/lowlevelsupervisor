//-----------------------------------------------------------------------------      
void Display (void)
{
/**
 *\brief write numbers on display via I2C bus
  *
 */
    unsigned char Arrow;
    int Temp_Val;
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
    
    DispDigit(Temp_Val/10, 10);  // convert value in two digits base 10

    if(!ErrCode)
    { 
      I2cDisplay (BatteryLevel(Batt1_Vin_Val), Digit_T, Digit_U, BatteryLevel(Batt2_Vin_Val), Arrow);
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
   I2cDisplay (BatteryLevel(Batt1_Vin_Val), (ErrCode/10 | 0XF30), (ErrCode | 0XF30), BatteryLevel(Batt2_Vin_Val), 0);
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
void DispDigit (unsigned int Digit, int Base)
{
/**
 *\brief split the digit to show on display in tens and units using the desired base
 */
  Digit_U=(Digit % Base); 
  Digit_T=(Digit / Base) ;
}
