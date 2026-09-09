#define EN RD3                  // Enable Pin
#define RS RD2                  // Register Select Pin
#define LCD_PORT PORTD          // LCD PORT

#define PIR_br RB0
#define door_outer_IR RB1
#define door_inner_IR RB2
#define bathroom_outer_IR RB3
#define bathroom_inner_IR RB4
#define led RA0
#define room_light RA1
#define bathroom_led RA2
#define WiFi_Switch RA3
#define BT_Switch RA41
#define bathroom_light RC2

// LCD Functions Declarations
extern void lcd_busy(void); // Function for LCD Delay
extern void send_config(unsigned char instruction); // Function for Command/Instruction Passing 
extern void send_char(unsigned char character); // Function for Character/Data Passing
extern void send_string(const char *s); // Function for Displaying the Characters in LCD
extern void lcd_clr(void); // Function for Clearing the LCD Screen
extern void lcd_init(void); // Function for LCD Initialization
extern void Lcd_Set_Cursor(unsigned char row, unsigned char col); // Function for Setting the Cursor in LCD

// I2C Functions Declarations
extern void i2c_init(void); // Function to initialize I2C
extern void i2c_busIdle(void); // Function to check if the bus is idle
extern void i2c_start(void); // Start condition function
extern void i2c_stop(void); // Stop condition function
extern void i2c_repeatedStart(void); // Function to initiate repeated start
extern void receive_enable(void); // Function to enable Reception of data
extern void I2C_Write(unsigned char data); // Function to Write Data to SSPBUF Buffer register
extern unsigned char I2C_Read(unsigned char ack); // Function to Read the Data from SSPBUF


// DS3231 RTC Functions Declarations
extern unsigned char BCD_2_DEC(unsigned char BCD); // Function for READING the data from SSBUF
extern unsigned char DEC_2_BCD(unsigned char DEC); // For WRITING the data to SSPBUF
extern void i2c_transmit(void); // Function to transmit data
extern void i2c_receive(void); // Function to receive data

// Custom_Characters Functions Declarations
extern void custom_char(unsigned char loc, unsigned char *p); // Function for storing the Custom Characters in CGRAM Address

// Main Function Declarations
void maindoor(void); // Function for detecting main door IRs
void bathroom_door(void); // Function for detecting bathroom door IRs
void Cooling_System(void); // Function for Cooling System
void Room_Light(void);  // Function for Room Light
void WiFi_BT_Switch(void); // Function for WiFi and Bluetooth Mode Switching
void display_clock(void); // Function to display data 
void display(unsigned char data); // Convert and display received data from HEX to ASCII
void check_weekday(unsigned char day); // Function to check Weekday
void Change_States(void); // Function to check the FSM States
void display_set_time(void); // Function to display Time setting Interface
void display_set_date(void); // Function to display Date setting Interface
void display_AM_PM(void); // Function to display AM or PM in Time setting Interface
void display_people_count(void); // Function to display Number of people in the Main room
void Mode_Switch(unsigned char x); // Mode Switch (RB7) Press and Release Event Function  
void Next_Switch(unsigned char x); // Next Switch (RB6) Press and Release Event Function  
void Select_Switch(unsigned char x); // Select Switch (RB5) Press and Release Event Function  
void Mode_Switch_States(void); // Function to display States according to the user's input from Mode Switch
void Next_Switch_States(void); // Function to display States according to the user's input from Next Switch
void Select_Switch_States(void); // Function to display States according to the user's input from Select Switch
void Main_Interface(void); // Function to Display Main Interface of LCD
void Menu_Interface(void); // Function to Display Menu Interface of LCD

unsigned char count = 0, unit = 0, tens = 0;
static __bit person_entered_room_flag, person_exited_room_flag, person_in_br;
static __bit used_outer = 0, used_inner = 0, used_br_outer = 0, used_br_inner = 0;
static __bit Room_Light_ON = 0, Room_Light_OFF = 0, Cooling_System_ON = 0, Cooling_System_OFF = 0;
static __bit OWiFi_BT_Light_ON, OWiFi_BT_Light_OFF, IWiFi_BT_Light_ON, IWiFi_BT_Light_OFF;
static __bit OWiFi_BT_CS_ON, OWiFi_BT_CS_OFF, IWiFi_BT_CS_ON, IWiFi_BT_CS_OFF;
unsigned int count_outer, count_inner, count_br_outer, count_br_inner;
char alfa;
unsigned char sec = 0, min, hour, hr, date, month, year, day, Temperature;
unsigned char Flag, Long_press_on_state, Transmit_on_state;
static __bit Switch_RB7, Switch_RB6, Switch_RB5, Switch_BT, Switch_WiFi, room_light_Flag, Long_press, TMR_Start, Blink_State, PM_mode, WiFi_ON, BT_ON;
unsigned int Pressed_for, Increment_Interval, delay, conv_temp; 

enum STATES { // FSM States
    Main, // State 0
    Menu_Time, // State 1
    Menu_Date, // State 2
    
    Set_Hour, // State 3
    Set_Min, // State 4
    Set_AM_PM, // State 5

    Set_Date, // State 6
    Set_Month, // State 7
    Set_Year, // State 8
    Set_Day, // State 9
    
    People_Count // State 10
} state;

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <pic16f887.h>
#include "Config_Hardware.h"
#include "Usart_Config.h"
#include "I2C.h"
#include "DS3231_I2C.h"
#include "LCD.h"
#include "Custom_Characters.h"

void __interrupt() my_isr_routine(void) {
    if (TMR0IF == 1) {
        TMR0IF = 0;
        if (used_outer == 1) {
            count_outer++;

            if (count_outer == 200) // Generating 10 seconds
            {
                count_outer = 0;
                used_outer = 0;
                /* 
                    if(count == 0) knock_wait = 0; else knock_wait = 1;
                /* If person inside the room doesn't open door within 3 seconds then, 'outer IR' previously detected will not be considered
                   Only if people are inside, when outer IR detected because of knock, then Waiting operation completion Flag is set  */
            }
        }

        if (used_inner == 1) // Case - When Inner IR detects Falsely, Basically setting time limit for Inner IR flag 
        {
            count_inner++;

            if (count_inner == 200) // Generating 1 seconds
            {
                count_inner = 0;
                used_inner = 0;
            }
        }
        //////////////////////// Bathroom IR Count ////////////////////////////
//        if (used_br_outer == 1) {
//            count_br_outer++;
//
//            if (count_br_outer == 200) // Generating 1 seconds
//            {
//                count_br_outer = 0;
//                used_br_outer = 0;
//            }
//        }

        if (used_br_inner == 1) // Case - When Inner IR detects Falsely, Basically setting time limit for Inner IR flag 
        {
            count_br_inner++;

            if (count_br_inner == 200) // Generating 1 seconds
            {
                count_br_inner = 0;
                used_br_inner = 0;
            }
        }

        ////////////////////////////////////////////////////////////////////////

        if (TMR_Start == 1) {
            /* When a switch is pressed, if is being pressed for more than 750ms, then it is considered as Long Press
             */
            Pressed_for++;
            if (Pressed_for == 150) // Generating 750ms
            {
                Pressed_for = 0;
                Long_press = 1;
                TMR_Start = 0;
            }
        }

        if (Long_press_on_state == 4) // If user Long pressed on "State 4", i.e., for incrementing Hour Values  
        {
            Increment_Interval++;
            if (Increment_Interval == 50) // Generating interval of 250ms between each Increment
            {
                Increment_Interval = 0;
                hr++;
                if (hr > 12) hr = 1;
                Transmit_on_state = 4; // Flag for I2C Transmit operation on State 4          
            }
        }

        if (Long_press_on_state == 5) // If user Long pressed on "State 5", i.e., for incrementing Minute Values  
        {
            Increment_Interval++;
            if (Increment_Interval == 20) // Generating interval of 100ms between each Increment
            {
                Increment_Interval = 0;
                min++;
                if (min > 59) min = 0;
                Transmit_on_state = 5; // Flag for I2C Transmit operation on State 5   
            }
        }

        if (Long_press_on_state == 7) // If user Long pressed on "State 7", i.e., for incrementing Date Values  
        {
            Increment_Interval++;
            if (Increment_Interval == 40) // Generating interval of 200ms between each Increment
            {
                Increment_Interval = 0;
                date++;
                if (date > 31) date = 1;
                Transmit_on_state = 7; // Flag for I2C Transmit operation on State 7  
            }
        }

        if (Long_press_on_state == 8) // If user Long pressed on "State 8", i.e., for incrementing Month Values  
        {
            Increment_Interval++;
            if (Increment_Interval == 50) // Generating interval of 250ms between each Increment
            {
                Increment_Interval = 0;
                month++;
                if (month > 12) month = 1;
                Transmit_on_state = 8; // Flag for I2C Transmit operation on State 8
            }
        }

        if (Long_press_on_state == 9) // If user Long pressed on "State 9", i.e., for incrementing Year Values  
        {
            Increment_Interval++;
            if (Increment_Interval == 20) // Generating interval of 100ms between each Increment
            {
                Increment_Interval = 0;
                year++;
                if (year > 99) year = 0;
                Transmit_on_state = 9; // Flag for I2C Transmit operation on State 9
            }
        }

        if (Long_press_on_state == 10) // If user Long pressed on "State 10", i.e., for incrementing Day Values  
        {
            Increment_Interval++;
            if (Increment_Interval == 80) // Generating interval of 400ms between each Increment
            {
                Increment_Interval = 0;
                day++;
                if (day > 7) day = 1;
                Transmit_on_state = 10; // Flag for I2C Transmit operation on State 10
            }
        }
    }

    if (RBIF == 1) // PORTB Change Interrupt Flag bit
    {
        RBIF = 0;

        /////////////////////////// Main Door IR ///////////////////////////////////////////

        if (door_outer_IR == 0 && used_outer == 0) // If Outer IR of Main Door detects then
        {
            count_inner = 0;

            used_outer = 1; // Outer IR detected

            if (used_inner == 1) // If Inner IR already detected when Outer IR detects
            {
                person_exited_room_flag = 1; // Person exiting room flag is set
            }
        }

        if (door_inner_IR == 0 && used_inner == 0) {
            count_outer = 0;

            if (count == 0) // If people are not present in room them
            {
                used_inner = 0; // Even if Inner IR detects it won't be counted (Count wont be decremented to 255)
            } else
                used_inner = 1; // Inner IR detects

            if (used_outer == 1) // If Outer IR already detected
            {
                person_entered_room_flag = 1; // Person entering room flag is set
            }
        }

        /////////////////////////// Bathroom Door IR (Two IRs) ///////////////////////////////////////////

        if (bathroom_outer_IR == 0 && used_br_outer == 0) // If Outer IR of Main Door detects then
        {
            count_br_inner = 0;

            if (used_br_inner == 1 && person_in_br == 1) // If Inner IR already detected when Outer IR detects
            {
                Flag = 100;
            }
        }

        if (bathroom_inner_IR == 0 && used_br_inner == 0) {
            used_br_inner = 1; // Inner IR detects
        }

        if (count > 0) {
            if (PIR_br == 1) {
                person_in_br = 1;
            }
        } else
            person_in_br = 0;

        /////////////////////// Interrupt for Switches //////../////////////////

        if (state == 0) {
            Mode_Switch(1); // When in Home, if Mode Switch is pressed then making an Interrupt by using Flag = 1 
            Select_Switch(20); // If Select Switch is pressed on Main Interface, then making an Interrupt by using Flag = 20 
        }

        if (state == 1) {
            Mode_Switch(2); // When in Menu, if Mode Switch is pressed then making an Interrupt by using Flag = 2 
            Next_Switch(6); // If Next Switch is pressed on Menu Interface, then making an Interrupt by using Flag = 6
            Select_Switch(10); // If Select Switch is pressed for selecting Set Time in Menu Interface, then making an Interrupt by using Flag = 10
        }

        if (state == 2) {
            Mode_Switch(2); // When in Menu, if Mode Switch is pressed then making an Interrupt by using Flag = 2 
            Next_Switch(6); // If Next Switch is pressed on Menu Interface, then making an Interrupt by using Flag = 6
            Select_Switch(11); // If Select Switch is pressed for selecting Set Date in Menu Interface, then making an Interrupt by using Flag = 11        }
        }
        
        if (state == 3) {
            Mode_Switch(3); // After setting Time, if Mode Switch is pressed then making an Interrupt by using Flag = 3 
            Next_Switch(7); // If Next Switch is pressed for Changing Hour values, then making an Interrupt by using Flag = 7
            Select_Switch(13); // If Select Switch is pressed on Time Values in Time Setup Interface, then making an Interrupt by using Flag = 13
        }

        if (state == 4) {
            Mode_Switch(3); // After setting Time, if Mode Switch is pressed then making an Interrupt by using Flag = 3 
            Next_Switch(8); // If Next Switch is pressed for Changing Min values, then making an Interrupt by using Flag = 8
            Select_Switch(13); // If Select Switch is pressed on Time Values in Time Setup Interface, then making an Interrupt by using Flag = 13
        }

        if (state == 5) {

            Mode_Switch(3); // After setting Time, if Mode Switch is pressed then making an Interrupt by using Flag = 3 

            if (RB6 == 0) // If Next Switch is pressed for Changing AM/PM
            {
                for (delay = 0; delay < 30000; delay++) {
                } // 30ms Delay, Checking Debounce state
                if (RB6 == 0) {
                    Switch_RB6 = 1;
                }
            }

            if (RB6 == 1 && Switch_RB6 == 1) // "PRESS & RELEASE EVENT" by using Flag as Confirmation of 'PRESS EVENT'
            {
                Switch_RB6 = 0;
                Flag = 9; // Making an Interrupt by using Flag = 9
            }

            Select_Switch(13); // If Select Switch is pressed on Time Values in Time Setup Interface, then making an Interrupt by using Flag = 13
        }

        if (state == 6) {
            Mode_Switch(4); // After setting Date/Day, if Mode Switch is pressed then making an Interrupt by using Flag = 4
            Next_Switch(16); // If Next Switch is pressed for Changing Date values, then making an Interrupt by using Flag = 16
            Select_Switch(14); // If Select Switch is pressed on Date Values in Date Setup Interface, then making an Interrupt by using Flag = 14
        }

        if (state == 7) {
            Mode_Switch(4); // After setting Date/Day, if Mode Switch is pressed then making an Interrupt by using Flag = 4
            Next_Switch(17); // If Next Switch is pressed for Changing Month values, then making an Interrupt by using Flag = 17
            Select_Switch(14); // If Select Switch is pressed on Date Values in Date Setup Interface, then making an Interrupt by using Flag = 14
        }

        if (state == 8) {
            Mode_Switch(4); // After setting Date/Day, if Mode Switch is pressed then making an Interrupt by using Flag = 4
            Next_Switch(18); // If Next Switch is pressed for Changing Year values, then making an Interrupt by using Flag = 18
            Select_Switch(14); // If Select Switch is pressed on Date Values in Date Setup Interface, then making an Interrupt by using Flag = 14
        }

        if (state == 9) {
            Mode_Switch(4); // After setting Date/Day, if Mode Switch is pressed then making an Interrupt by using Flag = 4
            Next_Switch(19); // If Next Switch is pressed for Changing Day values, then making an Interrupt by using Flag = 19
            Select_Switch(14); // If Select Switch is pressed on Date Values in Date Setup Interface, then making an Interrupt by using Flag = 14
        }
        
        if(state == 10) {
            Select_Switch(21); // If Select Switch is pressed on State 10, then making an Interrupt by using Flag = 21
        }
    }

    if (RCIF == 1) {
        RCIF = 0;
        alfa = (char) RCREG;
        
        if(WiFi_ON == 1)
        {
            if (alfa == 'A') {
                Room_Light_ON = 1;
            } else if (alfa == 'B') {
                Room_Light_OFF = 1;
            } else if (alfa == 'C') {
                Cooling_System_ON = 1;
            } else if (alfa == 'D') {
                Cooling_System_OFF = 1;
            }
            if (alfa == 'a' || alfa == 'b' || alfa == 'c' || alfa == 'd'){} 
        }
        else if(BT_ON == 1)
        {
            if (alfa == 'a') {
                Room_Light_ON = 1;
            } else if (alfa == 'b') {
                Room_Light_OFF = 1;
            } else if (alfa == 'c') {
                Cooling_System_ON = 1;
            } else if (alfa == 'd') {
                Cooling_System_OFF = 1;
            }
            if (alfa == 'A' || alfa == 'B' || alfa == 'C' || alfa == 'D'){} 
        }
        else if(BT_ON == 0 && WiFi_ON == 0)
        {
            if (alfa == 'A' || alfa == 'B' || alfa == 'C' || alfa == 'D' ||
                alfa == 'a' || alfa == 'b' || alfa == 'c' || alfa == 'd'){
                Room_Light_ON = 0;
                Room_Light_OFF = 0;
                Cooling_System_ON = 0;
                Cooling_System_OFF = 0;
            } 
        }
            
    }
}

int main() {
    //OSCCON = (OSCCON | 0b01110000); // Oscillator Control Register, Initializing internal oscillator to 8MHz
    TRISA = 0b00011000; // RA<3:4> Pins are set for taking input for WiFi and Bluetooth Switches
    TRISB = 0b11111111; // RB<1:3> Pins are Set for taking Input Values of IR Sensor
    TRISC = 0b00011000; // The SCL and SDA pins must be configured as inputs (TRISC<4:3> set). The MSSP module(SSPEN Bit) will override the input state with the output data when required.
    TRISD = 0b00000000;
    TRISE = 0b00000000;

    PORTA = 0b00010000;
    PORTB = 0b11100000;
    PORTC = 0b00000000;
    PORTD = 0b00000000;
    PORTE = 0b00000000;

    C1ON = 0;
    C2ON = 0;
    ANSEL = 0x00;
    ANSELH = 0x00;

    USART_Init(9600); // Initializing USART Communication
    __delay_ms(50);
    
    EEADR = 0; // Read Internal EEPROM Location 0
    RD = 1; // Whatever data is in EEADR will come to EEDATA Register
    unit = EEDATA; // Taking the stored data in EEDATA Register 

    EEADR = 1; // Read Internal EEPROM Location 1
    RD = 1;
    tens = EEDATA;

    //***********************************************************************  
    /* EEDATA has the default value of '255' if we are first programming this into micro controller.
       So if we don't write this part of the code then our program will start counting from '255' onwards
     */
    if (unit >= 10) {
        unit = 0;
    }
    if (tens >= 10) {
        tens = 0;
    }

    count = (tens * 10) + unit; // Formation of count 

    //***********************************************************************

    /* Data stored in EEPROM should be given to Interrupt Variables before initializing Interrupt Control Register */

    TMR0 = 61; // 5ms Time Interval
    OPTION_REG = 0b10000110; // Bit 7: Pull up resistance, Pre scalar value <1:128>
    TMR0IF = 0;

    PR2 = 0xF9; // PR2 = 249, Period - 200us for PWM 
    T2CON = 0b00000101; // 1:1 as Post-Scalar Bit, TMR2 ON(Bit-2) and Pre-Scalar 4(Bit-0)

    INTCON = 0b11101000; // Global Interrupt Enabled(GIE), Peripheral Interrupt Enabled(PEIE), External Interrupt Enabled(INTE), PORTB Change Interrupt Enabled(RBIE)

    PIE1bits.RCIE = 1; // Enable Receive Interrupt 
    PIR1bits.RCIF = 0;

    IOCB = 0b11111111; // Interrupt-on-change enabled for RB0, RB1 & RB2 pins

    count_outer = 0;
    count_inner = 0;
    count_br_outer = 0;
    count_br_inner = 0;

    lcd_init(); // Function to initialize LCD
    custom_char(0, Temp_Symbol); // Building Character at Position '0'
    custom_char(1, WiFi_Symbol); // Building Character at Position '1'
    custom_char(2, Left_Symbol); // Building Character at Position '2'
    custom_char(3, Right_Symbol); // Building Character at Position '3'
    custom_char(4, Celcius_Symbol); // Building Character at Position '4'
    custom_char(5, Bluetooth_Symbol); // Building Character at Position '4'
    lcd_busy();

    Blink_State = 1;

    i2c_init(); // I2C initialization
    lcd_clr();

    while (1) 
    {
        WiFi_BT_Switch();
        maindoor(); // Executing function for detecting main door IRs 
        bathroom_door(); // Executing function for detecting bathroom door IRs
        Change_States();
        Mode_Switch_States();
        Next_Switch_States();
        Select_Switch_States();
       }
    return 0;
}

void maindoor() // Function for detecting main door IRs
{
    if (person_entered_room_flag == 1) {
        used_outer = 0; // Clearing all the Flags
        used_inner = 0;
        person_entered_room_flag = 0;
        count++; // Increase the Counter
    }

    if (person_exited_room_flag == 1) {
        used_inner = 0; // Clearing all the Flags
        used_outer = 0;
        person_exited_room_flag = 0;
        count--; // Decrease the Counter
    }

    unit = count % 10; // For Unit's Place
    tens = (count / 10) % 10; // For Ten's Place

    EEADR = 0; // EEADR 0th Address Location 
    EEDATA = unit; // Storing the value 
    WREN = 1; // Write Enable, We'll enable it only when needed
    EECON2 = 0x55; // 0b01010101
    EECON2 = 0xAA; // 0b10101010
    WR = 1; // Writing Operation starts
    while (WR == 1) {
    } // Till WR->0, Operation will continue
    WREN = 0; // Completion of WRITE Cycle

    EEADR = 1; // EEADR 1st Address Location 
    EEDATA = tens; // Storing the value 
    WREN = 1; // Write Enable, We'll enable it only when needed
    EECON2 = 0x55; // 0b01010101
    EECON2 = 0xAA; // 0b10101010
    WR = 1; // Writing Operation starts
    while (WR == 1) {
    } // Till WR->0, Operation will continue
    WREN = 0; // Completion of WRITE Cycle

    if (count > 0) 
    {
        led = 1; // Indicating People are present in room

        /**************************************************************************/
        if(WiFi_ON == 1 || BT_ON == 1)
        {
            if ((OWiFi_BT_Light_OFF == 1 || OWiFi_BT_Light_ON == 1) || (OWiFi_BT_Light_OFF == 0 && OWiFi_BT_Light_ON == 0)) {
                /* When a person turned ON/OFF the room light when room was empty using WiFi or BT, and if they enter the room then */

                OWiFi_BT_Light_OFF = 0;
                OWiFi_BT_Light_ON = 0;

                if (IWiFi_BT_Light_OFF == 0 && IWiFi_BT_Light_ON == 0) room_light_Flag = 1; // If WiFi or BT Communication is not used
                else if (IWiFi_BT_Light_OFF == 0 && IWiFi_BT_Light_ON == 1) room_light = 1; // If Turned ON the light using WiFi or BT
                else if (IWiFi_BT_Light_OFF == 1 && IWiFi_BT_Light_ON == 0) room_light = 0; // If Turned OFF the light using WiFi or BT 
            }

            if ((OWiFi_BT_CS_OFF == 1 || OWiFi_BT_CS_ON == 1) || (OWiFi_BT_CS_OFF == 0 && OWiFi_BT_CS_ON == 0)) {
                /* When a person turned ON/OFF the room light when room was empty using WiFi or BT, and if they enter the room then */

                OWiFi_BT_CS_OFF = 0;
                OWiFi_BT_CS_ON = 0;

                if (IWiFi_BT_CS_OFF == 0 && IWiFi_BT_CS_ON == 0) Cooling_System(); // If WiFi or BT Communication is not used
                else if (IWiFi_BT_CS_OFF == 0 && IWiFi_BT_CS_ON == 1) Cooling_System(); // If Turned ON the Cooling System using WiFi or BT
                else if (IWiFi_BT_CS_OFF == 1 && IWiFi_BT_CS_ON == 0) // If Turned OFF the Cooling System using WiFi or BT
                {
                    CCPR2L = 0b00000000; // Turning OFF the Cooling System
                    CCP2CON = 0b00001100; 
                }
            }
        }
        else if(WiFi_ON == 0 && BT_ON == 0)
        {
            IWiFi_BT_Light_ON = 0;
            IWiFi_BT_Light_OFF = 0;
            IWiFi_BT_CS_ON = 0;
            IWiFi_BT_CS_OFF = 0;
            
            Room_Light();
            Cooling_System();
        }
        
        if(room_light_Flag == 1)
        {
            room_light_Flag = 0;
            Room_Light();
            Cooling_System();
        }
        
        /******** If a person uses WiFi or BT Communication inside the room then ********/

        if (Room_Light_ON == 1) // If person uses WiFi or BT to Turn ON the Lights then 
        {
            Room_Light_ON = 0;
            IWiFi_BT_Light_OFF = 0;
            IWiFi_BT_Light_ON = 1;
        }
        if (Room_Light_OFF == 1) // If person uses WiFi or BT to Turn OFF the Lights then
        {
            Room_Light_OFF = 0;
            IWiFi_BT_Light_ON = 0;
            IWiFi_BT_Light_OFF = 1;
        }
        if (Cooling_System_ON == 1) // If person uses WiFi or BT to Turn ON the Lights then 
        {
            Cooling_System_ON = 0;
            IWiFi_BT_CS_OFF = 0;
            IWiFi_BT_CS_ON = 1;
        }
        if (Cooling_System_OFF == 1) // If person uses WiFi or BT to Turn OFF the Lights then
        {
            Cooling_System_OFF = 0;
            IWiFi_BT_CS_ON = 0;
            IWiFi_BT_CS_OFF = 1;
        }
    }
    else // If people are not present in room then
    {
        led = 0; // Indicating People are not present in room

        /****************************************************************************/
        if(WiFi_ON == 1 || BT_ON == 1)
        {
            if ((IWiFi_BT_Light_OFF == 1 || IWiFi_BT_Light_ON == 1) || (IWiFi_BT_Light_OFF == 0 && IWiFi_BT_Light_ON == 0)) {
                /* When a person turned ON/OFF the light inside the room using WiFi or BT, and if they ebxit the room then */

                IWiFi_BT_Light_OFF = 0;
                IWiFi_BT_Light_ON = 0;

                if (OWiFi_BT_Light_OFF == 0 && OWiFi_BT_Light_ON == 0) room_light = 0; // If WiFi or BT Communication is not used
                else if (OWiFi_BT_Light_OFF == 0 && OWiFi_BT_Light_ON == 1) room_light = 1; // If Turned ON the light using WiFi or BT
                else if (OWiFi_BT_Light_OFF == 1 && OWiFi_BT_Light_ON == 0) room_light = 0; // If Turned OFF the light using WiFi or BT 
            }
            
            if ((IWiFi_BT_CS_OFF == 1 || IWiFi_BT_CS_ON == 1) || (IWiFi_BT_CS_OFF == 0 && IWiFi_BT_CS_ON == 0)) {
                /* When a person turned ON/OFF the room light when room was empty using WiFi or BT, and if they enter the room then */

                IWiFi_BT_CS_OFF = 0;
                IWiFi_BT_CS_ON = 0;

                if (OWiFi_BT_CS_OFF == 0 && OWiFi_BT_CS_ON == 0) // If WiFi or BT Communication is not used
                {
                    CCPR2L = 0b00000000; // Turning OFF the Cooling System
                    CCP2CON = 0b00001100; 
                }
                else if (OWiFi_BT_CS_OFF == 0 && OWiFi_BT_CS_ON == 1) Cooling_System(); // If Turned ON the Cooling System using WiFi or BT
                else if (OWiFi_BT_CS_OFF == 1 && OWiFi_BT_CS_ON == 0) // If Turned OFF the Cooling System using WiFi or BT
                {
                    CCPR2L = 0b00000000; // Turning OFF the Cooling System
                    CCP2CON = 0b00001100; 
                }
            }
            /***************************************************************************/

            /******** If a person uses WiFi or BT Communication outside the room then ********/

            if (Room_Light_ON == 1) // If person uses WiFi or BT to Turn ON the Lights then 
            {
                Room_Light_ON = 0;
                OWiFi_BT_Light_OFF = 0;
                OWiFi_BT_Light_ON = 1;
            }
            if (Room_Light_OFF == 1) // If person uses WiFi or BT to Turn OFF the Lights then
            {
                Room_Light_OFF = 0;
                OWiFi_BT_Light_ON = 0;
                OWiFi_BT_Light_OFF = 1;
            }
            if (Cooling_System_ON == 1) // If person uses WiFi or BT to Turn ON the Lights then 
            {
                Cooling_System_ON = 0;
                OWiFi_BT_CS_OFF = 0;
                OWiFi_BT_CS_ON = 1;
            }
            if (Cooling_System_OFF == 1) // If person uses WiFi or BT to Turn OFF the Lights then
            {
                Cooling_System_OFF = 0;
                OWiFi_BT_CS_ON = 0;
                OWiFi_BT_CS_OFF = 1;
            }
        }
        else
        {
            OWiFi_BT_Light_ON = 0;
            OWiFi_BT_Light_OFF = 0;
            OWiFi_BT_CS_ON = 0;
            OWiFi_BT_CS_OFF = 0;
            
            room_light = 0;
            CCPR2L = 0b00000000; // Turning OFF the Cooling System
            CCP2CON = 0b00001100; 
        }
        /****************************************************************************/
    }
}

void bathroom_door() // Function for detecting bathroom door IRs
{
    if (Flag == 100) {
        __delay_ms(5000);
        //Flag = 0;

        if (PIR_br == 1) // If PIR is still detecting (If person is still inside) [NOTE: Bathroom Door should not be in the range of PIR Sensor]
        {
            person_in_br = 1;
        } else {
            person_in_br = 0;
        }
    }

    if (person_in_br == 1) // If Person is in Bathroom
    {
        bathroom_led = 1;
        
        if ((hr < 6 && (PM_mode == 0)) || (hr == 12 && (PM_mode == 0))) // From 12:00AM to 5:59AM, We are reducing Brightness by 60%
        {
            CCPR1L = 0b00011001; // Storing the MSB Bits of 400 in CCPR1L Register
            CCP1CON = 0b00001100; // Storing the LSB Bits in DC1B<1:0> bits of the CCP1CON<4:5> Register
        } else if (hr > 5 && (PM_mode == 1)) // From 6:00PM to 11:59PM, Full Brightness
        {
            CCPR1L = 0b11111010; // Storing the MSB Bits of 1000 in CCPR1L Register (Full Brightness)
            CCP1CON = 0b00001100; // Storing the LSB Bits in DC1B<1:0> bits of the CCP1CON<4:5> Register
        } else if (hr == 12 && (PM_mode == 1)) // From 12:00PM to 12:59PM
        {
            CCPR1L = 0b00000000; // Turn OFF           
            CCP1CON = 0b00001100;
        } else // Rest of the Time
        {
            CCPR1L = 0b00000000; // Turn OFF           
            CCP1CON = 0b00001100;
        }
        Flag = 0;
    }
    else 
    {
        bathroom_led = 0;
        CCPR1L = 0b00000000; // Turn OFF           
        CCP1CON = 0b00001100; // Storing the LSB Bits in DC1B<1:0> bits of the CCP1CON<4:5> Register
        Flag = 0;
    }
}

void Cooling_System(void)
{
    conv_temp =  60*Temperature - 1100; 
    /* Using Formula for System of Linear Equations and method of Common Differences, this equation is created.
       If Temperature is 35, then Light will be turned ON with full brightness (Prototype for Cooling System)
       and if Temperature is 20, then Light's Brightness will be reduced by 90%, and if the Temperature gradually decreases from 35 to 20
       then Brightness will reduce in a systematic manner based on the equation, which indicates that cooling decreases as Temperature decreases.  
     */
    CCPR2L = conv_temp; // Storing the MSB Bits of Converted Temperature in CCPR2L Register
    CCP2CON = 0b00001100; // Storing the LSB Bits in DC1B<1:0> bits of the CCP2CON<4:5> Register
}

void Room_Light(void)
{
    if((((hr > 4 && min > 29) || hr > 5) && (PM_mode == 1)) || ((hr < 6 && (PM_mode == 0)) || (hr == 12 && (PM_mode == 0)))) // From 5:30PM to 5:59AM
    {
        room_light = 1;
    }
    else if(hr == 12 && (PM_mode == 1)) // From 12:00PM to 12:59PM
    {
        room_light = 0;
    }
    else // Rest of the Time
    {
        room_light = 0;
    }
}

void WiFi_BT_Switch(void)
{
    ///////////////////////////// WiFi Switch //////////////////////////////////
    
    if ((WiFi_Switch == 0 && WiFi_ON == 0) || (WiFi_Switch == 0 && WiFi_ON == 1)) // If WiFi Switch is pressed
    {
        for (delay = 0; delay < 30000; delay++) {} // 30ms Delay
        if (WiFi_Switch == 0) {
            Switch_WiFi = 1; // We made a variable 'HIGH' to confirm that switch has been pressed for first time
        }
    }

    if (WiFi_Switch == 1 && Switch_WiFi == 1 && WiFi_ON == 0) // "PRESS & RELEASE EVENT" by using Flag as Confirmation of 'PRESS EVENT'
    {
        Switch_WiFi = 0;   // Removing the First Press Flag  
        WiFi_ON = 1;        // Turning ON the WiFi Mode   
        BT_ON = 0;          // Turning OFF the Bluetooth Mode when WiFi Mode is turned ON
    }
    
    if (WiFi_Switch == 1 && Switch_WiFi == 1 && WiFi_ON == 1) // "PRESS & RELEASE EVENT" by using Flag as Confirmation of 'PRESS EVENT'
    {
        Switch_WiFi = 0;   // Removing the First Press Flag  
        WiFi_ON = 0;        // Turning OFF the WiFi Mode 
    }
    
    //////////////////////////// Bluetooth Switch //////////////////////////////
    
    if ((BT_Switch == 0 && BT_ON == 0) || (BT_Switch == 0 && BT_ON == 1)) // If WiFi Switch is pressed
    {
        for (delay = 0; delay < 30000; delay++) {} // 30ms Delay
        if (BT_Switch == 0) {
            Switch_BT = 1; // We made a variable 'HIGH' to confirm that switch has been pressed for first time
        }
    }

    if (BT_Switch == 1 && Switch_BT == 1 && BT_ON == 0) // "PRESS & RELEASE EVENT" by using Flag as Confirmation of 'PRESS EVENT'
    {
        Switch_BT = 0;   // Removing the First Press Flag  
        BT_ON = 1;          // Turning ON the BT Mode 
        WiFi_ON = 0;        // Turning OFF the WiFi Mode when Bluetooth Mode is turned ON
    }
    
    if (BT_Switch == 1 && Switch_BT == 1 && BT_ON == 1) // "PRESS & RELEASE EVENT" by using Flag as Confirmation of 'PRESS EVENT'
    {
        Switch_BT = 0;   // Removing the First Press Flag  
        BT_ON = 0;          // Turning OFF the BT Mode 
    }
}

void Change_States(void) {
    switch (state) {
        case Main:
            Main_Interface();
            break;

        case Menu_Time:
            Menu_Interface();
            break;

        case Menu_Date:
            Menu_Interface();
            break;

        case Set_Hour:
            display_set_time();
            break;

        case Set_Min:
            display_set_time();
            break;

        case Set_AM_PM:
            display_set_time();
            break;

        case Set_Date:
            display_set_date();
            break;

        case Set_Month:
            display_set_date();
            break;

        case Set_Year:
            display_set_date();
            break;

        case Set_Day:
            display_set_date();
            break;
            
        case People_Count:
            display_people_count();
            break;
    }
}

void Mode_Switch(unsigned char x) {
    if (RB7 == 0) // RB7 is for Mode Switch
    {
        for (delay = 0; delay < 30000; delay++) {
        } // 30ms Delay
        if (RB7 == 0) {
            Switch_RB7 = 1; // We made a variable 'HIGH' to confirm that switch has been pressed for first time
        }
    }

    if (RB7 == 1 && Switch_RB7 == 1) // "PRESS & RELEASE EVENT" by using Flag as Confirmation of 'PRESS EVENT'
    {
        Switch_RB7 = 0; // Removing the First Press Flag  
        Flag = x;
    }
}

void Next_Switch(unsigned char x) {
    if (Long_press == 0) {
        if (RB6 == 0) {
            for (delay = 0; delay < 30000; delay++) {
            } // 30ms Delay
            if (RB6 == 0) {
                TMR_Start = 1; // When Next Button is pressed, we're checking whether if it is a Long press or Short press with the help of TMR0
                Switch_RB6 = 1;
            }
        }

        if (RB6 == 1 && Switch_RB6 == 1) // "PRESS & RELEASE EVENT" by using Flag as Confirmation of 'PRESS EVENT'
        {
            RB3 = 0;
            Pressed_for = 0;
            TMR_Start = 0;
            Switch_RB6 = 0; // Removing the First Press Flag  
            Flag = x;
        }
    } else {
        if (RB6 == 1 && Switch_RB6 == 0) {
            Blink_State = 1;
            Long_press_on_state = 0;
            Increment_Interval = 0;
            Long_press = 0;
        }
    }
}

void Select_Switch(unsigned char x) {
    if (RB5 == 0) // RB5 is for Select Switch
    {
        for (delay = 0; delay < 30000; delay++) {
        } // 30ms Delay
        if (RB5 == 0) {
            Switch_RB5 = 1; // We made a variable 'HIGH' to confirm that switch has been pressed for first time
        }
    }

    if (RB5 == 1 && Switch_RB5 == 1) // "PRESS & RELEASE EVENT" by using Flag as Confirmation of 'PRESS EVENT'
    {
        Switch_RB5 = 0; // Removing the First Press Flag  
        Flag = x;
    }
}

void Mode_Switch_States(void) {
    if (Flag == 1) {
        Flag = 0;
        lcd_clr();
        state = 1;
    }

    if (Flag == 2) {
        Flag = 0;
        lcd_clr();
        state = 0;
    }

    if (Flag == 3) {
        Flag = 0;
        lcd_clr();
        Lcd_Set_Cursor(1, 4);
        send_string("Time Set!!");
        __delay_ms(1000);
        lcd_clr();
        state = 1;
        lcd_busy();
    }

    if (Flag == 4) {
        Flag = 0;
        lcd_clr();
        Lcd_Set_Cursor(1, 2);
        send_string("Date/Day Set!!");
        __delay_ms(1000);
        lcd_clr();
        state = 2;
        lcd_busy();
    }
}

void Next_Switch_States(void) {
    ////////////////////// Date and Time Next States ///////////////////////////
    ////////////////////////  First: For Menu Case /////////////////////////////

    if (state == 1 || state == 2) {
        if (Flag == 6) {
            Flag = 0;
            state++;
            if (state == Set_Hour) state = 1; // Changing the states between states 1,2,3  
        }
    }

    /////////////////////// Second: For Changing Hours /////////////////////////

    if (state == 3) {
        if (Long_press == 1) {
            Blink_State = 0;
            Switch_RB6 = 0;
            Long_press_on_state = 4;
        }

        if (Transmit_on_state == 4) {
            hour = hr;
            if (PM_mode == 0) hour = DEC_2_BCD(hr) | 0b01000000;
            else hour = DEC_2_BCD(hr) | 0b01100000;
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
            Transmit_on_state = 0;
        }

        if (Flag == 7) {
            Flag = 0;
            hr++;
            if (hr > 12) hr = 1;
            hour = hr;
            if (PM_mode == 0) hour = DEC_2_BCD(hr) | 0b01000000;
            else hour = DEC_2_BCD(hr) | 0b01100000;
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
        }
    }

    //////////////////////// Third: For Changing Minutes ///////////////////////
    if (state == 4) {
        if (Long_press == 1) {
            Blink_State = 0;
            Switch_RB6 = 0;
            Long_press_on_state = 5;
        }

        if (Transmit_on_state == 5) {
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
            Transmit_on_state = 0;
        }

        if (Flag == 8) {
            Flag = 0;
            min++;
            if (min > 59) min = 0;
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
        }
    }

    //////////////////// Fourth: For Changing AM & PM Modes ////////////////////

    if (Flag == 9) {
        Flag = 0;
        PM_mode = !PM_mode;

        if (PM_mode == 0) {
            hour = DEC_2_BCD(hr) | 0b01000000;
        } else if (PM_mode == 1) {
            hour = DEC_2_BCD(hr) | 0b01100000;
        }
        i2c_transmit();
    }

    //////////////////// Fifth: For Changing Dates /////////////////////////////

    if (state == 6) {
        if (Long_press == 1) {
            Blink_State = 0;
            Switch_RB6 = 0;
            Long_press_on_state = 7;
        }

        if (Transmit_on_state == 7) {
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
            Transmit_on_state = 0;
        }

        if (Flag == 16) {
            Flag = 0;
            date++;
            if (date > 31) date = 1;
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
        }
    }

    //////////////////// Sixth: For Changing Months ////////////////////////////

    if (state == 7) {
        if (Long_press == 1) {
            Blink_State = 0;
            Switch_RB6 = 0;
            Long_press_on_state = 8;
        }

        if (Transmit_on_state == 8) {
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
            Transmit_on_state = 0;
        }

        if (Flag == 17) {
            Flag = 0;
            month++;
            if (month > 12) month = 1;
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
        }
    }

    //////////////////// Seventh: For Changing Years /////////////////////////////

    if (state == 8) {
        if (Long_press == 1) {
            Blink_State = 0;
            Switch_RB6 = 0;
            Long_press_on_state = 9;
        }

        if (Transmit_on_state == 9) {
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
            Transmit_on_state = 0;
        }

        if (Flag == 18) {
            Flag = 0;
            year++;
            if (year > 99) year = 0;
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
        }
    }

    //////////////////// Eighth: For Changing Days /////////////////////////////

    if (state == 9) {
        if (Long_press == 1) {
            Blink_State = 0;
            Switch_RB6 = 0;
            Long_press_on_state = 10;
        }

        if (Transmit_on_state == 10) {
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
            Transmit_on_state = 0;
        }

        if (Flag == 19) {
            Flag = 0;
            day++;
            if (day > 7) day = 1;
            i2c_transmit(); // Transmit data i.e sec, min, hours etc
        }
    }
}

void Select_Switch_States(void) {
    if (Flag == 10) // If Select Switch is pressed for selecting Set Time in Menu Interface, Jumping to State 3 to change hour values 
    {
        Flag = 0;
        lcd_clr();
        state = 3;
    }

    if (Flag == 11) // If Select Switch is pressed for selecting Set Date in Menu Interface, Jumping to State 6 to change Date values 
    {
        Flag = 0;
        lcd_clr();
        state = 6;
    }

    if (Flag == 13) {
        Flag = 0;
        state++;
        if (state == Set_Date) state = 3; // Changing states
    }

    if (Flag == 14) {
        Flag = 0;
        state++;
        if (state == 0) state = 6; // Changing states
    }
    
    if (Flag == 20) // If Select Switch is pressed on Main Interface, Jumping to State 10 to display number people in the room
    {
        Flag = 0;
        lcd_clr();
        state = 10;
        i2c_receive();
    }
    
    if (Flag == 21) // If Select Switch is pressed on State 10, Jumping to State 0 (Main Interface)
    {
        Flag = 0;
        lcd_clr();
        state = 0;
        i2c_receive();
    }
}

void Main_Interface(void) {
    Lcd_Set_Cursor(1, 1); // First Row 1st Position in LCD Interface 
    i2c_receive(); // Receive data from RTC

    display_clock(); // Display date and time

    Lcd_Set_Cursor(1, 10); // First Row 10th Position in lCD Interface

    if (((unsigned) hour & 0b01100000) == 0b01100000) PM_mode = 1;
    else if (((unsigned) hour & 0b01000000) == 0b01000000) PM_mode = 0;

    display_AM_PM();

    Lcd_Set_Cursor(1, 14); // First Row 14th Position in lCD Interface
    check_weekday(day); // Display Weekdays

    Lcd_Set_Cursor(2, 10); // Second Row 10th Position in lCD Interface
    if(BT_ON == 1) send_char(0x05); // Display "Bluetooth Icon" which is stored at 'CGRAM Location - 5'
    else if(BT_ON == 0 && WiFi_ON == 0) send_char(' ');
    else if(WiFi_ON == 1) send_char(0x01); // Display "WiFi Icon" which is stored at 'CGRAM Location - 1'
    
    Lcd_Set_Cursor(2, 12); // Second Row 12th Position in lCD Interface
    send_char(0x00); // Displaying "Temperature symbol" which is stored at 'CGRAM Location - 0'

    Lcd_Set_Cursor(2, 13); // Second Row 13th Position in lCD Interface
    if (Temperature < 0) {
        Temperature = abs(Temperature);
        send_char('-');
        display(Temperature);
    } else {
        send_char(' ');
        display(Temperature);
    }
    Lcd_Set_Cursor(2, 16); // Second Row 16th Position in lCD Interface
    send_char(0x04); // Displaying "Celcius symbol" which is stored at 'CGRAM Location - 0'

}

void Menu_Interface() {
    Lcd_Set_Cursor(1, 7);
    send_string("MENU");

    Lcd_Set_Cursor(2, 1);
    send_char(0x02); // Display "Left Symbol Icon" which is stored at 'CGRAM Location - 2'

    Lcd_Set_Cursor(2, 16);
    send_char(0x03); // Display "Left Symbol Icon" which is stored at 'CGRAM Location - 2'

    switch (state) {
        case Menu_Time:
            Lcd_Set_Cursor(2, 5);
            send_string("Set Time");
            __delay_ms(500);
            Lcd_Set_Cursor(2, 2);
            send_string("             ");
            break;

        case Menu_Date:
            Lcd_Set_Cursor(2, 5);
            send_string("Set Date");
            __delay_ms(500);
            Lcd_Set_Cursor(2, 5);
            send_string("        ");
            break;
    }
}

void display_set_time(void) {
    Lcd_Set_Cursor(1, 2);
    send_string("TIME");
    send_char('-');

    switch (state) {
        case Set_Hour:

            Lcd_Set_Cursor(1, 10);
            send_char(':');
            display(min);

            Lcd_Set_Cursor(1, 14);
            display_AM_PM();

            Lcd_Set_Cursor(2, 8);
            send_string("hr");
            Lcd_Set_Cursor(2, 11);
            send_string("   ");

            if (Blink_State == 1) {
                Lcd_Set_Cursor(1, 8);
                display(hr);
                __delay_ms(200);
                Lcd_Set_Cursor(1, 8);
                send_string("  ");
            } else {
                Lcd_Set_Cursor(1, 8);
                display(hr);
            }
            break;

        case Set_Min:

            Lcd_Set_Cursor(1, 8);
            display(hr);
            send_char(':');

            Lcd_Set_Cursor(1, 14);
            display_AM_PM();

            Lcd_Set_Cursor(2, 8);
            send_string("  ");
            Lcd_Set_Cursor(2, 11);
            send_string("min");

            if (Blink_State == 1) {
                Lcd_Set_Cursor(1, 11);
                display(min);
                __delay_ms(200);
                Lcd_Set_Cursor(1, 11);
                send_string("  ");
            } else {
                Lcd_Set_Cursor(1, 11);
                display(min);
            }
            break;

        case Set_AM_PM:

            Lcd_Set_Cursor(1, 8);
            display(hr);
            send_char(':');
            display(min);

            Lcd_Set_Cursor(2, 8);
            send_string("hr");
            Lcd_Set_Cursor(2, 11);
            send_string("min");

            Lcd_Set_Cursor(1, 14);
            display_AM_PM();
            __delay_ms(200);
            Lcd_Set_Cursor(1, 14);
            send_string("  ");
            break;
    }
}

void display_set_date(void) {
    Lcd_Set_Cursor(1, 2);
    send_string("DATE:");

    Lcd_Set_Cursor(2, 3);
    send_string("DAY:");

    Lcd_Set_Cursor(2, 8);
    send_char(0x02); // Display "Left Symbol Icon" which is stored at 'CGRAM Location - 2'

    Lcd_Set_Cursor(2, 12);
    send_char(0x03); // Display "Left Symbol Icon" which is stored at 'CGRAM Location - 2'

    switch (state) {
        case Set_Date:

            if (Blink_State == 1) {
                Lcd_Set_Cursor(1, 8);
                display(date);
                __delay_ms(200);
                Lcd_Set_Cursor(1, 8);
                send_string("  ");
            } else {
                Lcd_Set_Cursor(1, 8);
                display(date);
            }

            send_char('/');
            display(month);
            send_char('/');
            display(year);

            Lcd_Set_Cursor(2, 9);
            check_weekday(day);
            break;

        case Set_Month:

            Lcd_Set_Cursor(1, 8);
            display(date);
            send_char('/');

            if (Blink_State == 1) {
                display(month);
                __delay_ms(200);
                Lcd_Set_Cursor(1, 11);
                send_string("  ");
            } else {
                display(month);
            }

            send_char('/');
            display(year);
            Lcd_Set_Cursor(2, 9);
            check_weekday(day);
            break;

        case Set_Year:

            Lcd_Set_Cursor(1, 8);
            display(date);
            send_char('/');
            display(month);
            send_char('/');

            if (Blink_State == 1) {
                display(year);
                __delay_ms(200);
                Lcd_Set_Cursor(1, 14);
                send_string("  ");
            } else {
                display(year);
            }

            Lcd_Set_Cursor(2, 9);
            check_weekday(day);
            break;

        case Set_Day:

            Lcd_Set_Cursor(1, 8);
            display(date);
            send_char('/');
            display(month);
            send_char('/');
            display(year);

            if (Blink_State == 1) {
                Lcd_Set_Cursor(2, 9);
                check_weekday(day);
                __delay_ms(200);
                Lcd_Set_Cursor(2, 9);
                send_string("   ");
            } else {
                Lcd_Set_Cursor(2, 9);
                check_weekday(day);
            }
            break;
    }
}

void display_people_count(void)
{
    Lcd_Set_Cursor(1, 1);
    send_string("No. of People in");
    
    Lcd_Set_Cursor(2, 1);
    send_string("the Room:");
    
    Lcd_Set_Cursor(2, 15);
    send_char(tens + 48);           // Display Counter Ten's Place
    lcd_busy();
    send_char(unit + 48);           // Display Counter Unit's Place
    lcd_busy();
}

void display_AM_PM(void) {
    if (PM_mode == 0) {
        send_string("AM");
    } else if (PM_mode == 1) {
        send_string("PM");
    }
}

void display(unsigned char data) // Function to convert received 'Decimal' no to 'ASCII'
{
    send_char((data / 10) + 48); // For Ten's Place         
    __delay_ms(5);
    lcd_busy();
    send_char((data % 10) + 48); // FOr Unit's Place
    __delay_ms(5);
    lcd_busy();
}

void display_clock(void) // Function to display Date and Time
{
    hr = BCD_2_DEC((unsigned) hour & 0b00011111); // AND-ing 'hour' Register with lower five bits set because Bit-5,6 contains info about AM/PM & 12/24hr Mode 
    display(hr);
    send_char(':');

    display(min);
    send_char(':');

    display(sec);

    Lcd_Set_Cursor(2, 1); // Setting the cursor to Second row First Column
    __delay_ms(5);

    display(date);
    send_char('/');

    display(month);
    send_char('/');

    display(year);
}

void check_weekday(unsigned char day) // Function to check the weekday
{
    switch (day) {
        case 1:
            send_string("MON");
            break;
        case 2:
            send_string("TUE");
            break;
        case 3:
            send_string("WED");
            break;
        case 4:
            send_string("THU");
            break;
        case 5:
            send_string("FRI");
            break; ;
        case 6:
            send_string("SAT");
            break;
        case 7:
            send_string("SUN");
            break;
    }
}