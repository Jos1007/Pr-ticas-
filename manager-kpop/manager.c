#include <stdio.h>
#include <string.h>

/* 
   HU-01: Definir la estructura Idol
   Criterios: typedef struct, nombre[30], popularidad, energia, energia_max, fans.
*/
typedef struct {
    char nombre[30];
    int popularidad;
    int energia;
    int energia_max;
    int fans;
} Idol;

/* 
   HU-03: Mostrar una integrante
   Criterios: const Idol *i, uso del operador flecha (->).
   Si se intenta modificar un campo dentro de esta función, dará error de compilación.
*/
void mostrar(const Idol *i) {
    if (i == NULL) return; // Buena práctica de seguridad contra NULL
    printf("--- DATOS DE LA IDOL ---\n");
    printf("Nombre:      %s\n", i->nombre);
    printf("Popularidad: %d\n", i->popularidad);
    printf("Energia:     %d / %d\n", i->energia, i->energia_max);
    printf("Fans:        %d\n", i->fans);
    printf("-----------------------\n\n");
}

/* 
   HU-04: Limitar un valor a un rango
   Criterios: Devuelve mínimo si valor < mínimo, máximo si valor > máximo, o valor.
   No usa variables globales.
*/
int limitar(int valor, int minimo, int maximo) {
    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}

/* 
   HU-05: Ensayar para ganar popularidad
   Criterios: +5 popularidad, -20 energía usando limitar.
   Parámetro Idol *i sin const para modificar el struct original.
*/
void ensayar(Idol *i) {
    if (i == NULL) return;
    i->popularidad += 5;
    i->energia = limitar(i->energia - 20, 0, i->energia_max);
}

/* 
   HU-06: Descansar para recuperar energía
   Criterios: Recupera +30 energía usando limitar entre 0 y energia_max.
*/
void descansar(Idol *i) {
    if (i == NULL) return;
    i->energia = limitar(i->energia + 30, 0, i->energia_max);
}

/* 
   HU-07: Saber si una integrante está activa
   Criterios: const Idol *i (solo lectura).
   Devuelve 1 si energia > 0, devuelve 0 si energia es 0.
*/
int esta_activa(const Idol *i) {
    if (i == NULL) return 0;
    return (i->energia > 0) ? 1 : 0;
}

/* 
   HU-08 (Demostración): Paso por valor
   Recibe el struct por valor (se genera una copia local).
   Los cambios realizados dentro NO afectan a la variable original de main.
*/
void ensayar_por_valor_demo(Idol i) {
    i.popularidad += 100;
    i.energia = 0;
    printf("[Demo por valor] Dentro de la funcion: Popularidad = %d, Energia = %d\n", 
           i.popularidad, i.energia);
}

int main(void) {
    printf("=== SPRINT 1: MANAGER DE UN GRUPO DE K-POP ===\n\n");

    /* ----------------------------------------------------
       HU-01: Crear jennie con valores iniciales
       ---------------------------------------------------- */
    Idol jennie = {"jennie", 50, 100, 100, 12000};


    /* ----------------------------------------------------
       HU-02: Leer campos con punto, desreferencia y flecha
       ---------------------------------------------------- */
    printf("--- HU-02: Pruebas de acceso y direcciones %p ---\n", (void*)0);
    Idol *p = &jennie;

    // Tres formas de acceder a los fans
    printf("1. jennie.fans:   %d\n", jennie.fans);
    printf("2. (*p).fans:     %d\n", (*p).fans);
    printf("3. p->fans:       %d\n", p->fans);

    // Comparar direcciones con %p cast a (void*)
    printf("Direccion &jennie: %p\n", (void*)&jennie);
    printf("Direccion p:       %p\n\n", (void*)p);


    /* ----------------------------------------------------
       HU-03: Mostrar integrante
       ---------------------------------------------------- */
    printf("--- HU-03: Mostrar estado inicial ---\n");
    mostrar(&jennie);


    /* ----------------------------------------------------
       HU-04: Probar la función limitar
       ---------------------------------------------------- */
    printf("--- HU-04: Pruebas de limitar() ---\n");
    printf("limitar(-5, 0, 100)   = %d (Esperado: 0)\n", limitar(-5, 0, 100));
    printf("limitar(150, 0, 100)  = %d (Esperado: 100)\n", limitar(150, 0, 100));
    printf("limitar(40, 0, 100)   = %d (Esperado: 40)\n\n", limitar(40, 0, 100));


    /* ----------------------------------------------------
       HU-05 & HU-06: Pruebas de ensayar, descansar y límites
       ---------------------------------------------------- */
    printf("--- HU-05: Ensayando una vez ---\n");
    ensayar(&jennie);
    mostrar(&jennie);

    printf("--- Prueba de límite inferior (20 ensayos seguidos) ---\n");
    for (int k = 0; k < 20; k++) {
        ensayar(&jennie);
    }
    printf("Energia final tras 20 ensayos: %d (No debe bajar de 0)\n", jennie.energia);
    mostrar(&jennie);

    printf("--- HU-06: Descansando (20 descansos seguidos) ---\n");
    for (int k = 0; k < 20; k++) {
        descansar(&jennie);
    }
    printf("Energia final tras 20 descansos: %d (No debe pasar de energia_max: %d)\n", 
           jennie.energia, jennie.energia_max);
    mostrar(&jennie);


    /* ----------------------------------------------------
       HU-07: Saber si la integrante está activa
       ---------------------------------------------------- */
    printf("--- HU-07: Comprobar si esta activa ---\n");
    printf("Con energia %d -> esta_activa(&jennie) = %d (Esperado: 1)\n", 
           jennie.energia, esta_activa(&jennie));

    // Forzar energía a 0 para probar
    jennie.energia = 0;
    printf("Con energia %d -> esta_activa(&jennie) = %d (Esperado: 0)\n\n", 
           jennie.energia, esta_activa(&jennie));

    // Restaurar energía
    descansar(&jennie);


    /* ----------------------------------------------------
       HU-08: Demostración Paso por Valor vs. Paso por Apuntador
       ---------------------------------------------------- */
    printf("--- HU-08: Demostracion Paso por Valor vs Apuntador ---\n");
    printf("Estado antes de llamar a la funcion por valor:\n");
    printf("Jennie -> Popularidad: %d, Energia: %d\n", jennie.popularidad, jennie.energia);

    // Llamada por valor (se le pasa 'jennie' en vez de '&jennie')
    ensayar_por_valor_demo(jennie);

    printf("Estado DESPUES de la funcion por valor (en main):\n");
    printf("Jennie -> Popularidad: %d, Energia: %d\n", jennie.popularidad, jennie.energia);
    printf("(Como ves, el struct original en main NUNCA cambio).\n\n");

    return 0;
}