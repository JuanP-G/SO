[Volver al inicio](../../README.md) · Anterior: [P5 · Discoteca](../PR5-sincronizacion_de_hilos/explicacion.md)

# P6 · Salvajes y cocinero: semáforos con nombre y memoria compartida

Un solo ejercicio: **un proceso cocinero** y **N procesos salvaje** (programas distintos, en terminales distintos) que comparten un caldero con `M = 10` raciones.

Vídeo explicativo: [salvajes.mp4](videos/salvajes.mp4)

Código: [cocinero.c](ejercicio1/cocinero.c) · [salvajes.c](ejercicio1/salvajes.c) · [caldero.h](ejercicio1/caldero.h) · [Makefile](ejercicio1/Makefile)

## 1. Qué cambia respecto a P5

| | P5 (discoteca) | P6 (salvajes) |
|---|---|---|
| Unidad | hilos del mismo proceso | **procesos** distintos |
| Datos compartidos | variables globales | **memoria compartida** (`shm_open` + `mmap`) |
| Sincronización | mutex + variables de condición | **semáforos con nombre** (`sem_open`) |
| Limpieza | `destroy` al final | `sem_unlink` / `shm_unlink` (viven en el sistema hasta borrarlos) |

Procesos distintos no comparten variables globales ni mutex normales: hace falta algo con **nombre** que ambos puedan abrir.

## 2. Qué dice el temario (Tema 3.3)

| Idea del tema | En esta práctica |
|---|---|
| Semáforo = **contador + cola** de procesos bloqueados | `sem_t` con nombre, compartido por nombre entre procesos |
| `wait`: `c = c - 1`; si `c < 0`, **bloquea**. `signal`: `c = c + 1`; si hay bloqueados, **despierta** a uno | `sem_wait` / `sem_post` |
| `c >= 0`: nº de `wait` que pasan sin bloquear. `c < 0`: `abs(c)` = nº de bloqueados | con `empty = 0` y `full = 0`, el primer `wait` ya bloquea |
| Uso 1: **exclusión mutua** (semáforo a 1, sección crítica entre `wait` y `signal`) | `mutex` |
| Uso 2: **condiciones sobre enteros** (productor-consumidor: `items`, `gaps`) | `empty` y `full` actúan como el par `gaps` / `items` de una sola posición |
| Uso 3: **cola de espera** (semáforo a 0 + entero protegido por un mutex) | el salvaje espera en `full`; `servings` es el entero, protegido por `mutex` |
| Procesos **no emparentados** en la misma máquina: **semáforos con nombre** (`sem_open`) | cocinero y salvajes son programas distintos |
| Procesos emparentados (`fork`): semáforos sin nombre (`sem_init` con `shared = 1`, dentro de memoria compartida) | no aplica aquí |

El patrón es el del productor-consumidor con memoria compartida de las transparencias:

| Paso | Productor del tema | Cocinero (P6) | Consumidor del tema | Salvaje (P6) |
|---|---|---|---|---|
| Objeto compartido | `open` + `ftruncate` | `shm_open` + `ftruncate` | `open` | `shm_open` sin `O_CREAT` |
| Mapear | `mmap` (`MAP_SHARED`) | `mmap` | `mmap` | `mmap` |
| Semáforos | `sem_open(..., O_CREAT, ...)` | igual | `sem_open(nombre, 0)` | igual |
| Al terminar | `munmap`, `close`, `unlink`, `sem_close`, `sem_unlink` | igual | `munmap`, `close`, `sem_close` | igual, sin `unlink` |

Diferencia: el tema usa un fichero normal (`open`); el enunciado pide un objeto POSIX (`shm_open`), que cumple la misma función. El tema dice que el consumidor "debe esperar a que el archivo esté creado": en la práctica el salvaje no espera, **avisa y termina** si el cocinero no está.

## 3. Reglas -> herramientas

| Regla | Cómo se garantiza |
|---|---|
| Salvaje no coge si está vacío | si `servings == 0` avisa al cocinero y espera |
| Cocinero repone solo si está vacío | duerme hasta que un salvaje le avise |
| Dos salvajes no tocan el contador a la vez | semáforo de exclusión mutua |
| Cocinero limpia todo con SIGTERM / SIGINT | `sigaction` + limpieza tras el bucle |

## 4. Tres semáforos y una variable

| Recurso | Valor inicial | Para qué |
|---|---|---|
| `servings` (memoria compartida, un `int`) | 0 | raciones que quedan |
| `mutex` | 1 | un salvaje a la vez en el caldero |
| `empty` | 0 | salvaje -> cocinero: "está vacío, repón" |
| `full` | 0 | cocinero -> salvaje: "ya repuse" |

Un semáforo con valor 0 es una **señal**: quien hace `sem_wait` duerme hasta que otro haga `sem_post`.

## 5. Funciones que hay que conocer

| Función | Qué hace |
|---|---|
| `shm_open(nombre, O_CREAT\|O_RDWR, 0600)` | crea/abre el objeto de memoria compartida, devuelve un fd |
| `ftruncate(fd, tamaño)` | le da tamaño (nace con 0 bytes) |
| `mmap(NULL, tamaño, PROT_READ\|PROT_WRITE, MAP_SHARED, fd, 0)` | lo mapea como memoria normal; `MAP_SHARED` para que los cambios los vean los demás |
| `sem_open(nombre, O_CREAT, 0600, valor)` | crea el semáforo con ese valor inicial |
| `sem_open(nombre, 0)` | solo abre uno existente (falla si no está: así detecta el salvaje que no hay cocinero) |
| `sem_wait` / `sem_post` | bajar (esperar) / subir (avisar) |
| `sem_close`, `munmap`, `close(fd)` | cada proceso suelta su propia referencia |
| `sem_unlink`, `shm_unlink` | borran el nombre del sistema (solo lo hace el cocinero) |

Los nombres empiezan por `/` y no son ficheros reales: en Linux se ven en `/dev/shm`.

## 6. Cómo construirla paso a paso

1. **`caldero.h`**: los nombres de los recursos en un solo sitio, para que cocinero y salvaje usen exactamente los mismos.
2. **Cocinero, arranque**: `shm_unlink`/`sem_unlink` de restos -> `shm_open` -> `ftruncate` -> `mmap` -> `*servings = 0` -> tres `sem_open` con O_CREAT.
3. **Cocinero, señales**: `sigaction` para SIGTERM y SIGINT que ponen `finish = 1`.
4. **Cocinero, bucle**: `putServingsInPot`: `sem_wait(empty)`, `*servings = M`, `sem_post(full)`.
5. **Cocinero, final**: `munmap`, `shm_unlink`, y `sem_close` + `sem_unlink` de los tres.
6. **Salvaje, arranque**: `shm_open` sin O_CREAT (si falla, mensaje "ejecuta primero el cocinero"), `mmap`, tres `sem_open` sin O_CREAT.
7. **Salvaje, `getServingsFromPot`**: `sem_wait(mutex)` -> si vacío: `sem_post(empty)` y `sem_wait(full)` -> `(*servings)--` -> `sem_post(mutex)`.
8. **Salvaje, final**: `sem_close` x3 y `munmap` (nunca `unlink`).

## 7. Cronología de un caldero vacío

```text
Salvaje A                 Salvaje B               Cocinero
---------                 ---------               --------
wait(mutex)  ok
servings == 0
post(empty)  ------------------------------------> despierta
wait(full)   (duerme)     wait(mutex) (bloqueado)  servings = 10
                                                   post(full)
despierta <---------------------------------------
servings-- (9)
post(mutex)
                          wait(mutex) ok
                          servings-- (8)
```

El salvaje A espera **con el `mutex` cogido**. Es correcto: nadie puede coger ración mientras esté vacío, y el cocinero no necesita el mutex. En P5 era un error esperar con el mutex; aquí no lo es porque el cocinero no lo usa.

## 8. Solución comentada

**`cocinero.c`**

| Pieza | Por qué |
|---|---|
| `volatile sig_atomic_t finish` | la modifica un manejador de señal |
| `unlink` de restos al inicio | si una ejecución anterior murió sin limpiar, `O_CREAT` reutilizaría semáforos con valores viejos |
| `sigaction` **sin** `SA_RESTART` | así la señal interrumpe `sem_wait` (devuelve -1, `EINTR`) y el bucle puede ver `finish` |
| `if (sem_wait(empty) == -1) return;` | salir de `putServingsInPot` sin reponer cuando llega la señal |
| `cleanup()` después del bucle | el manejador solo marca `finish`; limpiar dentro del manejador no es seguro |

**`salvajes.c`**

| Pieza | Por qué |
|---|---|
| `shm_open` / `sem_open` sin `O_CREAT` | si el cocinero no existe, falla y se avisa |
| `srand(getpid())` | sin semilla, todos los procesos hacen el mismo `rand()` y comen igual de rápido |
| `close(fd)` tras `mmap` | el mapeo sigue válido; el fd ya no hace falta |

## 9. Probar

```bash
cd ejercicio1
make
./cocinero &                                  # un cocinero
for i in 1 2 3 4 5; do ./salvajes & done      # 5 salvajes, 3 raciones cada uno = 15
wait
kill -TERM %1                                 # el cocinero limpia
ls /dev/shm                                   # no debe quedar nada "caldero"
```

Resultado esperado con 5 salvajes: 15 raciones, el cocinero repone 2 veces (10 + 10 >= 15), ningún "left" negativo y `/dev/shm` limpio. Si lanzas `./salvajes` sin cocinero: `No hay caldero: ejecuta primero ./cocinero`.

## 10. Errores típicos

| Error | Consecuencia |
|---|---|
| Semáforo `empty` o `full` con valor inicial 1 | el cocinero repone sin que esté vacío (o el salvaje coge sin reposición) |
| Olvidar `ftruncate` | `mmap` sobre 0 bytes: `SIGBUS` al escribir |
| `MAP_PRIVATE` en vez de `MAP_SHARED` | cada proceso ve su copia; el cocinero repone y los salvajes siguen viendo 0 |
| Variable global en lugar de memoria compartida | procesos distintos = copias distintas |
| `sigaction` con `SA_RESTART` | Ctrl-C no interrumpe `sem_wait`: el cocinero no termina |
| Salvaje hace `sem_unlink` | borra el caldero mientras otros lo usan |
| No limpiar al salir | quedan `/dev/shm/caldero_*` y `/dev/shm/sem.caldero_*` con valores viejos |
| `*servings` leído/escrito sin `mutex` | dos salvajes cogen la última ración |

## 11. Preguntas de repaso para el examen

1. ¿Por qué no sirve un mutex de `pthread` entre procesos distintos y qué lo sustituye aquí?
2. ¿Qué valor inicial tienen `mutex`, `empty` y `full`, y qué ocurre si cambias uno?
3. ¿Por qué el salvaje puede esperar en `full` con el `mutex` cogido sin provocar interbloqueo?
4. ¿Qué diferencia hay entre `sem_close` y `sem_unlink`? ¿Y entre `munmap` y `shm_unlink`?
5. ¿Qué pasa con un semáforo con nombre si el proceso que lo creó muere sin hacer `sem_unlink`?
6. ¿Por qué el manejador solo pone `finish = 1` y no limpia?
7. ¿Qué pasaría con dos cocineros? ¿Y con ningún cocinero?
8. ¿Cuándo se usa `sem_init` y cuándo `sem_open`? ¿Qué debe cumplirse con `sem_init(..., 1, ...)`?
9. Si `empty` valiera -2, ¿qué significaría?
