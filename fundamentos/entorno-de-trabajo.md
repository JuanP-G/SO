# SO - Entorno de trabajo

Parte de [Inicio](../README.md). Relacionado: [Herramientas - gcc make gdb](herramientas-gcc-make-gdb.md)

---

## 1. Mi entorno

| | |
|---|---|
| Maquina | VirtualBox, VM reaprovechada de Redes |
| SO | **Ubuntu 22.04.5 LTS** (soporte hasta abril 2027) |
| Disco | 98 GB, ~78 GB libres |
| Host | Windows 11 (portatil y sobremesa) |

> [!note] No hizo falta reinstalar
> La "VM de SO" de la guia oficial no tiene nada especial: los pasos 1-7 son montar un Ubuntu cualquiera en VirtualBox. Lo unico que la convierte en la VM de la asignatura es el **Paso 8**, que es un `apt install`. Cualquier Ubuntu LTS con esos paquetes sirve.

---

## 2. Paquetes de la asignatura

```bash
sudo apt update && sudo apt full-upgrade
sudo apt install build-essential gcc make gdb glibc-doc git man-db \
     bash-completion whois strace time wget curl manpages-dev \
     procps htop valgrind
```

| Paquete | Para que |
|---------|----------|
| `build-essential` | gcc + cabeceras + `libc6-dev`. Sin el, faltan cosas al compilar |
| `gcc` `make` `gdb` | compilar, construir proyectos, depurar |
| **`glibc-doc`** `manpages-dev` | las paginas `man 3 ...` de las funciones de C. **Sin esto `man 3 getopt` da "No manual entry"** |
| `strace` | ver que llamadas al sistema hace un proceso. Oro puro a partir del Modulo 3 |
| **`valgrind`** | detectar fugas de memoria y accesos fuera de rango. No sale en el Paso 8 del PDF pero si en el Dockerfile del entorno de VSCode |
| `whois` `time` `htop` `procps` | utilidades sueltas que usan las practicas |

### Comprobar que esta todo

```bash
lsb_release -d
df -h /
for p in gcc make gdb git strace valgrind; do
    printf "%-10s " "$p"; which $p || echo "NO INSTALADO"
done
valgrind --version
man 3 getopt | head -5      # si sale la firma, el entorno esta listo
```

---

## 3. Guest Additions (VirtualBox)

Sin ellas no se redimensiona la ventana ni funciona el portapapeles compartido con Windows.

```bash
sudo apt install linux-headers-$(uname -r) build-essential
# menu Devices -> Insert Guest Additions CD image...
cd /media/$USER/VBox_GAs_*/
sudo ./VBoxLinuxAdditions.run
sudo reboot
```

Y en *Settings -> General -> Advanced*, poner **Shared Clipboard: Bidirectional**.

---

## 4. Pasar ficheros entre Windows y la VM

Opcion recomendada: **carpeta compartida**.

1. VM apagada -> *Settings -> Shared Folders -> +*
2. Elegir la carpeta de Windows, marcar **Auto-mount** y **Make Permanent**
3. Arrancar la VM y darse permiso:

```bash
sudo usermod -aG vboxsf $USER
# cerrar sesion y volver a entrar
ls /media/sf_<nombre_carpeta>
```

> [!warning] Compila SIEMPRE dentro de la VM
> Puedes editar desde Windows en la carpeta compartida, pero `gcc`, `make` y `gdb` tienen que ejecutarse en el Ubuntu. Un ejecutable compilado en Windows no vale para nada aqui.

---

## 5. Snapshots: haz uno AHORA

Antes de empezar a trastear con las practicas:

*VirtualBox -> la VM -> Snapshots -> Take*

Nombralo `entorno SO limpio`. Si algun dia rompes el sistema (y en esta asignatura se rompen cosas: drivers en la P7), vuelves ahi en 10 segundos en vez de reinstalar.

---

## 6. Alias que ahorran tiempo

En `~/.bashrc`:

```bash
alias gccw='gcc -Wall -Wextra -g'
alias ll='ls -lah'
alias vg='valgrind --leak-check=full'
```

```bash
source ~/.bashrc      # aplicar sin reabrir el terminal
```

Luego: `gccw hola.c -o hola && ./hola`

---

## 7. El entorno alternativo de VSCode + Docker

El `.tar.gz` que dio el profesor (`entorno-vscode-docker`) es un **devcontainer** con Debian 11.7 y los mismos paquetes. Es un **camino alternativo** a la VM, no un complemento.

> [!tip] Si ya tienes la VM, ignoralo
> Dos entornos solo sirven para acabar sin saber donde dejaste los ficheros. Ademas Debian 11 lleva gcc 10, mas viejo que el gcc 11 de Ubuntu 22.04.

---

## 8. Erratas de la guia oficial

- El texto dice *Ubuntu 26.04.1 LTS* pero la captura muestra `ubuntu-24.04.1-desktop-amd64.iso`. Cualquier LTS reciente vale.
- El Paso 8 no incluye `build-essential` ni `valgrind`, que si estan en el Dockerfile del entorno de VSCode. Instalalos igualmente.
