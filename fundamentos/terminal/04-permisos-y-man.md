# SO - Terminal 04 - Permisos y man

Parte de [Inicio](../../README.md). Anterior: [Terminal 03 - Redirecciones y pipes](03-redirecciones-y-pipes.md) | Siguiente: [Terminal 05 - Scripts de bash](05-scripts-de-bash.md)

---

## 1. Leer el manual (`man`) - esto es la mitad de la asignatura

El enunciado de la Practica 1 te dice literalmente `man 3 getopt`, `man 5 passwd`, `man 3 strtol`, `man 1 printf`. **Ese numero es la seccion del manual**, y hay que saber lo que significa porque el mismo nombre existe en varias secciones.

| Seccion | Contiene | Ejemplo |
|---------|----------|---------|
| **1** | Ordenes de usuario (programas del shell) | `man 1 printf` -> el programa printf |
| 2 | Llamadas al sistema (interfaz con el kernel) | `man 2 open` |
| **3** | Funciones de biblioteca de C | `man 3 printf` -> la funcion printf de C |
| 4 | Ficheros especiales (`/dev`) | `man 4 null` |
| **5** | Formatos de fichero | `man 5 passwd` -> el formato de /etc/passwd |
| 7 | Miscelanea | `man 7 signal` |
| 8 | Administracion del sistema | `man 8 mount` |

```bash
man ls              # te da la primera seccion donde aparezca
man 3 printf        # la FUNCION de C, no el programa
man -k socket       # buscar por palabra clave (= apropos)
man -f printf       # en que secciones existe 'printf'  (= whatis)
```

### Moverse dentro de `man`

| Tecla | Accion |
|-------|--------|
| `Espacio` / `f` | pagina siguiente |
| `b` | pagina anterior |
| `/texto` | buscar hacia abajo |
| `n` / `N` | siguiente / anterior coincidencia |
| `g` / `G` | principio / final |
| `q` | **salir** |

### Como se lee una pagina de manual

```
NAME          <- que hace, en una linea
SYNOPSIS      <- los #include que necesitas y la firma exacta   *** LO MAS IMPORTANTE ***
DESCRIPTION   <- explicacion detallada
RETURN VALUE  <- que devuelve y que devuelve si falla           *** SEGUNDO MAS IMPORTANTE ***
ERRORS        <- valores de errno
EXAMPLES      <- a veces hay codigo listo para copiar
SEE ALSO      <- funciones relacionadas
```

> [!tip] Metodo rapido
> Abre `man`, pulsa `/` y escribe `RETURN VALUE`. En el 80% de los casos lo que necesitas saber es que devuelve la funcion y como detecta el error.

Para ordenes internas del shell (`cd`, `read`, `echo`...) no hay `man`, se usa `help`:

```bash
help read      # el enunciado de la P1 te lo dice: "consulta help read"
help cd
```

---

## 2. Permisos

```
-rwxr-xr--
│└┬┘└┬┘└┬┘
│ │  │  └── OTROS (o): el resto del mundo    -> r--  leer
│ │  └── GRUPO (g)                           -> r-x  leer y ejecutar
│ └── PROPIETARIO (u)                        -> rwx  leer, escribir, ejecutar
└── tipo de fichero
```

| Letra | En un **fichero** | En un **directorio** |
|-------|-------------------|----------------------|
| `r` (4) | leer su contenido | listar lo que hay dentro (`ls`) |
| `w` (2) | modificarlo | crear/borrar ficheros dentro |
| `x` (1) | **ejecutarlo** | entrar en el (`cd`) |

> [!important] Ojo con el directorio
> Poder borrar un fichero depende del permiso `w` **del directorio**, no del fichero. Por eso puedes borrar un fichero que no es tuyo si el directorio es tuyo.

### `chmod`

**Modo octal** (el que se usa en el examen): suma r=4, w=2, x=1 para cada uno de los tres grupos.

| Octal | Permisos | Tipico de |
|-------|----------|-----------|
| `755` | `rwxr-xr-x` | programas y directorios |
| `644` | `rw-r--r--` | ficheros de texto normales |
| `700` | `rwx------` | privado tuyo |
| `600` | `rw-------` | ficheros con datos sensibles |
| `777` | `rwxrwxrwx` | todo el mundo todo (evitalo) |

```bash
chmod 755 script.sh
chmod 644 notas.txt
```

**Modo simbolico** (mas comodo para cambios sueltos):

```bash
chmod +x script.sh      # dar permiso de ejecucion a todos
chmod u+x script.sh     # solo al propietario
chmod go-w fichero      # quitar escritura a grupo y otros
chmod a=r fichero       # todos: solo lectura
```

> [!warning] El fallo numero 1 al escribir tu primer script
> Creas `miscript.sh`, escribes `./miscript.sh` y te dice **"Permission denied"**.
> Falta el permiso de ejecucion: `chmod +x miscript.sh`.

### Quien soy y usuarios

```bash
whoami        # mi nombre de usuario
id            # mi UID, GID y grupos
sudo orden    # ejecutar como root (te pide TU contrasena)
chown juan fichero          # cambiar propietario (necesita sudo)
```

El **UID 0** es siempre `root`, el superusuario: se salta todas las comprobaciones de permisos. Lo veras en `/etc/passwd` en la primera linea.

---

## 3. Variables de entorno

```bash
echo $HOME       # /home/juan
echo $PATH       # donde busca el shell los ejecutables
echo $USER
echo $PWD
env              # ver TODAS las variables de entorno
export MIVAR=hola    # crear/exportar una variable
```

`PATH` es una lista de directorios separados por `:`. Cuando escribes `gcc`, el shell recorre esa lista buscando un ejecutable llamado `gcc`. Como `.` no esta en la lista, para tu programa necesitas `./programa`.

---

## Ejercicios

1. Averigua en que secciones del manual existe `printf` y mira las dos.
2. Busca en `man 3 getopt` que son `optarg` y `optind`. Los necesitas para el ejercicio 3 de la P1.
3. Mira `man 5 passwd` y comprueba que los 7 campos coinciden con lo que viste en [Terminal 02 - Ver y buscar texto](02-ver-y-buscar-texto.md).
4. Crea un fichero, mira sus permisos, ponlos en `700` y comprueba con `ls -l`.
5. Crea un fichero `hola.sh` con `echo hola` dentro, intenta ejecutarlo, falla, arreglalo.
6. Mira tu `PATH`. Cuantos directorios tiene? Esta el directorio actual?
