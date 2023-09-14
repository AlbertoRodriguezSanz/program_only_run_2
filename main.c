
/*
 * PIC firmware for interfacing with an absolute CUI encoder through CAN.
 *
 * Copyright: 2023 (C) Universidad Carlos III de Madrid
 * Original author: Alberto Rodríguez Sanz (master's thesis, UC3M, 2023)
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later
 */


#include "mcc_generated_files/mcc.h"
#include <math.h>
/*
                         Main application
 */

// Configurable variable (CAN_ID)
// --------------------------------------------------------------
const unsigned long canId = 3;        //120
// --------------------------------------------------------------

// -- Function prototypes
void sendData(unsigned int);
void sendAck(unsigned int);
void setZero();					
void selfCalibration();
void program_AksIM();

// -- CAN bus communication variables

int pushFlag = 0; // flag para saber si está ejecutando el PUSH
uint8_t delay;
uint8_t  message[4];
uint32_t position_bits;
double degrees;
uint8_t x,y;
uint8_t delay;
uCAN_MSG ReceiveMsg;
int cont = 0;

//SPI communication variables
uint8_t data_RX[4];       /* Data that will be received */


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
        
    
    }    
}

void sendData(unsigned int op)
{
    
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
    AckMsg.frame.data0 = 0x00;
   
    
	while (!x)
	{
        x = CAN_transmit (&AckMsg);

	}
    
}


/**
 End of File
*/