' The PIC running the clock is running the following PicBasic Pro code. The clunky assembly routines at the end gave me the timing needed to reproduce the pulsing of the clock's solenoid. The main routine keeps the clock going at a reasonable rate, and checks for incoming serial data. When it gets any data in, it speeds the clock up. The more data it gets, the faster the clock goes.	

' created 11/02 
' by Tom Igoe

INCLUDE "modedefs.bas"
trisb.1 = 0
trisB.0 = 0
OPTION_REG.7 = 0
interval con 24
second con 1000 
minTickPause con  10
tickRate var word
indata var byte(4)
storageArray var word (4)
 
pauseTime var word
currentByte var byte
i var byte
 
clear

pause 500
' tell Sp to enable UDP Receives
' (set address 0FF20h to 1):
serout2 PORTC.6, 84, [$90, $20, $FF, $01] 
 
 
main:
   ' if a second has passed with no tick, then tick:
   if pauseTime > second  then
        gosub tick
        pauseTime = 0
   endif        
   
  ' add new data to existing data:
  for i = 0 to 3
        storageArray[i] = storageArray[i] + indata[i]
        inData[i] = 0
   next
   
   ' set the rate based on the value of the storageArray:
   tickRate = minTickPause * 5 
   for i = 0 to 3
        if storageArray[i] > 0 then 
            tickRate = (tickRate - (minTickPause)) max minTickPause
        endif
   next
   
    ' if there's anything in the current byte, tick and decrement:   
 	if pauseTime > tickRate then
   		if storageArray[currentByte] > 0 then
        	gosub tick
        	storageArray[currentByte] = storageArray[currentByte] - 1
        	pauseTime = 0
   		else
   			' if there's nothing in the current byte, 
   			' then increment the current byte:
        	if currentByte < 4 then
            	currentByte = currentByte + 1  
        	else
            	currentByte = 0
        	endif
   		endif
   endif
   
   ' look for new data:
   serin2 portc.7, 84, 10, onward, [STR inData\4]
 goto main
 
onward:
       pauseTime = pauseTime + 10   
goto main


tick:
   asm
     	bsf PORTB, 1
     	bcf PORTB, 0
   endasm
   pause interval
   
   asm
   		bsf PORTB, 0
   		bcf PORTB, 1
   endasm 
   pause interval
   
   asm
    	bsf portb, 1
   endasm
   pause interval
   
   asm
   		bcf PORTB, 1
   		bcf PORTB, 0
   endasm
   pause interval 
   
   asm
   		bsf PORTB, 1
   		bsf PORTB, 0
   endasm
return