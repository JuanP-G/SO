<p align="center"><img src="assets/banner.svg" alt="Sistemas Operativos" width="100%"></p>

Material de la asignatura **Sistemas Operativos** (UCM): apuntes de C y terminal, código de las prácticas, correcciones comentadas y vídeos explicativos. Todo en rutas relativas: clona el repositorio y funciona igual en cualquier equipo.

## Contenido

| | Bloque | Qué hay | Enunciado | Notas | Vídeos |
|---|---|---|---|---|---|
| <img src="assets/icons/note.svg" width="24" height="24" align="absmiddle"> | [Temario](temario/) | Teoría (módulos 1 a 3) y hojas de ejercicios | | PDF | |
| <img src="assets/icons/book.svg" width="24" height="24" align="absmiddle"> | [Fundamentos](fundamentos/) | C, terminal, entorno, herramientas y guías | | Sí | |
| <img src="assets/icons/file.svg" width="24" height="24" align="absmiddle"> | [PR1 · Introducción](practicas/PR1-introduccion/) | Ejercicios 1 a 4 y manual | [pdf](practicas/PR1-introduccion/practica1.pdf) | [Runbook ej. 1](practicas/PR1-introduccion/ejercicio1-runbook.md) | |
| <img src="assets/icons/file.svg" width="24" height="24" align="absmiddle"> | [PR2 · Ficheros stdio](practicas/PR2-ficheros_stdio/) | Ejercicios 1 a 3 | [pdf](practicas/PR2-ficheros_stdio/practica2.pdf) | [Corrección](practicas/PR2-ficheros_stdio/correccion-comentada.md) | |
| <img src="assets/icons/file.svg" width="24" height="24" align="absmiddle"> | [PR3 · Procesos e hilos](practicas/PR3-procesos_e_hilos/) | Ejercicios 1 a 4 con `check_*` | [pdf](practicas/PR3-procesos_e_hilos/practica3.pdf) | | |
| <img src="assets/icons/process.svg" width="24" height="24" align="absmiddle"> | [PR4 · Procesos, hilos, señales y ficheros](practicas/PR4-sincronizacion-hilos/) | Ejercicios 1 a 4 | [pdf](practicas/PR4-sincronizacion-hilos/practica4.pdf) | [Corrección](practicas/PR4-sincronizacion-hilos/correccion-comentada.md) | [5 vídeos](practicas/PR4-sincronizacion-hilos/videos/) |
| <img src="assets/icons/sync.svg" width="24" height="24" align="absmiddle"> | [PR5 · Sincronización de hilos](practicas/PR5-sincronizacion_de_hilos/) | Discoteca con mutex y condiciones | [pdf](practicas/PR5-sincronizacion_de_hilos/practica5.pdf) | [Explicación](practicas/PR5-sincronizacion_de_hilos/explicacion.md) | [vídeo](practicas/PR5-sincronizacion_de_hilos/videos/discoteca.mp4) |

## Mapa por temas

| Tema | Dónde |
|---|---|
| <img src="assets/icons/c.svg" width="20" height="20" align="absmiddle"> Punteros, memoria dinámica, structs | [fundamentos/c](fundamentos/c/) |
| <img src="assets/icons/terminal.svg" width="20" height="20" align="absmiddle"> Redirecciones, pipes, permisos, scripts | [fundamentos/terminal](fundamentos/terminal/) |
| <img src="assets/icons/tools.svg" width="20" height="20" align="absmiddle"> gcc, make, gdb | [herramientas](fundamentos/herramientas-gcc-make-gdb.md) |
| <img src="assets/icons/file.svg" width="20" height="20" align="absmiddle"> stdio: fopen, fread, fwrite | [PR2](practicas/PR2-ficheros_stdio/) |
| <img src="assets/icons/process.svg" width="20" height="20" align="absmiddle"> fork, exec, wait | [PR4 · ejercicio 1](practicas/PR4-sincronizacion-hilos/ejercicio1/) |
| <img src="assets/icons/thread.svg" width="20" height="20" align="absmiddle"> pthread, paso de argumentos | [PR4 · ejercicio 2](practicas/PR4-sincronizacion-hilos/ejercicio2/) |
| <img src="assets/icons/signal.svg" width="20" height="20" align="absmiddle"> alarm, kill, sigaction | [PR4 · ejercicio 3](practicas/PR4-sincronizacion-hilos/ejercicio3/) |
| <img src="assets/icons/file.svg" width="20" height="20" align="absmiddle"> lseek, pwrite, descriptores | [PR4 · ejercicio 4](practicas/PR4-sincronizacion-hilos/ejercicio4/) |
| <img src="assets/icons/sync.svg" width="20" height="20" align="absmiddle"> mutex, variables de condición | [PR5](practicas/PR5-sincronizacion_de_hilos/) |

## Estructura

```text
.
├── README.md
├── assets/                          iconos y banner (SVG)
├── temario/                         teoría y hojas de ejercicios (PDF)
├── fundamentos/
│   ├── guias/                       guías de entorno y máquina virtual (PDF)
│   ├── c/                           apuntes de C (01-06)
│   ├── terminal/                    apuntes de terminal (01-05)
│   ├── entorno-de-trabajo.md
│   └── herramientas-gcc-make-gdb.md
└── practicas/
    ├── PR1-introduccion/
    ├── PR2-ficheros_stdio/
    ├── PR3-procesos_e_hilos/
    ├── PR4-sincronizacion-hilos/
    └── PR5-sincronizacion_de_hilos/
        ├── README.md                índice de la práctica
        ├── practicaN.pdf            enunciado
        ├── ejercicioN/              código propio y Makefile
        ├── referencia/              soluciones de apoyo (si las hay)
        ├── videos/                  vídeos explicativos (si los hay)
        └── *.md                     explicación / corrección comentada
```

## Cómo compilar y probar

Requiere Linux, WSL o una máquina virtual con `gcc`, `make` y las cabeceras POSIX.

```bash
cd practicas/PR5-sincronizacion_de_hilos/ejercicio1
make
./disco ejemplo.txt
```

Cada carpeta de ejercicio incluye su `Makefile`; `make clean` borra los binarios.

## Orden de estudio recomendado

1. [Entorno de trabajo](fundamentos/entorno-de-trabajo.md) y [herramientas](fundamentos/herramientas-gcc-make-gdb.md).
2. Apuntes de [C](fundamentos/c/) y de [terminal](fundamentos/terminal/).
3. Prácticas en orden. En cada una: leer la explicación, ver el vídeo si lo hay y después abrir el código.

## Sobre los vídeos

Cada vídeo explica un ejercicio con animaciones y metáforas. La narración es voz sintética en español, por lo que alguna palabra técnica puede sonar rara. Los archivos pesan entre 4 y 13 MB.
