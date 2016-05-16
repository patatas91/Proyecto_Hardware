
#ifndef CABECERA_H
#define CABECERA_H
#include <inttypes.h>
// Tama�os de la cuadricula
// Se utilizan 16 columnas para facilitar la visualizaci�n
enum {NUM_FILAS = 9, NUM_COLUMNAS = 16};

// Definiciones para valores muy utilizados
enum {FALSE = 0, TRUE = 1};

// La informaci�n de cada celda est� codificada en 16 bits
// con el siguiente formato (empezando en el bit m�s significativo):
// 4 MSB VALOR
// 1 bit PISTA
// 1 bit ERROR
// 1 bit no usado
// 9 LSB CANDIDATOS
typedef uint16_t CELDA;

#endif
