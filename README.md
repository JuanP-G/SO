<p align="center"><img src="assets/banner.svg" alt="Sistemas Operativos" width="100%"></p>

Material de la asignatura **Sistemas Operativos** (UCM): apuntes de C y terminal, soluciones comentadas de las prácticas y vídeos explicativos de cada ejercicio. Todo en rutas relativas: clona el repositorio y funciona igual en cualquier equipo.

## Contenido

| | Bloque | Qué hay | Estado |
|---|---|---|---|
| <img src="assets/icons/book.svg" width="24" height="24" align="absmiddle"> | [Fundamentos](fundamentos/) | C, terminal, entorno de trabajo y herramientas | Completo |
| <img src="assets/icons/file.svg" width="24" height="24" align="absmiddle"> | [P1 · Introducción](practicas/p1-introduccion/) | Runbook del ejercicio 1 | Parcial |
| <img src="assets/icons/file.svg" width="24" height="24" align="absmiddle"> | [P2 · Ficheros con stdio](practicas/p2-ficheros-stdio/) | Corrección comentada y código | Completo |
| <img src="assets/icons/file.svg" width="24" height="24" align="absmiddle"> | [P3 · Llamadas al sistema](practicas/p3-llamadas-al-sistema/) | `copy.c` con open/read/write | Parcial |
| <img src="assets/icons/process.svg" width="24" height="24" align="absmiddle"> | [P4 · Procesos, hilos, señales y ficheros](practicas/p4-procesos-hilos-senales/) | Corrección, código y 5 vídeos | Completo |
| <img src="assets/icons/sync.svg" width="24" height="24" align="absmiddle"> | [P5 · Sincronización de hilos](practicas/p5-sincronizacion-hilos/) | Explicación, código y vídeo | Completo |

## Mapa por temas

| Tema | Dónde |
|---|---|
| <img src="assets/icons/c.svg" width="20" height="20" align="absmiddle"> Punteros, memoria dinámica, structs | [fundamentos/c](fundamentos/c/) |
| <img src="assets/icons/terminal.svg" width="20" height="20" align="absmiddle"> Redirecciones, pipes, permisos, scripts | [fundamentos/terminal](fundamentos/terminal/) |
| <img src="assets/icons/tools.svg" width="20" height="20" align="absmiddle"> gcc, make, gdb | [herramientas](fundamentos/herramientas-gcc-make-gdb.md) |
| <img src="assets/icons/process.svg" width="20" height="20" align="absmiddle"> fork, exec, wait | [P4 · Ejercicio 1](practicas/p4-procesos-hilos-senales/) |
| <img src="assets/icons/thread.svg" width="20" height="20" align="absmiddle"> pthread, paso de argumentos | [P4 · Ejercicio 2](practicas/p4-procesos-hilos-senales/) |
| <img src="assets/icons/signal.svg" width="20" height="20" align="absmiddle"> alarm, kill, sigaction | [P4 · Ejercicio 3](practicas/p4-procesos-hilos-senales/) |
| <img src="assets/icons/file.svg" width="20" height="20" align="absmiddle"> lseek, pwrite, descriptores | [P4 · Ejercicio 4](practicas/p4-procesos-hilos-senales/) |
| <img src="assets/icons/sync.svg" width="20" height="20" align="absmiddle"> mutex, variables de condición | [P5](practicas/p5-sincronizacion-hilos/) |

## Estructura

```text
.
├── README.md
├── assets/                  iconos y banner (SVG)
├── fundamentos/
│   ├── c/                   apuntes de C (01-06)
│   ├── terminal/            apuntes de terminal (01-05)
│   ├── entorno-de-trabajo.md
│   └── herramientas-gcc-make-gdb.md
└── practicas/
    └── pN-tema/
        ├── README.md        índice de la práctica
        ├── *.md             explicación / corrección comentada
        ├── codigo/          fuentes y Makefile
        └── videos/          vídeos explicativos (si los hay)
```

## Cómo compilar y probar

Requiere Linux, WSL o una máquina virtual con `gcc`, `make` y las cabeceras POSIX.

```bash
cd practicas/p5-sincronizacion-hilos/codigo
make
./disco ejemplo.txt
```

Cada carpeta de código incluye su `Makefile`; `make clean` borra los binarios.

## Orden de estudio recomendado

1. [Entorno de trabajo](fundamentos/entorno-de-trabajo.md) y [herramientas](fundamentos/herramientas-gcc-make-gdb.md).
2. Apuntes de [C](fundamentos/c/) y de [terminal](fundamentos/terminal/).
3. Prácticas en orden: P2, P3, P4, P5. En cada una, leer la explicación, ver el vídeo y después abrir el código.

## Sobre los vídeos

Cada vídeo explica un ejercicio con animaciones y metáforas. La narración es voz sintética en español, por lo que alguna palabra técnica puede sonar rara. Los archivos pesan entre 4 y 13 MB.
