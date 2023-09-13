/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F26K83
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"
#include <math.h>
/*
                         Main application
 */

// MCC generated interrupts implementation, as specifed per Microchip Developer Help, these application-specific ISR functions need to be implemented in 
// in non-MCC supplied files like [main.c]

// The following implementations however follows the example for said implementation in [ecan.h]


// Variable configurable (CAN_ID)
// --------------------------------------------------------------
const unsigned long canId = 3;        //120
// --------------------------------------------------------------

// -- Prototipos de funciones
void sendData(unsigned int);
void sendAck(unsigned int);
void setZero();					
void selfCalibration();
void program_AksIM();

// -- Inicialización de variables para el envío

int pushFlag = 0; // flag para saber si está ejecutando el PUSH
uint8_t delay;
uint8_t  message[4];
//int  message[4];
uint32_t position_bits;
double degrees;
uint8_t x,y;
uint8_t delay;
uCAN_MSG ReceiveMsg;
int cont = 0;

//SPI testing variables
uint8_t data_TX = 0x32;          /* Data that will be transmitted */
uint8_t data_RX = 0;        /* Data that will be received */


void OverflowBuffer0 (void)
{
  

}


void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();    //This function from the mcc.h initializes the desired pins, oscillator, interrupts and SPI paramaeters
    
    TRISCbits.TRISC2 = 0;       //RC2 is an output
    TRISCbits.TRISC3 = 0;       //RC3 is an output
    TRISCbits.TRISC5 = 0;       //RC5 is an output
    //PORTCbits.RC2 = 1;          //SS APAGADO -> SPI APAGADO
    PORTAbits.RA5 = 1;

    LATB = 0x00;
    TRISBbits.TRISB0 = 0;       //RC6 is an output
    TRISBbits.TRISB1 = 0;       //RC7 is an output
    
    //ECAN_SetRXB0OverflowHandler(OverflowBuffer0);
    
    //Acknowledge message sent when the PIC is powered on
    sendAck(0x200);

    
    while (1)
    {   

        
        /*
        if (RXB0CONbits.RXFUL == 0)
        {
            LATCbits.LATC6 = 1;
        }
        else if(RXB1CONbits.RXFUL == 0)
        {
            LATCbits.LATC7 = 1;
        }
        
        */
       /*
         LATCbits.LATC6 = 1;
        LATCbits.LATC7 = 1;
       
        LATBbits.LATB0 = 1;
        LATBbits.LATB1 = 1;
        PORTBbits.RB0 = 1;
        PORTBbits.RB1 = 1;*/
        
        //Try with the ADCON register to see if the LEDS light up that way
        
         //--------------- Recibo!!! ------------------
        
        if (CAN_receive(&ReceiveMsg))
        {
            if (ReceiveMsg.frame.id!= canId)
            {
                if (pushFlag)
                {
                    sendData(0x80);
                    __delay_ms(10);    // For a 16MHz frequency the period equates to 0.0625 µs. 
                }

                continue;   
            }
            
            switch (ReceiveMsg.frame.data0)
            {
                case 0x00: //stablish a connection with the CAN master 
                    
                    sendAck(0x200);
                    break;

                case 0x01: // start push (continuous mode)
                    delay = ReceiveMsg.frame.data1;
                    pushFlag = 1;
                    sendAck(0x100);
                    break;

                case 0x02: // stop push
                    pushFlag = 0;
                    sendAck(0x100);
                    break;

                case 0x03: // pull (polling mode)
                    sendData(0x180);
                    break;

                //case 0xFF: // zero (mainly used for autocalibration)
                //    //setZero();
                //    sendAck(0x200);
                //    break;
            } // switch
            
        }
        
        
        
     //*/
    SPI1_Close();    
    }    
}

void sendData(unsigned int op)
{
    uint8_t data_RX[4];
    
    /*
    The SPI module is setup to be in "Full-Duplex Mode" (TXR = 1, RXR = 1) even though the slave device (encoder) does not read the MOSI line. 
    The transmitted values were selected for ease of measuring with an oscilloscope.
    
    *   Bit 31:10 -> Posición del encoder + zero padding bits - Alineado a la izquierda, MSB primero    ->   El módelo seleccionado MB049 tiene una resolución de 19 bits (28-10, 3 bits)
	*	Bit 9 -> Error, si es '0' la información de la posición no es válida
	*	Bit 8 -> Advertencia, si es '0' la información de la posición es válida pero opera en condición límites
	*	Bit 7:0 ->	CRC invertido, 0x97 polinomial
	*
	*	Estos bits se dividen en 4 bytes que envía el encoder
	*	Byte 0: 31 - 24
	*	Byte 1: 23 - 16
	*	Byte 2: 15 - 8
	*	Byte 3: 7 - 0
    
    */  
    
    
    //SPI module is enabled
    SPI1_Open(MASTER_0);
    //SS line is held low to begin communication
    PORTAbits.RA5 = 0;  //This is not really necessary since the module is configured to begin transfer once the transfer counter and the transmit buffer are loaded
    SPI1TCNTL = 4;  //The transfer counter is loaded 
    
	SPI1TXB = 0xFF; // First byte is exchanged, 0x33 = 0b01101001  -> buffer[0]  
    SPI1TXB = 0x00; // Second byte is exchanged, 0x55 = 0b01010101  -> buffer[1]
    __delay_us(3.75);	// A delay is added matching the clock frequency for 15 bits so that the first byte is already exchanged (the second close to ending). A new byte can be loaded into the transfer buffer
    data_RX[0] = SPI1RXB;
    SPI1TXB = 0xFF; // Third byte is exchanged, 0xAA = 0b10101010  -> buffer[2]
    __delay_us(3.75);   
    data_RX[1] = SPI1RXB;
    SPI1TXB = 0x00; // Fourth byte is exchanged, 0xAA = 0b10101010  -> buffer[2] 
    __delay_us(3.75);
    data_RX[2] = SPI1RXB;
    __delay_us(3.75);
    data_RX[3] = SPI1RXB;
    
    //SS line is held high to end communication     
    PORTAbits.RA5 = 1;
    //SPI module is disabled
    SPI1_Close();
      
    
    
    position_bits = ((uint32_t)data_RX[2] >> 2) + ((uint32_t)data_RX[1] << 6) + ((uint32_t)data_RX[3] << 14);
	//position_bits = data_RX[2] >> 2 | data_RX[1] << 6 | data_RX[3] << 14;
    degrees = (360 / pow (2,17)-1) * position_bits;      // One unit is subtracted from the total number of bits since it begins in 0
    

    /*
    x = 0;
    uCAN_MSG SendMsg;
    SendMsg.frame.idType = dSTANDARD_CAN_MSG_ID_2_0B;
    SendMsg.frame.id = op+canId;
    SendMsg.frame.dlc = 3;
    SendMsg.frame.data0 = position_bits & 0xFF;
    SendMsg.frame.data1 = (position_bits >> 8) & 0xFF;
    SendMsg.frame.data2 = (position_bits >> 16) & 0xFF;
    */
    
    x = 0;
    uCAN_MSG SendMsg;
    SendMsg.frame.idType = dSTANDARD_CAN_MSG_ID_2_0B;
    SendMsg.frame.id = op+canId;
    SendMsg.frame.dlc = 1;
    SendMsg.frame.data0 = degrees;
    
    while (!x)
    {
        x = CAN_transmit (&SendMsg);
    }
     
     
    
}

void sendAck(unsigned int op)
{
	x = 0;
    uCAN_MSG AckMsg;
    AckMsg.frame.idType = dSTANDARD_CAN_MSG_ID_2_0B;
    AckMsg.frame.id = op+canId;      //op+canId 
    AckMsg.frame.dlc = 1; 
    //AckMsg.frame.data0 = 0x32;  //Prueba CAN bus loop
    //AckMsg.frame.data0 = 0x33;  //Prueba SPI
    
    
    if(PORTAbits.RA4 == 1){
        AckMsg.frame.data0 = 0x33;  //Prueba SPI
    }
    else if (PORTAbits.RA4 == 0){
       AckMsg.frame.data0 = 0x32;  //Prueba CAN bus loop 
    }
    
	while (!x)
	{
        x = CAN_transmit (&AckMsg);
        //PORTCbits.RC6 = 1;  //LED is lit if there is an stablished connection with the PC through CAN bus
		//x = ECANSendMessage(op + canId, NULL, 0, txFlags);	
		//Esta función es de tipo bool, por lo que el bucle tratará de enviar el mesanje hasta que se haya encontrado un buffer vacío al que se le pueda enviar los datos .

	}
    /*
    LATCbits.LC2 = 1;
            __delay_ms(1000);
            LATCbits.LC2 = 0;
    */
    x=0;
}


/**
 End of File
*/