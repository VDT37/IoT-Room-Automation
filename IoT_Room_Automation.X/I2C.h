#ifndef I2C_H
#define I2C_H

/**************************************** SFR Initializations for I2C MASTER Mode *****************************************************/
void i2c_init(void)
{
    SSPCON = 0b00101000;        // SSP CONTROL REGISTER 1 - Configuring RC3 and RC4 as SCL and SDA Pins, Enabling I2C MASTER Mode through SSPM<3:0> Bits
    SSPCON2 = 0b00000000;       // SSP CONTROL REGISTER 2 - Initially this register is cleared before initializing I2C Communication, because these are ENABLE Bits, we'll enable it when required.
    SSPSTAT = 0b00000000;       // SSP STATUS REGISTER - Initially '0', once Status is detected its corresponding bit will become HIGH.
    SSPADD = 0b00000100;        // SSP ADDRESS REGISTER - 400KHz High Speed Communication
/* 
 * The Baud Rate Generator(BRG) reload value is contained in the lower 7 bits of the SSPADD register. 
   The Baud Rate Generator will automatically begin counting on a write to the SSPBUF Buffer register.
   When the BRG is loaded with this value, the BRG counts down to 0 and stops until another reload has taken place. 
*/                                 
}

/********************************************** BUS IDLE or Not BUSY Function *********************************************************/

/* BUS IDLE TIME is Time between STOP and START conditions when both SDA and SCL are inactive and in their logic-high states.
 * When the bus is idle, it often initiates a low-power mode for slave devices.
 */
void i2c_busIdle()
{ 
    while((SSPCON2 & 0b00011111) || (SSPSTAT & 0b00000100));    // Check if bus is idle/ not busy. If Busy then wait till the condition is finished.
/*
 * For bits ACKEN, RCEN, PEN, RSEN, SEN of SSPCON2: If the I2C module is not in the Idle mode, 
these bits may not be set(or not transiting) and the SSPBUF may not be written (or writes to the SSPBUF are disabled).
 
 * OR-ing R/W(Bit 2) bit of SSPSTAT with SEN, RSEN, PEN, RCEN, or ACKEN will indicate if the MSSP is in Idle mode.
 * If Transmission is in progress, then R/W of SSPSTAT will become 1 and indicates the bus is not idle.
 * Once transmission is finished, R/W of SSPSTAT will become "0" indicating Bus is not busy or IDLE. Then we can send the new data.
 */ 
}

void i2c_start(void)    // Function to initiate Start Condition
{
    SEN = 1;            // The user generates a Start condition by setting the Start Enable (SEN) bit (SSPCON2 register).
    i2c_busIdle();      // Checks if Bus is IDLE or not
}

void i2c_stop(void)     // Function to initiate Stop Condition
{
    PEN=1;              // The user generates a Stop condition by setting the Stop Enable bit PEN (SSPCON2 register).
    i2c_busIdle();      // Checks if Bus is IDLE or not
}

void i2c_repeatedStart(void)    // Initiating Repeated Start Condition
{
    RSEN=1;             // A Repeated Start condition occurs when the RSEN bit (SSPCON2 register) is programmed high and the I2C logic module is in the Idle state. 
    i2c_busIdle();      // Checks if Bus is IDLE or not
}

void receive_enable(void)      // Function to enable Master Mode Reception
{
    RCEN =1;            // Master mode reception is enabled by programming the Receive Enable bit, RCEN (SSPCON2 register). 
    i2c_busIdle();      // Checks if Bus is IDLE or not
}

void I2C_Write(unsigned char data)  // Function to Write Data to SSPBUF Buffer register
{
    SSPBUF = data;        // Loading the data into SSPBUF
    i2c_busIdle();        // Checks if Bus is IDLE or not
}

unsigned char I2C_Read(unsigned char ack)   // Function to Read the Data from SSPBUF
{
    unsigned char incoming;

    receive_enable();
    incoming = SSPBUF;  // Get the data saved in SSPBUF
    i2c_busIdle();

    ACKDT = ack;        // Check if 'ack' bit received  
    ACKEN = 1;          // An Acknowledge sequence is enabled by setting the ACKEN bit (SSPCON2 register).
    i2c_busIdle();      // Checks if Bus is IDLE or not
        
    /*  ACKDT - ACKNOWLEDGE DATA BIT, This bit should be set before starting an Acknowledge Sequence.  
        In MASTER RECEIVE MODE, ACKDT is the Value transmitted when the user initiates an Acknowledge sequence at the end of a receive.
           
            "ACKDT = 0 -> For Acknowledge" and "ACKDT = 1 -> For Not Acknowledge"
    */
    return incoming;
}

#endif  // I2C_H