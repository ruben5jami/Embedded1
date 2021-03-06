/********************************************************************
 FileName:     main.c
 Dependencies: See INCLUDES section
 Processor:   PIC18 or PIC24 USB Microcontrollers
 Hardware:    The code is natively intended to be used on the following
        hardware platforms: PICDEM� FS USB Demo Board, 
        PIC18F87J50 FS USB Plug-In Module, or
        Explorer 16 + PIC24 USB PIM.  The firmware may be
        modified for use on other USB platforms by editing the
        HardwareProfile.h file.
 Complier:    Microchip C18 (for PIC18) or C30 (for PIC24)
 Company:   Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the �Company�) for its PIC� Microcontroller is intended and
 supplied to you, the Company�s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

********************************************************************
 File Description:

 Change History:
  Rev   Date         Description
  1.0   11/19/2004   Initial release
  2.1   02/26/2007   Updated for simplicity and to use common
                     coding style
********************************************************************/


//	========================	INCLUDES	========================
#ifdef _VISUAL
#include "VisualSpecials.h"
#endif // VISUAL

#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"

#include "mtouch.h"

#include "BMA150.h"

#include "my_oled.h"

#include "soft_start.h"


//	========================	CONFIGURATION	========================

#if defined(PIC18F46J50_PIM)
   //Watchdog Timer Enable bit:
     #pragma config WDTEN = OFF          //WDT disabled (control is placed on SWDTEN bit)
   //PLL Prescaler Selection bits:
     #pragma config PLLDIV = 3           //Divide by 3 (12 MHz oscillator input)
   //Stack Overflow/Underflow Reset Enable bit:
     #pragma config STVREN = ON            //Reset on stack overflow/underflow enabled
   //Extended Instruction Set Enable bit:
     #pragma config XINST = OFF          //Instruction set extension and Indexed Addressing mode disabled (Legacy mode)
   //CPU System Clock Postscaler:
     #pragma config CPUDIV = OSC1        //No CPU system clock divide
   //Code Protection bit:
     #pragma config CP0 = OFF            //Program memory is not code-protected
   //Oscillator Selection bits:
     #pragma config OSC = ECPLL          //HS oscillator, PLL enabled, HSPLL used by USB
   //Secondary Clock Source T1OSCEN Enforcement:
     #pragma config T1DIG = ON           //Secondary Oscillator clock source may be selected
   //Low-Power Timer1 Oscillator Enable bit:
     #pragma config LPT1OSC = OFF        //Timer1 oscillator configured for higher power operation
   //Fail-Safe Clock Monitor Enable bit:
     #pragma config FCMEN = OFF           //Fail-Safe Clock Monitor disabled
   //Two-Speed Start-up (Internal/External Oscillator Switchover) Control bit:
     #pragma config IESO = OFF           //Two-Speed Start-up disabled
   //Watchdog Timer Postscaler Select bits:
     #pragma config WDTPS = 32768        //1:32768
   //DSWDT Reference Clock Select bit:
     #pragma config DSWDTOSC = INTOSCREF //DSWDT uses INTOSC/INTRC as reference clock
   //RTCC Reference Clock Select bit:
     #pragma config RTCOSC = T1OSCREF    //RTCC uses T1OSC/T1CKI as reference clock
   //Deep Sleep BOR Enable bit:
     #pragma config DSBOREN = OFF        //Zero-Power BOR disabled in Deep Sleep (does not affect operation in non-Deep Sleep modes)
   //Deep Sleep Watchdog Timer Enable bit:
     #pragma config DSWDTEN = OFF        //Disabled
   //Deep Sleep Watchdog Timer Postscale Select bits:
     #pragma config DSWDTPS = 8192       //1:8,192 (8.5 seconds)
   //IOLOCK One-Way Set Enable bit:
     #pragma config IOL1WAY = OFF        //The IOLOCK bit (PPSCON<0>) can be set and cleared as needed
   //MSSP address mask:
     #pragma config MSSP7B_EN = MSK7     //7 Bit address masking
   //Write Protect Program Flash Pages:
     #pragma config WPFP = PAGE_1        //Write Protect Program Flash Page 0
   //Write Protection End Page (valid when WPDIS = 0):
     #pragma config WPEND = PAGE_0       //Write/Erase protect Flash Memory pages starting at page 0 and ending with page WPFP[5:0]
   //Write/Erase Protect Last Page In User Flash bit:
     #pragma config WPCFG = OFF          //Write/Erase Protection of last page Disabled
   //Write Protect Disable bit:
     #pragma config WPDIS = OFF          //WPFP[5:0], WPEND, and WPCFG bits ignored
  
#else
    #error No hardware board defined, see "HardwareProfile.h" and __FILE__
#endif



//	========================	Global VARIABLES	========================
#pragma udata
//You can define Global Data Elements here

//	========================	PRIVATE PROTOTYPES	========================
static void InitializeSystem(void);
static void ProcessIO(void);
static void UserInit(void);
static void YourHighPriorityISRCode();
static void YourLowPriorityISRCode();

BOOL CheckButtonPressed(void);

//	========================	VECTOR REMAPPING	========================
#if defined(__18CXX)
  //On PIC18 devices, addresses 0x00, 0x08, and 0x18 are used for
  //the reset, high priority interrupt, and low priority interrupt
  //vectors.  However, the current Microchip USB bootloader 
  //examples are intended to occupy addresses 0x00-0x7FF or
  //0x00-0xFFF depending on which bootloader is used.  Therefore,
  //the bootloader code remaps these vectors to new locations
  //as indicated below.  This remapping is only necessary if you
  //wish to program the hex file generated from this project with
  //the USB bootloader.  If no bootloader is used, edit the
  //usb_config.h file and comment out the following defines:
  //#define PROGRAMMABLE_WITH_SD_BOOTLOADER
  
  #if defined(PROGRAMMABLE_WITH_SD_BOOTLOADER)
    #define REMAPPED_RESET_VECTOR_ADDRESS     0xA000
    #define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS  0xA008
    #define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS 0xA018
  #else 
    #define REMAPPED_RESET_VECTOR_ADDRESS     0x00
    #define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS  0x08
    #define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS 0x18
  #endif
  
  #if defined(PROGRAMMABLE_WITH_SD_BOOTLOADER)
  extern void _startup (void);        // See c018i.c in your C18 compiler dir
  #pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
  void _reset (void)
  {
      _asm goto _startup _endasm
  }
  #endif
  #pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
  void Remapped_High_ISR (void)
  {
       _asm goto YourHighPriorityISRCode _endasm
  }
  #pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
  void Remapped_Low_ISR (void)
  {
       _asm goto YourLowPriorityISRCode _endasm
  }
  
  #if defined(PROGRAMMABLE_WITH_SD_BOOTLOADER)
  //Note: If this project is built while one of the bootloaders has
  //been defined, but then the output hex file is not programmed with
  //the bootloader, addresses 0x08 and 0x18 would end up programmed with 0xFFFF.
  //As a result, if an actual interrupt was enabled and occured, the PC would jump
  //to 0x08 (or 0x18) and would begin executing "0xFFFF" (unprogrammed space).  This
  //executes as nop instructions, but the PC would eventually reach the REMAPPED_RESET_VECTOR_ADDRESS
  //(0x1000 or 0x800, depending upon bootloader), and would execute the "goto _startup".  This
  //would effective reset the application.
  
  //To fix this situation, we should always deliberately place a 
  //"goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS" at address 0x08, and a
  //"goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS" at address 0x18.  When the output
  //hex file of this project is programmed with the bootloader, these sections do not
  //get bootloaded (as they overlap the bootloader space).  If the output hex file is not
  //programmed using the bootloader, then the below goto instructions do get programmed,
  //and the hex file still works like normal.  The below section is only required to fix this
  //scenario.
  #pragma code HIGH_INTERRUPT_VECTOR = 0x08
  void High_ISR (void)
  {
       _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
  }
  #pragma code LOW_INTERRUPT_VECTOR = 0x18
  void Low_ISR (void)
  {
       _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
  }
  #endif  //end of "#if defined(||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER))"

  #pragma code
  
//	========================	Application Interrupt Service Routines	========================
  //These are your actual interrupt handling routines.
  #pragma interrupt YourHighPriorityISRCode
  void YourHighPriorityISRCode()
  {
    //Check which interrupt flag caused the interrupt.
    //Service the interrupt
    //Clear the interrupt flag
    //Etc.
  
  } //This return will be a "retfie fast", since this is in a #pragma interrupt section 
  #pragma interruptlow YourLowPriorityISRCode
  void YourLowPriorityISRCode()
  {
    //Check which interrupt flag caused the interrupt.
    //Service the interrupt
    //Clear the interrupt flag
    //Etc.
  
  } //This return will be a "retfie", since this is in a #pragma interruptlow section 
#endif




//	========================	Board Initialization Code	========================
#pragma code
#define ROM_STRING rom unsigned char*

/******************************************************************************
 * Function:        void UserInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine should take care of all of the application code
 *                  initialization that is required.
 *
 * Note:            
 *
 *****************************************************************************/
void UserInit(void)
{

	// Soft Start the APP_VDD
   while(!AppPowerReady())
		;

  /* Initialize the mTouch library */
  mTouchInit();

  /* Call the mTouch callibration function */
  mTouchCalibrate();

  /* Initialize the accelerometer */
  InitBma150(); 

  /* Initialize the oLED Display */
   ResetDevice();  
   FillDisplay(0x00);
   oledPutROMString((ROM_STRING)" PIC18F Starter Kit  ",0,0);
}//end UserInit


/********************************************************************
 * Function:        static void InitializeSystem(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        InitializeSystem is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.                  
 *
 * Note:            None
 *******************************************************************/
static void InitializeSystem(void)
{
    #if (defined(__18CXX) & !defined(PIC18F87J50_PIM))
        ADCON1 |= 0x0F;                 // Default all pins to digital


    #if defined(PIC18F87J50_PIM) || defined(PIC18F46J50_PIM)
  //On the PIC18F87J50 Family of USB microcontrollers, the PLL will not power up and be enabled
  //by default, even if a PLL enabled oscillator configuration is selected (such as HS+PLL).
  //This allows the device to power up at a lower initial operating frequency, which can be
  //advantageous when powered from a source which is not gauranteed to be adequate for 48MHz
  //operation.  On these devices, user firmware needs to manually set the OSCTUNE<PLLEN> bit to
  //power up the PLL.
    {
        unsigned int pll_startup_counter = 600;
        OSCTUNEbits.PLLEN = 1;  //Enable the PLL and wait 2+ms until the PLL locks before enabling USB module
        while(pll_startup_counter--);
    }
    //Device switches over automatically to PLL output after PLL is locked and ready.
    #endif

    #if defined(PIC18F46J50_PIM)
  //Configure all I/O pins to use digital input buffers.  The PIC18F87J50 Family devices
  //use the ANCONx registers to control this, which is different from other devices which
  //use the ADCON1 register for this purpose.
    ANCON0 = 0xFF;                  // Default all pins to digital
    ANCON1 = 0xFF;                  // Default all pins to digital
    #endif
    
    
    UserInit();

}//end InitializeSystem


//	========================	Application Code	========================



//=========================itoa for potetiometre====================

char *itoa_pot(int integer, char *string){

	int temp;  
	int i;
		for(i=3; i>=0; i--) { 
	     	temp = integer%10; 
	     	integer=integer/10;
			string[i]=temp+48;    //Thus the number 'v' gets stored as character or ASCII 
		}
}

//==========================itoa for negative=======================

char *itoa(int integer, char *string){

	int temp;  
	int i;
	if(integer<0){
		integer = -integer;
		for(i=3; i>=0; i--) { 
	     	temp = integer%10; 
	     	integer=integer/10;
			string[i]=temp+48;    
		}
		string[0]= '-';
	}else{
		for(i=3; i>=0; i--) { 
	     	temp = integer%10; 
	     	integer=integer/10;
			string[i]=temp+48;    
		}
		string[0]= '+';
	}

	return string;
	 
}


//==========================left right button========================
void displayLeftRight(void){
	char convertStringLeft[5];
	char convertStringRight[5];
	int left = 0;
	int right = 0;
	left = mTouchReadButton(3);
	right = mTouchReadButton(0);
	itoa(left, convertStringLeft);
	itoa(right, convertStringRight);
	convertStringLeft[4] = '\0';
	convertStringRight[4] = '\0';
		if((left>=600)&&(right>=600)){
			oledPutROMString("<<--->>", 4 , 0);
			FillPage(0, 0xb4);	
		}
		if((left<600)&&(right>600)){
			oledPutROMString("<------", 4 , 0);
			FillPage(0, 0xb4);
		}
		if((right<600)&&(left>600)){
			oledPutROMString("------>", 4 , 0);
			FillPage(0, 0xb4);
		}
}

//===========================Up Down Code============================
void displayUpDown(void){
	char convertStringUp[5];
	char convertStringDown[5];
	int up = 0;
	int down = 0;
	up = mTouchReadButton(1);
	down = mTouchReadButton(2);
	itoa(up, convertStringUp);
	itoa(down, convertStringDown);
	convertStringUp[4] = '\0';
	convertStringDown[4] = '\0';
	if((up>=500)&&(down>=500)){
		FillPage(0, 0xb6);	
	}
	if(up<500){
		oledWriteChar1x('_' ,0xb6 , 0);
	}
	if(down<500){
		oledWriteChar1x('?' ,0xb6 , 10);
	}
}

//==========================potentiometer code=======================

int displayPotentiometer(int counter){
	int pot = 0;
	char convertStringPot[5];
	int i;
	ADCON0&=0xA0;
	ADCON0|=0x11;
	ADCON0|=0x02;
	// read meter param
	while(ADCON0&2)
			;
		// set meter param into an integer type
		pot = ((int)ADRESH << 8) + ADRESL;
		itoa_pot(pot, convertStringPot);
		convertStringPot[4] = '\0';
		oledPutROMString("Pot:",0,0);
		oledPutString(convertStringPot,0,25);
		pot = pot/10;
		if(counter == 20){		//delay so that the graph doesnt blink
			FillPage(0, 0xb2);	//new function to fil a specific page with a byte
			counter = 0 ;
		}
		counter++;
		oledRepeatByte('^', pot, 0xb2, 0);	//new function to repeat a black square	
	return counter;
}

//=======================Button Press code ==========================

void displayButtonPress(void){
	oledPutROMString("On/Off:",0,52);
    if(PORTBbits.RB0 == 0){
      oledWriteChar1x('}' ,0xb1 , 96);
    }
    else{
		oledWriteChar1x('{' ,0xb1 , 96);
	}
  		  
}


//=======================temperature code===========================

void displayTemperature(void){
	int temperature = 0;
	char convertStringTemperature[5];
	char convertStringTemp[4];
	temperature = BMA150_ReadByte(0x08);
	
	itoa(temperature , convertStringTemp);
	convertStringTemp[4] = '\0';
	convertStringTemperature[0] = convertStringTemp[2];
	convertStringTemperature[1] = convertStringTemp[3];
	convertStringTemperature[2] = '~';
	convertStringTemperature[3] = '\0';
	oledPutROMString("Temp:",2,0);
	oledPutString(convertStringTemperature,2 ,30);
	
}




//===================accelerometer x code=============================

int displayAccelerometerX(int x_max){
	int x_h =0;
	int x_l =0;
	int x_msb = 0;
	int x = 0;
	char convertStringAccX[5];
	x_h = BMA150_ReadByte(0x03);
	x_l = BMA150_ReadByte(0x02);
	x = ((int)x_h<<2)+(x_l>>6);
	x_msb = x;
	x_msb = x_msb>>9;
	if(x_msb){				//if msb is 1, extend sign
		x = x|0xfc00; 
	}
	if(x > 0){
		if(x_max>=0){
			if(x > x_max){
				x_max = x;
			}
		}
		if(x_max < 0){
				if(-x_max < x){
					x_max = x;
				}
		} 
	}
	if(x < 0){
		if(x_max < 0){
			if(x<x_max){
				x_max = x;
			}
		}
		if(x_max >= 0){
			if(-x>x_max){
				x_max = x;
			}
		}
	}

	if((x_max>-50)&&(x_max<50)){		//if chip hasent been moved much
		oledWriteChar1x('[' ,0xb3 , 70);
		oledWriteChar1x('#' ,0xb3 , 74);
		oledWriteChar1x('#' ,0xb3 , 78);
		oledWriteChar1x('$' ,0xb3 , 82);
		oledWriteChar1x('#' ,0xb3 , 86);
		oledWriteChar1x('#' ,0xb3 , 90);
		oledWriteChar1x(']' ,0xb3 , 94);
	}
	if((x_max>=50)&&(x_max<=150)){
		oledWriteChar1x('[' ,0xb3 , 70);
		oledWriteChar1x('#' ,0xb3 , 74);
		oledWriteChar1x('#' ,0xb3 , 78);
		oledWriteChar1x('#' ,0xb3 , 82);
		oledWriteChar1x('$' ,0xb3 , 86);
		oledWriteChar1x('#' ,0xb3 , 90);
		oledWriteChar1x(']' ,0xb3 , 94);
	}
	if((x_max<=-50)&&(x_max>=-150)){
		oledWriteChar1x('[' ,0xb3 , 70);
		oledWriteChar1x('#' ,0xb3 , 74);
		oledWriteChar1x('$' ,0xb3 , 78);
		oledWriteChar1x('#' ,0xb3 , 82);
		oledWriteChar1x('#' ,0xb3 , 86);
		oledWriteChar1x('#' ,0xb3 , 90);
		oledWriteChar1x(']' ,0xb3 , 94);
	}
	if((x_max>=151)&&(x_max<=511)){
		oledWriteChar1x('[' ,0xb3 , 70);
		oledWriteChar1x('#' ,0xb3 , 74);
		oledWriteChar1x('#' ,0xb3 , 78);
		oledWriteChar1x('#' ,0xb3 , 82);
		oledWriteChar1x('#' ,0xb3 , 86);
		oledWriteChar1x('$' ,0xb3 , 90);
		oledWriteChar1x(']' ,0xb3 , 94);
	}
	if((x_max<=-151)&&(x_max>=-511)){
		oledWriteChar1x('[' ,0xb3 , 70);
		oledWriteChar1x('$' ,0xb3 , 74);
		oledWriteChar1x('#' ,0xb3 , 78);
		oledWriteChar1x('#' ,0xb3 , 82);
		oledWriteChar1x('#' ,0xb3 , 86);
		oledWriteChar1x('#' ,0xb3 , 90);
		oledWriteChar1x(']' ,0xb3 , 94);
	}
	itoa(x ,convertStringAccX);
	convertStringAccX[4] = '\0';
	oledPutROMString("X:", 4,70);
	oledPutString(convertStringAccX ,4 ,80);
	return x_max;
}

//===================accelerometer y code=============================

int displayAccelerometerY(int y_max){
	int y_h =0;
	int y_l =0;
	int y_msb = 0;
	int y = 0;
	char convertStringAccY[5];
	y_h = BMA150_ReadByte(0x05);
	y_l = BMA150_ReadByte(0x04);
	y = ((int)y_h<<2)+(y_l>>6);
	y_msb = y;
	y_msb = y_msb>>9;
	if(y_msb){			//if msb is 1, extend sign
		y = y|0xfc00; 
	}
	if(y_max>=0){
		if(y>y_max){
			y_max=y;
		}
	}
	if(y_max < 0){
		if(y < y_max){
			y_max = y;
		}
	}

	if((y_max>-50)&&(y_max<50)){		//if chip hasent been moved much
		oledWriteChar1x('(' ,0xb0 , 112);
		oledWriteChar1x('%' ,0xb1 , 112);
		oledWriteChar1x('%' ,0xb2 , 112);
		oledWriteChar1x('&' ,0xb3 , 112);
		oledWriteChar1x('%' ,0xb4 , 112);
		oledWriteChar1x('%' ,0xb5 , 112);
		oledWriteChar1x(')' ,0xb6 , 112);
	}
	if((y_max>=50)&&(y_max<=150)){
		oledWriteChar1x('(' ,0xb0 , 112);
		oledWriteChar1x('%' ,0xb1 , 112);
		oledWriteChar1x('&' ,0xb2 , 112);
		oledWriteChar1x('%' ,0xb3 , 112);
		oledWriteChar1x('%' ,0xb4 , 112);
		oledWriteChar1x('%' ,0xb5 , 112);
		oledWriteChar1x(')' ,0xb6 , 112);
	}
	if((y_max<=-50)&&(y_max>=-150)){
		oledWriteChar1x('(' ,0xb0 , 112);
		oledWriteChar1x('%' ,0xb1 , 112);
		oledWriteChar1x('%' ,0xb2 , 112);
		oledWriteChar1x('%' ,0xb3 , 112);
		oledWriteChar1x('&' ,0xb4 , 112);
		oledWriteChar1x('%' ,0xb5 , 112);
		oledWriteChar1x(')' ,0xb6 , 112);
	}
	if((y_max>=151)&&(y_max<=511)){
		oledWriteChar1x('(' ,0xb0 , 112);
		oledWriteChar1x('&' ,0xb1 , 112);
		oledWriteChar1x('%' ,0xb2 , 112);
		oledWriteChar1x('%' ,0xb3 , 112);
		oledWriteChar1x('%' ,0xb4 , 112);
		oledWriteChar1x('%' ,0xb5 , 112);
		oledWriteChar1x(')' ,0xb6 , 112);
	}
	if((y_max<=-151)&&(y_max>=-511)){
		oledWriteChar1x('(' ,0xb0 , 112);
		oledWriteChar1x('%' ,0xb1 , 112);
		oledWriteChar1x('%' ,0xb2 , 112);
		oledWriteChar1x('%' ,0xb3 , 112);
		oledWriteChar1x('%' ,0xb4 , 112);
		oledWriteChar1x('&' ,0xb5 , 112);
		oledWriteChar1x(')' ,0xb6 , 112);
	}
	itoa(y ,convertStringAccY);
	convertStringAccY[4] = '\0';
	oledPutROMString("Y:", 6,70);
	oledPutString(convertStringAccY ,6 ,80);
	return y_max;
}

//===================accelerometer z code=============================

int displayAccelerometerZ(void){
	int z_h =0;
	int z_l =0;
	int z_msb = 0;
	int z = 0;
	char convertStringAccZ[5];
	z_h = BMA150_ReadByte(0x07);
	z_l = BMA150_ReadByte(0x06);
	z = ((int)z_h<<2)+(z_l>>6);
	z_msb = z;
	z_msb = z_msb>>9;
	if(z_msb){
		z = z|0xfc00; 
	}
	itoa(z ,convertStringAccZ);
	convertStringAccZ[4] = '\0';
	return z;
}

//============================Main Code===========================

void main(void)
{

	int x_max = 0;
	int y_max = 0;
	int z = 0;
	int counter =0;
    InitializeSystem();
	FillDisplay(0);
	
    while(1)							//Main is Usualy an Endless Loop
    {
		
		counter = displayPotentiometer(counter);
		displayLeftRight();
		displayUpDown();
		displayButtonPress();
		displayTemperature();
		x_max = displayAccelerometerX(x_max);
		y_max = displayAccelerometerY(y_max);
		z = displayAccelerometerZ();
		if(z<0){
			x_max = 0;
			y_max = 0;
		}

    }
}//end main


/** EOF main.c *************************************************/
//#endif
