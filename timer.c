/*********************************************************************************************
* Fichero:		timer.c
* Autor:
* Descrip:		funciones de control del timer0 del s3c44b0x
* Version:
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"
#include "timer.h"

/*--- variables globales ---*/
int switch_leds = 0;
unsigned int timer2_num_int = 0; // Cuenta de los periodos completos

/*--- variables rebotes ---*/
int Eint4567_signal = 0; // Señal para indicar al timer que debe gestionar rebotes
int Esperar_signal = 0;
volatile tp_estado_boton estado_boton;
int contador = 300;
int miliseg = 0;
int seg = 0;
extern void actualizar_estado_boton_desde_timer_int(void);

/*--- declaracion de funciones ---*/
void timer_ISR(void) __attribute__((interrupt("IRQ")));
void timer2_ISR(void) __attribute__((interrupt("IRQ")));
void timer_init(void);
void timer2_Inicializar(void);
void timer2_Empezar(void);
unsigned int timer2_Leer();
void timer2_Esperar(void);
void timer2_Eint4567(void);
void timer2_ResetEsperar(void);
unsigned int getContador();
unsigned int segundos();

//extern void push_debug(int ID_evento, int auxData);

/*--- codigo de las funciones ---*/
void timer_ISR(void)
{
	switch_leds = 1;

	/* borrar bit en I_ISPC para desactivar la solicitud de interrupción*/
	rI_ISPC |= BIT_TIMER0; // BIT_TIMER0 está definido en 44b.h y pone un uno en el bit 13 que correponde al Timer0
}

void timer_init(void)
{
	/* Configuraion controlador de interrupciones */
	rINTMOD = 0x0; // Configura las linas como de tipo IRQ
	rINTCON = 0x1; // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK = ~(BIT_GLOBAL | BIT_TIMER0); // Emascara todas las lineas excepto Timer0 y el bit global (bits 26 y 13, BIT_GLOBAL y BIT_TIMER0 están definidos en 44b.h)

	/* Establece la rutina de servicio para TIMER0 */
	pISR_TIMER0 = (unsigned) timer_ISR;

	/* Configura el Timer0 */
	rTCFG0 = 255; // ajusta el preescalado
	rTCFG1 = 0x0; // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.
	rTCNTB0 = 65535;// valor inicial de cuenta (la cuenta es descendente)
	rTCMPB0 = 12800;// valor de comparación
	/* establecer update=manual (bit 1) + inverter=on (¿? será inverter off un cero en el bit 2 pone el inverter en off)*/
	rTCON = 0x2; // [1]
	/* iniciar timer (bit 0) con auto-reload (bit 3)*/
	rTCON = 0x09; // [0,3]
}

/*--- VALORES INICIALES ---*/
void timer2_Inicializar(void)
{
	/* Configuracion controlador de interrupciones */
	rINTMOD = 0x0; // Configura las linas como de tipo IRQ
	rINTCON = 0x1; // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK = rINTMSK & ~(BIT_TIMER2); // Emascara todas las lineas excepto Timer2 y el bit global (bits 26 y 13, BIT_GLOBAL y BIT_TIMER2 están definidos en 44b.h)

	/* Establece la rutina de servicio para TIMER2 */
	pISR_TIMER2 = (unsigned) timer2_ISR;

	/* Configura el Timer2 */
	// F = MCLK/ ((valor de pre-escalado + 1)(valor del divisor))
	rTCFG0 &= 0xFFFF00FF; // ajusta el preescalado
	rTCFG1 &= 0xFFFF0FF; // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.
	rTCNTB2 = 33333; // valor inicial de cuenta (la cuenta es descendente) 32x10^3
	rTCMPB2 = 0x0; // valor de comparación
}

/*--- INICIO DEL TIMER2:  ---*/
void timer2_Empezar(void)
{
	timer2_num_int = 0; // inicializar contador periodos
	// Activar el manual update bit del temporizador y start a 0
	rTCON &= 0xFFF6FFF;
	rTCON |= 0x2000; // [13]
	// Activar el bit de comienzo (start bit) con autoreload al mismo tiempo que se desactiva el manual update bit
	rTCON ^= 0x000B000; // activar 12, 15 y bajar 13
}

/*--- LECTURA DEL TIMER2:  ---*/
unsigned int timer2_Leer(void) //devuelve el tiempo en milisegundos
{
	unsigned int actual, parcial, total;
	total = timer2_num_int;
	return total; // miliseg
}

/*--- RUTINA SERVICIO TIMER2 ---*/
void timer2_ISR(void)
{
	// Cada vez que se produce una interrupcion aumentamos el contador
	timer2_num_int++;

	// Si esta activa la gestion de rebotes la realizamos
	if(Eint4567_signal == 1) {
		actualizar_estado_boton_desde_timer_int();
		/*
		 * Comprueba si ha terminado el gestor de espera
		 */
		if(estado_boton == inicial) {
			// deshabilitamos la señal de gestion de rebotes
			Eint4567_signal = 0;
		}
	}
	if (Esperar_signal == 1) {
		contador--;
	}

	/* borrar bit en I_ISPC para desactivar la solicitud de interrupción*/
	rI_ISPC |= BIT_TIMER2; // BIT_TIMER2 está definido en 44b.h y pone un uno en el bit 11 que correponde al Timer2
}

/*--- GESTION REBOTES ---*/
// indicamos al timer que tiene que gestionar los rebotes
void timer2_Eint4567(void) {
	Eint4567_signal = 1;
}

void timer2_Esperar(void) {
	Esperar_signal = 1;
}

void timer2_ResetEsperar(void) {
	Esperar_signal = 0;
	contador = 300;
}
unsigned int getContador() {
	return contador;
}


