#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NOMBRE 30
#define MAX_HABILIDAD 30
#define MAX_EVENTO 60

/* Valores iniciales documentados del refugio (HU-04 / HU-11) */
#define INI_AGUA 100
#define INI_COMIDA 100
#define INI_MEDICAMENTOS 50
#define INI_MUNICIONES 30

/* Costo y probabilidades de expedicion (HU-08) */
#define EXP_COSTO_MUNICIONES 5
#define EXP_DANO_SALUD 20
#define EXP_GANANCIA_AGUA 20
#define EXP_GANANCIA_COMIDA 20
#define EXP_GANANCIA_MEDICAMENTOS 10
#define EXP_PROB_DESAPARECER 25 /* 25% si queda con salud <= 0 tras el dano */

/* Rutas de archivos de guardado y temporales (HU-10 / HU-11) */
#define ARCH_SUP "save_supervivientes.dat"
#define ARCH_REC "save_recursos.dat"
#define ARCH_EVT "save_eventos.dat"
#define TMP_SUP "save_supervivientes.tmp"
#define TMP_REC "save_recursos.tmp"
#define TMP_EVT "save_eventos.tmp"

typedef struct {
    char nombre[MAX_NOMBRE + 1];
    int edad;
    char habilidad[MAX_HABILIDAD + 1];
    int salud;
} Superviviente;

typedef struct {
    int agua;
    int comida;
    int medicamentos;
    int municiones;
} Recursos;

typedef struct {
    int dia;
    char descripcion[MAX_EVENTO + 1];
} Evento;

typedef struct {
    Superviviente *lista;
    int n;
    int capacidad;
    Evento *historial;
    int n_eventos;
    int cap_eventos;
    int dia_actual;
    Recursos recursos;
} Refugio;

enum {
    REC_AGUA = 1,
    REC_COMIDA = 2,
    REC_MEDICAMENTOS = 3,
    REC_MUNICIONES = 4
};

/* Helpers de E/S segura */
static void limpiar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

static void leer_cadena(char *dest, int tam, const char *msg) {
    printf("%s", msg);
    if (fgets(dest, tam, stdin) == NULL) {
        dest[0] = '\0';
        return;
    }
    size_t len = strlen(dest);
    if (len > 0 && dest[len - 1] == '\n') {
        dest[len - 1] = '\0';
    } else {
        limpiar_buffer();
    }
}

static int leer_entero(const char *msg) {
    char buf[64];
    int val;
    char extra;
    while (1) {
        printf("%s", msg);
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            return 0;
        }
        if (sscanf(buf, " %d %c", &val, &extra) == 1) {
            return val;
        }
        printf("Entrada invalida. Ingrese un numero entero.\n");
    }
}

static void normalizar_recursos_no_negativos(Recursos *rec) {
    if (rec == NULL) {
        return;
    }
    if (rec->agua < 0) rec->agua = 0;
    if (rec->comida < 0) rec->comida = 0;
    if (rec->medicamentos < 0) rec->medicamentos = 0;
    if (rec->municiones < 0) rec->municiones = 0;
}

/* HU-01 / HU-04: Inicializacion sin variables globales */
void inicializar_refugio(Refugio *r) {
    if (r == NULL) {
        return;
    }
    r->lista = NULL;
    r->n = 0;
    r->capacidad = 0;
    r->historial = NULL;
    r->n_eventos = 0;
    r->cap_eventos = 0;
    r->dia_actual = 1;
    r->recursos.agua = INI_AGUA;
    r->recursos.comida = INI_COMIDA;
    r->recursos.medicamentos = INI_MEDICAMENTOS;
    r->recursos.municiones = INI_MUNICIONES;
}

/* HU-06: Registrar evento en historial dinamico */
int registrar_evento(Refugio *r, const char *texto) {
    if (r == NULL || texto == NULL || texto[0] == '\0') {
        return 0;
    }

    if (r->n_eventos == r->cap_eventos) {
        int nueva_cap = (r->cap_eventos == 0) ? 8 : r->cap_eventos * 2;
        Evento *aux = (r->cap_eventos == 0)
            ? malloc((size_t)nueva_cap * sizeof(Evento))
            : realloc(r->historial, (size_t)nueva_cap * sizeof(Evento));
        if (aux == NULL) {
            printf("Error de memoria al registrar evento.\n");
            return 0;
        }
        r->historial = aux;
        r->cap_eventos = nueva_cap;
    }

    Evento ev;
    ev.dia = r->dia_actual;
    snprintf(ev.descripcion, sizeof(ev.descripcion), "%s", texto);

    r->historial[r->n_eventos] = ev;
    r->n_eventos++;
    return 1;
}

void mostrar_historial(const Refugio *r) {
    if (r == NULL || r->n_eventos == 0) {
        printf("No hay acontecimientos registrados.\n");
        return;
    }
    printf("--- HISTORIAL DE ACONTECIMIENTOS ---\n");
    for (int i = 0; i < r->n_eventos; i++) {
        printf("Dia %d: %s\n", r->historial[i].dia, r->historial[i].descripcion);
    }
}

/* Helper interno para agregar superviviente programaticamente o desde menu */
static int insertar_superviviente_datos(Refugio *r, const char *nombre, int edad,
                                        const char *habilidad, int salud) {
    if (r == NULL || nombre == NULL || nombre[0] == '\0') {
        return 0;
    }
    if (edad <= 0 || habilidad == NULL || habilidad[0] == '\0') {
        return 0;
    }
    if (salud < 0 || salud > 100) {
        return 0;
    }

    if (r->n == r->capacidad) {
        int nueva_cap = (r->capacidad == 0) ? 4 : r->capacidad * 2;
        Superviviente *aux = (r->capacidad == 0)
            ? malloc((size_t)nueva_cap * sizeof(Superviviente))
            : realloc(r->lista, (size_t)nueva_cap * sizeof(Superviviente));
        if (aux == NULL) {
            printf("Error de memoria: se conservan los registros anteriores.\n");
            return 0;
        }
        r->lista = aux;
        r->capacidad = nueva_cap;
    }

    Superviviente nuevo;
    snprintf(nuevo.nombre, sizeof(nuevo.nombre), "%s", nombre);
    nuevo.edad = edad;
    snprintf(nuevo.habilidad, sizeof(nuevo.habilidad), "%s", habilidad);
    nuevo.salud = salud;

    r->lista[r->n] = nuevo;
    r->n++;
    return 1;
}

/* HU-02: Registrar superviviente con validacion y memoria dinamica */
int agregar_superviviente(Refugio *r) {
    if (r == NULL) {
        return 0;
    }

    char nombre[MAX_NOMBRE + 1];
    char habilidad[MAX_HABILIDAD + 1];
    leer_cadena(nombre, sizeof(nombre), "Nombre: ");
    if (nombre[0] == '\0') {
        printf("Error: el nombre no puede estar vacio.\n");
        return 0;
    }

    int edad = leer_entero("Edad: ");
    if (edad <= 0) {
        printf("Error: la edad debe ser mayor que 0.\n");
        return 0;
    }

    leer_cadena(habilidad, sizeof(habilidad), "Habilidad especial: ");
    if (habilidad[0] == '\0') {
        printf("Error: la habilidad especial no puede estar vacia.\n");
        return 0;
    }

    int salud = leer_entero("Salud (0-100): ");
    if (salud < 0 || salud > 100) {
        printf("Error: la salud debe estar entre 0 y 100.\n");
        return 0;
    }

    if (!insertar_superviviente_datos(r, nombre, edad, habilidad, salud)) {
        return 0;
    }

    char msg[MAX_EVENTO + 1];
    snprintf(msg, sizeof(msg), "Ingreso de superviviente: %.30s", nombre);
    registrar_evento(r, msg);
    return 1;
}

/* HU-03: Listar supervivientes */
void listar_supervivientes(const Refugio *r) {
    if (r == NULL || r->n == 0) {
        printf("No hay supervivientes registrados.\n");
        return;
    }
    for (int i = 0; i < r->n; i++) {
        printf("%d. %s | Edad: %d | Habilidad: %s | Salud: %d\n",
               i + 1,
               r->lista[i].nombre,
               r->lista[i].edad,
               r->lista[i].habilidad,
               r->lista[i].salud);
    }
}

/* HU-04: Mostrar recursos */
void mostrar_recursos(const Refugio *r) {
    if (r == NULL) {
        return;
    }
    printf("Agua: %d\nComida: %d\nMedicamentos: %d\nMuniciones: %d\n",
           r->recursos.agua,
           r->recursos.comida,
           r->recursos.medicamentos,
           r->recursos.municiones);
}

/* HU-05: Entradas y salidas validadas (cantidad > 0 entrada, < 0 salida) */
int modificar_recurso(Refugio *r, int tipo, int cantidad) {
    if (r == NULL || cantidad == 0) {
        return 0;
    }

    int *dest = NULL;
    if (tipo == REC_AGUA) dest = &r->recursos.agua;
    else if (tipo == REC_COMIDA) dest = &r->recursos.comida;
    else if (tipo == REC_MEDICAMENTOS) dest = &r->recursos.medicamentos;
    else if (tipo == REC_MUNICIONES) dest = &r->recursos.municiones;
    else return 0;

    if (cantidad < 0 && (-cantidad) > *dest) {
        return 0;
    }

    *dest += cantidad;
    return 1;
}

/* HU-07: Dar de baja por nombre recorriendo elementos */
int dar_de_baja(Refugio *r, const char *nombre) {
    if (r == NULL || nombre == NULL || nombre[0] == '\0' || r->n == 0) {
        printf("No se puede dar de baja: lista vacia o nombre invalido.\n");
        return 0;
    }

    int idx = -1;
    for (int i = 0; i < r->n; i++) {
        if (strcmp(r->lista[i].nombre, nombre) == 0) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("Superviviente '%s' no encontrado. No se modifico nada.\n", nombre);
        return 0;
    }

    char eliminado[MAX_NOMBRE + 1];
    snprintf(eliminado, sizeof(eliminado), "%s", r->lista[idx].nombre);

    for (int i = idx; i < r->n - 1; i++) {
        r->lista[i] = r->lista[i + 1];
    }
    r->n--;

    if (r->n == 0) {
        free(r->lista);
        r->lista = NULL;
        r->capacidad = 0;
    } else if (r->capacidad > 4 && r->n <= r->capacidad / 4) {
        int nueva_cap = r->capacidad / 2;
        if (nueva_cap < 4) nueva_cap = 4;
        Superviviente *aux = realloc(r->lista, (size_t)nueva_cap * sizeof(Superviviente));
        if (aux != NULL) {
            r->lista = aux;
            r->capacidad = nueva_cap;
        }
    }

    char msg[MAX_EVENTO + 1];
    snprintf(msg, sizeof(msg), "Baja de superviviente: %.30s", eliminado);
    registrar_evento(r, msg);
    return 1;
}

/* HU-09: Perdida de recursos (nunca negativa, registra evento) */
void perder_recurso(Refugio *r, int tipo, int cantidad) {
    if (r == NULL || cantidad <= 0) {
        return;
    }

    int *dest = NULL;
    const char *nombre_rec = NULL;
    if (tipo == REC_AGUA) { dest = &r->recursos.agua; nombre_rec = "agua"; }
    else if (tipo == REC_COMIDA) { dest = &r->recursos.comida; nombre_rec = "comida"; }
    else if (tipo == REC_MEDICAMENTOS) { dest = &r->recursos.medicamentos; nombre_rec = "medicamentos"; }
    else if (tipo == REC_MUNICIONES) { dest = &r->recursos.municiones; nombre_rec = "municiones"; }
    else return;

    int perdida_real = (cantidad >= *dest) ? *dest : cantidad;
    *dest -= perdida_real;
    if (*dest < 0) {
        *dest = 0;
    }

    char msg[MAX_EVENTO + 1];
    snprintf(msg, sizeof(msg), "Perdida de %d de %s", perdida_real, nombre_rec);
    registrar_evento(r, msg);
}

/* HU-08: Simular expedicion */
int expedicion(Refugio *r) {
    if (r == NULL) {
        return 0;
    }

    int candidato = -1;
    for (int i = 0; i < r->n; i++) {
        if (r->lista[i].salud > 0) {
            candidato = i;
            break;
        }
    }

    if (candidato == -1) {
        printf("No hay ningun superviviente con salud mayor que 0 para salir.\n");
        return 0;
    }

    if (!modificar_recurso(r, REC_MUNICIONES, -EXP_COSTO_MUNICIONES)) {
        printf("No alcanzan las municiones (%d requeridas). La expedicion no sale.\n",
               EXP_COSTO_MUNICIONES);
        return 0;
    }

    modificar_recurso(r, REC_AGUA, EXP_GANANCIA_AGUA);
    modificar_recurso(r, REC_COMIDA, EXP_GANANCIA_COMIDA);
    modificar_recurso(r, REC_MEDICAMENTOS, EXP_GANANCIA_MEDICAMENTOS);

    char nombre_part[MAX_NOMBRE + 1];
    snprintf(nombre_part, sizeof(nombre_part), "%s", r->lista[candidato].nombre);

    r->lista[candidato].salud -= EXP_DANO_SALUD;
    if (r->lista[candidato].salud < 0) {
        r->lista[candidato].salud = 0;
    }

    char msg[MAX_EVENTO + 1];
    if (r->lista[candidato].salud == 0 && (rand() % 100) < EXP_PROB_DESAPARECER) {
        snprintf(msg, sizeof(msg), "Expedicion: %.20s desaparecio", nombre_part);
        registrar_evento(r, msg);
        dar_de_baja(r, nombre_part);
    } else {
        snprintf(msg, sizeof(msg), "Expedicion exitosa con %.25s", nombre_part);
        registrar_evento(r, msg);
    }

    return 1;
}

/* HU-10: Guardar partida de forma segura (temporales + rename) */
static void limpiar_temporales(void) {
    remove(TMP_SUP);
    remove(TMP_REC);
    remove(TMP_EVT);
}

int guardar_partida(const Refugio *r) {
    if (r == NULL) {
        return 0;
    }

    FILE *f_sup = fopen(TMP_SUP, "wb");
    if (f_sup == NULL) {
        printf("Error al abrir archivo temporal de supervivientes.\n");
        return 0;
    }
    if (fwrite(&r->n, sizeof(int), 1, f_sup) != 1 ||
        (r->n > 0 && fwrite(r->lista, sizeof(Superviviente), (size_t)r->n, f_sup) != (size_t)r->n) ||
        fclose(f_sup) != 0) {
        printf("Error al escribir/cerrar archivo temporal de supervivientes.\n");
        if (f_sup != NULL) fclose(f_sup);
        limpiar_temporales();
        return 0;
    }

    FILE *f_rec = fopen(TMP_REC, "wb");
    if (f_rec == NULL) {
        printf("Error al abrir archivo temporal de recursos.\n");
        limpiar_temporales();
        return 0;
    }
    if (fwrite(&r->dia_actual, sizeof(int), 1, f_rec) != 1 ||
        fwrite(&r->recursos, sizeof(Recursos), 1, f_rec) != 1 ||
        fclose(f_rec) != 0) {
        printf("Error al escribir/cerrar archivo temporal de recursos.\n");
        limpiar_temporales();
        return 0;
    }

    FILE *f_evt = fopen(TMP_EVT, "wb");
    if (f_evt == NULL) {
        printf("Error al abrir archivo temporal de eventos.\n");
        limpiar_temporales();
        return 0;
    }
    if (fwrite(&r->n_eventos, sizeof(int), 1, f_evt) != 1 ||
        (r->n_eventos > 0 && fwrite(r->historial, sizeof(Evento), (size_t)r->n_eventos, f_evt) != (size_t)r->n_eventos) ||
        fclose(f_evt) != 0) {
        printf("Error al escribir/cerrar archivo temporal de eventos.\n");
        limpiar_temporales();
        return 0;
    }

    remove(ARCH_SUP);
    remove(ARCH_REC);
    remove(ARCH_EVT);

    if (rename(TMP_SUP, ARCH_SUP) != 0 ||
        rename(TMP_REC, ARCH_REC) != 0 ||
        rename(TMP_EVT, ARCH_EVT) != 0) {
        printf("Error al renombrar archivos temporales.\n");
        limpiar_temporales();
        return 0;
    }

    return 1;
}

/* HU-13: Liberar toda la memoria y dejar punteros en NULL */
void liberar_refugio(Refugio *r) {
    if (r == NULL) {
        return;
    }
    free(r->lista);
    r->lista = NULL;
    r->n = 0;
    r->capacidad = 0;

    free(r->historial);
    r->historial = NULL;
    r->n_eventos = 0;
    r->cap_eventos = 0;
}

/* HU-11: Cargar partida con validacion de rangos y recuperacion ante errores */
int cargar_partida(Refugio *r) {
    if (r == NULL) {
        return 0;
    }

    FILE *f_sup = fopen(ARCH_SUP, "rb");
    FILE *f_rec = fopen(ARCH_REC, "rb");
    FILE *f_evt = fopen(ARCH_EVT, "rb");

    if (f_sup == NULL && f_rec == NULL && f_evt == NULL) {
        inicializar_refugio(r);
        printf("Primera ejecucion o sin archivos guardados: iniciando refugio vacio.\n");
        return 1;
    }

    if (f_sup == NULL || f_rec == NULL || f_evt == NULL) {
        if (f_sup) fclose(f_sup);
        if (f_rec) fclose(f_rec);
        if (f_evt) fclose(f_evt);
        inicializar_refugio(r);
        printf("Archivos de guardado incompletos o danados: se inicia refugio limpio.\n");
        return 0;
    }

    int n_sup = 0;
    Superviviente *lista_tmp = NULL;
    if (fread(&n_sup, sizeof(int), 1, f_sup) != 1 || n_sup < 0 || n_sup > 100000) {
        fclose(f_sup); fclose(f_rec); fclose(f_evt);
        inicializar_refugio(r);
        printf("Datos de supervivientes fuera de rango o danados.\n");
        return 0;
    }
    if (n_sup > 0) {
        lista_tmp = malloc((size_t)n_sup * sizeof(Superviviente));
        if (lista_tmp == NULL || fread(lista_tmp, sizeof(Superviviente), (size_t)n_sup, f_sup) != (size_t)n_sup) {
            free(lista_tmp);
            fclose(f_sup); fclose(f_rec); fclose(f_evt);
            inicializar_refugio(r);
            printf("Error leyendo registros de supervivientes.\n");
            return 0;
        }
        for (int i = 0; i < n_sup; i++) {
            lista_tmp[i].nombre[MAX_NOMBRE] = '\0';
            lista_tmp[i].habilidad[MAX_HABILIDAD] = '\0';
            if (lista_tmp[i].nombre[0] == '\0' || lista_tmp[i].habilidad[0] == '\0' ||
                lista_tmp[i].edad <= 0 || lista_tmp[i].salud < 0 || lista_tmp[i].salud > 100) {
                free(lista_tmp);
                fclose(f_sup); fclose(f_rec); fclose(f_evt);
                inicializar_refugio(r);
                printf("Registro de superviviente corrupto detectado: se aborta carga.\n");
                return 0;
            }
        }
    }

    int dia_tmp = 1;
    Recursos rec_tmp;
    if (fread(&dia_tmp, sizeof(int), 1, f_rec) != 1 ||
        fread(&rec_tmp, sizeof(Recursos), 1, f_rec) != 1 ||
        dia_tmp <= 0 || rec_tmp.agua < 0 || rec_tmp.comida < 0 ||
        rec_tmp.medicamentos < 0 || rec_tmp.municiones < 0) {
        free(lista_tmp);
        fclose(f_sup); fclose(f_rec); fclose(f_evt);
        inicializar_refugio(r);
        printf("Datos de recursos o dia danados: se aborta carga.\n");
        return 0;
    }

    int n_ev = 0;
    Evento *ev_tmp = NULL;
    if (fread(&n_ev, sizeof(int), 1, f_evt) != 1 || n_ev < 0 || n_ev > 100000) {
        free(lista_tmp);
        fclose(f_sup); fclose(f_rec); fclose(f_evt);
        inicializar_refugio(r);
        printf("Cantidad de eventos fuera de rango.\n");
        return 0;
    }
    if (n_ev > 0) {
        ev_tmp = malloc((size_t)n_ev * sizeof(Evento));
        if (ev_tmp == NULL || fread(ev_tmp, sizeof(Evento), (size_t)n_ev, f_evt) != (size_t)n_ev) {
            free(lista_tmp);
            free(ev_tmp);
            fclose(f_sup); fclose(f_rec); fclose(f_evt);
            inicializar_refugio(r);
            printf("Error leyendo historial de eventos.\n");
            return 0;
        }
        for (int i = 0; i < n_ev; i++) {
            ev_tmp[i].descripcion[MAX_EVENTO] = '\0';
            if (ev_tmp[i].dia <= 0 || ev_tmp[i].descripcion[0] == '\0') {
                free(lista_tmp);
                free(ev_tmp);
                fclose(f_sup); fclose(f_rec); fclose(f_evt);
                inicializar_refugio(r);
                printf("Evento corrupto detectado: se aborta carga.\n");
                return 0;
            }
        }
    }

    fclose(f_sup);
    fclose(f_rec);
    fclose(f_evt);

    liberar_refugio(r);
    r->lista = lista_tmp;
    r->n = n_sup;
    r->capacidad = n_sup;
    r->historial = ev_tmp;
    r->n_eventos = n_ev;
    r->cap_eventos = n_ev;
    r->dia_actual = dia_tmp;
    r->recursos = rec_tmp;
    return 1;
}

/* HU-12: Cinco eventos aleatorios sin recursos negativos */
void evento_aleatorio(Refugio *r) {
    if (r == NULL) {
        return;
    }

    int tipo_ev = rand() % 5;
    if (tipo_ev == 0) {
        if (insertar_superviviente_datos(r, "Dr. Silva", 36, "Medico", 85)) {
            registrar_evento(r, "Evento: Llego un medico");
            printf("¡Llego un medico! Se unio al refugio.\n");
        }
    } else if (tipo_ev == 1) {
        perder_recurso(r, REC_MUNICIONES, 6);
        perder_recurso(r, REC_MEDICAMENTOS, 5);
        registrar_evento(r, "Evento: La horda ataco");
        printf("La horda ataco: se consumieron municiones y medicamentos.\n");
    } else if (tipo_ev == 2) {
        perder_recurso(r, REC_COMIDA, 15);
        registrar_evento(r, "Evento: Alguien robo la comida");
        printf("Alguien robo la comida.\n");
    } else if (tipo_ev == 3) {
        modificar_recurso(r, REC_AGUA, 15);
        modificar_recurso(r, REC_COMIDA, 15);
        modificar_recurso(r, REC_MEDICAMENTOS, 10);
        registrar_evento(r, "Evento: Encontraron una mochila");
        printf("Encontraron una mochila con suministros.\n");
    } else {
        registrar_evento(r, "Evento: Se fue la luz");
        printf("¡Se fue la luz! Guardando y recargando partida...\n");
        if (guardar_partida(r)) {
            cargar_partida(r);
        }
    }

    normalizar_recursos_no_negativos(&r->recursos);
}

int main(void) {
    srand((unsigned int)time(NULL));

    Refugio refugio;
    inicializar_refugio(&refugio);
    cargar_partida(&refugio);

    int op;
    do {
        printf("\n=== REFUGIO DE SUPERVIVIENTES (Dia %d) ===\n", refugio.dia_actual);
        printf("1. Registrar superviviente\n");
        printf("2. Listar supervivientes\n");
        printf("3. Mostrar recursos\n");
        printf("4. Modificar recurso (entrada/salida)\n");
        printf("5. Ver historial de acontecimientos\n");
        printf("6. Dar de baja superviviente\n");
        printf("7. Simular expedicion\n");
        printf("8. Registrar perdida de recurso\n");
        printf("9. Disparar evento aleatorio\n");
        printf("10. Guardar partida\n");
        printf("0. Guardar y salir\n");
        op = leer_entero("Opcion: ");

        if (op == 1) {
            if (agregar_superviviente(&refugio)) {
                printf("Superviviente registrado.\n");
            }
        } else if (op == 2) {
            listar_supervivientes(&refugio);
        } else if (op == 3) {
            mostrar_recursos(&refugio);
        } else if (op == 4) {
            int tipo = leer_entero("Tipo (1 agua, 2 comida, 3 medicamentos, 4 municiones): ");
            int cant = leer_entero("Cantidad (+ entrada, - salida): ");
            if (!modificar_recurso(&refugio, tipo, cant)) {
                printf("Rechazado: cantidad invalida, tipo invalido o salida mayor a lo disponible.\n");
            } else {
                printf("Recurso actualizado.\n");
            }
        } else if (op == 5) {
            mostrar_historial(&refugio);
        } else if (op == 6) {
            char nombre[MAX_NOMBRE + 1];
            leer_cadena(nombre, sizeof(nombre), "Nombre a dar de baja: ");
            if (dar_de_baja(&refugio, nombre)) {
                printf("Baja realizada.\n");
            }
        } else if (op == 7) {
            expedicion(&refugio);
            refugio.dia_actual++;
        } else if (op == 8) {
            int tipo = leer_entero("Tipo (1 agua, 2 comida, 3 medicamentos, 4 municiones): ");
            int cant = leer_entero("Cantidad perdida (> 0): ");
            perder_recurso(&refugio, tipo, cant);
        } else if (op == 9) {
            evento_aleatorio(&refugio);
        } else if (op == 10) {
            if (guardar_partida(&refugio)) {
                printf("Partida guardada de forma segura.\n");
            }
        } else if (op == 0) {
            if (!guardar_partida(&refugio)) {
                printf("Advertencia: no se pudo guardar al salir.\n");
            }
        } else {
            printf("Opcion no valida.\n");
        }
    } while (op != 0);

    liberar_refugio(&refugio);
    return 0;
}