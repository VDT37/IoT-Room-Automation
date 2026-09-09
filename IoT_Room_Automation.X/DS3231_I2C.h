#ifndef DS3231_H
#define	DS3231_H

/************************************ Functions for RTC module *******************************************************/

/*  All the contents of Time & Calender including Alarms are stored in "Binary Coded Decimal"(BCD) format,
 *  when 'Reading' we need to convert it to 'Decimal' and back to 'BCD' when 'Writing'.  
 */  
unsigned char BCD_2_DEC(unsigned char BCD) // For READING the data from SSPBUF 
{
   return (((BCD >> 4) * 10) + (BCD & 0b1111)); 
}

unsigned char DEC_2_BCD (unsigned char DEC) // For WRITING the data to SSPBUF
{
   return (((DEC / 10) << 4) + (DEC % 10));
}

/********************************************** Function to Transmit Data *********************************************************/
/*
 * Transmission of a data byte is accomplished by simply writing a value to the SSPBUF register.
 * Procedure is: (Leaving the Interrupt cases):
   1) The user generates a Start condition by setting the Start Enable (SEN) bit (SSPCON2 register).
   2) The user loads the SSPBUF with the address to transmit.
   3) Address is shifted out the SDA pin until all eight bits(including R/W bit(Logic '0')) are transmitted.
   4) The MSSP module shifts in the ACK bit from the slave device and writes its value into the ACKSTAT bit (SSPCON2 register).
   5) The user loads the SSPBUF with eight bits of DATA.
   6) Data is shifted out the SDA pin until all eight bits are transmitted.
   7) The MSSP module shifts in the ACK bit from the slave device and writes its value into the ACKSTAT bit (SSPCON2 register).
   8) The user generates a Stop condition by setting the Stop Enable bit PEN (SSPCON2 register).
 */

void i2c_transmit()         
{   
    i2c_start();               // Start condition
    i2c_busIdle();
    
    I2C_Write(0b11010000);     // Slave Address of DS3231 - 0x68 along with R/W = 0
    
    I2C_Write(0b00000000);     // Register address from where you want to start writing (Pointer will start from First location of Memory addresses sequence)
    
    I2C_Write(sec);        // SECONDS REGISTER (00h Address Location)

    I2C_Write(DEC_2_BCD(min));        // MINUTES REGISTER (01h Address Location)

    I2C_Write(hour);    // HOURS REGISTER (02h Address Location), Or-ing for enabling 12hr mode(Bit-6) and AM mode(Bit->0)

    I2C_Write(DEC_2_BCD(day));        // DAY REGISTER (03h Address Location)

    I2C_Write(DEC_2_BCD(date));       // DATE REGISTER (04h Address Location)

    I2C_Write(DEC_2_BCD(month));      // MONTH/CENTURY REGISTER (05h Address Location)
    
    I2C_Write(DEC_2_BCD(year));       // YEAR REGISTER (01h Address Location)

    i2c_stop();
}

/********************************************** Function to Receive Data *********************************************************/
/* Steps that need to followed are:
    1) Initialize Start Condition
    2) Load the SSPBUF with Slave Address with "R/W bit = 0" (i.e, Writing Operation)
    3) The MSSP module shifts in the ACK bit from the slave device and writes its value into the ACKSTAT bit (SSPCON2 register).
    4) Write Memory Address where we want to read. Now MASTER keeps track of the Memory address.
    5) Step 3
    6) Generate Repeated Start Condition
    7) Load the SSPBUF with Slave Address with "R/W bit = 1" (i.e, Reading Operation) 
    8) Step 3.
    9) MASTER is Configured as RECEIVER by enabling SSPCON2<3> (RCEN = 1)
    10) The Baud Rate Generator begins counting, and on each rollover, the state of the SCL pin changes (high-to-low/low-to-high) and data is shifted into the SSPSR. 
    11) After the falling edge of the eighth clock, the RCEN bit is automatically cleared, the contents of the SSPSR are loaded into the SSPBUF.
    12) Read the Value that is loaded in SSPBUF Register. (Convert it into Decimal)
    13) Send an Acknowledge bit ACK or NACK at the end of the reception as applicable.
    14) Generate Stop Condition.
 */  
void i2c_receive(void)
{
    i2c_start();                // Start condition  
    i2c_busIdle();              // Checks if Bus is IDLE or not
    
    I2C_Write(0b11010000);        // Slave Address of DS3231 - 0x68 along with R/W = 0

    I2C_Write(0x00);              // Write memory address from where you want to start reading 

    i2c_repeatedStart();        // Repeated start condition
    i2c_busIdle();              // Checks if Bus is IDLE or not
    
    I2C_Write(0b11010001);        // Slave Address + Read (R/W = 1)

    sec = BCD_2_DEC(I2C_Read(0));        // Receive Seconds
 
    min = BCD_2_DEC(I2C_Read(0));     // Receive Minutes
          
    hour = I2C_Read(0);              // Receive Hours
     
    day = BCD_2_DEC(I2C_Read(0));         // Receive Days
         
    date = BCD_2_DEC(I2C_Read(0));   // Receive Date
    
    month = BCD_2_DEC(I2C_Read(0));  // Receive Month
 
    year = BCD_2_DEC(I2C_Read(1));   // Receive Year

    i2c_repeatedStart();        // Repeated start condition
    i2c_busIdle();              // Checks if Bus is IDLE or not
    
    I2C_Write(0b11010000);        // Slave Address of DS3231 - 0x68 along with R/W = 0

    I2C_Write(0x0E);              // Writing memory address of Control Register

    i2c_repeatedStart();        // Repeated start condition
    i2c_busIdle();              // Checks if Bus is IDLE or not
 
    I2C_Write(0b11010001);        // Slave Address + Read (R/W = 1)
    
    I2C_Read(0);      // Reading Control Register
    
    I2C_Read(0);       // Reading Status Register
    
    I2C_Read(0);                    // Skipping Aging OFFSET Register
    
    Temperature = I2C_Read(1);      // Reading Temperature MSB Values and initializing Acknowledgement sequence
    
    i2c_stop();
}

#endif  // DS3231_H