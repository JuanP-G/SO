[Volver al inicio](../../README.md) · Anterior: [P4 · Corrección comentada](../PR4-sincronizacion-hilos/correccion-comentada.md)

# P5 · Discoteca con aforo y VIP

Un solo ejercicio: **M hilos cliente**, aforo **N = 5**, clientes `vip` y normales. Vídeo explicativo: [discoteca.mp4](videos/discoteca.mp4).

## 1. Qué nos dan (`disco.c`)

Solo el esqueleto: `CAPACITY 5`, `VIPSTR`, `dance()` (printf + sleep) y funciones vacías `enter_normal_client`, `enter_vip_client`, `disco_exit`, `client`, `main`. Hay que rellenar todo lo demás.

## 2. Reglas -> condiciones

| Regla | Cómo se garantiza |
|---|---|
| Aforo lleno -> esperar | `inside >= CAPACITY` bloquea |
| VIP antes que normales | el normal también espera si `vip_waiting > 0` |
| Orden de llegada dentro de cada grupo | **turnos** (ticket) por grupo |

## 3. Herramientas

| Herramienta | Para qué |
|---|---|
| `pthread_mutex_t` | exclusión mutua: solo un hilo toca los contadores a la vez |
| `pthread_cond_wait(&cond, &mutex)` | suelta el mutex y duerme; al despertar lo recupera |
| `pthread_cond_broadcast(&cond)` | despierta a TODOS los dormidos de esa condición |
| `while` (no `if`) | al despertar hay que **volver a comprobar** la condición |

**Dos variables de condición**, una por cola: `normal_changed` (banco de los normales) y `vip_changed` (banco de los VIP). Así un aviso solo despierta a quien tiene opciones de entrar.

## 4. Estado compartido (todo protegido por `mutex`)

| Variable | Significado |
|---|---|
| `inside` | clientes dentro |
| `vip_waiting` | vips que han llegado y aún no han entrado |
| `next_vip_ticket` / `vip_turn` | dispensador / cartel de turno VIP |
| `next_normal_ticket` / `normal_turn` | ídem para normales |

## 5. Cómo funciona cada función

**`wake_next()`**: decide a quién avisar según **quién puede entrar**, no según quién acaba de salir. Si `vip_waiting > 0` hace `broadcast` a `vip_changed`; si no, a `normal_changed`. Se llama siempre con el mutex cogido.

**`enter_vip_client`**: coge el mutex, saca ticket, `vip_waiting++`, espera en `vip_changed` mientras `ticket != vip_turn || inside >= CAPACITY`. Al entrar: `vip_waiting--`, `vip_turn++`, `inside++` y `wake_next()` si aún hay sitio.

**`enter_normal_client`**: igual, pero espera en `normal_changed` mientras `ticket != normal_turn || inside >= CAPACITY || vip_waiting > 0`.

**`dance`**: **sin** el mutex cogido (si no, nadie podría entrar ni salir).

**`disco_exit`**: mutex, `inside--`, `wake_next()`.

**Por qué `wake_next()` en los tres sitios**:

| Dónde | Qué cambia | Qué puede desbloquear |
|---|---|---|
| `disco_exit` | `inside--` | aparece un **hueco** |
| `enter_*` | `vip_turn++`, `normal_turn++` o `vip_waiting--` | cambia **a quién le toca** (el siguiente VIP, o los normales cuando `vip_waiting` llega a 0) |

Al entrar solo se avisa si `inside < CAPACITY`: con la sala llena nadie más puede pasar y el siguiente `disco_exit` ya avisará.

**`client`**: copia los argumentos (`struct client_args`) a la pila, hace `free(arg)`, y llama a `enter_*` -> `dance` -> `disco_exit`.

**`main`**: `fopen` + `fscanf` (M y luego un 0/1 por cliente), un `malloc` por hilo para sus argumentos (así no hay carrera con una variable compartida), `pthread_create` (comprueba el error), `pthread_join` de todos, y al final `destroy` de mutex y conds.

## 6. Código de solución

Es el mismo que [ejercicio1/disco.c](ejercicio1/disco.c).

## 7. Prueba realizada

Con `grande.txt` (14 clientes, 4 vips: 6, 8, 9 y 11): el máximo `inside` impreso fue **5/5** y una ejecución dio este orden de entrada:

`0 4 3 7 2` (normales que llegaron primero) -> `6 8 11 9` (VIPs, se saltan a los normales que esperaban) -> `10 5 1 12 13` (normales restantes).

El orden exacto de los normales depende de qué hilo llega antes a la puerta, por eso cambia entre ejecuciones. Compilar y ejecutar: `make && ./disco grande.txt`.

## 8. Errores típicos

| Error | Consecuencia |
|---|---|
| `if` en vez de `while` alrededor de `cond_wait` | entra alguien sin cumplir la condición |
| Bailar con el mutex cogido | todos bloqueados, nadie entra/sale |
| Avisar solo en `disco_exit` y no en `enter_*` | puede quedar un hueco libre con clientes dormidos |
| Avisar solo a quien sale (VIP o normal) en vez de a quien puede entrar | cuelgue: se despierta al banco equivocado |
| Un único `int` compartido para los argumentos del hilo | carrera: dos hilos con el mismo `id` |
| Normal sin comprobar `vip_waiting` | los VIP no tienen prioridad |
| Sin turnos | no se respeta el orden de llegada |
