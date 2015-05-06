//-----------------------------------------------------------------------------                                                      
void I2cSonar (void)
{ // read a word (2 bytes) from compass sensor
  int Val = 0;
  int I2cNum = 11; // number of bytes to read
  
  Wire.beginTransmission(I2C_SONAR);
  Wire.write(0);     // sets register pointer to 0
  Wire.endTransmission(); // stop transmitting
  Wire.requestFrom(I2C_SONAR, I2cNum);
  if(I2cNum <= Wire.available())   // if all the bytes were received
  {
    I2cTime = millis();  // packet correctly received, reset I2C watchdog
    Obst[0] = Wire.read(); // Left Left
    Obst[1] = Wire.read(); // Left Center
    Obst[2] = Wire.read(); // Center Left
    Obst[3] = Wire.read(); // Center Center
    Obst[4] = Wire.read(); // Center Right
    Obst[5] = Wire.read(); // Right Center
    Obst[6] = Wire.read(); // Right Right
    
    Obst[7] = Wire.read(); // Left Left RAW
    Obst[8] = Wire.read(); // Center Left RAW
    Obst[9] = Wire.read(); // Center Right RAW
    Obst[10]= Wire.read(); // Right Right RAW
  }
  else
  {
    unsigned long I2cElapsed = millis()-I2cTime;
    if ((I2cTimeout != 0) && (I2cElapsed > I2cTimeout))
    {// no receiving from HLS. Timeout = 0 means never timeout
       Defcon2(9); // never returns because this procedure hangs the program
    }
    #ifdef DEBUG_MODE
      Serial.print("***** No I2C replay from sonar since: ");
      Serial.print(I2cElapsed);
      Serial.println(" ms");
    #endif
  }
}

//-----------------------------------------------------------------------------      
void I2cDisplay (int Level1, int Tens, int Units, int Level2, int Arrow)
{
/**
 *\brief display values
  *
 */
    Wire.beginTransmission(I2C_DISP);
      Wire.write(0x00);                //write on first register 
      Wire.write(Level1);              // LEDs bar left 
      Wire.write(Tens);                // Degrees units 
      Wire.write(Units);               // Degrees tens
      Wire.write(Level2);              // LEDs bar right 
    Wire.write(Arrow);                 // Arrows, DN means Temp 1 
    Wire.endTransmission(); 
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

