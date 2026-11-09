# SO - Terminal 02 - Ver y buscar texto

Parte de [Inicio](../../README.md). Anterior: [Terminal 01 - Navegacion y ficheros](01-navegacion-y-ficheros.md) | Siguiente: [Terminal 03 - Redirecciones y pipes](03-redirecciones-y-pipes.md)

---

## Idea de fondo: filosofia UNIX

> Cada programa hace **una sola cosa** y la hace bien, y lee/escribe **texto plano** para poder encadenarse con otros.

Por eso `/etc/passwd` es un fichero de texto y no una base de datos binaria. Y por eso el ejercicio 5 de la Practica 1 te pide sacar lo mismo que un programa en C usando cuatro ordenes encadenadas.

---

## 1. Ver el contenido

| Orden | Para que |
|-------|----------|
| `cat fich` | volcar el fichero entero a pantalla |
| `cat f1 f2` | concatenar varios (de ahi el nombre) |
| `less fich` | ver paginado. `q` para salir, `/texto` para buscar, flechas para moverte |
| `head -n 5 fich` | primeras 5 lineas |
| `tail -n 5 fich` | ultimas 5 lineas |
| `tail -f log` | ir mostrando lo que se anade en tiempo real |
| `wc -l fich` | contar lineas (`-w` palabras, `-c` bytes) |

```bash
cat /etc/passwd          # el fichero de la practica 1
head -n 3 /etc/passwd
wc -l /etc/passwd        # cuantos usuarios hay
```

---

## 2. El fichero `/etc/passwd` (lo necesitas para la P1)

Una linea por usuario, campos separados por `:`

```
root:x:0:0:root:/root:/bin/bash
juan:x:1000:1000:Juan Perez,,,:/home/juan:/bin/bash
 │   │  │    │        │            │          │
 │   │  │    │        │            │          └── 7. shell por defecto
 │   │  │    │        │            └── 6. directorio home
 │   │  │    │        └── 5. GECOS (nombre real, comentario)
 │   │  │    └── 4. GID (grupo)
 │   │  └── 3. UID (identificador numerico de usuario)
 │   └── 2. password ('x' = esta en /etc/shadow)
 └── 1. nombre de usuario
```

`man 5 passwd` te da esta misma tabla. El **5** es la seccion del manual (ver mas abajo en [Terminal 04 - Permisos y man](04-permisos-y-man.md)).

---

## 3. `grep` - buscar lineas

```bash
grep root /etc/passwd            # lineas que contengan 'root'
grep -i ROOT /etc/passwd         # ignorando mayus/minus
grep -v nologin /etc/passwd      # lineas que NO contengan 'nologin'  <-- muy util
grep -n printf hola.c            # mostrando el numero de linea
grep -c bash /etc/passwd         # solo contar cuantas casan
grep -r malloc .                 # buscar recursivamente en todo el directorio
grep '^root' /etc/passwd         # las que EMPIEZAN por root
grep 'bash$' /etc/passwd         # las que TERMINAN en bash
```

| Opcion | Efecto |
|--------|--------|
| `-i` | ignorar mayusculas |
| `-v` | invertir: mostrar las que NO casan |
| `-n` | numero de linea |
| `-c` | contar |
| `-r` | recursivo por directorios |
| `-w` | palabra completa |

Expresiones regulares basicas: `^` principio de linea, `$` final, `.` un caracter cualquiera, `*` repeticion del anterior, `[0-9]` un digito.

---

## 4. `cut` - quedarse con columnas

```bash
cut -d':' -f1 /etc/passwd        # campo 1 usando ':' como delimitador -> los usuarios
cut -d':' -f1,6 /etc/passwd      # campos 1 y 6 -> usuario y home
cut -d':' -f6 /etc/passwd        # solo los homes
cut -c1-5 fichero                # caracteres 1 a 5 de cada linea
```

| Opcion | Significa |
|--------|-----------|
| `-d` | **d**elimitador (por defecto TAB) |
| `-f` | **f**ield, que campo(s) quiero |
| `-c` | por **c**aracteres, no por campos |

> [!tip] Ejercicio 5 de la Practica 1
> Te piden "todos los homes que empiecen por /home" combinando `cut` y `grep`. Ya tienes las dos piezas. Solo falta el pegamento, que es la tuberia `|` de [Terminal 03 - Redirecciones y pipes](03-redirecciones-y-pipes.md).

---

## 5. Ordenar, contar, quitar repetidos

```bash
sort fichero              # ordenar alfabeticamente
sort -n numeros.txt       # ordenar numericamente (10 despues de 9, no antes)
sort -r fichero           # al reves
sort -t':' -k3 -n /etc/passwd    # ordenar por el campo 3 (UID) numericamente
uniq fichero              # quitar duplicados CONSECUTIVOS (casi siempre tras sort)
sort f | uniq -c          # contar cuantas veces aparece cada linea
tr ':' ' ' < /etc/passwd  # traducir/sustituir caracteres
```

---

## 6. `find` - buscar ficheros

```bash
find . -name "*.c"                 # todos los .c desde aqui hacia abajo
find /etc -name "passwd"           # buscar por nombre
find . -type d                     # solo directorios
find . -name "*.o" -delete         # buscar y borrar
find . -name "*.c" -exec wc -l {} \;   # ejecutar algo sobre cada resultado
```

`grep` busca **dentro** de los ficheros; `find` busca **los ficheros**.

---

## Ejercicios

Todos sobre `/etc/passwd`, en tu VM:

1. Cuantas lineas tiene.
2. Saca solo la lista de nombres de usuario.
3. Saca solo los usuarios cuyo shell sea `/bin/bash`.
4. Cuenta cuantos usuarios NO pueden hacer login (shell `nologin` o `false`).
5. Saca usuario y home, separados por `:`, ordenados alfabeticamente.
6. Encuentra todos los ficheros `.c` de la Practica 1 y cuenta cuantos son.

(Las soluciones de los que necesitan `|` estan en la nota siguiente.)
