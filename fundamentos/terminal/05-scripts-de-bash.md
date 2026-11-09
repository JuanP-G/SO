# SO - Terminal 05 - Scripts de bash

Parte de [Inicio](../../README.md). Anterior: [Terminal 04 - Permisos y man](04-permisos-y-man.md) | Siguiente: [C 01 - Del codigo al ejecutable](../c/01-del-codigo-al-ejecutable.md)

---

## 1. Anatomia de un script

```bash
#!/bin/bash
# esto es un comentario

echo "Hola mundo"
```

| Elemento | Explicacion |
|----------|-------------|
| `#!/bin/bash` | **shebang**. Le dice al SO con que interprete ejecutar el fichero. Tiene que ser la **primera linea**. |
| `#` | comentario |

Para ejecutarlo:

```bash
chmod +x script.sh      # una sola vez: darle permiso de ejecucion
./script.sh             # ejecutarlo (crea un proceso bash nuevo)

bash script.sh          # alternativa: no necesita permiso x
source script.sh        # lo ejecuta en TU shell actual, sin crear proceso nuevo
```

**Comandos internos vs externos** (esto cae en el examen):

| Tipo | Que es | Ejemplos |
|------|--------|----------|
| Interno (*built-in*) | Parte del propio shell, no hay fichero | `cd`, `echo`, `read`, `export`, `test`, `[`, `pwd` |
| Externo | Un programa instalado en el sistema | `ls`, `grep`, `cut`, `gcc`, `cat` |

`type cd` te dice cual es cual. `cd` **tiene** que ser interno: si fuese un programa externo cambiaria el directorio de su propio proceso y al terminar no pasaria nada.

---

## 2. Variables

```bash
nombre="Juan"        # SIN espacios alrededor del '='   <-- error clasico
edad=21
echo $nombre         # expansion: Juan
echo "${nombre}s"    # las llaves delimitan el nombre: Juans
```

| Forma | Cuando usarla |
|-------|---------------|
| `$var` | normal |
| `"$var"` | **casi siempre**: protege espacios y caracteres raros |
| `'$var'` | comillas simples: NO expande, imprime literalmente `$var` |
| `${var}` | cuando pega con texto detras |
| `$(orden)` | **sustitucion de orden**: mete la SALIDA de la orden |
| `$(( 2+3 ))` | aritmetica entera |

```bash
fecha=$(date)              # guarda la salida de date
n=$(wc -l < /etc/passwd)   # numero de usuarios
suma=$(( 5 + 3 ))          # 8
doble=$(( n * 2 ))
```

> [!warning] Bash solo sabe de enteros
> `$(( 7 / 2 ))` da **3**, no 3.5. Para decimales hace falta `bc` o `awk`.

### Variables especiales

| Variable | Contiene |
|----------|----------|
| `$0` | nombre del script |
| `$1`, `$2`, ... | argumentos de la linea de comandos |
| `$#` | numero de argumentos |
| `$@` | todos los argumentos |
| `$?` | codigo de salida de la ultima orden |
| `$$` | PID del shell actual |
| `IFS` | separador de campos (por defecto espacio/tab/salto de linea) |

---

## 3. Condicionales

```bash
if [ "$edad" -ge 18 ]; then
    echo "mayor de edad"
elif [ "$edad" -ge 12 ]; then
    echo "adolescente"
else
    echo "nino"
fi
```

> [!danger] Los espacios dentro de `[ ]` son obligatorios
> `[` **es un programa** (un alias de `test`), no sintaxis. `[$a -eq 1]` falla siempre.

### Comparaciones

| Numeros | Cadenas | Significado |
|---------|---------|-------------|
| `-eq` | `=` | igual |
| `-ne` | `!=` | distinto |
| `-lt` | `<` | menor |
| `-le` | | menor o igual |
| `-gt` | `>` | mayor |
| `-ge` | | mayor o igual |

### Comprobaciones sobre ficheros (las de la P1)

| Test | Verdadero si... |
|------|-----------------|
| `-e fich` | existe |
| `-f fich` | existe y es fichero normal |
| **`-d dir`** | **existe y es un directorio** |
| `-r` / `-w` / `-x` | se puede leer / escribir / ejecutar |
| `-z "$s"` | la cadena esta vacia |
| `-n "$s"` | la cadena NO esta vacia |

Combinadores: `!` (no), `-a` o `&&` (y), `-o` o `||` (o).

```bash
if [ -d "/home/juan" ]; then echo "es un directorio"; fi
if [ "$(dirname "$home")" = "/home" ]; then echo "cuelga de /home"; fi
```

---

## 4. Bucles

```bash
for i in 1 2 3 4 5; do
    echo "numero $i"
done

for i in $(seq 1 10); do echo $i; done     # del 1 al 10
for f in *.c; do echo "fuente: $f"; done   # sobre ficheros
for ((i=0; i<10; i++)); do echo $i; done   # estilo C

while [ $n -lt 10 ]; do
    echo $n
    n=$(( n + 1 ))
done
```

---

## 5. `read` - el corazon del ejercicio 5 de la Practica 1

`read` lee **una linea** de la entrada estandar, la parte en palabras usando `IFS` como separador, y asigna cada palabra a una variable.

```bash
read var1 var2 var3        # lee del teclado
```

Si sobran palabras, **todas las restantes van a la ultima variable**.

### Patron 1: recorrer un fichero linea a linea

```bash
while read linea
do
    echo "$linea"
done < fichero            # <-- la redireccion va en el 'done'
```

### Patron 2: cambiar el separador con `IFS`

```bash
while IFS=':' read usuario passwd uid gid gecos home shell
do
    echo "Usuario: $usuario"
done < /etc/passwd
```

Poner `IFS=':'` **delante del `read`** hace que solo valga para esa orden, no para todo el script. Es exactamente lo que te pide el enunciado.

### Script completo tipo `show-passwd` (ejercicio 5)

```bash
#!/bin/bash
# Muestra las entradas de /etc/passwd

while IFS=':' read user pass uid gid gecos home shell
do
    echo "-----------------------------"
    echo -e "Usuario:\t$user"
    echo -e "UID:\t\t$uid"
    echo -e "GID:\t\t$gid"
    echo -e "Nombre:\t\t$gecos"
    echo -e "Home:\t\t$home"
    echo -e "Shell:\t\t$shell"
done < /etc/passwd
```

`echo -e` interpreta `\t` (tabulador) y `\n` (salto de linea). Sin `-e` los imprime literalmente.

### Variante: solo los que tengan el home colgando de `/home`

```bash
#!/bin/bash
while IFS=':' read user pass uid gid gecos home shell
do
    if [ "$(dirname "$home")" = "/home" ]; then
        echo -e "$user\t$home"
    fi
done < /etc/passwd
```

`dirname /home/juan` devuelve `/home`. `dirname /root` devuelve `/`. Por eso el `if` filtra justo lo que se pide.

### Y la version de una sola linea con pipes

```bash
cut -d':' -f6 /etc/passwd | grep '^/home'
```

> [!note] Moraleja de la Practica 1
> El mismo trabajo se puede hacer con un programa en C de 200 lineas, con un script de bash de 15, o con una tuberia de 1. Parte del objetivo del ejercicio es que veas esa diferencia.

---

## 6. `printf` en bash

Mas potente que `echo` para formatear. Mismos formatos que el `printf()` de C:

```bash
printf "%-10s %5d\n" "$user" "$uid"    # cadena a izquierda en 10, entero en 5
```

| Formato | Imprime |
|---------|---------|
| `%s` | cadena |
| `%d` | entero |
| `%-10s` | cadena alineada a la izquierda en 10 caracteres |
| `%5d` | entero alineado a la derecha en 5 |
| `\n` | salto de linea (aqui no hace falta `-e`) |

---

## Ejercicios

1. Escribe un script que salude usando el primer argumento: `./saluda.sh Juan` -> `Hola Juan`.
2. Que imprima cuantos argumentos ha recibido.
3. Que reciba un nombre de fichero y diga si existe, y si es fichero o directorio.
4. El script `show-passwd` de arriba, escrito por ti de memoria.
5. Modificalo para que solo muestre usuarios con UID >= 1000 (los usuarios reales, no los del sistema).
6. Cuenta con un bucle `while read` cuantas lineas tiene `/etc/passwd`, sin usar `wc`.
