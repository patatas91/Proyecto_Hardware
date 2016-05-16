/*********************************************************************************************
* Fichero:	main.c
* Autor:
* Descrip:	punto de entrada de C
* Version:  <P4-ARM.timer-leds>
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "stdio.h"
#include "cabecera.h"
#include "bmp.h"

/*--- funciones externas ---*/
extern void timer_init();
extern void timer2_Inicializar();
extern void timer2_Empezar();
extern void Eint4567_init();
extern void D8Led_init();
extern void D8Led_symbol(int value);
extern void init_debugstack();
extern void sudoku9x9(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
extern int botonPulsado();
/*--- LCD ---*/
extern INT8U g_auc_Ascii8x16[];
extern void Lcd_Init();
extern void Lcd_Clr();
extern void Lcd_Active_Clr();
extern void Lcd_DspAscII8x16(INT16U x0, INT16U y0, INT8U ForeColor, INT8U * s);
extern void empezado(void);

CELDA cuadricula [NUM_FILAS][NUM_COLUMNAS] __attribute__ ((aligned (32)));

/*--- declaracion de funciones ---*/
void Main(void);
void error(void);


/*--- codigo de funciones ---*/
void error(void)
{
	while (1)
	{
		leds_on();
		D8Led_symbol(14);
	}
}

void Main(void)
{

	/* Inicializa controladores */
	sys_init();        // Inicializacion de la placa, interrupciones y puertos
	timer_init();	   // Inicializacion del temporizador
	timer2_Inicializar();
	timer2_Empezar();
	Eint4567_init();	// inicializamos los pulsadores. Cada vez que se pulse se verá reflejado en el 8led
	D8Led_init(); // inicializamos el 8led
	init_debugstack();

	/* LCD */
	Lcd_Init();
	Lcd_Clr();
	Lcd_Active_Clr();
	// Pintar autores/instrucciones
	Lcd_DspAscII8x16(28,5,BLACK,"*********************************");
	Lcd_DspAscII8x16(28,20,BLACK,"*          SUDOKU 2015          *");
	Lcd_DspAscII8x16(28,35,BLACK,"*         *************         *");
	Lcd_DspAscII8x16(28,50,BLACK,"* Fernando Aliaga Ramon 610610  *");
	Lcd_DspAscII8x16(28,65,BLACK,"* Cristian Simon Moreno 611487  *");
	Lcd_DspAscII8x16(28,80,BLACK,"*                               *");
	Lcd_DspAscII8x16(28,95,BLACK,"*********************************");
	Lcd_DspAscII8x16(28,115,BLACK,"         Instrucciones:         ");
	Lcd_DspAscII8x16(18,135,BLACK,"Use el boton izquiero para aumentar");
	Lcd_DspAscII8x16(18,150,BLACK," el numero de filas/columnas/valor,");
	Lcd_DspAscII8x16(10,165,BLACK,"pulse el boton derecho para confirmar.");
	Lcd_DspAscII8x16(18,180,BLACK,"Para finalizar introduzca la fila A");
	Lcd_DspAscII8x16(35,205,BLACK,"**Pulse un boton para empezar**");
	Lcd_Dma_Trans();



	//unsigned int tiempo = timer2_Leer();
	//Delay(1000000);
	//unsigned int tiempo2 = timer2_Leer() - tiempo;


	// Bucle infinito para iniciar partida cada vez que finalice la anterior
	while(1) {
		while (botonPulsado() == 0);
		sudoku9x9(cuadricula);
	}

}
