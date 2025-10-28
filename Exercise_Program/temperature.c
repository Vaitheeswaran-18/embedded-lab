#include <nxp\iolpc2148.h>
#include <stdio.h>

#define PCLKFREQ 12000000

#define rs IO0PIN_bit.P0_8
#define en IO0PIN_bit.P0_11

// LCD command array for initialization
unsigned char commandarray[5] = {0x38, 0x01, 0x06, 0x0C, 0x80};

// ---------------- Delay Function ----------------
void delay(unsigned int x)
{
    unsigned int i;
    for (i = 0; i < x; i++);
}

// ---------------- LCD Functions ----------------
void lcd_command()
{
    int i;
    for (i = 0; i < 5; i++)
    {
        IO1PIN = commandarray[i] << 16;
        rs = 0;
        en = 1;
        delay(4095);
        en = 0;
    }
}

void single_command(unsigned char command1)
{
    IO1PIN = command1 << 16;
    rs = 0;
    en = 1;
    delay(4095);
    en = 0;
}

void lcd_data(unsigned char dataarray[])
{
    int i;
    for (i = 0; dataarray[i] != '\0'; i++)
    {
        IO1PIN = dataarray[i] << 16;
        rs = 1;
        en = 1;
        delay(4095);
        en = 0;
    }
}

// ---------------- ADC Initialization ----------------
void adc_init()
{
    PINSEL1 = 0x01000000; // Select ADC0.1 pin (P0.28)
    AD0CR_bit.CLKDIV = 0x02; // PCLK/2
    AD0CR_bit.BURST = 0;     // Single conversion mode
    AD0CR_bit.CLKS = 0;      // 10-bit conversion
    AD0CR_bit.PDN = 1;       // ADC operational
    AD0CR_bit.SEL = 0x02;    // Select channel AD0.1
}

// ---------------- Read ADC Value ----------------
unsigned int read_adc()
{
    unsigned int val;
    AD0CR_bit.START = 1;            // Start conversion
    while (AD0DR1_bit.DONE == 0);   // Wait until done
    val = AD0DR1_bit.RESULT;        // Read result
    AD0CR_bit.START = 0;            // Stop conversion
    return val;
}

// ---------------- Main Program ----------------
void main(void)
{
    unsigned int val = 0, volt;
    char lcd_buffer[16];

    // GPIO & LCD setup
    PINSEL0 = 0x00000000;
    PINSEL2 = 0x00000000;
    IO0DIR_bit.P0_8 = 1;
    IO0DIR_bit.P0_11 = 1;
    IO1DIR = 0x00FF0000;

    // Initialize LCD
    lcd_command();

    // Initialize ADC
    adc_init();

    while (1)
    {
        val = read_adc();
        volt = (val * 3.3 * 100) / 1023; // Convert ADC to temperature (approx for LM35: 10mV/°C)

        single_command(0x80);
        lcd_data("Temperature:    ");

        single_command(0xC0);
        sprintf(lcd_buffer, "%d C      ", volt);
        lcd_data(lcd_buffer);

        delay(65000);
    }
}
