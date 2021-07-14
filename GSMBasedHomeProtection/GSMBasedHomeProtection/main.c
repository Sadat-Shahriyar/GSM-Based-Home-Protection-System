#define F_CPU 1000000
#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7
#define RS eS_PORTC6
#define EN eS_PORTC7

#include <avr/io.h>
#include <stdlib.h>
#include "lcd.h"
#include "adc.h"
#include "mq5.h"
#include <avr/interrupt.h>


char ATcommand[] = {'A', 'T', '\n\r', '\0'};
char ATDcommand[] = {'A', 'T', 'D', '+', '8', '8', '0', '1', '7','0','3', '8', '2', '0', '5', '6', '5', ';', '\n\r', '\0'};
char ATHcommand[] = {'A', 'T','H', '\n\r', '\0'};
char setBaudRate[] = {'A', 'T', '+', 'I', 'P', 'R', '=', '9', '6', '0', '0', '\n\r','\0'};
char ATCMGFcommand[] = {'A', 'T', '+', 'C', 'M', 'G', 'F', '=', '1','\n\r','\0'};
char ATCMGScommand[] = {'A', 'T', '+', 'C', 'M', 'G', 'S', '=', '"', '+', '8', '8', '0', '1', '7','0','3', '8', '2', '0', '5', '6', '5', '"', '\n\r', '\0'};


unsigned char Res[16], lpg[16], smoke[16];
float Ro=10;    //Ro is initialized to 10 kilo ohms
volatile uint8_t gc,fc;
volatile unsigned char toggle = 0;

ISR(INT1_vect)//STEP2
{
	if(toggle == 0){
		toggle = 1;
	}
	else{
		toggle = 0;
	}
}


void USART_init(uint16_t ubrr_value)
{
	UCSRA = 0b00000000;  
	UCSRB = 0b00011000;  // Enable Tx and Rx, polling
	UCSRC = 0b10000110;  // Async mode, no parity, 1 stop bit, 8 data bits
	//in double-speed mode, UBRR = clock/(8xbaud rate) - 1
	UBRRH = 0;
	UBRRL = 51; // Baud rate 1200bps, assuming 1MHz clock
}

void USART_send(unsigned char data){
	// wait until UDRE flag is set to logic 1
	while ((UCSRA & (1<<UDRE)) == 0x00);
	UDR = data; // Write character to UDR for transmission
}

unsigned char USART_receive(void){
	// Wait until RXC flag is set to logic 1
	while ((UCSRA & (1<<RXC)) == 0x00);
	return UDR; // Read the received character from UDR
}

void USART_send_string(char c[]){
	int i = 0;
	while(c[i] != '\0'){
		USART_send(c[i]);
		i++;
	}
}

void GSM_init(){
	Lcd4_Clear();
	Lcd4_Write_String("Initializing GSM");	
	_delay_ms(7000);
	USART_send_string(ATcommand);
	//USART_send_string(setBaudRate);
	_delay_ms(500);
	Lcd4_Clear();
	Lcd4_Write_String("GSM initialized");
	_delay_ms(500);
}

void GSMcall(){
	Lcd4_Clear();
	Lcd4_Write_String("Calling");
	//_delay_ms(20000);
	USART_send_string(ATcommand);
	USART_send_string(ATDcommand);
	_delay_ms(20000);
}

void GSMHangUpCall(){
	Lcd4_Clear();
	Lcd4_Write_String("Hanging up");
	_delay_ms(1000);
	USART_send_string(ATcommand);
	USART_send_string(ATHcommand);
	_delay_ms(10000);
	Lcd4_Clear();
	Lcd4_Write_String("Call ended");
}

void GSMSendMessage(char message[]){
	_delay_ms(1000);
	
	Lcd4_Clear();
	Lcd4_Write_String("Sending message");
	
	USART_send_string(ATcommand);
	_delay_ms(2000);
	
	USART_send_string(ATCMGFcommand);
	_delay_ms(2000);
	
	USART_send_string(ATCMGScommand);
	_delay_ms(2000);
	
	USART_send_string(message);
	USART_send(26);
	USART_send('\n\r');
	_delay_ms(10000);
	
	Lcd4_Clear();
	Lcd4_Write_String("Message sent");
	_delay_ms(3000);
}


float takeGasSensorInput(){
	float result = 0;
	int low = 0;
	int high = 0;
	
	
	ADCSRA |= (1 << ADSC);
	
	while(ADCSRA & (1 << ADSC));
	
	low = ADCL;
	high = ADCH;
	result = (high << 8) + low;
	
	result = result*5/1024;
	
	return result;
}

void gasDetected(){
	ledLightAndBuzzer();
	GSMcall();
	_delay_ms(5000);
	char mes[] = "Critical level gas detected";	
	GSMSendMessage(mes);
}

void flameDetected(){
	ledLightAndBuzzer();
	GSMcall();
	_delay_ms(5000);
	char mes[] = "Fire detected";
	//GSMSendMessage(mes);
}
void motionDetected(){
	ledLightAndBuzzer();
	GSMcall();
	_delay_ms(5000);
	char mes[] = "motion detected";
	//GSMSendMessage(mes);
}

void gasSensorInit(){
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,1);
	Lcd4_Write_String("Calibrating gas");
	Lcd4_Set_Cursor(2,1);
	Lcd4_Write_String("sensor");
	Ro = SensorCalibration();
	dtostrf(Ro, 6, 2, Res);
	Lcd4_Clear();
	_delay_ms(100);
	Lcd4_Write_String("Calibration done...");
	Lcd4_Clear();
	Lcd4_Set_Cursor(2,3);
	Lcd4_Write_String("Ro=");
	Lcd4_Write_String(Res);
	Lcd4_Write_String("KOhm ");
	_delay_ms(10000);
	
	Lcd4_Clear();
	_delay_ms(100);
	
	Lcd4_Set_Cursor(1,1);
	Lcd4_Write_String("LPG:");
	Lcd4_Set_Cursor(1,13);
	Lcd4_Write_String("PPM");
	Lcd4_Set_Cursor(2,1);
	Lcd4_Write_String("SMOKE:");
	Lcd4_Set_Cursor(2,13);
	Lcd4_Write_String("PPM");
}



void getGasSensorInput(){
	//Lcd4_Clear();
	Lcd4_Set_Cursor(1,1);
	Lcd4_Write_String("LPG:");
	Lcd4_Set_Cursor(1,13);
	Lcd4_Write_String("PPM");
	Lcd4_Set_Cursor(2,1);
	Lcd4_Write_String("SMOKE:");
	Lcd4_Set_Cursor(2,13);
	Lcd4_Write_String("PPM");
	
	float lpgAmount = GetGasPercentage(ReadSensor()/Ro, LPG);
	itoa(lpgAmount, lpg,10);	
	Lcd4_Set_Cursor(1,5);
	Lcd4_Write_String(lpg);
	
	int smokeAmount = GetGasPercentage(ReadSensor()/Ro,SMOKE);
	itoa(smokeAmount, smoke, 10);
	Lcd4_Set_Cursor(2,7);
	Lcd4_Write_String(smoke);
	if(lpgAmount > 50){
		Lcd4_Clear();
		Lcd4_Set_Cursor(1,1);
		Lcd4_Write_String("Critical amount gas detected");
		_delay_ms(3000);
		gasDetected();
		Lcd4_Clear();
		Lcd4_Set_Cursor(1,1);
		Lcd4_Write_String("LPG:");
		Lcd4_Set_Cursor(1,13);
		Lcd4_Write_String("PPM");
		Lcd4_Set_Cursor(2,1);
		Lcd4_Write_String("SMOKE:");
		Lcd4_Set_Cursor(2,13);
		Lcd4_Write_String("PPM");
	}
	_delay_ms(2000);
	Lcd4_Set_Cursor(1,5);
	Lcd4_Write_String("        ");
	Lcd4_Set_Cursor(2,7);
	Lcd4_Write_String("      ");
}
float flameSensorInit(){
	float val=0;
	for(int i=0;i<10;i++){
		val+=adcread(1);
	}
	val=val/10.0;
	return val;
}

void flameSensorInput(){	
	int flameAnalogValue=adcread(1);
	char fval[5];
	dtostrf(flameAnalogValue,3,1,fval);
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,1);
	Lcd4_Write_String("flame: ");	
	Lcd4_Write_String(fval);
	Lcd4_Write_String(" nm");
	_delay_ms(3000);
	
	if(flameAnalogValue<100){
		fc++;
		//dtostrf(fc,3,1,fval);
		//Lcd4_Set_Cursor(2,1);
		//Lcd4_Write_String("fc: ");
		//Lcd4_Write_String(fval);
		if(fc==3){
			Lcd4_Clear();
			Lcd4_Set_Cursor(1,1);
			Lcd4_Write_String("flame detected");
			_delay_ms(3000);
			flameDetected();
		}
		
	}else{
		fc=0;
	}
}
void motionSensorInput(){
	//int motionAnalogValue=adcread(2);
	char fval[5];
	//dtostrf(motionAnalogValue,3,1,fval);
	if(toggle == 1){
		Lcd4_Clear();
		Lcd4_Set_Cursor(1,1);
		Lcd4_Write_String("PIR active");
		_delay_ms(1000);
	}
	else{
		Lcd4_Clear();
		Lcd4_Set_Cursor(1,1);
		Lcd4_Write_String("PIR inactive");
		_delay_ms(1000);
	}	
	unsigned char in = PINB & 0x01;
	if(in && toggle){
		Lcd4_Clear();
		Lcd4_Set_Cursor(1,1);
		Lcd4_Write_String("Motion detected");
		_delay_ms(2000);
		motionDetected();
		
	}
	else{
		Lcd4_Clear();
		Lcd4_Set_Cursor(1,1);
		Lcd4_Write_String("No Motion");
		_delay_ms(2000);
	}
	//dtostrf(in, 2,1,fval);
	//Lcd4_Clear();
	//Lcd4_Set_Cursor(1,1);
	//Lcd4_Write_String("motion: ");
	//Lcd4_Write_String(fval);
	//Lcd4_Write_String(" ");
	//_delay_ms(1000);
}

void ledLightAndBuzzer(){
	PORTB = 0x06;
}

int main(void)
{	
	gc = 0;
	fc = 0;
    DDRC = 0xFF;
    DDRD = 0xFF;
	DDRA = 0x00;
	DDRB= 0x06;
	ADMUX = 0b01000000;
	ADCSRA = 0b10000111;
	
	PORTB = 0x00;
	GICR = (1<<INT1); //STEP3
	//MCUCR = MCUCR & 0b11110011;//STEP4
	MCUCR = 1 << ISC10 | 1 << ISC11;
	sei();//STEP5

	USART_init(51);
    Lcd4_Init();
	
	_delay_ms(2000);
	
	GSM_init();
	//float val=flameSensorInit();
	
	gasSensorInit();
	//Lcd4_Set_Cursor(1,1);
	//Lcd4_Write_String("init");
	//_delay_ms(2000);
    while (1) 
    {
		getGasSensorInput();
		PORTB = 0x00;
		//flameSensorInput();
		//PORTB = 0x00;
		//motionSensorInput();
		//PORTB = 0x00;
    }
	
	return 0;
}

