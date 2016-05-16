/*********************************************************************************************
* Fichero:	button.c
* Autor:
* Descrip:	Funciones de manejo de los pulsadores (EINT6-7)
* Version:
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "def.h"
#include "timer.h"

/*--- variables globales ---*/
/* int_count la utilizamos para sacar un número por el 8led.
  Cuando se pulsa un botón sumamos y con el otro restamos. ¡A veces hay rebotes! */
unsigned int int_count = 0;
volatile int mantenido = 0;

/*--- variables retardos ---*/
unsigned int pulsed = 0; // indica si el boton esta pulsado
volatile tp_estado_boton estado_boton;
static unsigned int cuenta_espera = 0;
static volatile unsigned int buttonNumber = 0; // boton pulsado: 1 izq, 2 dcha

/*--- declaracion de funciones ---*/
void Eint4567_ISR(void) __attribute__((interrupt("IRQ")));
void Eint4567_init(void);
unsigned int botonPulsado(void);
void restaurarBoton (void);
extern void D8Led_symbol(int value);
void actualizar_estado_boton_desde_timer_int(void);

/*--- codigo de funciones ---*/
void Eint4567_init(void)
{
	/* Configuracion del controlador de interrupciones. Estos registros están definidos en 44b.h */
	rI_ISPC    = 0x3ffffff;	// Borra INTPND escribiendo 1s en I_ISPC
	rEXTINTPND = 0xf;       // Borra EXTINTPND escribiendo 1s en el propio registro
	rINTMOD    = 0x0;		// Configura las linas como de tipo IRQ
	rINTCON    = 0x1;	    // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK    = rINTMSK & ~(BIT_EINT4567); // Enmascara todas las lineas excepto eint4567, el bit global y el timer0

	/* Establece la rutina de servicio para Eint4567 */
	pISR_EINT4567 = (int)Eint4567_ISR;

	/* Configuracion del puerto G */
	rPCONG  = 0xffff;        		// Establece la funcion de los pines (EINT0-7)
	rPUPG   = 0x0;                  // Habilita el "pull up" del puerto
	rEXTINT = rEXTINT | 0x22222222;   // Configura las lineas de int. como de flanco de bajada

	/* Por precaucion, se vuelven a borrar los bits de INTPND y EXTINTPND */
	rI_ISPC    |= (BIT_EINT4567);
	rEXTINTPND = 0xf;
}

// ESTADO 1 / pulsacion detectada
void Eint4567_ISR(void)
{
	/* Deshabilitar las interrupciones de los pulsadores */
	rINTMSK |= BIT_EINT4567;

	/* Identificar la interrupcion (hay dos pulsadores)*/
	int which_int = rEXTINTPND;
	switch (which_int)
	{
		// Boton izquierdo
		case 4:
			buttonNumber = 1;
			//int_count++; // incrementamos el contador
			break;
		// Boton derecho
		case 8:
			buttonNumber = 2;
			//int_count--; // decrementamos el contador
			break;
		default:
			break;
	}
	//15 20
	cuenta_espera = 15;
	estado_boton = rebotes_subida;
	mantenido = 1;
	// Llamada al timer para avisarle que tiene que gestionar
	timer2_Eint4567();
	/* Finalizar ISR */
	rEXTINTPND = 0xf;				// borra los bits en EXTINTPND
	rI_ISPC   |= BIT_EINT4567;		// borra el bit pendiente en INTPND

}
/**
 * Proceso principal para la gestion de rebotes
 */
void actualizar_estado_boton_desde_timer_int() {
	switch(estado_boton) {
			case inicial: {
				break;
			}
			case rebotes_subida: {
				cuenta_espera--;
				if (cuenta_espera == 0) {
					estado_boton = leer_boton;
					cuenta_espera = 10;
					break;
				}
				break;
			}
			case leer_boton: {
				cuenta_espera--;
				if (cuenta_espera == 0) {
					if ((rPDATG & 0xC0) == 0xC0) {
						estado_boton = rebotes_bajada;
						cuenta_espera = 20;
						mantenido = 0;
					}
					else {
						cuenta_espera = 10;
					}
				}
				break;
			}
			case rebotes_bajada: {
				cuenta_espera--;
				if (cuenta_espera == 0) {
					estado_boton = habilitar_interrupciones;
					break;
				}
				break;
			}
			case habilitar_interrupciones: {
				cuenta_espera = 0;
				estado_boton = inicial;
				rEXTINTPND = 0xf;				// borra los bits en EXTINTPND
				rI_ISPC   |= BIT_EINT4567;		// borra el bit pendiente en INTPND
				// Activamos las interrupciones
				rINTMSK &= ~(BIT_EINT4567);
				//D8Led_symbol(int_count&0x000f); // sacamos el valor por pantalla (módulo 16)
				break;
			}
	}
}

/* Devuelve el boton pulsado */
unsigned int botonPulsado(void) {
	int aux = buttonNumber;
	buttonNumber = 0;
	return aux;
}

/* Devuelve el boton pulsado */
unsigned int getMantenido(void) {
	return mantenido;
}
