#include "soc-hw.h"

uart_t  *uart0  = (uart_t *)   0x20000000;
timer_t *timer0 = (timer_t *)  0x30000000;
gpio_t  *gpio0  = (gpio_t *)   0x40000000;
//uart_t  *uart1  = (uart_t *)   0x20000000;
spi_t   *spi0   = (spi_t *)    0x50000000;
//i2c_t   *i2c0   = (i2c_t *)    0x70000000;

isr_ptr_t isr_table[32];

void prueba()
{
	   uart0->rxtx=30;
	   timer0->tcr0 = 0xAA;
	   gpio0->ctrl=0x55;
	   spi0->rxtx=1;
	   spi0->nop1=2;
	   spi0->cs=3;
	   spi0->divisor=4;
	   spi0->nop2=5;

}

/* Variables*/
 
// Interruption vector pointer 
isr_ptr_t isr_table[32];

// Duty cycle array
uint32_t pwm_d[] = {16159,
16161,
16210,
16336,
16481,
16588,
16638,
16645,
16647,
16705,
16884,
17163,
17466,
17720,
17922,
18084,
18196,
18294,
18480,
18808,
19286,
19971,
20920,
21974,
22794,
23352,
23896,
24551,
25259,
25968,
26715,
27597,
28597,
29611,
30561,
31330,
31847,
32112,
32163,
32166,
32192,
32203,
32217,
32206,
32210,
32204,
32183,
32184,
32178,
32169,
32176,
32186,
32198,
32206,
32196,
32194,
32188,
32178,
32178,
32175,
32169,
32179,
32181,
32186,
32191,
32191,
32195,
32201,
32205,
32220,
32224,
32221,
32207,
32184,
32163,
32147,
32142,
32084,
31894,
31602,
31278,
30972,
30714,
30508,
30345,
30190,
30029,
29883,
29721,
29522,
29274,
28953,
28555,
28121,
27723,
27451,
27320,
27312,
27361,
27321,
27120,
26818,
26529,
26342,
26255,
26230,
26205,
26062,
25777,
25433,
25084,
24736,
24410,
24193,
24161,
24264,
24449,
24639,
24763,
24825,
24822,
24729,
24533,
24236,
23893,
23557,
23249,
22996,
22797,
22668,
22645
};

// PWM max period
// Time (in miliseconds) of duration of PWM cycle
// Max value 1 milisecond
uint32_t pwm_p = 0.0226;

// Mode state
int mode = 0;

void tic_isr_0();
void tic_isr_1();
/***************************************************************************
 * IRQ handling
 */
void isr_null()
{
}

void irq_handler(uint32_t pending)
{
	int i;

	for(i=0; i<32; i++) {
		if (pending & 0x01) (*isr_table[i])();
		pending >>= 1;
	}
}

void isr_init()
{
	int i;
	for(i=0; i<32; i++)
		isr_table[i] = &isr_null;
}

void isr_register(int irq, isr_ptr_t isr)
{
	isr_table[irq] = isr;
}

void isr_unregister(int irq)
{
	isr_table[irq] = &isr_null;
}

void tic_isr_0()
{
	uint32_t out_state = 0;
	
	//uart_putstr("Interruption Timer 0\n");
	
	out_state = gpio0->out;
	gpio0->out = out_state | 0x01;
	
	timer0->counter0 = 0;
	timer0->tcr0   = TIMER_EN | TIMER_AR | TIMER_IRQEN;
	
	timer0->compare1 = conf_util(pwm_d[0]);
	timer0->counter1 = 0;
	timer0->tcr1   = TIMER_EN | TIMER_AR | TIMER_IRQEN;
}

void tic_isr_1()
{
	uint32_t out_state = 0;
	
	//uart_putstr("Interruption Timer 1\n");
	
	out_state = gpio0->out;
	gpio0->out = out_state & 0xFE;
	
	timer0->tcr1     = 0x00;
}

/*void tic_isr_2()
{	
	pwm_d[0] = mode;
	pwm_d[1] = mode;
	pwm_d[2] = mode;
	pwm_d[3] = mode;
	if(mode < 100)
		mode = mode + 5;
	
}*/

/***************************************************************************
 * TIMER Functions
 */
void msleep(uint32_t msec)
{
	uint32_t tcr;

	// Use timer0.1
	timer0->compare1 = (FCPU/1000)*msec;
	timer0->counter1 = 0;
	timer0->tcr1 = TIMER_EN;

	do {
		//halt();
 		tcr = timer0->tcr1;
 	} while ( ! (tcr & TIMER_TRIG) );
}

void nsleep(uint32_t nsec)
{
	uint32_t tcr;

	// Use timer0.1
	timer0->compare1 = (FCPU/1000000)*nsec;
	timer0->counter1 = 0;
	timer0->tcr1 = TIMER_EN;

	do {
		//halt();
 		tcr = timer0->tcr1;
 	} while ( ! (tcr & TIMER_TRIG) );
}


uint32_t tic_msec;

void tic_isr()
{
	tic_msec++;
	timer0->tcr0     = TIMER_EN | TIMER_AR | TIMER_IRQEN;
}

void tic_init()
{
	gpio0->out=0x0F;

	// Setup timer0.0
	timer0->compare0 = conf_per();
	timer0->counter0 = 0;
	timer0->tcr0     = TIMER_EN | TIMER_AR | TIMER_IRQEN;

	// Setup timer0.1
	timer0->compare1 = conf_util(pwm_d[0]);
	timer0->counter1 = 0;
	timer0->tcr1     = TIMER_EN | TIMER_AR | TIMER_IRQEN;

	isr_register(3, &tic_isr_0);
	//boton	
	isr_register(15, &tic_isr_1);
	
}


uint32_t conf_per() {
			return (FCPU/1000)*pwm_p;
		}

uint32_t conf_util(uint32_t percentage){
			return (FCPU/44100/100)*pwm_p*percentage;  
		}

/***************************************************************************
 * UART Functions
 */
void uart_init()
{
	//uart0->ier = 0x00;  // Interrupt Enable Register
	//uart0->lcr = 0x03;  // Line Control Register:    8N1
	//uart0->mcr = 0x00;  // Modem Control Register

	// Setup Divisor register (Fclk / Baud)
	//uart0->div = (FCPU/(57600*16));
}

char uart_getchar()
{   
	while (! (uart0->ucr & UART_DR)) ;
	return uart0->rxtx;
}

void uart_putchar(char c)
{
	while (uart0->ucr & UART_BUSY) ;
	uart0->rxtx = c;
}

void uart_putstr(char *str)
{
	char *c = str;
	while(*c) {
		uart_putchar(*c);
		c++;
	}
}
/*************************************************************************
GPIO 
*/
void gpio_test()
{
 gpio0->out=0xA;
}

void gpio_output(char ampPWM)
{
 gpio0->out=ampPWM;
}


