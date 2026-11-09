# SO - P4 - Procesos e hilos (correccion comentada)

Relacionado: [Inicio](../../README.md) · [P2 - Correccion comentada](../PR2-ficheros_stdio/correccion-comentada.md) · [C 05 - Memoria dinamica](../../fundamentos/c/05-memoria-dinamica.md) · [Herramientas - gcc make gdb](../../fundamentos/herramientas-gcc-make-gdb.md)
Video explicativo: `SO - P4 - Video explicativo.mp4` (casi 6 min, una escena por ejercicio).

> [!tip] Idea de toda la practica
> **fork copia la memoria pero comparte las entradas de fichero abiertas. exec cambia el programa, no el proceso. Los hilos comparten memoria. Las senales son avisos asincronos.**
> Pregunta de examen: *que se copia y que se comparte?*

---
## Ejercicio 1 - `run_commands` (fork + exec + wait)

### Teoria
- `fork()` devuelve **0 en el hijo** y el **PID del hijo en el padre**. Despues hay dos procesos casi iguales.
- `execvp(argv[0], argv)`: sustituye el programa del proceso. `p` = busca en `PATH`; `v` = argumentos en un vector. Si funciona **no vuelve**; si vuelve, fallo.
- Tras un `execvp` fallido: `perror` + `_exit(127)`. `_exit` y no `exit` para no vaciar dos veces los buferes de stdio duplicados por el fork.
- `waitpid(pid, &status, 0)` devuelve el estado **en bruto**: `256 = codigo 1 << 8`. Con macros: `WIFEXITED`, `WEXITSTATUS`, `WIFSIGNALED`, `WTERMSIG`.
- `-b`: se lanzan todos y se recoge con `wait(&status)` **en el orden en que terminan**; hay que asociar PID -> numero de comando.

### Codigo (`ejercicio1/run_commands.c`)
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>     /* fork, execvp, _exit, getopt */
#include <sys/types.h>  /* pid_t */
#include <sys/wait.h>   /* wait, waitpid */

#define MAX_COMMANDS 100
#define MAX_LINE     1024

/* Crea un hijo que ejecuta argv[0] con sus argumentos. Devuelve el PID del
 * hijo SIN esperar a que termine. */
pid_t launch_command(char **argv)
{
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {                  /* hijo */
        execvp(argv[0], argv);
        perror(argv[0]);             /* solo se llega aquí si execvp falla */
        _exit(127);                  /* _exit: no vaciar los búferes de stdio heredados */
    }
    return pid;                      /* padre */
}

static void free_command(char **argv)
{
    for (int i = 0; argv[i] != NULL; i++)
        free(argv[i]);
    free(argv);
}

/* parse_command(): el del enunciado, SIN tocar */

/* Espera a un hijo concreto y devuelve su estado en bruto (como pide el enunciado). */
static int wait_for(pid_t pid)
{
    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }
    return status;
}

/* -x: un comando, y esperar a que acabe */
static void run_one(const char *command)
{
    int cmd_argc;
    char **cmd_argv = parse_command(command, &cmd_argc);

    if (cmd_argc > 0)
        wait_for(launch_command(cmd_argv));
    free_command(cmd_argv);
}

/* -s sin -b: un comando por línea, cada uno espera al anterior */
static void run_file_sequential(FILE *file)
{
    char line[MAX_LINE];
    int number = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        int cmd_argc;
        line[strcspn(line, "\n")] = '\0';      /* quitar el salto de línea */
        char **cmd_argv = parse_command(line, &cmd_argc);

        if (cmd_argc > 0) {
            printf("@@ Running command #%d: %s\n", number, line);
            fflush(stdout);                    /* que salga antes que la salida del hijo */
            pid_t pid = launch_command(cmd_argv);
            int status = wait_for(pid);
            printf("@@ Command #%d terminated (pid: %d, status: %d)\n", number, pid, status);
            number++;
        }
        free_command(cmd_argv);
    }
}

/* -s -b: lanzar todos sin esperar y luego recoger terminaciones en el orden en que ocurran */
static void run_file_background(FILE *file)
{
    char line[MAX_LINE];
    pid_t pids[MAX_COMMANDS];
    int launched = 0;

    while (launched < MAX_COMMANDS && fgets(line, sizeof(line), file) != NULL) {
        int cmd_argc;
        line[strcspn(line, "\n")] = '\0';
        char **cmd_argv = parse_command(line, &cmd_argc);

        if (cmd_argc > 0) {
            printf("@@ Running command #%d: %s\n", launched, line);
            fflush(stdout);
            pids[launched] = launch_command(cmd_argv);
            launched++;
        }
        free_command(cmd_argv);
    }

    for (int remaining = launched; remaining > 0; remaining--) {
        int status;
        pid_t pid = wait(&status);           /* el primero que termine, sea cual sea */
        if (pid == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        for (int number = 0; number < launched; number++)    /* PID -> nº de comando */
            if (pids[number] == pid)
                printf("@@ Command #%d terminated (pid: %d, status: %d)\n", number, pid, status);
    }
}

int main(int argc, char *argv[])
{
    const char *command = NULL;
    const char *script = NULL;
    int background = 0;
    int opt;

    while ((opt = getopt(argc, argv, "x:s:b")) != -1) {
        switch (opt) {
        case 'x': command = optarg;  break;
        case 's': script = optarg;   break;
        case 'b': background = 1;    break;
        default:
            fprintf(stderr, "Usage: %s [-x \"command\"] [-s file [-b]]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (command == NULL && script == NULL) {
        fprintf(stderr, "Usage: %s [-x \"command\"] [-s file [-b]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (command != NULL)
        run_one(command);

    if (script != NULL) {
        FILE *file = fopen(script, "r");
        if (file == NULL) {
            perror(script);
            return EXIT_FAILURE;
        }
        if (background)
            run_file_background(file);
        else
            run_file_sequential(file);
        fclose(file);
    }
    return EXIT_SUCCESS;
}
```

### Probar
```bash
make
./run_commands -x "ls -l"
./run_commands -s test1          # secuencial
./run_commands -b -s test2       # segundo plano
```
Con `test2` (sleeps de 6, 3, 1 s): secuencial ~10 s, background ~6 s (el tiempo del mas largo). Orden de aviso en background: #0, #3, #4, #2, #1.

### Preguntas
**1. `-x` sin comillas / comillas simples / `echo $HOME`**
- Sin comillas, la **shell** trocea el texto: `-x ls -l` pasa `-x` con argumento `ls` y `-l` se queda suelto -> `getopt` lo trata como opcion invalida. Con dobles o simples llega un unico argumento `ls -l` y funciona.
- Diferencia dobles/simples: la shell **expande variables dentro de dobles** (`"echo $HOME"` llega como `echo /home/usuario`) pero **no dentro de simples** (`'echo $HOME'` llega literal y `echo` imprimira `$HOME`, porque `execvp` no expande nada). Para `ls -l` no hay variables, asi que funcionan igual.

**2. `execlp` en vez de `execvp`?**
Si, pero `execlp("ls","ls","-l",NULL)` exige conocer **en tiempo de compilacion** el numero de argumentos. Aqui vienen de `parse_command` (numero variable), asi que habria que hacer un `switch` por numero de args -> inviable. `execvp` recibe el vector ya construido.

**3. `"echo hola > a.txt"` y `"cat run_commands.c | grep int"`**
`parse_command` solo trocea por espacios; **`>` y `|` los entiende la shell, no `exec`**. `execvp` los pasa como argumentos normales:
- `echo hola > a.txt` imprime `hola > a.txt` y **no crea** `a.txt`.
- `cat run_commands.c | grep int` intenta abrir `|`, `grep` e `int` como ficheros -> tres errores `No such file or directory`.
Para redirecciones y tuberias habria que implementarlas (`open`+`dup2`, `pipe`) o lanzar `sh -c "..."`.

---
## Ejercicio 2 - Hilos y paso de parametros

### Teoria
Los hilos de un proceso **comparten** codigo, globales y heap; cada uno tiene **su pila**. Por eso el argumento de `pthread_create` (un `void *`) debe vivir **en el heap y ser distinto por hilo**: si se pasa la direccion de una unica variable, todos leen la misma y hay **condicion de carrera**.
- `pthread_create` devuelve el **codigo de error** (no usa `errno`) -> `strerror(error)`.
- Compilar con `-pthread` (ya en el Makefile).

### Codigo correcto (`hilos.c`): un `malloc` por hilo
```c
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#define NUM_THREADS 5

struct thread_args {
    int  number;      /* nº de hilo */
    char priority;    /* 'P' prioritario, 'N' no prioritario */
};

void *thread_usuario(void *arg)
{
    struct thread_args args = *(struct thread_args *)arg;   /* copia local */
    free(arg);                                              /* ya no hace falta */

    pthread_t id = pthread_self();
    printf("Hilo id=%lu, numero=%d, prioridad=%c\n",
           (unsigned long)id, args.number, args.priority);
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t threads[NUM_THREADS];

    (void)argc;
    (void)argv;

    for (int i = 0; i < NUM_THREADS; i++) {
        struct thread_args *args = malloc(sizeof(*args));   /* uno por hilo */
        if (args == NULL) {
            perror("malloc");
            return 1;
        }
        args->number = i;
        args->priority = (i % 2 == 0) ? 'P' : 'N';

        int error = pthread_create(&threads[i], NULL, thread_usuario, args);
        if (error != 0) {      /* pthread_create devuelve el error, no usa errno */
            fprintf(stderr, "pthread_create: %s\n", strerror(error));
            free(args);
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    return 0;
}
```

### Experimento: una unica variable compartida (`hilos_compartido.c`)
```c
/* Experimento del enunciado: UNA sola variable de argumentos para todos los hilos.
 * Compilar con -DRETRASO=1 para que el hilo tarde un poco en leerla (hace visible la carrera). */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 5

struct thread_args {
    int  number;
    char priority;
};

void *thread_usuario(void *arg)
{
#ifdef RETRASO
    usleep(100000);                       /* 100 ms: el main ya habrá cambiado la variable */
    struct thread_args args = *(struct thread_args *)arg;     /* se lee DESPUÉS de crear el hilo */
    printf("Hilo numero=%d, prioridad=%c\n", args.number, args.priority);
    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];
    struct thread_args shared;            /* UNA variable, compartida por todos */

    for (int i = 0; i < NUM_THREADS; i++) {
        shared.number = i;
        shared.priority = (i % 2 == 0) ? 'P' : 'N';
        pthread_create(&threads[i], NULL, thread_usuario, &shared);
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    return 0;
}
```

Resultado medido (5 ejecuciones): casi todos los hilos leen **4** (el main ya habia avanzado el bucle); en ninguna salieron 0,1,2,3,4. Con `usleep` (compilando `-DRETRASO`) siempre `4,4,4,4,4`. El orden de salida de los hilos lo decide el planificador, no el programa.

> [!warning] Fallo tipico
> Pasar `&i` o `&shared` a todos los hilos. Funciona "a veces" -> el peor tipo de bug.

---
## Ejercicio 3 - Senales + fork (`signalsyfork`)

### Teoria
- Una senal puede: accion por defecto, **ignorarse** (`SIG_IGN`) o tener **manejador** (`sigaction`).
- `alarm(5)` -> el kernel manda `SIGALRM` en 5 s. El manejador hace `kill(child_pid, SIGKILL)`.
- `SIGKILL` **no se puede ignorar ni capturar**.
- La alarma **interrumpe `waitpid`**: devuelve `-1` con `errno == EINTR` -> hay que **reintentar** en un bucle.
- `SIG_IGN` **se hereda a traves de `exec`**: por eso se pone `SIGINT` en `SIG_IGN` en el padre **despues del fork** (si no, el hijo `sleep` tambien ignoraria Ctrl+C).
- Ctrl+C va a **todo el grupo en primer plano**: el hijo muere por la senal 2 y el padre sigue.

### Codigo
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TIMEOUT_SECONDS 5

/* programa que temporiza la ejecución de un proceso hijo */

static pid_t child_pid;    /* global: el manejador de señal no recibe parámetros propios */

static void on_alarm(int signal_number)
{
    (void)signal_number;
    kill(child_pid, SIGKILL);   /* se acabó el tiempo: matar al hijo */
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Uso: %s comando [argumentos...]\n", argv[0]);
        return 1;
    }

    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        return 1;
    }
    if (child_pid == 0) {                       /* hijo: cambia de ejecutable */
        execvp(argv[1], &argv[1]);
        perror(argv[1]);                        /* solo si execvp falla */
        _exit(127);
    }

    /* padre: manejador de SIGALRM */
    struct sigaction on_alarm_action;
    memset(&on_alarm_action, 0, sizeof(on_alarm_action));
    on_alarm_action.sa_handler = on_alarm;
    sigemptyset(&on_alarm_action.sa_mask);
    if (sigaction(SIGALRM, &on_alarm_action, NULL) == -1) {
        perror("sigaction SIGALRM");
        return 1;
    }

    /* padre: ignorar SIGINT (Ctrl+C). Se hace DESPUÉS del fork: una señal ignorada se
     * hereda a través de exec, y el hijo no debe ignorarla. */
    struct sigaction ignore_action;
    memset(&ignore_action, 0, sizeof(ignore_action));
    ignore_action.sa_handler = SIG_IGN;
    sigemptyset(&ignore_action.sa_mask);
    sigaction(SIGINT, &ignore_action, NULL);

    alarm(TIMEOUT_SECONDS);

    int status;
    while (waitpid(child_pid, &status, 0) == -1) {
        if (errno != EINTR) {                   /* la alarma interrumpe waitpid: reintentar */
            perror("waitpid");
            return 1;
        }
    }

    if (WIFEXITED(status))
        printf("El hijo terminó normalmente con código %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("El hijo terminó por la señal %d (%s)\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));

    return 0;
}
```

### Probar
```bash
./signalsyfork echo hola      # termino normalmente con codigo 0
./signalsyfork nopexiste      # codigo 127
./signalsyfork sleep 20       # a los 5 s: senal 9 (Killed)
```

---
## Ejercicio 4 - Fichero compartido tras `fork`

### Teoria (la clave)
Tres capas: **tabla de descriptores del proceso -> entrada de fichero abierto (guarda el offset) -> fichero**.
`fork` copia la tabla de descriptores pero **no la entrada**: padre e hijo comparten el mismo **offset**.

### Por que falla `inicial.c`
El padre guarda la posicion, hace fork y el hijo hace `lseek(i*5)` + `write`: ese `lseek` **mueve el offset del padre tambien**. Despues el padre avanza con `lseek(fd, 5, SEEK_CUR)`; si el hijo ya habia escrito, se pasa de largo y queda un hueco de ceros. Resultado dependiente de quien llegue antes (**287 de 300 ejecuciones salieron mal**).

### Solucion A: un descriptor propio por proceso
Cada proceso hace su propio `open` -> su propia entrada, su propio offset. Se escribe en `index*5` sin esperar.
```c
/* Cuestión A: escritura concurrente, sin sincronizar.
 * Cada proceso abre SU PROPIO descriptor (su propio desplazamiento) y escribe en la
 * posición que le corresponde (i * 5), sin leer nunca la posición de otro. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROCESSES 10
#define BLOCK_SIZE    5

static void write_block(int index)
{
    char text[BLOCK_SIZE + 1];
    snprintf(text, sizeof(text), "%05d", index * 11111);       /* 0 -> "00000", 1 -> "11111" */

    int fd = open("output.txt", O_WRONLY);                     /* descriptor propio */
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    lseek(fd, index * BLOCK_SIZE, SEEK_SET);                   /* posición absoluta */
    write(fd, text, BLOCK_SIZE);
    close(fd);
}

int main(void)
{
    int fd = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    close(fd);                         /* el fichero ya existe y está vacío */

    for (int i = 1; i < NUM_PROCESSES; i++) {
        if (fork() == 0) {             /* hijo i */
            write_block(i);
            exit(0);
        }
    }
    write_block(0);                    /* el padre escribe el bloque 0 */

    while (wait(NULL) != -1)
        ;

    fd = open("output.txt", O_RDONLY);
    char c;
    printf("File contents are:\n");
    while (read(fd, &c, sizeof(c)) > 0)
        printf("%c", c);
    printf("\n");
    close(fd);
    return 0;
}
```

### Variante A2: `pwrite`
La posicion va dentro de la llamada y no toca el offset compartido.
```c
/* Cuestión A, variante con pwrite(): escribe en una posición indicada SIN tocar el
 * desplazamiento del descriptor, así que se puede compartir el mismo fd sin problemas. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROCESSES 10
#define BLOCK_SIZE    5

int main(void)
{
    int fd = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    for (int i = 1; i < NUM_PROCESSES; i++) {
        if (fork() == 0) {
            char text[BLOCK_SIZE + 1];
            snprintf(text, sizeof(text), "%05d", i * 11111);
            pwrite(fd, text, BLOCK_SIZE, i * BLOCK_SIZE);
            exit(0);
        }
    }
    pwrite(fd, "00000", BLOCK_SIZE, 0);

    while (wait(NULL) != -1)
        ;

    char c;
    lseek(fd, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd, &c, sizeof(c)) > 0)
        printf("%c", c);
    printf("\n");
    close(fd);
    return 0;
}
```

### Solucion B: compartir el offset a proposito
El padre escribe, el hijo escribe **a continuacion** (sin `lseek`) y el padre hace `wait` antes de la siguiente vuelta.
```c
/* Cuestión B: el padre escribe su "00000" entre los bloques de los hijos.
 * Aquí SÍ se aprovecha que padre e hijos comparten el mismo descriptor, y por tanto el
 * mismo desplazamiento: cada escritura avanza la posición para todos. Para que el orden
 * sea el correcto, el padre espera a cada hijo antes de seguir. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROCESSES 10
#define BLOCK_SIZE    5

int main(void)
{
    int fd = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    for (int i = 1; i < NUM_PROCESSES; i++) {
        write(fd, "00000", BLOCK_SIZE);            /* el padre, antes de cada hijo */

        if (fork() == 0) {
            char text[BLOCK_SIZE + 1];
            snprintf(text, sizeof(text), "%05d", i * 11111);
            write(fd, text, BLOCK_SIZE);           /* sin lseek: continúa donde dejó el padre */
            exit(0);
        }
        wait(NULL);                                /* el hijo termina antes del siguiente bloque */
    }

    char c;
    lseek(fd, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd, &c, sizeof(c)) > 0)
        printf("%c", c);
    printf("\n");
    close(fd);
    return 0;
}
```

### Resultados medidos
| Version | Ejecuciones | Fichero mal |
|---|---|---|
| `inicial.c` | 300 | **287** |
| A (`open` propio) | 500 | 0 |
| A2 (`pwrite`) | 500 | 0 |
| B (offset compartido + `wait`) | 300 | 0 |

---
## Checklist de examen
- [ ] `fork`: valor devuelto en padre/hijo; que se copia (memoria) y que se comparte (entradas de fichero abierto).
- [ ] `exec`: no vuelve si funciona; `p`/`v`/`l`; hereda descriptores y `SIG_IGN`.
- [ ] `wait`/`waitpid`: estado en bruto y macros; `wait` recoge en orden de terminacion.
- [ ] Shell vs `exec`: comillas, `$VAR`, `>` y `|` son cosa de la shell.
- [ ] Hilos: argumento propio en heap por hilo; `pthread_create` devuelve codigo de error.
- [ ] Senales: `EINTR`, `SIGKILL` inmanejable, heredar `SIG_IGN`, grupo de procesos.
- [ ] Offset compartido: `pwrite` o descriptores independientes.

Siguiente: [Inicio](../../README.md)
