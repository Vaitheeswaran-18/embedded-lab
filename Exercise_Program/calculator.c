#include <nxp/iolpc2148.h>

// LCD Pins
#define rs IO0PIN_bit.P0_8
#define en IO0PIN_bit.P0_11

// Delay
void delay(unsigned int x) {
    unsigned int i;
    for (i = 0; i < x; i++);
}

// LCD Initialization
unsigned char commandarray[5] = {0x38, 0x01, 0x06, 0x0c, 0x80};

void lcd_init() {
    int i;
    for (i = 0; i < 5; i++) {
        IO1PIN = commandarray[i] << 16;
        rs = 0;
        en = 1;
        delay(4095);
        en = 0;
    }
}

void lcd_cmd(unsigned char cmd) {
    IO1PIN = cmd << 16;
    rs = 0;
    en = 1;
    delay(4095);
    en = 0;
}

void lcd_data(unsigned char data) {
    IO1PIN = data << 16;
    rs = 1;
    en = 1;
    delay(500);
    en = 0;
}

void lcd_print(char *str) {
    while (*str != '\0') {
        lcd_data(*str);
        str++;
    }
}

void intToStr(int num, char *str) {
    int i = 0, j, rem;
    char temp[10];
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    if (num < 0) {
        str[i++] = '-';
        num = -num;
    }
    while (num != 0) {
        rem = num % 10;
        temp[i++] = rem + '0';
        num /= 10;
    }
    int start = (str[0] == '-') ? 1 : 0;
    for (j = 0; j < i; j++) {
        str[start + j] = temp[i - j - 1];
    }
    str[start + i] = '\0';
}

char get_key() {
    int row_vals[] = {0x000E0000, 0x000D0000, 0x000B0000, 0x00070000};
    char key_map[4][4] = {
        {'1', '2', '3', '+'},
        {'4', '5', '6', '-'},
        {'7', '8', '9', '*'},
        {'C', '0', '=', '/'}
    };

    int row, col;

    for (row = 0; row < 4; row++) {
        IO0PIN = row_vals[row]; // set one row low at a time
        delay(1000);
        int col_val = IO0PIN & 0x00F00000;
        if (col_val != 0x00F00000) {
            for (col = 0; col < 4; col++) {
                if ((col_val & (1 << (20 + col))) == 0) {
                    while ((IO0PIN & 0x00F00000) != 0x00F00000); // wait for release
                    return key_map[row][col];
                }
            }
        }
    }
    return 0;
}

void main() {
    char key, op = 0;
    char buffer[20];
    int num1 = 0, num2 = 0, result = 0;
    int isSecond = 0;

    // GPIO Setup
    PINSEL0 = 0x00000000;
    PINSEL2 = 0x00000000;
    IO0DIR |= 0x000F0000;
    IO0DIR &= ~0x00F00000;
    IO0DIR_bit.P0_8 = 1;
    IO0DIR_bit.P0_11 = 1;
    IO1DIR = 0x00FF0000;

    lcd_init();
    lcd_cmd(0x80);
    lcd_print("Calc Ready");

    while (1) {
        key = get_key();
        if (key >= '0' && key <= '9') {
            lcd_data(key);
            if (!isSecond) {
                num1 = num1 * 10 + (key - '0');
            } else {
                num2 = num2 * 10 + (key - '0');
            }
        } else if (key == '+' || key == '-' || key == '*' || key == '/') {
            op = key;
            lcd_data(op);
            isSecond = 1;
        } else if (key == '=') {
            lcd_data('=');
            switch (op) {
                case '+': result = num1 + num2; break;
                case '-': result = num1 - num2; break;
                case '*': result = num1 * num2; break;
                case '/': result = (num2 != 0) ? (num1 / num2) : 0; break;
                default: result = 0;
            }
            intToStr(result, buffer);
            lcd_cmd(0xC0);
            lcd_print("Ans=");
            lcd_print(buffer);
        } else if (key == 'C') {
            num1 = num2 = result = 0;
            op = 0;
            isSecond = 0;
            lcd_cmd(0x01);
            lcd_cmd(0x80);
            lcd_print("Calc Ready");
        }
        delay(100000);
    }
}
