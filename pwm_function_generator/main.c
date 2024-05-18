// Using 8MHz internal clock
// PWM out - PB0
// Switches - PB1, PB2
// Analog in (frequency control) - PB4 (ADC2)


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include <stdbool.h>


#define LUT_SIZE 256
#define PWM_MAX 255


unsigned char sineLUT[LUT_SIZE] = {
	128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173,
	176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
	218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244,
	245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255,
	255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246,
	245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220,
	218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179,
	176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131,
	128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82,
	79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40,
	37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11,
	10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9,
	10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35,
	37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76,
	79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124,
};

volatile unsigned char acc = 0;
volatile unsigned int overflowValue;


int main()
{
    // Set data direction of port B pin 0 to 1 (output)
    DDRB = 1 << DDB0;
	// Set data direction of port B pins 1 and 2 to 0 (input)
	DDRB &= ~(1 << DDB1) & ~(1 << DDB2);
	
	PORTB |= (1 << PORTB1) | (1 << PORTB2);
		
	// Configure Timer/Counter Control Register A
	// Enable Compare Match Output and set Waveform Generation mode to fast PWM
	TCCR0A = 1 << COM0A1| 1 << WGM01 | 1 <<  WGM00;
	
	// Configure Timer/Counter Control Register B
	// Disable prescaling
	TCCR0B = 1 << CS00;
	
	
	// Configure timer1 to trigger interrupt at a constant frequency
	TCCR1 &= ~(1 << COM1A1) & ~(1 << COM1A0); // Disconnect comparator A from output pin
	GTCCR &= ~(1 << COM1B1) & ~(1 << COM1B0); // Disconnect comparator A from output pin
	
	TCCR1 &= ~(1 << CS13) & ~(1 << CS12) & ~(1 << CS11);
	TCCR1 |= (1 << CS10);
	
	TIMSK |= (1 << TOIE1); // Enable interrupt
	sei();
	TCNT1 = 0;
	
	
	//https://www.instructables.com/ATTiny-Port-Manipulation-Part-2-AnalogRead/
	ADMUX |= (1 << REFS0);   //sets reference voltage to internal 1.1V
	
	ADMUX = 0b10100010;   //sets 1.1V IRV, sets ADC3 as input channel, and left adjusts
	ADCSRA = 0b10000011;  //turn on ADC, keep ADC single conversion mode,
						  //and set division factor-8 for 125kHz ADC clock
	ADCSRA = 0b10000011;  //turn on the ADC, keep ADC single conversion mode
						  //set division factor-8 for 125kHz ADC clock
	
    while (1) {
		
		ADCSRA |= (1 << ADSC);   //start conversion
		overflowValue = ADCH; //store data in variable
		
		unsigned char waveSetting = 0; // 2 bit number that determines which waveform is created
		if (PINB & (1 << PINB1)) { // set bit 1 of waveSetting to PINB1 value
			waveSetting += 1 << 1;
		}
		if (PINB & (1 << PINB2)) { // set bit 1 of waveSetting to PINB1 value
			waveSetting += 1;
		}
		
		unsigned char currentAcc = acc;
		
		switch (waveSetting) {
			case 0b00: // SINE
				OCR0A = sineLUT[currentAcc];
				break;
				
			case 0b01: // TRIANGLE
				if (currentAcc < 128) {
					OCR0A = 2*currentAcc;
				} else {
					OCR0A = 2*(255-currentAcc);
				}
				break;
				
			case 0b10: // SQUARE
				if (currentAcc < 128) {
					OCR0A = PWM_MAX;
				} else {
					OCR0A = 0;
				}
				break;
			
			case 0b11: // SAW
				OCR0A = currentAcc;
				break;
		}
		
		
	}
    return 0;
}

ISR(TIM1_OVF_vect){
	TCNT1 = overflowValue;
	acc += 1;
}