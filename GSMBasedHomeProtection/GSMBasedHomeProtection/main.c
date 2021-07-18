#define F_CPU 1000000
#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7
#define RS eS_PORTC6
#define EN eS_PORTC7


//#define DEFAULT_RESPONSE_SIZE 2000

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
//#include <bits/stdc++.h>
#include "lcd.h"
#include "adc.h"
#include "mq5.h"
#include <avr/interrupt.h>



char ATcommand[] = {'A', 'T', '\n\r', '\0'};
char ATDcommand[] = {'A', 'T', 'D', '+', '8', '8', '0', '1', '7','9','9', '3', '5', '8', '5', '9', '1', ';', '\n\r', '\0'};
char ATHcommand[] = {'A', 'T','H', '\n\r', '\0'};
char setBaudRate[] = {'A', 'T', '+', 'I', 'P', 'R', '=', '9', '6', '0', '0', '\n\r','\0'};
char ATCMGFcommand[] = {'A', 'T', '+', 'C', 'M', 'G', 'F', '=', '1','\n\r','\0'};
char ATCMGScommand[] = {'A', 'T', '+', 'C', 'M', 'G', 'S', '=', '"', '+', '8', '8', '0', '1', '7','9','9', '3', '5', '8', '5', '9', '1', '"', '\n\r', '\0'};
char ATSapbrPost1[] = {'A','T', '+', 'S', 'A', 'P', 'B', 'R', '=', '3', ',' , '1',',', '"', 'C', 'o', 'n', 't', 'y', 'p', 'e', '"', ',', '"', 'G', 'P', 'R', 'S', '"', '\n\r' , '\0'};
char ATSapbrPost2[] = {'A','T', '+', 'S', 'A', 'P', 'B', 'R', '=', '3', ',' , '1', '"', 'A', 'P', 'N', '"', ',', '"','R', 'o', 'b', 'i', '-', 'I', 'N', 'T', 'E', 'R', 'N', 'E', 'T', '"', '\n\r' , '\0'};
char ATSapbrPost3[] = {'A','T', '+', 'S', 'A', 'P', 'B', 'R', '=', '1', ',' , '1', '\n\r' , '\0'};
char ATSapbrPost4[] = {'A','T', '+', 'S', 'A', 'P', 'B', 'R', '=', '2', ',' , '1', '\n\r' , '\0'};
char httpiInitPost5[] = {'A', 'T', '+', 'H', 'T', 'T', 'P', 'I', 'N', 'I', 'T', '\n\r', '\0'};
char AThttpparaPost6[] = {'A', 'T', '+', 'H', 'T', 'T', 'P', 'P', 'A', 'R', 'A', '=','"', 'C', 'I', 'D','"', ',', '1', '\n\r', '\0'};
char AThttpparaPost7[] = {'A', 'T', '+', 'H', 'T', 'T', 'P', 'P', 'A', 'R', 'A', '=', '"', 'U', 'R', 'L','"', ',', '"', 'a', 'p', 'i', '.', 't', 'h', 'i', 'n', 'g', 's', 'p', 'e', 'a', 'k', '.', 'c', 'o', 'm', '/', 'u', 'p', 'd', 'a', 't', 'e', '"', '\n\r', '\0'};
char ATHTTPDataPost8[] = {'A', 'T', '+', 'H', 'T', 'T', 'P', 'D', 'A', 'T', 'A', '=', '0',',', '1', '0','0','0','0', '\n\r', '\0'};
char writeApikeyPost9[] = {'a', 'p', 'i', '_', 'k', 'e', 'y', '=', 'K','R','J','5','C','7','M','A','M','R','W','H','S','S','K','O', '&','f','i','e','l','d','1','=','1','\n\r', '\0'};
char ATHTTPAction1Post10[] = {'A', 'T', '+', 'H', 'T', 'T', 'P', 'A', 'C', 'T', 'I', 'O', 'N', '=', '1', '\n\r', '\0'}; 


char ATURL[] = {'A', 'T', '+', 'H', 'T', 'T', 'P', 'P', 'A', 'R', 'A', '=', '"', 'U', 'R', 'L','"', ',', '"', 'a', 'p', 'i', '.', 't', 'h', 'i', 'n', 'g', 's', 'p', 'e', 'a', 'k', '.', 'c', 'o', 'm', '/', 'u', 'p', 'd', 'a', 't', 'e',  '?','a','p','i','_','k','e','y','=','K','R','J','5','C','7','M','A','M','R','W','H','S','S','K','O','&','f','i','e','l','d','1','=','1','0', '0', '"', '\n\r', '\0'};
char ATHTTPAction0[] = {'A', 'T', '+', 'H', 'T', 'T', 'P', 'A', 'C', 'T', 'I', 'O', 'N', '=', '0', '\n\r', '\0'}; 
char ATHTTPRead[] = {'A','T','+','H','T','T','P','R','E','A','D', '\n\r', '\0'};
char ATHTTPTerm[] = {'A','T','+','H','T','T','P','T','E','R','M','\n\r', '\0'};
	
//char response[DEFAULT_RESPONSE_SIZE];
//uint16_t counter = 0;

unsigned char Res[16], lpg[16], smoke[16];
float Ro=10;    //Ro is initialized to 10 kilo ohms
volatile uint8_t gc,fc;
volatile unsigned char toggle = 0;
volatile uint16_t pirTimeCount = 0;
volatile float flame = 0, smoke2 = 0, lpg2 = 0;

ISR(INT1_vect)//STEP2
{
	sei();
	if(toggle == 0){
		toggle = 1;
	}
	else{
		toggle = 0;
	}
}

ISR(TIMER1_OVF_vect){
	sei();
	pirTimeCount++;
}

ISR(INT0_vect){
	if(toggle == 1){
		sei();
		pirTimeCount = 0;
		Lcd4_Clear();
		Lcd4_Set_Cursor(2,1);
		Lcd4_Write_String("Motion detected");
		_delay_ms(2000);
		motionDetected();
	}
}

//ISR(USART_RXC_vect){
	//sei();
	//response[counter++] = UDR;
//}
//
//void responseBufferFlush(){
	//memset(response, 0, DEFAULT_RESPONSE_SIZE);
	//counter = 0;
//}

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
	GSMSendMessage(mes);
}
void motionDetected(){
	ledLightAndBuzzer();
	GSMcall();
	_delay_ms(5000);
	char mes[] = "motion detected";
	GSMSendMessage(mes);
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
	
	
	if(lpgAmount < 0) lpgAmount = 99999;
	lpg2 = lpgAmount;
	
	int smokeAmount = GetGasPercentage(ReadSensor()/Ro,SMOKE);
	itoa(smokeAmount, smoke, 10);
	Lcd4_Set_Cursor(2,7);
	Lcd4_Write_String(smoke);
	smoke2 = (float)smokeAmount;
	
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

float getTemperatureInput(){
	int temp=adcread(2);
	float temperature=temp*125/256.0;
	return temperature;
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
	_delay_ms(500);
	
	float temperature = getTemperatureInput();
	char tempVal[5];
	dtostrf(temperature,3,1,tempVal);
	flame = temperature;
	//Lcd4_Clear();
	Lcd4_Set_Cursor(2,1);
	Lcd4_Write_String("temp: ");
	Lcd4_Write_String(tempVal);
	Lcd4_Write_String(" C");
	_delay_ms(3000);
	
	if(flameAnalogValue<100){
		//fc++;
		//dtostrf(fc,3,1,fval);
		//Lcd4_Set_Cursor(2,1);
		//Lcd4_Write_String("fc: ");
		//Lcd4_Write_String(fval);
		//if(fc==3){
			Lcd4_Clear();
			Lcd4_Set_Cursor(1,1);
			Lcd4_Write_String("flame detected");
			_delay_ms(3000);
			flameDetected();
			
		//}
		
	}
	//else{
		////fc=0;
	//}
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
	//unsigned char in = PINB & 0x01;
	//if(in && toggle){
		//Lcd4_Clear();
		//Lcd4_Set_Cursor(1,1);
		//Lcd4_Write_String("Motion detected");
		//_delay_ms(2000);
		//motionDetected();
		//
	//}
	//else{
		//Lcd4_Clear();
		Lcd4_Set_Cursor(2,1);
		Lcd4_Write_String("No Motion");
		_delay_ms(2000);
	//}
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


void HTTPSendTest(){
	Lcd4_Clear();
	Lcd4_Write_String("Sending");
	
	USART_send_string(ATcommand);
	_delay_ms(1500);
	USART_send_string(ATSapbrPost1);
	_delay_ms(1500);
	USART_send_string(ATSapbrPost2);
	_delay_ms(1500);
	USART_send_string(ATSapbrPost3);
	_delay_ms(1500);
	USART_send_string(ATSapbrPost4);
	_delay_ms(1500);
	USART_send_string(httpiInitPost5);
	_delay_ms(1500);
	USART_send_string(AThttpparaPost6);
	_delay_ms(1500);
	USART_send_string(AThttpparaPost7);
	_delay_ms(1500);
	USART_send_string(ATHTTPDataPost8);
	_delay_ms(1500);
	USART_send_string(writeApikeyPost9);
	_delay_ms(1500);
	USART_send_string(ATHTTPAction1Post10);
	_delay_ms(20000);
	
	Lcd4_Clear();
	Lcd4_Write_String("sent");
}

void HTTPSendTest2(float lpgVal, float smokeVal, float temperatureVal, float motionDetectionTime, float PIRStatus){
	//responseBufferFlush();
	
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,1);
	Lcd4_Write_String("Sending");
	//char ATURL[] = {'A', 'T', '+', 'H', 'T', 'T', 'P', 'P', 'A', 'R', 'A', '=', '"', 'U', 'R', 'L','"', ',', '"', 'a', 'p', 'i', '.', 't', 'h', 'i', 'n', 'g', 's', 'p', 'e', 'a', 'k', '.', 'c', 'o', 'm', '/', 'u', 'p', 'd', 'a', 't', 'e',  '?','a','p','i','_','k','e','y','=','K','R','J','5','C','7','M','A','M','R','W','H','S','S','K','O','&','f','i','e','l','d','1','=','1','0', '0', '"', '\n\r', '\0'};

	char command[100];
	sprintf(command, "AT+HTTPPARA=\"URL\",\"api.thingspeak.com/update?api_key=KRJ5C7MAMRWHSSKO&field1=%d&field2=%d&field3=%d&field4=%d&field5=%d\"", (int)lpgVal, (int)smokeVal, (int)temperatureVal, (int)motionDetectionTime, (int)PIRStatus);
	
	USART_send_string(ATcommand);
	_delay_ms(1500);
	USART_send_string(ATSapbrPost1);
	_delay_ms(1500);
	USART_send_string(ATSapbrPost2);
	_delay_ms(1500);
	USART_send_string(ATSapbrPost3);
	_delay_ms(1500);
	USART_send_string(ATSapbrPost4);
	_delay_ms(1500);
	USART_send_string(httpiInitPost5);
	_delay_ms(1500);
	USART_send_string(AThttpparaPost6);
	_delay_ms(1500);
	//USART_send_string(ATURL);
	USART_send_string(command);
	USART_send('\n\r');
	_delay_ms(1500);
	USART_send_string(ATHTTPAction0);
	_delay_ms(1500);
	USART_send_string(ATHTTPRead);
	_delay_ms(1500);
	USART_send_string(ATHTTPTerm);
	_delay_ms(1500);
	
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,1);
	Lcd4_Write_String("sent");
	_delay_ms(15000);
}


//void HTTPGetTest(){
	//responseBufferFlush();
	//
	//Lcd4_Clear();
	//Lcd4_Write_String("Receiving");
	////char ATURL[] = {'A', 'T', '+', 'H', 'T', 'T', 'P', 'P', 'A', 'R', 'A', '=', '"', 'U', 'R', 'L','"', ',', '"', 'a', 'p', 'i', '.', 't', 'h', 'i', 'n', 'g', 's', 'p', 'e', 'a', 'k', '.', 'c', 'o', 'm', '/', 'u', 'p', 'd', 'a', 't', 'e',  '?','a','p','i','_','k','e','y','=','K','R','J','5','C','7','M','A','M','R','W','H','S','S','K','O','&','f','i','e','l','d','1','=','1','0', '0', '"', '\n\r', '\0'};
//
	//char command[100];
	//sprintf(command, "AT+HTTPPARA=\"URL\",\"api.thingspeak.com/channels/1446074/fields/5.json?results=1\"");
	//
	//USART_send_string(ATcommand);
	//_delay_ms(1500);
	//USART_send_string(ATSapbrPost1);
	//_delay_ms(1500);
	//USART_send_string(ATSapbrPost2);
	//_delay_ms(1500);
	//USART_send_string(ATSapbrPost3);
	//_delay_ms(1500);
	//USART_send_string(ATSapbrPost4);
	//_delay_ms(1500);
	//USART_send_string(httpiInitPost5);
	//_delay_ms(1500);
	//USART_send_string(AThttpparaPost6);
	//_delay_ms(1500);
	////USART_send_string(ATURL);
	//USART_send_string(command);
	//USART_send('\n\r');
	//_delay_ms(1500);
	//USART_send_string(ATHTTPAction0);
	//_delay_ms(1500);
	//USART_send_string(ATHTTPRead);
	//_delay_ms(1500);
	//USART_send_string(ATHTTPTerm);
	//_delay_ms(1500);
	//
	//Lcd4_Clear();
	//Lcd4_Write_String("sent");
	//_delay_ms(2000);
	//
	////Lcd4_Clear();
	////Lcd4_Write_String(response);
	////_delay_ms(500);
	////for(int i=0;i<100;i++){
		//////Lcd4_Shift_Left();
		////_delay_ms(500);
	////}
//}



void timerInit(){
	TCCR1A = 0x00;
	TCCR1B = 0x01;
	TIMSK = 1 << TOIE1;
}

int getPirTime(){
	int timeCount = (pirTimeCount*65535+TCNT1)/60;
	return timeCount;
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
	MCUCR = 1 << ISC10 | 1 << ISC11 | 1 << ISC00 | 1 << ISC01 ;
	GICR = (1<<INT1 | 1 << INT0); //STEP3
	//MCUCR = MCUCR & 0b11110011;//STEP4
	sei();//STEP5
	
	timerInit();

	USART_init(51);
    Lcd4_Init();
	
	_delay_ms(2000);
	
	GSM_init();
	//float val=flameSensorInit();
	
	gasSensorInit();
	//Lcd4_Set_Cursor(1,1);
	//Lcd4_Write_String("init");
	//_delay_ms(2000);
	
	//HTTPSendTest();
	uint16_t val = 0;
    while (1) 
    {
		getGasSensorInput();
		PORTB = 0x00;
		flameSensorInput();
		PORTB = 0x00;
		motionSensorInput();
		PORTB = 0x00;
		HTTPSendTest2(lpg2, smoke2, flame, getPirTime(), toggle);
		//val++;
    }
	
	return 0;
}

