/*
 * File:   esp.c
 * Author: dell
 *
 * Created on July 27, 2018, 6:19 PM
 */


#include <xc.h>
#define _XTAL_FREQ 16000000

#define RS RC0
#define RW RC1
#define EN RC2
#define dataport PORTD

void lcd_on();
void enable_pulse();
void lcd_cmd(unsigned char);
void lcd_data(unsigned char);
void lcd_text(unsigned char*);
void uart_init();
unsigned char uart_recv();
void uart_send(unsigned char*);
void clear_lcd();
void adc_init();
unsigned int adc_read(unsigned char);
void tostring(char [], int);

void main(void) {
    float celsius;
    char Temp[10];
    int t,i=0;
    TRISD=0x00;
    TRISC=0x00;
    TRISA=0xFF;
    TRISCbits.RC7=1;
    RW=0;
    uart_init();
    adc_init();
    lcd_on();
    while(RCIF==0);
    lcd_text("esp connected!!!");
    __delay_ms(1000);
    uart_send("AT\r\n");          //Sending ATtention commands.
    __delay_ms(1000);
    /*uart_send("AT+CWMODE?\r\n");
    __delay_ms(1000);*/
    uart_send("AT+CWQAP\r\n");
    __delay_ms(1000);
    uart_send("AT+CWJAP=\"DESKTOP\",\"1234567890\"\r\n");        //connecting with hotspot named desktop and pass-1234567890
    __delay_ms(1000);
    uart_send("AT+CIFSR\r\n");
    __delay_ms(5000);
    uart_send("AT+CIPMUX=0\r\n");
    __delay_ms(1000);
    while(1){
        lcd_text("Temp= ");
        celsius=(adc_read(0)*0.488);
        t=(int)celsius;
        tostring(Temp,t);
        lcd_text(Temp);
        lcd_data(39);
        lcd_data('C');
        uart_send("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n");      //ip address of thingspeak server = 184.106.153.149
        __delay_ms(1000);
        uart_send("AT+CIPSEND=54\r\n");
        __delay_ms(1000);
        //while(uart_recv()!='>');
        uart_send("GET /update?api_key=DSJR3CGC6TD2OL3X&field1="); //sending to thingspeak channel
        uart_send(Temp);
        uart_send("\r\n");
        __delay_ms(4000);
        clear_lcd();
        uart_send("AT+CIPCLOSE\r\n");
        //uart_send("ATE\r\n");
        //__delay_ms(100);
    }
    //while(1);
    return;
}
void tostring(char str[], int num)
{
    int i, rem, len = 0, n;
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem+'0';
    }
    str[len] = '\0';
}
unsigned int adc_read(unsigned char channel){
    if(channel > 7)
        return 0;
    //ADCON0 &= 0xC5;
    ADCON0=0x01;
    ADON=1;
    ADCON0 |= (channel << 3);
    __delay_ms(5);
    ADCON0bits.GO=1;
    while(ADCON0bits.DONE == 1);
    return ((ADRESH<<8)+ADRESL);
}
void adc_init(){                    //to initialise A to D converter
    ADCON1=0x00;
    ADCON0=0x00;
    ADCON2=0xB2;
}
void clear_lcd(){
    lcd_cmd(0x01);
    __delay_ms(1);
}
void uart_send(unsigned char* ptr){
    while(*ptr){
        while(!TRMT);
        TXREG=*ptr++;
    }
}
void uart_init(){
    TXSTA=0x24;
    RCSTA=0x90;
    BAUDCON=0x40;
    SPBRG=103;
}
unsigned char uart_recv(){
    while(RCIF==0);
    RCIF=0;
    return(RCREG);
}
void lcd_text(unsigned char *text)
{
    while(*text)
    {
        lcd_data(*text++);
    }
}
void enable_pulse(){
    EN=1;
    __delay_ms(5);
    EN=0;
    __delay_ms(5);
}
void lcd_on(){
    lcd_cmd(0x38);
    __delay_ms(1);
    lcd_cmd(0x0E);
    __delay_ms(1);
}
void lcd_cmd(unsigned char cmd){
    RS=0;
    dataport=cmd;
    enable_pulse();
}
void lcd_data(unsigned char data){
    RS=1;
    dataport=data;
    enable_pulse();
}
