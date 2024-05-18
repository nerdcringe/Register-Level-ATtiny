#include <avr/io.h>
#include <util/delay.h>

int main()
{
    // Set data direction of port B pin 0 to 1 (output)
    DDRB = 1 << DDB0;
	
	// Configure Timer/Counter Control Register A
	// Enable Compare Match Output and set Waveform Generation mode to fast PWM
	TCCR0A = 1 << COM0A1 | 1 << WGM01 | 1 <<  WGM00;
	
	// Configure Timer/Counter Control Register B
	// Disable prescaling
	TCCR0B = 1 << CS00;
	
    while (1) {
        OCR0A = 0;
		_delay_ms(1000);
        OCR0A = 127;
		_delay_ms(1000);
        OCR0A = 255;
		_delay_ms(1000);
	}
    return 0;
}