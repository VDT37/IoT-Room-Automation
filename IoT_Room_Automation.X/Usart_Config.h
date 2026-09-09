#ifndef USART_HEADER_FILE_H
#define	USART_HEADER_FILE_H

#define F_CPU 20000000/64            /* Define ferquency */
#define BAUDRATE (((float)(F_CPU)/(float)baud_rate)-1)/* Define Baud value */


void USART_Init(long baud_rate)
{
    TRISC6 = 0;                      /* Make Tx pin as output */
    TRISC7 = 1;                      /* Make Rx pin as input */
    SPBRG = (unsigned int)BAUDRATE;  /* Baud rate, SPBRG = (F_CPU /(64*baud_rate))-1 */
 
    
    TXSTA = 0x20;   // 0010 0000              /* Transmit Enable(TX) enable */ 
    RCSTA = 0x90;   // 1001 0000              /* Receive Enable(RX) enable and serial port enable */
}
/******************TRANSMIT FUNCTION*****************************************/ 
void USART_TxChar(char data)
{        
    while(!TXIF)                    /* Wait for buffer empty */
    TXREG = data;                   /* Copy data to be transmit in Tx register */
}
/*******************RECEIVE FUNCTION*****************************************/
char USART_RxChar()
{
    while(!RCIF);                   /* Wait for receive interrupt flag*/
    return(RCREG);                  /* Return received data */
}

/*******************SEND STRING FUNCTION*****************************************/
void USART_Sendstring(const char *str)
{
   while(*str!='\0')                /* Transmit data until null */
   {            
        USART_TxChar(*str);
        str++;
   }
}


#endif	/* USART_HEADER_FILE_H */
