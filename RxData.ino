/*-------------------------------------------------------------------------------
Copyright 2013 Guido Ottaviani
guido@guiott.com
part of "ProtocolTester.ino"
acts as a tester exchanging data with the protocol:
http://www.guiott.com/Rino/CommandDescr/Protocol.htm
*/

boolean RX_ID_FLAG  = false;// command decode flag

char HEADER = '@';          // header define
int ID = 1;                 // this board identification
byte ChkSum = 0;

/*-----------------------------------------------------------------------------*/
void RxData(void)
{ 
#ifdef DEBUG_MODE
     Serial.print("in: ");
     Serial.print(RxPtrIn);
     Serial.print(" out: ");
     Serial.print(RxPtrOut); 
     Serial.print("  buff: ");
     Serial.print(RxBuff[RxPtrOut]);
     Serial.print("  stat: ");
     Serial.println(RxStatus);
#endif
     
  	switch (RxStatus)
	{// Finite State Machine driven by the received bytes  
          case 0:	// idle
 	    if (RxBuff[RxPtrOut] == HEADER)
 	    {
               StartTime=millis(); // start timeout counting
	       RxStatus = 1;	// header OK -> next status
	       RxPtrStart = RxPtrOut;  // queue index for this packet
	       ChkSum = ChkSum + RxBuff[RxPtrOut];
	       RxPtrOut ++;	// byte read -> point to next byte
             }
	     else  // no header. out of command sequence -> error
	     {
	        RxError(2);		   	
	     }
	     break;
		
	  case 1:	// header received correctly
 	    if(RxBuff[RxPtrOut]==ID||RxBuff[RxPtrOut]==0)
 	    {// command addressed to this board or broadcast
             // Id received and recognized
 	     	RX_ID_FLAG = 1;		// it has to decode the command
 	     	RxStatus = 2;		// next FSM status 
	     }
	     else if (RxBuff[RxPtrOut] == 'z') // special case for manual test of the communication
	     {
 	         RxStatus=99;	// enables command parser 
 	     	 RxCmd=RxBuff[RxPtrOut];
 	     }
	     else
	     {// Id received but NOT recognized 
 	         RX_ID_FLAG = 0;	// receives but does not decode the command
 	     	 RxStatus = 2;		// next FSM status 
	      }
 	      ChkSum = ChkSum + RxBuff[RxPtrOut];
	      RxPtrOut ++;		// next byte
	      break;
		
	  case 2:	// command received
 	      RxStatus = 3;		// next FSM status
 	      RxCmd=RxBuff[RxPtrOut];
 	      ChkSum = ChkSum + RxBuff[RxPtrOut];
 	      RxPtrOut ++;		// next byte
	      break;
		
	  case 3:	// command length received
			// calculating end pointer of command string
               RxCmdLen=RxBuff[RxPtrOut];
	       RxPtrEnd=RxCmdLen+RX_HEADER_LEN+RxPtrStart;
	       if (RxPtrEnd >= MAX_BUFF)RxPtrEnd -= MAX_BUFF;
	       if (RxPtrEnd >= MAX_BUFF)	// still bigger?
	       {
	          RxError(6);	// queue overflow	
	       }
	       else
	       {
 	          RxStatus = 4;	     // next FSM status
 	          ChkSum = ChkSum + RxBuff[RxPtrOut];
 	          RxPtrData=RxPtrOut;// pointer to last header byte, payload after this
 	          RxPtrOut ++;	     // next byte
 	       }
	       break;
			
	   case 4:	// waiting for command end
 	       if (RxPtrOut == RxPtrEnd)
 	       {	
	           if (ChkSum == (byte)(RxBuff[RxPtrOut]))		   	    
	   	   {// checksum OK?
	   	       if (RX_ID_FLAG)	
	   	       {// if right Id -> packet is addressed to me
		           RxStatus=99;// enables command parser 
			   RxPtrOut ++;	// next byte
		       }
		       else
		       {// flush 
		           RxStatus=0;	// end of command receive 
			   RxPtrOut ++;	// next byte
		       }
		    }
		    else
		    {
	                RxError(5); // checksum error	
	            }
	            ChkSum=0;
 	       }
 	       else
 	       {
 	           ChkSum = ChkSum + RxBuff[RxPtrOut];
		   RxPtrOut ++;	// next byte
	        }
		break;
		
	  default:// error: not a known status
		RxError(7);				   	
		break;
      } // switch end	
      if (RxPtrOut >= MAX_BUFF) RxPtrOut=0;//restart circular queue
}

