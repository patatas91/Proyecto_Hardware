#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include "cabecera.h"
#include <44blib.h>
#include <44b.h>
#include "timer.h"
#include "bmp.h"
#include "lcd.h"

static CELDA cuadricula_inicial[NUM_FILAS][NUM_COLUMNAS] = { 0x9800, 0x6800,
		0x0000, 0x0000, 0x0000, 0x0000, 0x7800, 0x0000, 0x8800, 0, 0, 0, 0, 0,
		0, 0, 0x8800, 0x0000, 0x0000, 0x0000, 0x0000, 0x4800, 0x3800, 0x0000,
		0x0000, 0, 0, 0, 0, 0, 0, 0, 0x1800, 0x0000, 0x0000, 0x5800, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x1800, 0x7800, 0x6800, 0, 0, 0, 0, 0,
		0, 0, 0x2800, 0x0000, 0x0000, 0x0000, 0x9800, 0x3800, 0x0000, 0x0000,
		0x5800, 0, 0, 0, 0, 0, 0, 0, 0x7800, 0x0000, 0x8800, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0, 0x0000, 0x0000,
		0x7800, 0x0000, 0x3800, 0x2800, 0x0000, 0x4800, 0x0000, 0, 0, 0, 0, 0,
		0, 0, 0x3800, 0x8800, 0x2800, 0x1800, 0x0000, 0x5800, 0x6800, 0x0000,
		0x0000, 0, 0, 0, 0, 0, 0, 0, 0x0000, 0x4800, 0x1800, 0x0000, 0x0000,
		0x9800, 0x5800, 0x2800, 0x0000, 0, 0, 0, 0, 0, 0, 0, };

/* VARIABLES */
unsigned int filas; // filas introducidas por el usuario
unsigned int columnas; // columnas introducidas por el usuario
unsigned int valor; // valor introducido por el usuario
int celdas_vacias;     //numero de celdas aun vacias
unsigned int digito = 1;	//digitos para el 8Led
unsigned int tiempoTotal = 0;
enum {
	actualizar,
	pedir_filas,
	confirmar_filas,
	pedir_columnas,
	confirmar_columnas,
	pedir_valor,
	confirmar_valor,
	escribir_celda,
	confirmacion
} estado_juego;
//unsigned int tiempoCalculos = 0;
enum {
	MAX_CHARS = 32
};
static char cadena[MAX_CHARS] = { 0 }; // Cadena para itoa
static char cadena2[MAX_CHARS] = { 0 };
enum {
	margen_izdo_px = 54, margen_arriba_px = 15
};
unsigned int espaciado_celdas_px = 23.33;
unsigned int espaciado_candidatos_px = 3.88;
// cadenas LCD resultado
static const char derrota_str[] = { "D E R R O T A" };
static const char victoria_str[] = { "V I C T O R I A" };
static const char pulse_str[] = { "*Pulse un boton para jugar*" };
/*--- variables globales ---*/
extern unsigned int botonPulsado(void);
extern unsigned int timer2_Leer();
extern void leds_on();
extern void leds_off();
extern int getMantenido();
extern void timer2_Esperar();
extern void timer2_ResetEsperar();
extern unsigned int getContador();
extern void D8Led_symbol(int value);
extern void push_debug(int ID_evento, int auxData);
/*--- LCD ---*/
extern void Lcd_PintarLineas(void);
extern void Lcd_InitTablero();
extern unsigned int segundos();

// modifica el valor almacenado en la celda indicada
inline void
celda_poner_valor(CELDA *celdaptr, uint8_t val);

void celda_poner_valor(CELDA *celdaptr, uint8_t val) {
	if ((*celdaptr & 0x800) == 0) {
		*celdaptr = (*celdaptr & 0x0FFF) | ((val & 0x000F) << 12);
	}
}

// modifica el valor almacenado en la celda indicada
inline void
celda_poner_error(CELDA *celdaptr);

void celda_poner_error(CELDA *celdaptr) {
	*celdaptr = *celdaptr | 0x400;
}

// deshabilita el bit de error en la celda indicada
inline void
celda_quitar_error(CELDA *celdaptr);

void celda_quitar_error(CELDA *celdaptr) {
	*celdaptr = *celdaptr & 0xFBFF;
}

// lee el valor almacenado en la celda indicada
inline uint8_t
celda_leer_valor(CELDA celda);

uint8_t celda_leer_valor(CELDA celda) {
	return (celda >> 12);
}

// funcion a implementar en ARM
extern int
sudoku_recalcular_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

// funcion a implementar en Thumb
extern int
sudoku_candidatos_thumb(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila,
		uint8_t columna);

// funcion a implementar en ARM
extern int
sudoku_candidatos_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila,
		uint8_t columna);

void
sudoku9x9(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

////////////////////////////////////////////////////////////////////////////////
// dada una determinada celda encuentra los posibles valores candidatos
// y guarda el mapa de bits en la celda
// retorna false si la celda esta vacia, true si contiene un valor
int sudoku_candidatos_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila,
		uint8_t columna) {
	// iniciar candidatos
	uint16_t valor = 0;
	uint16_t candidatos = 0;
	int region_v = (fila / 3) * 3;
	int region_h = (columna / 3) * 3;

	// Ponemos a 0 los candidatos de la celda
	cuadricula[fila][columna] = cuadricula[fila][columna] & 0xFE00;

	// recorrer fila recalculando candidatos
	for (int x = 0; x < NUM_FILAS; x += 1) {
		if (x != columna) {
			valor = celda_leer_valor(cuadricula[fila][x]);
			if (valor != 0) {
				candidatos = candidatos | ((0x1) << (valor - 1));
			}
		}
	}

	// recorrer columna recalculando candidatos
	for (int y = 0; y < NUM_FILAS; y += 1) {
		if (y != fila) {
			valor = celda_leer_valor(cuadricula[y][columna]);
			if (valor != 0) {
				candidatos = candidatos | ((0x1) << (valor - 1));
			}
		}
	}

	// recorrer region recalculando candidatos
	for (int x = region_v; x < (region_v + 3); x += 1) {
		for (int y = region_h; y < (region_h + 3); y += 1) {
			if ((x != fila) | (y != columna)) {
				valor = celda_leer_valor(cuadricula[x][y]);
				if (valor != 0) {
					candidatos = candidatos | ((0x1) << (valor - 1));
				}
			}
		}
	}
	cuadricula[fila][columna] = cuadricula[fila][columna] | candidatos;
	//celda_actualizar_candidatos(&cuadricula[fila][columna], candidatos);

	/*	Se debe comprobar, al evaluar los candidatos de una celda, si finalmente su valor,
	 *	(sea Pista o valor introducido por el usuario), está o no en la lista de candidatos.
	 *	Si no está deberemos marcar el bit correspondiente de esa celda como errónea.
	 *	La función devolverá como resultado: 0 celda vacía, 1 celda llena, -1 celda llena
	 *	pero error detectado.
	 */
	valor = celda_leer_valor(cuadricula[fila][columna]);
	//Si la celda está vacia devolvemos 0
	if (valor == 0) {
		return 0;
	}
	//Si el valor introducido no está en la lista de candidatos devolvemos -1
	if (candidatos & ((0x1) << (valor - 1))) {
		celda_poner_error(&cuadricula[fila][columna]);
		return -1;
	}
	celda_quitar_error(&cuadricula[fila][columna]);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// recalcula todo el tablero (9x9)
// retorna el numero de celdas vacias
int sudoku_recalcular_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]) {
	int celdas_vacias = 0;
	int valor = 0;
	int errores = 0;
	//para cada fila
	for (int x = 0; x < NUM_FILAS; x += 1) {
		//para cada columna
		for (int y = 0; y < NUM_FILAS; y += 1) {
			//determinar candidatos
			//if(!sudoku_candidatos_thumb(cuadricula,x,y)){
			//if(!sudoku_candidatos_arm(cuadricula,x,y)){
			valor = sudoku_candidatos_c(cuadricula, x, y);
			if (valor == 0) {
				//actualizar contador de celdas vacias
				celdas_vacias++;
			}
			if (valor == -1) {
				errores--;
			}
		}
	}
	//retornar el numero de celdas vacias
	if (errores < 0) {
		return errores;
	}
	return celdas_vacias;
}

/**
 * Inicializa el tablero del juego
 */
void rellenar_cuadricula(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]) {
	// Pintamos el tablero
	Lcd_InitTablero();
	for (int x = 0; x < NUM_FILAS; x += 1) {
		//para cada columna
		for (int y = 0; y < NUM_FILAS; y += 1) {
			cuadricula[x][y] = cuadricula_inicial[x][y];
		}
	}
}

/**
 * Actualiza el tablero del juego pintando los diferentes tipos de datos
 * del sudoku
 */
void actualizarTablero(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]) {
	const int desp_table[3] = { espaciado_candidatos_px * 5,
			espaciado_candidatos_px * 3, espaciado_candidatos_px };
	uint16_t valor = 0;
	//para cada fila
	for (int x = 0; x < NUM_FILAS; x += 1) {
		//para cada columna
		for (int y = 0; y < NUM_FILAS; y += 1) {
			//limpiamos la celda en el lcd
			LcdClrRect(((y * espaciado_celdas_px) + 2) + margen_izdo_px,
					((x * espaciado_celdas_px) + 2) + margen_arriba_px,
					(((y + 1) * espaciado_celdas_px) - 2) + margen_izdo_px,
					(((x + 1) * espaciado_celdas_px) - 2) + margen_arriba_px,
					0);
			//ERROR
			if ((cuadricula[x][y] & 0x0400) != 0) {
				// Invertimos colores de la celda
				LcdClrRect(((y * espaciado_celdas_px) + 2) + margen_izdo_px,
						((x * espaciado_celdas_px) + 2) + margen_arriba_px,
						(((y + 1) * espaciado_celdas_px) - 2) + margen_izdo_px,
						(((x + 1) * espaciado_celdas_px) - 2)
								+ margen_arriba_px, 14);
				valor = celda_leer_valor(cuadricula[x][y]);
				//Si contiene un numero
				if (celda_leer_valor(cuadricula[x][y]) != 0) {
					itoa(valor, cadena);
					Lcd_DspAscII8x16(
							(((y + 1) * espaciado_celdas_px)
									- espaciado_celdas_px / 2)
									+ (margen_izdo_px - 2),
							(((x + 1) * espaciado_celdas_px)
									- espaciado_celdas_px / 2)
									+ (margen_arriba_px - 4), WHITE, cadena);
					//Si es pista la remarcamos
					if ((cuadricula[x][y] & 0x0800) != 0) {
						Lcd_Draw_Box_Pista(
								((y * espaciado_celdas_px) + 2)
										+ margen_izdo_px,
								((x * espaciado_celdas_px) + 2)
										+ margen_arriba_px,
								(((y + 1) * espaciado_celdas_px) - 2)
										+ margen_izdo_px,
								(((x + 1) * espaciado_celdas_px) - 2)
										+ margen_arriba_px, 0);

					}
				}
				//Candidatos
				else {
					// limpiar hueco
					LcdClrRect(((y * espaciado_celdas_px) + 2) + margen_izdo_px,
							((x * espaciado_celdas_px) + 2) + margen_arriba_px,
							(((y + 1) * espaciado_celdas_px) - 2)
									+ margen_izdo_px,
							(((x + 1) * espaciado_celdas_px) - 2)
									+ margen_arriba_px, 0);
					for (uint32_t i = 0; i < 9; i++) {
						if ((cuadricula[x][y] & (1 << i)) == 0) {
							int desp_x = desp_table[i % 3];
							int desp_y = desp_table[i / 3];
							Lcd_DspAscII8x16(
									(((y + 1) * espaciado_celdas_px)
											+ (margen_izdo_px - 4)) - desp_x,
									(((x + 1) * espaciado_celdas_px)
											+ (margen_arriba_px - 10)) - desp_y,
									BLACK, "o");
						}
					}

				}

			}
			// NO ERROR
			else {
				//Si contiene un numero
				if (celda_leer_valor(cuadricula[x][y]) != 0) {
					valor = celda_leer_valor(cuadricula[x][y]);
					itoa(valor, cadena);
					Lcd_DspAscII8x16(
							(((y + 1) * espaciado_celdas_px)
									- espaciado_celdas_px / 2)
									+ (margen_izdo_px - 2),
							(((x + 1) * espaciado_celdas_px)
									- espaciado_celdas_px / 2)
									+ (margen_arriba_px - 4), BLACK, cadena);
					//Si es pista la remarcamos
					if ((cuadricula[x][y] & 0x0800) != 0) {
						Lcd_Draw_Box_Pista(
								((y * espaciado_celdas_px) + 2)
										+ margen_izdo_px,
								((x * espaciado_celdas_px) + 2)
										+ margen_arriba_px,
								(((y + 1) * espaciado_celdas_px) - 2)
										+ margen_izdo_px,
								(((x + 1) * espaciado_celdas_px) - 2)
										+ margen_arriba_px, 14);

					}

				}
				//Candidatos
				else {
					for (uint32_t i = 0; i < 9; i++) {
						if ((cuadricula[x][y] & (1 << i)) == 0) {
							int desp_x = desp_table[i % 3];
							int desp_y = desp_table[i / 3];
							Lcd_DspAscII8x16(
									(((y + 1) * espaciado_celdas_px)
											+ (margen_izdo_px - 4)) - desp_x,
									(((x + 1) * espaciado_celdas_px)
											+ (margen_arriba_px - 10)) - desp_y,
									BLACK, "o");
						}
					}

				}
			}

			Lcd_Dma_Trans();

		}
	}
	// Marcamos las lineas
	Lcd_PintarLineas();
}


/**
 * Transforma un entero en una cadena de caracteres para poder mostrar
 * enteros en el LCD
 */
void itoa(int n, char* nF) {
	int i = 0;
	if (n == 0) {
		nF[i++] = '0';
	}
	while (n > 0) {
		nF[i++] = '0' + n % 10;
		n /= 10;
	}
	nF[i] = '\0';
}

/**
 *
 */
void pintarSeg() {
	char salida2[32] = { 0 };
	int largo, i, x;
	// Tiempo total
	LcdClrRect(105, 0, 146, 12, 0); // limpiamos total lcd
	tiempoTotal = (timer2_Leer() / 1000);
	itoa(tiempoTotal, cadena);
	// invertimos la cadena
	largo = strlen(cadena) - 1;
	for (i = largo, x = 0; i >= 0; i--, x++)
		salida2[x] = cadena[i];
	Lcd_DspAscII8x16(105, 0, BLACK, salida2); // TOTAL
	Lcd_Dma_Trans();
}

////////////////////////////////////////////////////////////////////////////////
// proceso principal del juego que recibe el tablero,
// y la señal de ready que indica que se han actualizado fila y columna
void sudoku9x9(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]) {
	pintarSeg();
	unsigned int fin = 0;
	int celdas_vacias;
	unsigned int tiempoCalculos = 0;
	unsigned int tiempo1;
	char salida[32] = { 0 };
	int largo, i, x;
	rellenar_cuadricula(cuadricula);
	estado_juego = actualizar;
	int digito = -1;
	unsigned int botonIzqPulsado = 0;
	unsigned int b = 0;
	//F por el 8Led
	D8Led_symbol(15);
	while (fin == 0) {
		pintarSeg();
		switch (estado_juego) {
		case actualizar: {
			// TIEMPO CALCULOS
			unsigned int tiempo = timer2_Leer();
			celdas_vacias = sudoku_recalcular_c(cuadricula);
			tiempo1 = timer2_Leer();
			tiempoCalculos += tiempo1 - tiempo;
			itoa(tiempoCalculos, cadena2);
			actualizarTablero(cuadricula);
			LcdClrRect(276, 0, 318, 12, 0); // limpiamos calculos lcd

			//push_debug(1, tiempoCalculos);

			// invertimos la cadena
			largo = strlen(cadena2) - 1;
			for (i = largo, x = 0; i >= 0; i--, x++)
				salida[x] = cadena2[i];
			Lcd_DspAscII8x16(278, 0, BLACK, salida); // CALCULOS
			Lcd_Dma_Trans();
			leds_off();
			// Comprobamos si hemos finalizado
			if (celdas_vacias == 0) {
				// Mostramos el resultado -> Partida completada por el usuario
				LcdClrRect(5, 80, 312, 175, 14); // Cuadro para el resultado
				LcdClrRect(15, 90, 302, 165, 0); // Cuadro para el resultado
				Lcd_DspAscII8x16(100, 110, BLACK, victoria_str);
				Lcd_DspAscII8x16(50, 120, BLACK, pulse_str);
				Lcd_Dma_Trans();
				fin = 1;
			}
			//F por el 8Led
			D8Led_symbol(15);
			estado_juego = pedir_filas;
			break;
		}
		case pedir_filas: {
			b = botonPulsado();
			if (b == 1) {
				//D8Led_symbol(digito+1);
				botonIzqPulsado = 1;
				if ((rPDATG & 0xC0) != 0xC0) {
					/* PULSACION CORTA*/
					// boton soltado
					digito++;
					digito = digito % 10;
					D8Led_symbol(digito+1);
					// Pulsacion larga
					while (getMantenido() == 1) {
						// indicamos al timer que inicie los 300 ms
						timer2_Esperar();
						if (getContador() == 0) {
							digito++;
							digito = (digito % 10);
							D8Led_symbol(digito+1);
							//deshabilitamos la interrupcion al timer y reseteamos contador
							timer2_ResetEsperar();
						}
					}
					//deshabilitamos la interrupcion al timer y reseteamos contador
					timer2_ResetEsperar();
				}
			} else if (b == 2) {
				if (botonIzqPulsado == 1) {
					botonIzqPulsado == 0;
					estado_juego = confirmar_filas;
				}
			}
			break;
		}
		case confirmar_filas: {
			filas = digito;
			// Comprobamos si fila A
			if (filas == 9) {
				// Leds encendidos
				leds_on();
				//informamos de las opciones
				LcdClrRect(5, 80, 312, 175, 14); // Cuadro para el resultado
				LcdClrRect(15, 90, 302, 165, 0); // Cuadro para el resultado
				Lcd_DspAscII8x16(50, 120, BLACK, "IZQ -> NO        DCHO -> SI");
				Lcd_Dma_Trans();
				// A por el 8Led
				D8Led_symbol(10);
				estado_juego = confirmacion;
			} else {
				//C por el 8Led
				D8Led_symbol(12);
				digito = -1;
				estado_juego = pedir_columnas;
			}
			break;
		}
		case pedir_columnas: {
			b = botonPulsado();
			if (b == 1) {
				botonIzqPulsado = 1;
				if ((rPDATG & 0xC0) != 0xC0) {
					/* PULSACION CORTA*/
					// boton soltado
					digito++;
					digito = digito % 9;
					D8Led_symbol(digito+1);
					// Pulsacion larga
					while (getMantenido() == 1) {
						// indicamos al timer que inicie los 300 ms
						timer2_Esperar();
						if (getContador() == 0) {
							digito++;
							digito = (digito % 9);
							D8Led_symbol(digito+1);
							//deshabilitamos la interrupcion al timer y reseteamos contador
							timer2_ResetEsperar();
						}
					}
					//deshabilitamos la interrupcion al timer y reseteamos contador
					timer2_ResetEsperar();
				}
			} else if (b == 2) {
				if (botonIzqPulsado == 1) {
					botonIzqPulsado == 0;
					estado_juego = confirmar_columnas;
				}
			}
			break;
		}
		case confirmar_columnas: {
			columnas = digito;
			//b por el 8Led
			D8Led_symbol(11);
			digito = 0;
			estado_juego = pedir_valor;
			break;
		}
		case pedir_valor: {
			b = botonPulsado();
			if (b == 1) {
				botonIzqPulsado = 1;
				if ((rPDATG & 0xC0) != 0xC0) {
					/* PULSACION CORTA*/
					// boton soltado
					digito++;
					digito = digito % 10;
					D8Led_symbol(digito);
					// Pulsacion larga
					while (getMantenido() == 1) {
						// indicamos al timer que inicie los 300 ms
						timer2_Esperar();
						if (getContador() == 0) {
							digito++;
							digito = (digito % 10);
							D8Led_symbol(digito);
							//deshabilitamos la interrupcion al timer y reseteamos contador
							timer2_ResetEsperar();
						}
					}
					//deshabilitamos la interrupcion al timer y reseteamos contador
					timer2_ResetEsperar();
				}
			} else if (b == 2) {
				if (botonIzqPulsado == 1) {
					botonIzqPulsado == 0;
					estado_juego = confirmar_valor;
				}
			}
			break;
		}
		case confirmar_valor: {
			valor = digito;
			//a por el 8Led
			D8Led_symbol(10);
			digito = -1;
			estado_juego = escribir_celda;
			break;
		}
		case escribir_celda: {
			celda_poner_valor(&(cuadricula[filas][columnas]), valor);
			estado_juego = actualizar;
			break;
		}
		case confirmacion: {
			b = botonPulsado();
			if (b == 1) {
				// NO ACABAR
				// Apagamos los leds
				leds_off();
				//quitamos el cuadro
				LcdClrRect(5, 80, 312, 175, 0);
				actualizarTablero(cuadricula);
				digito = 0;
				//F por el 8Led
				D8Led_symbol(15);
				estado_juego = pedir_filas;
			} else if (b == 2) {
				// ACABAR
				// Mostramos el resultado -> Partida finalizada por el usuario
				LcdClrRect(5, 80, 312, 175, 14); // Cuadro para el resultado
				LcdClrRect(15, 90, 302, 165, 0); // Cuadro para el resultado
				Lcd_DspAscII8x16(100, 110, BLACK, derrota_str);
				Lcd_DspAscII8x16(50, 120, BLACK, pulse_str);
				Lcd_Dma_Trans();
				leds_off();
				fin = 1;
			}
			break;
		}
		}
	}
}

