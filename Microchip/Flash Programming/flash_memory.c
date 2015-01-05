/*
********************************************************************************
                                                                                
Software License Agreement                                                      
                                                                                
Copyright © 2008 Microchip Technology Inc. and its licensors.  All         
rights reserved.                                                                
                                                                                
Microchip licenses to you the right to: (1) install Software on a single        
computer and use the Software with Microchip 16-bit microcontrollers and        
16-bit digital signal controllers ("Microchip Product"); and (2) at your        
own discretion and risk, use, modify, copy and distribute the device            
driver files of the Software that are provided to you in Source Code;           
provided that such Device Drivers are only used with Microchip Products         
and that no open source or free software is incorporated into the Device        
Drivers without Microchip's prior written consent in each instance.             
                                                                                
You should refer to the license agreement accompanying this Software for        
additional information regarding your rights and obligations.                   
                                                                                
SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY         
KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY              
WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A          
PARTICULAR PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE             
LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY,               
CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY           
DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY         
INCIDENTAL, SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR         
LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY,                 
SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY         
DEFENSE THEREOF), OR OTHER SIMILAR COSTS.                                       
                                                                                
********************************************************************************
*/

#include "Compiler.h"
#include "GenericTypedefs.h"
#include "Flash Programming\flash_memory.h"

/*******************************************************************************
  Function:
    void WriteMem(WORD cmd)

  Description:
    Write stored registers to flash memory  
    
  PreCondition:
    Appropriate data written to latches with WriteLatch

  Parameters:
    cmd - type of memory operation to perform
                               
  Returns:
    None.

  Remarks: 
*******************************************************************************/
void WriteMem(WORD cmd)
{
#if 0
/* Porting on PIC18 */			
	NVMCON = cmd;

	__builtin_write_NVM();

	while(NVMCONbits.WR == 1);	
#endif
}	


/*******************************************************************************
  Function:
    void WriteLatch(WORD page, WORD addrLo, WORD dataHi, WORD dataLo)

  Description:
    
    
  PreCondition:

  Parameters:
                               
  Returns:

  Remarks: 
*******************************************************************************/
void WriteLatch(WORD page, WORD addrLo, WORD dataHi, WORD dataLo)
{
#if 0
/* Porting on PIC18 */			
	TBLPAG = page;

	__builtin_tblwtl(addrLo,dataLo);
	__builtin_tblwth(addrLo,dataHi);
#endif
}	


/*******************************************************************************
  Function:
    DWORD ReadLatch(WORD page, WORD addrLo)

  Description:
    
    
  PreCondition:

  Parameters:
                               
  Returns:

  Remarks: 
*******************************************************************************/
DWORD ReadLatch(WORD page, WORD addrLo)
{
#if 0
/* Porting on PIC18 */			
	DWORD_VAL temp;

	TBLPAG = page;

	temp.word.LW = __builtin_tblrdl(addrLo);
	temp.word.HW = __builtin_tblrdh(addrLo);

	return temp.Val;
#endif
}


/*******************************************************************************
  Function:
    void Erase(WORD page, WORD addrLo, WORD cmd)

  Description:
    
    
  PreCondition:

  Parameters:
                               
  Returns:

  Remarks: 
*******************************************************************************/
void Erase(WORD page, WORD addrLo, WORD cmd)
{
#if 0
/* Porting on PIC18 */			
	WORD temp;	

	temp = TBLPAG;
	TBLPAG = page;

	NVMCON = cmd;

	__builtin_tblwtl(addrLo,addrLo);

	__builtin_write_NVM();

	while(NVMCONbits.WR == 1);

	TBLPAG = temp;
#endif
}

/*******************************************************************************
  Function:
    void WritePM(WORD length, DWORD_VAL address, BYTE * buffer)
    
  Description:
    
  PreCondition:

  Parameters:
                               
  Returns:

  Remarks: 
*******************************************************************************/
void WritePM(WORD length, DWORD_VAL address, BYTE * buffer)
{
  WORD bytesWritten;
  DWORD_VAL data;

	bytesWritten = 0;		
	
	//write length rows to flash
	while((bytesWritten) < length*PM_ROW_SIZE)
	{
		//get data to write from buffer
		data.v[0] = buffer[bytesWritten+0];
		data.v[1] = buffer[bytesWritten+1];
		data.v[2] = buffer[bytesWritten+2];
		data.v[3] = buffer[bytesWritten+3];
		
		//4 bytes per instruction: low word, high byte, phantom byte
		bytesWritten+=PM_INSTR_SIZE;

		//write data into latches
   		WriteLatch(address.word.HW, address.word.LW, data.word.HW, data.word.LW);

		//write to flash memory if complete row is finished
		if((bytesWritten % PM_ROW_SIZE) == 0)
		{
			//execute write sequence
			WriteMem(PM_ROW_WRITE);	
		}

		address.Val = address.Val + 2;  //increment addr by 2
	}
}

/*******************************************************************************
  Function:
    void ReadPM(WORD length, DWORD_VAL address, BYTE * buffer)
    
  Description:
    
  PreCondition:

  Parameters:
                               
  Returns:

  Remarks: 
*******************************************************************************/
void ReadPM(WORD length, DWORD_VAL address, BYTE * buffer)
{
	WORD bytesRead = 0;
	DWORD_VAL temp;

	//Read length instructions from flash
	while(bytesRead < length*PM_INSTR_SIZE)
	{
		//read flash
		temp.Val = ReadLatch(address.word.HW, address.word.LW);

		buffer[bytesRead+0] = temp.v[0];   	 
		buffer[bytesRead+1] = temp.v[1];	
		buffer[bytesRead+2] = temp.v[2];
		buffer[bytesRead+3] = temp.v[3];
	
		//4 bytes per instruction: low word, high byte, phantom byte
		bytesRead+=PM_INSTR_SIZE; 

		address.Val = address.Val + 2;  //increment addr by 2
	}
}

/*******************************************************************************
  Function:
    void ErasePM(WORD length, DWORD_VAL address)
    
  Description:
    
  PreCondition:

  Parameters:
                               
  Returns:

  Remarks: 
*******************************************************************************/
void ErasePM(WORD length, DWORD_VAL address)
{
  WORD bytesErased;
  
  while(bytesErased < length*PM_PAGE_SIZE)
  {
		Erase(address.word.HW, address.word.LW, PM_PAGE_ERASE);

		bytesErased += PM_PAGE_SIZE;

		address.Val += PM_PAGE_SIZE/2;	//increment by a page
  }
}

