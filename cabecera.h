
#ifndef CABECERA_H
#define CABECERA_H
#include <inttypes.h>
// Tamaños de la cuadricula
// Se utilizan 16 columnas para facilitar la visualización
enum {NUM_FILAS = 9, NUM_COLUMNAS = 16};

// Definiciones para valores muy utilizados
enum {FALSE = 0, TRUE = 1};

// La información de cada celda está codificada en 16 bits
// con el siguiente formato (empezando en el bit más significativo):
// 4 MSB VALOR
// 1 bit PISTA
// 1 bit ERROR
// 1 bit no usado
// 9 LSB CANDIDATOS
typedef uint16_t CELDA;

#endif
