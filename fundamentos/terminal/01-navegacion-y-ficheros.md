# SO - Terminal 01 - Navegacion y ficheros

Parte de [Inicio](../../README.md). Siguiente: [Terminal 02 - Ver y buscar texto](02-ver-y-buscar-texto.md)

---

## 1. El arbol de directorios

En Windows hay varias raices (`C:\`, `D:\`). **En Linux solo hay una: `/`**. Todo cuelga de ahi, incluido el pendrive que enchufes.

```
/
├── bin      -> programas basicos (ls, cp, cat...)
├── etc      -> ficheros de configuracion del sistema  <-- /etc/passwd vive aqui
├── home
│   └── juan -> TU carpeta personal  (atajo: ~)
├── tmp      -> ficheros temporales, se borran al reiniciar
├── usr      -> programas y librerias instalados
└── var      -> datos variables (logs, colas de impresion)
```

### Rutas: absolutas vs relativas

| Tipo | Empieza por | Ejemplo | Significa |
|------|-------------|---------|-----------|
| Absoluta | `/` | `/home/juan/so/p1.c` | Desde la raiz. Siempre apunta al mismo sitio. |
| Relativa | cualquier otra cosa | `so/p1.c` | Desde **donde estas ahora**. |

Simbolos que vas a usar constantemente:

| Simbolo | Significa |
|---------|-----------|
| `.` | el directorio actual |
| `..` | el directorio padre |
| `~` | tu carpeta personal (`/home/juan`) |
| `-` | el directorio anterior (con `cd -`) |
| `/` | la raiz del sistema |

> [!important] `./programa`
> Para ejecutar un programa que has compilado **tienes que escribir `./programa`**, no solo `programa`.
> Razon: el shell busca los programas en los directorios de la variable `PATH`, y el directorio actual NO esta en `PATH` por seguridad. `./` dice explicitamente "el que esta aqui".

---

## 2. Moverte

```bash
pwd             # print working directory: donde estoy
cd /etc         # ir a /etc (ruta absoluta)
cd so           # entrar en la carpeta 'so' que hay aqui (relativa)
cd ..           # subir un nivel
cd ../..        # subir dos niveles
cd              # ir a tu home (equivale a 'cd ~')
cd -            # volver al directorio donde estabas antes
```

### Listar

```bash
ls              # listado simple
ls -l           # listado LARGO: permisos, dueno, tamano, fecha
ls -a           # incluir ocultos (los que empiezan por punto)
ls -la          # las dos cosas
ls -lh          # tamanos legibles (4.0K en vez de 4096)
ls -lt          # ordenar por fecha de modificacion
ls /etc         # listar OTRO directorio sin moverte
```

Leer una linea de `ls -l`:

```
-rw-r--r--  1 juan juan  1234 sep 12 10:45 hola.c
│└┬┘└┬┘└┬┘  │ └─┬┘ └─┬┘  └─┬┘ └────┬─────┘ └──┬─┘
│ │  │  │   │   │    │     │       │          └── nombre
│ │  │  │   │   │    │     │       └── fecha de modificacion
│ │  │  │   │   │    │     └── tamano en bytes
│ │  │  │   │   │    └── grupo
│ │  │  │   │   └── propietario
│ │  │  │   └── numero de enlaces
│ │  │  └── permisos de "otros"
│ │  └── permisos del grupo
│ └── permisos del propietario
└── tipo:  '-' fichero normal,  'd' directorio,  'l' enlace simbolico
```

---

## 3. Crear, copiar, mover, borrar

| Orden | Que hace | Ejemplo |
|-------|----------|---------|
| `mkdir` | crear directorio | `mkdir practica1` |
| `mkdir -p` | crear con padres | `mkdir -p so/p1/ej1` |
| `touch` | crear fichero vacio **o actualizar su fecha** | `touch hola.c` |
| `cp` | copiar | `cp hola.c copia.c` |
| `cp -r` | copiar directorio entero | `cp -r p1 p1_backup` |
| `mv` | mover **y tambien renombrar** | `mv viejo.c nuevo.c` |
| `rm` | borrar fichero | `rm basura.txt` |
| `rm -r` | borrar directorio y su contenido | `rm -r carpeta` |
| `rmdir` | borrar directorio **vacio** | `rmdir vacia` |

> [!danger] `rm` no tiene papelera
> Lo que borras con `rm` desaparece. No hay Ctrl+Z. Ten muchisimo cuidado con `rm -rf`, y **nunca** ejecutes `rm -rf /` ni pegues comandos con `rm` que no entiendas.

> [!note] `touch` en la Practica 1
> El ejercicio 1.2 te pide `touch aux.c` para "marcarlo como modificado". Eso es exactamente lo que hace `touch` sobre un fichero que ya existe: le pone la fecha actual. `make` compara fechas, asi que cree que el fichero cambio y lo recompila. Ver [Herramientas - gcc make gdb](../herramientas-gcc-make-gdb.md).

---

## 4. Comodines (globbing)

Los expande **el shell**, no el programa. Cuando escribes `ls *.c`, el shell ya le pasa a `ls` la lista de ficheros.

| Comodin | Casa con | `*.c` en un dir con `a.c b.c a.h` |
|---------|----------|-----------------------------------|
| `*` | cualquier cosa (incluido nada) | `a.c b.c` |
| `?` | exactamente 1 caracter | `?.c` -> `a.c b.c` |
| `[abc]` | uno de esos caracteres | `[a]*` -> `a.c a.h` |
| `[a-z]` | un rango | |

```bash
rm *.o              # borrar todos los .o
cp *.c backup/      # copiar todos los fuentes
ls ejercicio?/      # ejercicio1/ ejercicio2/ ...
```

---

## 5. Cosas que te ahorran la vida

| Tecla | Efecto |
|-------|--------|
| **TAB** | autocompletar. **Usalo siempre.** Dos veces = ver opciones. |
| Flecha arriba | comando anterior |
| `Ctrl + R` | buscar en el historial (escribe parte del comando) |
| `Ctrl + C` | matar el programa que esta corriendo |
| `Ctrl + D` | fin de entrada (EOF) / cerrar la sesion |
| `Ctrl + L` | limpiar pantalla (igual que `clear`) |
| `Ctrl + A` / `Ctrl + E` | ir al principio / final de la linea |

```bash
history         # ver el historial de comandos
which gcc       # donde esta el ejecutable gcc
file hola       # que tipo de fichero es
du -sh carpeta  # cuanto ocupa una carpeta
df -h           # espacio libre en disco
```

---

## Chuleta para copiar y pegar

```bash
# Preparar el entorno de la Practica 1
cd ~                          # a mi home
mkdir -p so/p1                # crear estructura
cd so/p1                      # entrar
tar -xzvf ~/Descargas/ficheros_p1.tar.gz   # descomprimir la practica
ls -la                        # ver que ha salido
cd alumnos/ejercicio1         # entrar al primer ejercicio
```

`tar -xzvf`: **x**tract, **z** (gzip), **v**erbose, **f**ile. Para comprimir seria `-czvf`.

---

## Ejercicios para hacer AHORA en tu VM

1. Averigua en que directorio estas y ve a tu home por tres caminos distintos.
2. Crea `~/so/pruebas`, entra, crea 3 ficheros vacios `a.c`, `b.c`, `a.h`.
3. Lista solo los `.c`. Luego lista todo en formato largo y di quien es el propietario.
4. Copia la carpeta `pruebas` entera a `pruebas_backup`.
5. Renombra `a.c` a `main.c` sin usar `cp`.
6. Borra todos los `.c` de `pruebas` con un solo comando.
7. Averigua donde esta instalado `gcc` y que tipo de fichero es.
