#ifndef CUSTOM_CHARACTERS_H
#define	CUSTOM_CHARACTERS_H

unsigned char Temp_Symbol[7] = {
    0b00100,
    0b01010,
    0b01010,
    0b01110,
    0b11111,
    0b01110,
    0b00100
};

unsigned char Celcius_Symbol[7] = {
    0b01000,
    0b10100,
    0b01011,
    0b00100,
    0b00100,
    0b00100,
    0b00011
};

unsigned char WiFi_Symbol[7] = {
    0b00000,
    0b10001,
    0b10001,
    0b10101,
    0b10101,
    0b01010,
    0b00000
};

unsigned char Right_Symbol[7] = {
    0b10000,
    0b11000,
    0b11100,
    0b11110,
    0b11100,
    0b11000,
    0b10000
};

unsigned char Left_Symbol[7] = {
    0b00001,
    0b00011,
    0b00111,
    0b01111,
    0b00111,
    0b00011,
    0b00001
};

unsigned char Bluetooth_Symbol[7] = {
    0b00100,
    0b10110,
    0b01101,
    0b00110,
    0b01101,
    0b10110,
    0b00100
};

/***************************************************************** 
    Function Name: custom_char
    Input:  
       loc: Location where we want to store in CGRAM (0,1,2,..7)
       p: Pointer to pattern data
     Usage:
     custom_char(1,pattern);
     ******************************************************************/

void custom_char(unsigned char loc, unsigned char *p)
{
    unsigned char i;
    
    if(loc<8)                           // If valid address
    {
        send_config(0x40+(loc*8));      // Write to CGRAM [CGRAM Address(0b01000000)]
        for(i=0;i<7;i++)
            send_char(p[i]);            // Write the character pattern to CGRAM
    }
    send_config(0x80);                  // Shift back to DDRAM location 0
    lcd_busy();
}

#endif // CUSTOM_CHARACTERS_H