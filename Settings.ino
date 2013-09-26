//-----------------------------------------------------------------------------      

void LLSstartup ()
{ 
  int i=0;
  unsigned long Sum=0;
  
  digitalWrite(Sw_Power_latch,HIGH);
  Beep(50);
  digitalWrite(Led,HIGH);
  //I2cBeep(100);
  
  delay(2000);    // wait for stabilization
  
  // ---------------------------------------LLS PWR supply
  for(i=0; i<128; i++)  
  {
    Sum += analogRead(V7);
  }
  Sum = Sum >> 7; // average dividing by 128,
  V7_Val = int((float(Sum) * V7_K)+0.5);  // multiplying by K to obtain the final value int * 100
  Sum = 0;
  #ifdef DEBUG_MODE
    DelayBar(2000);
    Serial.print("Low Level Supervisor Power Supply = ");
    Serial.print(float(V7_Val)/100.0);
    Serial.println("V");
  #endif
  if((V7_Val < PWR_THRESHOLD_MIN) || (V7_Val > PWR_THRESHOLD_MAX))
  {
    #ifdef DEBUG_MODE
      Serial.println("***** ERROR: LLS Power Supply not in range *****");
    #endif
    Defcon1(1); // never returns because this procedure switches off LLS
  }
  else
  {
    #ifdef DEBUG_MODE
      Serial.println("      OK: LLS Power Supply is within the range");
    #endif    
  }
    
  // --------------------------------------- Batt1
  for(i=0; i<128; i++)  
  {
    Sum += analogRead(Batt1_Vin);
  }
  Sum = Sum >> 7; // average dividing by 128,
  Batt1_Vin_Val = int((float(Sum) * Batt1_Vin_K)+0.5);  // multiplying by K to obtain the final value int * 100
  Sum = 0;
  #ifdef DEBUG_MODE
    DelayBar(2000);  
    Serial.print("Battery 1 Level = ");
    Serial.print(float(Batt1_Vin_Val)/100.0); 
    Serial.println("V");
  #endif
  if(Batt1_Vin_Val < VBATT_THRESHOLD1) 
  {
    #ifdef DEBUG_MODE
      Serial.println("***** ERROR: Battery 1 level very low *****");
    #endif
    Defcon1(2); // never returns because this procedure switches off LLS
  }
  else if(Batt1_Vin_Val < VBATT_THRESHOLD) 
  {
    #ifdef DEBUG_MODE
      Serial.println("***** WARNING: Battery 1 level low *****");
    #endif
    Defcon2(51); // never returns because this procedure hangs the program
  }
  else
  {
    #ifdef DEBUG_MODE
      Serial.println("      OK: Battery 1 is within the range");
    #endif    
    ErrCode=0;
  }
       
  // --------------------------------------- Batt2
  for(i=0; i<128; i++)  
  {
    Sum += analogRead(Batt2_Vin);
  }
  Sum = Sum >> 7; // average dividing by 128,
  Batt2_Vin_Val = int((float(Sum) * Batt2_Vin_K)+0.5);  // multiplying by K to obtain the final value int * 100
  Sum = 0;
  #ifdef DEBUG_MODE
    DelayBar(2000);
    Serial.print("Battery 2 Level = ");
    Serial.print(float(Batt2_Vin_Val)/100.0); 
    Serial.println("V");
  #endif  
  if(Batt2_Vin_Val < VBATT_THRESHOLD1) 
  {
    #ifdef DEBUG_MODE
      Serial.println("***** ERROR: Battery 2 level very low *****");
    #endif
    Defcon2(3); // never returns because this procedure hangs the program
  }
  else if(Batt2_Vin_Val < VBATT_THRESHOLD) 
  {
    #ifdef DEBUG_MODE
      Serial.println("***** WARNING: Battery 2 level low *****");
    #endif
    Defcon2(52); // never returns because this procedure hangs the program
  }
  else
  {
    #ifdef DEBUG_MODE
      Serial.println("      OK: Battery 2 is within the range");
    #endif    
    ErrCode=0;
  }

  #ifdef DEBUG_MODE
      Serial.println("      Switching on Power Supply 1");
  #endif    
  digitalWrite(Pwr_1_En,HIGH);
   delay(500);


  // --------------------------------------- Power Supply 1
  for(i=0; i<128; i++)  
  {
    Sum += analogRead(Pwr1_Vin);
  }
  Sum = Sum >> 7; // average dividing by 128,
  Pwr1_Vin_Val = int((float(Sum) * Pwr1_Vin_K)+0.5);  // multiplying by K to obtain the final value int * 100
  Sum = 0;
  #ifdef DEBUG_MODE
    DelayBar(2000);
    Serial.print("Power Supply 1  = ");
    Serial.print(float(Pwr1_Vin_Val)/100.0);
    Serial.println("V");
  #endif      
  if((Pwr1_Vin_Val < PWR_THRESHOLD_MIN) || (Pwr1_Vin_Val > PWR_THRESHOLD_MAX))
  {
    #ifdef DEBUG_MODE
      Serial.println("***** ERROR: Power Supply 1 not in range *****");
    #endif
    Defcon2(4); // never returns because this procedure hangs the program
  }
  else
  {
    #ifdef DEBUG_MODE
      Serial.println("      OK: Power Supply 1 is within the range");
      Serial.println("      Switching on Power Supply 2");
    #endif  
    ErrCode=0;
  }
  
  digitalWrite(Pwr_2_En,HIGH);
  delay(500);
  
  // --------------------------------------- Power Supply 2
  for(i=0; i<128; i++)  
  {
    Sum += analogRead(Pwr2_Vin);
  }
  Sum = Sum >> 7; // average dividing by 128,
  Pwr2_Vin_Val = int((float(Sum) * Pwr2_Vin_K)+0.5);  // multiplying by K to obtain the final value int * 100
  Sum = 0;
  #ifdef DEBUG_MODE
    DelayBar(2000);
    Serial.print("Power Supply 2  = ");
    Serial.print(float(Pwr2_Vin_Val)/100.0);
    Serial.println("V");
  #endif     
  if((Pwr2_Vin_Val < PWR_THRESHOLD_MIN) || (Pwr2_Vin_Val > PWR_THRESHOLD_MAX))
  {
    #ifdef DEBUG_MODE
      Serial.println("***** ERROR: Power Supply 2 not in range *****");
    #endif
    Defcon2(5); // never returns because this procedure hangs the program
  }
  else
  {
    #ifdef DEBUG_MODE
      Serial.println("      OK: Power Supply 2 is within the range");
      Serial.println("      Testing temperature 1");
    #endif  
    ErrCode=0;
  }
  
  // --------------------------------------- Temperature 1
  for(i=0; i<128; i++)  
  {
    Sum += analogRead(Temp1);
  }
  Sum = Sum >> 7; // average dividing by 128,
  Temp1_Val = int((float(Sum) * Temp1_K)+0.5);  // multiplying by K to obtain the final value int * 100
  Sum = 0;
  #ifdef DEBUG_MODE
    DelayBar(2000);
    Serial.print("Temperature 1  = ");
    Serial.print(float(Temp1_Val)/10.0,1);
    Serial.println(" C");
  #endif      
  if((Temp1_Val > TEMP_THRESHOLD))
  {
    #ifdef DEBUG_MODE
      Serial.println("***** ERROR: Temperature 1 too high *****");
    #endif
    Defcon2(7); // never returns because this procedure hangs the program
  }
  else
  {
    #ifdef DEBUG_MODE
      Serial.println("      OK: Temperature 1 is within the range");
      Serial.println("      Testing temperature 2");
    #endif  
    ErrCode=0;
  }
  
  int Temp_Val_T=Temp1_Val/100;
  int Temp_Val_U=(Temp1_Val+5-(Temp_Val_T*100))/10;
     
 // --------------------------------------- Temperature 2
  for(i=0; i<128; i++)  
  {
    Sum += analogRead(Temp2);
  }
  Sum = Sum >> 7; // average dividing by 128,
  Temp2_Val = int((float(Sum) * Temp2_K)+0.5);  // multiplying by K to obtain the final value int * 100
  Sum = 0;
  #ifdef DEBUG_MODE
    DelayBar(2000);
    Serial.print("Temperature 2  = ");
    Serial.print(float(Temp2_Val)/10.0,1);
    Serial.println(" C");
  #endif      
    if((Temp2_Val > TEMP_THRESHOLD))
  {
    #ifdef DEBUG_MODE
      Serial.println("***** ERROR: Temperature 2 too high *****");
    #endif
    Defcon2(8); // never returns because this procedure hangs the program
  }
  else
  {
    #ifdef DEBUG_MODE
      Serial.println("      OK: Temperature 2 is within the range");
      Serial.println("      Switching on batteries");
    #endif  
    ErrCode=0;
  }
 
  Temp_Val_T=Temp2_Val/100;
  Temp_Val_U=(Temp2_Val+5-(Temp_Val_T*100))/10;
    
 // TO Be Done; Check boards healt
 
   digitalWrite(Batt_1_En,HIGH);
  #ifdef DEBUG_MODE
    DelayBar(2000);
  #endif     
  digitalWrite(Batt_2_En,HIGH);
  
  #ifdef DEBUG_MODE
    Serial.println("      Testing headlights");
  #endif  
    
  for(i=0; i<=255; i++)
  {
    analogWrite(Light_L,i);
    analogWrite(Light_R,i);
    delay(5);
  }
  
  for(i=0; i<=255; i++)
  {
    analogWrite(Light_L,i);
    analogWrite(Light_R,255-i);
    delay(5);
  }
   for(i=0; i<=255; i++)
  {
    analogWrite(Light_L,255-i);
    analogWrite(Light_R,i);
    delay(5);
  }
  
  for(i=0; i<=255; i++)
  {
    analogWrite(Light_L,i);
    analogWrite(Light_R,i);
    delay(5);
  }
  
  for(i=0; i<=255; i++)
  {
    analogWrite(Light_L,255-i);
    analogWrite(Light_R,255-i);
    delay(5);
  }

    #ifdef DEBUG_MODE
      Serial.println("      OK: everything works fine");
      Serial.println("      ROBOT OPERATIVE");
      Serial.println("  ");
      Serial.println("  ");
    #endif  
    
    #ifdef DEMO_MODE //switch off demonstration
      Shutdown(0);
    #endif
    
    delay(2000);    // wait for stabilization
    
    Wire.beginTransmission(I2C_DISP);
    Wire.write(0x00); //write on first register 
    Wire.write(BatteryLevel(Batt1_Vin_Val));    // LEDs bar left 
    Wire.write(0);                 
    Wire.write(0);                
    Wire.endTransmission();
    
    delay(500);
  
    Wire.beginTransmission(I2C_DISP);
    Wire.write(0x00); //write on first register 
    Wire.write(BatteryLevel(Batt1_Vin_Val));    // LEDs bar left 
    Wire.write(0);                  
    Wire.write(0);               
    Wire.write(BatteryLevel(Batt2_Vin_Val));    // LEDs bar right 
    Wire.endTransmission();

    delay(500);

    Wire.beginTransmission(I2C_DISP);
    Wire.write(0x00);                           //write on first register 
    Wire.write(BatteryLevel(Batt1_Vin_Val));    // LEDs bar left 
    Wire.write(Temp_Val_T);                     // Degrees units 
    Wire.write(Temp_Val_U);                     // Degrees tens
    Wire.write(BatteryLevel(Batt2_Vin_Val));    // LEDs bar right 
    Wire.write(DN);                             // Arrows, DN means Temp 1 
    Wire.endTransmission();
}


