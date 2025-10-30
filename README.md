# Tarea 2: Implementación de Lottery Scheduling en XV6

**Integrantes:**
* Ignacio Oyarzún
* Cristobal Pradines

---

## 1. Funcionamiento y Lógica de la Implementación

El objetivo de esta tarea fue reemplazar el planificador Round-Robin por defecto de XV6 por un sistema de **Lottery Scheduling**. La lógica central es que la fracción de CPU que recibe un proceso debe ser proporcional a la cantidad de "tickets" que posee.

La implementación se basó en los siguientes puntos clave:

1.  **Nuevos Campos de Proceso:** Se modificó la `struct proc` (en `kernel/proc.h`) para incluir dos nuevos campos:
    * `int tickets;`: Almacena la cantidad de tickets del proceso.
    * `int run_slices;`: Un contador para la contabilidad, que incrementa cada vez que el proceso es elegido para correr.

2.  **Inicialización:** En `kernel/proc.c`, la función `allocproc()` (que inicializa nuevos procesos) fue modificada para asignar los valores por defecto: `p->tickets = 100` y `p->run_slices = 0`.

3.  **Syscall `settickets`:** Se implementó una nueva llamada al sistema (`SYS_settickets 22`) que permite a un proceso modificar su propia cantidad de tickets. La lógica de esta función (en `kernel/sysproc.c`) obtiene el argumento entero `n` y lo asigna a `myproc()->tickets`.  Crucialmente, aplica la regla de robustez: si el `n` solicitado es menor que 1, se asigna 1 automáticamente.

4.  **Lógica del `scheduler()`:** Esta fue la modificación principal (en `kernel/proc.c`).  La función `scheduler()` ahora sigue estos pasos en cada ciclo, tal como lo indican los comentarios que añadimos:
    *  **Paso 1 (Calcular Total):** Itera sobre todos los procesos para calcular la suma total de tickets (`total_tickets`) de *todos* los procesos en estado `RUNNABLE`.
    *  **Paso 2 (Robustez):** Si `total_tickets` es 0 (ningún proceso está listo), simplemente continúa al siguiente ciclo, evitando bloquear el kernel.
    *  **Paso 3 (Generar Ganador):** Genera un número aleatorio `winner` entre 1 y `total_tickets`. (Para esto se implementó un generador LCG simple, `rand()`, al inicio de `proc.c`).
    * **Paso 4 (Encontrar Ganador):** Vuelve a iterar sobre los procesos `RUNNABLE`. Va sumando los tickets de cada proceso en un `accumulator`.  El primer proceso que hace que `accumulator >= winner` es el ganador.
    *  **Paso 5 (Contabilidad y Ejecución):** Justo antes de ejecutar al ganador (antes del `swtch()`), se incrementa su contador `p->run_slices++`.  Luego, el proceso se ejecuta.

## 2. Modificaciones Realizadas

Para implementar esta lógica, se modificaron los siguientes archivos:

* **`kernel/proc.h`:**
    *  Se añadieron `int tickets;` y `int run_slices;` a la `struct proc`.
* **`kernel/proc.c`:**
    * Se añadió la función `rand()` y su semilla `rand_seed` al inicio del archivo.
    *  En `allocproc()`, se inicializaron `p->tickets = 100;` y `p->run_slices = 0;`.
    *  Se reemplazó completamente la función `scheduler()` con la nueva lógica de lotería.
    * En `kexit()`, se añadió un `printf` para mostrar los *stats* (PID, Tickets, Slices) del proceso al morir. Esto fue clave para el análisis de resultados.
* **`kernel/syscall.h`:**
    *  Se añadió la definición `#define SYS_settickets 22`.
* **`kernel/syscall.c`:**
    *  Se añadió `extern uint64 sys_settickets(void);`.
    *  Se añadió `[SYS_settickets] sys_settickets,` al *array* `syscalls[]`.
* **`kernel/sysproc.c`:**
    *  Se implementó la función `uint64 sys_settickets(void)`, usando `argint` para obtener el número de tickets y aplicando la validación `n < 1`.
* **`user/user.h`:**
    *  Se añadió el prototipo `int settickets(int);` para el espacio de usuario.
    * Se corrigió `int pause(void);` a `int pause(int);` para que aceptara argumentos.
* **`user/usys.pl`:**
    * (Este archivo genera `usys.S`). Se añadió la línea `entry("settickets");` para que el *linker* pudiera encontrar la syscall.
* **`user/demo.c`:**
    *  Se creó este nuevo programa de prueba que usa `fork()` para crear 10 hijos.
    *  Cada hijo llama a `settickets(50 * (i + 1))` para asignarse tickets distintos (50, 100, 150...).
    * Cada hijo ejecuta un bucle infinito para consumir CPU.
    * El padre usa `pause(1000)` para dormir y luego mata a los hijos para terminar la prueba.
* **`Makefile`:**
    * Se añadió `$U/_demo` a la lista `UPROGS` para que el programa de prueba se compilara.

## 3. Dificultades Encontradas y Soluciones

Durante la implementación nos encontramos con varios desafíos, típicos del aprendizaje de XV6:

1.  **Error de Linker (`undefined reference to 'settickets'`)**: Este fue el problema más complejo. A pesar de haber definido la syscall en `syscall.h` y `user.h`, el compilador no la encontraba.
    * **Solución:** Descubrimos que esta versión de XV6 no genera `usys.S` mágicamente, sino que usa el script `user/usys.pl`. Tuvimos que añadir `entry("settickets");` a este script para que la syscall se "conectara" correctamente.
2.  **Error de Compilación en `demo.c`**: Inicialmente usamos la función `sleep(1000)` en el programa de prueba, pero el compilador arrojó un error de función implícita.
    * **Solución:** Revisamos `user/user.h` y `syscall.h` y notamos que la syscall se llamaba `SYS_pause`. Cambiamos la llamada a `pause(1000)` y se solucionó.
3.  **Error de IntelliSense en VS Code**: El editor no encontraba las cabeceras (ej. `"kernel/types.h"`).
    * **Solución:** Esto no era un error de compilación, sino del editor. Lo solucionamos creando un archivo `.vscode/c_cpp_properties.json` y añadiendo `"${workspaceFolder}/**"` al `includePath`.

## 4. Posibles Problemas del Lottery Scheduling 

Aunque es un sistema justo a largo plazo, el Lottery Scheduling tiene problemas inherentes:

1.  **Varianza (Injusticia a corto plazo)**: Al ser un método probabilístico, no es determinista. Un proceso con muchos tickets puede tener "mala suerte" y no ser elegido, mientras que uno con menos tickets puede tener "buena suerte". En nuestra propia prueba, vimos que el proceso con 450 tickets (PID 12) obtuvo 457 *slices*, ¡ligeramente más que el proceso con 500 tickets (PID 13) que obtuvo 426! Esto demuestra que la proporcionalidad solo se garantiza estadísticamente a largo plazo.
2.  **Inanición (Starvation)**: Aunque es muy improbable, un proceso con 1 ticket compitiendo contra procesos que tienen millones de tickets podría, teóricamente, no ser elegido por un período muy largo.
3.  **Overhead**: Es más costoso que Round-Robin. En nuestra implementación, el *scheduler* debe recorrer la lista de procesos *dos veces* en cada ciclo: una para sumar el total y otra para encontrar al ganador.
4.  **Gestión de Tickets**: Es difícil para un usuario o administrador saber cuántos tickets asignar a un proceso para que sea "justo". Un proceso malicioso podría asignarse una cantidad enorme de tickets y monopolizar la CPU.

## 5. Pruebas de Ejecución y Análisis de Resultados

Para validar la implementación, se ejecutó el programa de prueba `demo`. Este programa crea 10 procesos hijos (PIDs 4 al 13) que compiten por la CPU. El padre (PID 3) les asigna tickets de forma incremental (50, 100, 150... 500) y luego duerme, dejando que solo los hijos participen en la lotería.

Al finalizar la prueba (cuando el padre mata a los hijos), se utilizó un `printf` en la función `kexit()` del kernel para registrar los *stats* de cada proceso hijo al momento de terminar.

### Resultados Obtenidos

La siguiente tabla ordena los resultados de los 10 procesos hijos (PIDs 4-13) que se observan en la captura de pantalla:

| PID | Tickets Asignados | Slices de CPU Obtenidos |
|---|---|---|
| 4 | 50 | 76 |
| 5 | 100 | 127 |
| 6 | 150 | 168 |
| 7 | 200 | 257 |
| 8 | 250 | 327 |
| 9 | 300 | 350 |
| 10 | 350 | 411 |
| 11 | 400 | 430 |
| 12 | 450 | 457 |
| 13 | 500 | 426 |

*(Nota: El `PID 3` con 100 tickets y 144 slices que aparece en la captura es el proceso 'demo' padre, que no estaba compitiendo en el bucle principal, por lo que sus resultados no son comparables con los de los hijos).*

### Análisis de Resultados

El análisis de estos datos confirma exitosamente el cumplimiento de los objetivos de la tarea:

1.  **Se demuestra la Proporcionalidad:** Existe una correlación positiva y evidente: **a mayor cantidad de tickets, el proceso recibió una mayor cantidad de *slices* de CPU**.
    * El proceso con **50 tickets** (PID 4) obtuvo **76 *slices***.
    * El proceso con **100 tickets** (PID 5) obtuvo **127 *slices*** (un 67% más).
    * El proceso con **200 tickets** (PID 7) obtuvo **257 *slices*** (más del triple que el de 50 tickets).
    * La tendencia se mantiene de forma consistente a lo largo de toda la escala.

2.  **Se evidencia la Naturaleza Probabilística (Varianza):** Los resultados también demuestran perfectamente uno de los "problemas" o características clave del Lottery Scheduling: **la varianza a corto plazo**.
    * El proceso con **500 tickets** (PID 13) obtuvo **426 *slices***.
    * El proceso con **450 tickets** (PID 12) obtuvo **457 *slices***.
    * Es decir, el proceso con *menos* tickets (PID 12) "ganó" más tiempo de CPU que el proceso con más tickets (PID 13).

    Esto **no es un error en la implementación**, sino la prueba de que el planificador es *probabilístico*. Por "mala suerte", el PID 13 fue elegido menos veces que el PID 12 en este período de prueba corto. Esto valida la discusión teórica (Punto 4) con evidencia práctica.

En conclusión, los resultados documentados en la captura demuestran de forma robusta que la implementación del Lottery Scheduler es correcta: **asigna la CPU de forma proporcional a los tickets** y, al mismo tiempo, **exhibe la varianza estadística** esperada de un sistema de lotería.
