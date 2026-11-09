# SO - Terminal 03 - Redirecciones y pipes

Parte de [Inicio](../../README.md). Anterior: [Terminal 02 - Ver y buscar texto](02-ver-y-buscar-texto.md) | Siguiente: [Terminal 04 - Permisos y man](04-permisos-y-man.md)

---

## 1. Los tres descriptores de fichero

Todo proceso nace en UNIX con **tres canales abiertos**. Esto es teoria de examen, no solo practica.

| N | Nombre | Simbolo C | Por defecto apunta a | Para que |
|---|--------|-----------|----------------------|----------|
| **0** | `stdin` | `stdin` | teclado | entrada de datos |
| **1** | `stdout` | `stdout` | terminal | salida normal |
| **2** | `stderr` | `stderr` | terminal | mensajes de error |

```
                 ┌──────────────┐
   teclado ─0──> │              │ ──1──> pantalla
                 │   PROCESO    │
                 │              │ ──2──> pantalla
                 └──────────────┘
```

> [!important] Por que stdout y stderr estan separados
> Para poder guardar el resultado en un fichero y seguir viendo los errores en pantalla. Si fuesen el mismo canal, los errores se te colarian dentro de los datos.

---

## 2. Redirecciones

| Sintaxis | Que hace |
|----------|----------|
| `cmd > fich` | stdout **al fichero** (lo machaca si existe) |
| `cmd >> fich` | stdout al fichero, **anadiendo al final** |
| `cmd 2> fich` | stderr al fichero |
| `cmd 2>> fich` | stderr anadiendo |
| `cmd < fich` | el fichero como **stdin** |
| `cmd > out 2> err` | cada canal a su fichero |
| `cmd > fich 2>&1` | stderr **al mismo sitio** que stdout |
| `cmd &> fich` | atajo de lo anterior (bash) |
| `cmd > /dev/null` | tirar la salida a la basura |

```bash
ls -l > listado                   # guardar el listado
ls -l /etc >> listado             # anadir mas
ls /bin/bash /noexiste 2> errores # el error va al fichero, lo bueno a pantalla
find / -name "lib*" > libs 2>&1   # todo (salida y errores) al mismo fichero
gcc hola.c 2> errores.txt         # guardar los errores del compilador  <-- muy util
```

> [!warning] El orden importa en `2>&1`
> `cmd > f 2>&1` -> primero stdout va a `f`, luego stderr va "donde este stdout" = `f`. **Correcto.**
> `cmd 2>&1 > f` -> primero stderr va donde esta stdout (la pantalla), y luego stdout va a `f`. Los errores se quedan en pantalla. **Casi nunca es lo que quieres.**

### `/dev/null`

Es el "agujero negro" del sistema: todo lo que escribes ahi desaparece.

```bash
comando 2> /dev/null      # silenciar los errores
comando > /dev/null 2>&1  # silenciarlo todo
```

---

## 3. Tuberias (pipes) `|`

**La salida de un programa se convierte en la entrada del siguiente.** Sin ficheros intermedios.

```
  cmd1  ──stdout──>│pipe│──stdin──>  cmd2  ──stdout──> pantalla
```

```bash
ls -l | less                              # paginar un listado largo
cat /etc/passwd | grep bash               # lineas con bash
ps aux | grep root                        # procesos de root
cut -d':' -f7 /etc/passwd | sort | uniq -c   # cuantos usuarios por shell
```

### Resolviendo el ejercicio 5 de la Practica 1

> "Obtener del fichero /etc/passwd todos los homes que empiecen por /home, combinando `cut` y `grep`."

Pensarlo en pasos:

| Paso | Orden | Resultado |
|------|-------|-----------|
| 1. Quedarme con el campo del home | `cut -d':' -f6 /etc/passwd` | lista de todos los homes |
| 2. Filtrar los que empiezan por /home | `grep '^/home'` | solo esos |

```bash
cut -d':' -f6 /etc/passwd | grep '^/home'
```

Tambien vale al reves (`grep` primero, `cut` despues), pero ojo: `grep '/home'` sobre la linea entera casaria tambien con nombres de usuario que contengan "home". Filtrar **despues** de recortar es mas seguro.

### Se pueden combinar pipes y redirecciones

```bash
ps aux | grep -v root > ps.out          # filtrar y guardar
cat /etc/passwd | cut -d':' -f1 | sort > usuarios.txt
```

---

## 4. Listas de ordenes y el codigo de salida

Todo programa devuelve al terminar un **codigo de estado** (el `return 0;` de `main()`).

| Valor | Convenio |
|-------|----------|
| `0` | exito |
| distinto de `0` | fallo |

La variable `$?` guarda el codigo de la ultima orden:

```bash
ls /etc
echo $?        # 0

ls /noexiste
echo $?        # 2  (distinto de 0 = fallo)
```

| Sintaxis | Significado |
|----------|-------------|
| `cmd1 ; cmd2` | ejecuta cmd2 **siempre**, despues de cmd1 |
| `cmd1 && cmd2` | ejecuta cmd2 **solo si cmd1 tuvo exito** (status 0) |
| `cmd1 \|\| cmd2` | ejecuta cmd2 **solo si cmd1 fallo** |

```bash
make && ./getopt hola          # compila y, SI compilo bien, ejecuta
gcc hola.c -o hola && ./hola   # el patron que mas vas a usar
mkdir dir || echo "ya existia"
cd ~ ; ls ; pwd                # tres cosas seguidas
```

> [!tip] El truco que mas tiempo te va a ahorrar
> `gcc -Wall -g hola.c -o hola && ./hola`
> Asi nunca ejecutas por error el binario viejo cuando la compilacion ha fallado.

---

## 5. Primer y segundo plano

```bash
./programa          # primer plano: el shell espera a que acabe
./programa &        # segundo plano: te devuelve el prompt al instante
                    # imprime  [1] 7584   ->  [job ID] PID

jobs                # ver los trabajos en segundo plano
fg %1               # traer el trabajo 1 a primer plano
bg %1               # mandarlo a segundo plano
Ctrl + Z            # suspender el proceso en primer plano
Ctrl + C            # matarlo
kill 7584           # matar por PID
kill -9 7584        # matarlo a lo bruto (SIGKILL)
```

**PID** = Process ID, el identificador del proceso que le asigna el SO. Lo veras muchisimo en el Modulo 3.

---

## Ejercicios

1. Guarda en `usuarios.txt` la lista ordenada de nombres de usuario de `/etc/passwd`.
2. Cuenta cuantos usuarios usan cada shell distinto (pista: `cut`, `sort`, `uniq -c`).
3. Ejecuta `ls /etc /noexiste` mandando la salida buena a `ok.txt` y los errores a `err.txt`.
4. Haz lo mismo pero mandandolo todo a un unico fichero.
5. Compila `hello2.c` de la practica y ejecutalo solo si compila, en una sola linea.
6. Averigua el codigo de salida de `grep pepito /etc/passwd`. Que significa?
