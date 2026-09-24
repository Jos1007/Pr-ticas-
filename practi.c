#include <stdio.h>
#include <stdbool.h>

// Constantes globales del sistema
#define NUM_EQUIPOS 4
#define NUM_TIPOS_EVENTO 4
#define TOTAL_EVENTOS 10
#define UMBRAL_X 3 // Regla de detección: Intentos fallidos > X es Sospechoso

// Codificación de eventos:
// 'C': Acceso correcto
// 'I': Contraseña incorrecta
// 'U': Usuario inexistente
// 'B': Acceso bloqueado

// ----------------------------------------------------------------------------
// 4.3 Función con PASO POR VALOR
// Recibe el carácter por valor. Valida y mapea el código a su columna de matriz.
// ----------------------------------------------------------------------------
int obtenerIndiceEvento(char tipoEvento) {
    switch (tipoEvento) {
        case 'C': return 0; // Columna 0: Acceso correcto
        case 'I': return 1; // Columna 1: Contraseña incorrecta
        case 'U': return 2; // Columna 2: Usuario inexistente
        case 'B': return 3; // Columna 3: Acceso bloqueado
        default:  return -1; // Código invalido
    }
}

// ----------------------------------------------------------------------------
// 4.4 Función con PASO POR REFERENCIA
// Recibe la dirección de memoria de un contador e incrementa su valor directamente.
// ----------------------------------------------------------------------------
void actualizarContador(int *contador) {
    if (contador != NULL) {
        (*contador)++;
    }
}

// ----------------------------------------------------------------------------
// 4.1, 4.2 y 4.5 Procesamiento con ARITMÉTICA DE APUNTADORES
// Recorre los arreglos usando apuntadores y actualiza la matriz.
// ----------------------------------------------------------------------------
void procesarRegistros(const int *ptrEquipos, const char *ptrTipos, int matriz[NUM_EQUIPOS][NUM_TIPOS_EVENTO], int totalEventos) {
    for (int i = 0; i < totalEventos; i++) {
        // Aritmética de apuntadores
        int equipo = *(ptrEquipos + i);
        char tipo = *(ptrTipos + i);

        // Paso por valor
        int col = obtenerIndiceEvento(tipo);

        // Validación y actualización por referencia
        if (equipo >= 0 && equipo < NUM_EQUIPOS && col != -1) {
            actualizarContador(&matriz[equipo][col]);
        }
    }
}

// Imprime el log de eventos unidimensional recibido
void imprimirLogEventos(const int *ptrEquipos, const char *ptrTipos, int totalEventos) {
    printf("===========================================\n");
    printf("        LOG DE EVENTOS REGISTRADOS         \n");
    printf("===========================================\n");
    printf("No.\tEquipo\t\tTipo de Evento\n");
    printf("-------------------------------------------\n");
    for (int i = 0; i < totalEventos; i++) {
        printf("%d\tPC-0%d\t\t'%c'\n", i + 1, *(ptrEquipos + i) + 1, *(ptrTipos + i));
    }
    printf("\n");
}

// ----------------------------------------------------------------------------
// 4.6 Aplicación de la Regla de Detección e Impresión de Resultados
// ----------------------------------------------------------------------------
void evaluarYSacarResumen(int (*ptrMatriz)[NUM_TIPOS_EVENTO], int numEquipos) {
    printf("===================================================================================\n");
    printf("                     RESUMEN POR EQUIPO Y EVALUACION DE SEGURIDAD                  \n");
    printf("===================================================================================\n");
    printf("Equipo\tCorrecto\tIncorr.\t\tInexist.\tBloqueado\tTotal\tEstado\n");
    printf("-----------------------------------------------------------------------------------\n");

    for (int i = 0; i < numEquipos; i++) {
        int *fila = *(ptrMatriz + i);

        int correctos   = *(fila + 0);
        int incorrectos = *(fila + 1);
        int inexist     = *(fila + 2);
        int bloqueados  = *(fila + 3);

        int totalEventos = correctos + incorrectos + inexist + bloqueados;
        int intentosFallidos = incorrectos + inexist;

        // Regla: Intentos fallidos > UMBRAL_X
        bool esSospechoso = intentosFallidos > UMBRAL_X;

        printf("PC-0%d\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t%s\n",
               i + 1,
               correctos,
               incorrectos,
               inexist,
               bloqueados,
               totalEventos,
               esSospechoso ? "[SOSPECHOSO]" : "Normal");
    }
    printf("===================================================================================\n");
    printf("Criterio: Equipos con mas de %d intentos fallidos acumulados son marcados como SOSPECHOSOS.\n\n", UMBRAL_X);
}

int main() {
    // 4.1 Arreglos unidimensionales (Historial de accesos)
    int equipos[TOTAL_EVENTOS]       = {0,   1,   0,   1,   1,   2,   1,   3,   3,   1};
    char tiposEventos[TOTAL_EVENTOS] = {'C', 'I', 'C', 'I', 'I', 'U', 'I', 'I', 'U', 'B'};

    // 4.2 Arreglo bidimensional (Matriz de resumen)
    int matrizResumen[NUM_EQUIPOS][NUM_TIPOS_EVENTO] = {0};

    // Procesamiento
    imprimirLogEventos(equipos, tiposEventos, TOTAL_EVENTOS);
    procesarRegistros(equipos, tiposEventos, matrizResumen, TOTAL_EVENTOS);
    evaluarYSacarResumen(matrizResumen, NUM_EQUIPOS);

    return 0;
}