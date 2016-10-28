/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       main.c
 * \brief      Ping-Pong example application on how to use Semtech's Radio
 *             drivers.
 *
 * \version    2.0
 * \date       Nov 21 2012
 * \author     Miguel Luis
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "platform.h"
#include "led.h"

#if USE_UART
#include "uart.h"
#endif

#include "radio.h"


#define BUFFER_SIZE     30                          // Define the payload size here

static uint16_t BufferSize = BUFFER_SIZE;			// RF buffer size
static uint8_t  Buffer[BUFFER_SIZE];				// RF buffer

static uint8_t EnableMaster = true; 				// Master/Slave selection

tRadioDriver *Radio = NULL;

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";
const uint8_t MY_TEST_Msg[] = "YQH_SX1278_TEST";


void OnMaster( void );
void OnSlave( void );
/*
 * Manages the master operation
 */
void OnMaster( void )
{
    uint8_t i;
    switch( Radio->Process( ) )
    {
    case RF_RX_TIMEOUT:
		printf("\r\n RF_RX_TIMEOUT \r\n");
        // Send the next PING frame
        Buffer[0] = 'P';
        Buffer[1] = 'I';
        Buffer[2] = 'N';
        Buffer[3] = 'G';
        for( i = 4; i < BufferSize; i++ )
        {
            Buffer[i] = i - 4;
        }
        Radio->SetTxPacket( Buffer, BufferSize );   //RFLR_STATE_TX_INIT
        break;
    case RF_RX_DONE:
		printf("\r\n RF_RX_DONE \r\n");
        Radio->GetRxPacket( Buffer, ( uint16_t* )&BufferSize );


		if( BufferSize > 0 )
        {
            if( strncmp( ( const char* )Buffer, ( const char* )PongMsg, 4 ) == 0 )
            {
                // Indicates on a LED that the received frame is a PONG
				
                printf("Recv: ");
				for(i=0;i<4;i++)
				{
					USART_putchar(USART2,Buffer[i]);
				}
				printf("\r\n");
    

                // Send the next PING frame            
                Buffer[0] = 'P';
                Buffer[1] = 'I';
                Buffer[2] = 'N';
                Buffer[3] = 'G';
                // We fill the buffer with numbers for the payload 
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }
                Radio->SetTxPacket( Buffer, BufferSize );    //RFLR_STATE_TX_INIT
            }
            else if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
            { // A master already exists then become a slave
                EnableMaster = false;
            }
        }            
        break;
    case RF_TX_DONE:
		printf("\r\n RF_TX_DONE \r\n");
        // Indicates on a LED that we have sent a PING
        Radio->StartRx( );   //RFLR_STATE_RX_INIT
        break;
    default:
        break;
    }
}

/*
 * Manages the slave operation
 */
void OnSlave( void )
{
    uint8_t i;
    switch( Radio->Process( ) )
    {
    case RF_RX_DONE:
		printf("\r\n RF_RX_DONE \r\n");
        Radio->GetRxPacket( Buffer, ( uint16_t* )&BufferSize );
        
	
	
        if( BufferSize > 0 )
        {
            if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
            {
                // Indicates on a LED that the received frame is a PING

				for(i=0;i<4;i++)
				{
					USART_putchar(USART2,Buffer[i]);
				}
				printf("\n");
				
               // Send the reply to the PONG string
                Buffer[0] = 'P';
                Buffer[1] = 'O';
                Buffer[2] = 'N';
                Buffer[3] = 'G';
                // We fill the buffer with numbers for the payload 
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }

                Radio->SetTxPacket( Buffer, BufferSize );      //RFLR_STATE_TX_INIT
            }
        }
        break;
    case RF_TX_DONE:
        // Indicates on a LED that we have sent a PONG
//        LedToggle( LED_RED );
	printf("\r\n RF_TX_DONE \r\n");
        Radio->StartRx( );   //RFLR_STATE_RX_INIT
        break;
    default:
        break;
    }
}


#define SX1278_RX
//#define SX1278_TX

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
	uint8_t i;
	//stm32 config
	uart_init();
//    while(1)
//        USART_putchar(USART2,'A');
    
	printf("\r\n sys init finished \r\n");
	
	BoardInit( );
    printf("\r\n Board init finished \r\n");
	
    Radio = RadioDriverInit( );
    
    Radio->Init( );
	printf("\r\n Radio init finished\r\n");
	
#if defined (SX1278_RX)
    Radio->StartRx( );   //RFLR_STATE_RX_INIT
	printf("\r\n StartRx  \r\n");
#elif defined (SX1278_TX)
	Radio->SetTxPacket( MY_TEST_Msg, 18 );
	printf("\r\n SetTxPacket start \r\n");
#endif
	
	while( 1 )
    {
        if( EnableMaster == true )
        {
            OnMaster( );
        }
        else
        {
            OnSlave( );
        }    
       
    }
#ifdef __GNUC__
    return 0;
#endif
	
}	





