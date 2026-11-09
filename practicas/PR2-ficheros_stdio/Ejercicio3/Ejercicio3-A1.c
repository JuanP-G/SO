/*
 * write_records_text.c  --  Ejercicio 3, parte A.1
 *
 * Escribe un array de registros en un fichero en FORMATO TEXTO:
 *     id valor etiqueta\n      (una línea por registro)
 *
 * Uso: ./write_records_text fichero_salida
 */
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

 
#define LABEL_MAX_LEN 15 // Max chars sin contar el '\0'

typedef struct {
     int id; // Identificador entero
     double value; // Un valor de punto flotante
     char label[LABEL_MAX_LEN + 1]; // Etiqueta de texto (tamaño fijo)
} SimpleRecord;




