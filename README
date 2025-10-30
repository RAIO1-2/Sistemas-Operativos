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

3.  **Syscall `settickets`:** Se implementó una nueva llamada al sistema (`SYS_settickets 22`) que permite a un proceso modificar su propia cantidad de tickets. La lógica de esta función (en `kernel/sysproc.c`) obtiene el argumento entero `n` y lo asigna a `myproc()->tickets`. [cite_start]Crucialmente, aplica la regla de robustez[cite: 12, 29]: si el `n` solicitado es menor que 1, se asigna 1 automáticamente.

4.  **Lógica del `scheduler()`:** Esta fue la modificación principal (en `kernel/proc.c`). [cite_start]La función `scheduler()` ahora sigue estos pasos en cada ciclo, tal como lo indican los comentarios que añadimos[cite: 19]:
    * [cite_start]**Paso 1 (Calcular Total):** Itera sobre todos los procesos para calcular la suma total de tickets (`total_tickets`) de *todos* los procesos en estado `RUNNABLE`[cite: 20].
    * [cite_start]**Paso 2 (Robustez):** Si `total_tickets` es 0 (ningún proceso está listo), simplemente continúa al siguiente ciclo, evitando bloquear el kernel[cite: 30].
    * [cite_start]**Paso 3 (Generar Ganador):** Genera un número aleatorio `winner` entre 1 y `total_tickets`[cite: 21]. (Para esto se implementó un generador LCG simple, `rand()`, al inicio de `proc.c`).
    * **Paso 4 (Encontrar Ganador):** Vuelve a iterar sobre los procesos `RUNNABLE`. Va sumando los tickets de cada proceso en un `accumulator`. [cite_start]El primer proceso que hace que `accumulator >= winner` es el ganador[cite: 22].
    * [cite_start]**Paso 5 (Contabilidad y Ejecución):** Justo antes de ejecutar al ganador (antes del `swtch()`), se incrementa su contador `p->run_slices++`[cite: 26]. [cite_start]Luego, el proceso se ejecuta[cite: 23].

## 2. Modificaciones Realizadas

Para implementar esta lógica, se modificaron los siguientes archivos:

* **`kernel/proc.h`:**
    * [cite_start]Se añadieron `int tickets;` y `int run_slices;` a la `struct proc`[cite: 8, 25].
* **`kernel/proc.c`:**
    * Se añadió la función `rand()` y su semilla `rand_seed` al inicio del archivo.
    * [cite_start]En `allocproc()`, se inicializaron `p->tickets = 100;` y `p->run_slices = 0;`[cite: 9].
    * [cite_start]Se reemplazó completamente la función `scheduler()` con la nueva lógica de lotería[cite: 19].
    * En `kexit()`, se añadió un `printf` para mostrar los *stats* (PID, Tickets, Slices) del proceso al morir. Esto fue clave para el análisis de resultados.
* **`kernel/syscall.h`:**
    * [cite_start]Se añadió la definición `#define SYS_settickets 22`[cite: 15].
* **`kernel/syscall.c`:**
    * [cite_start]Se añadió `extern uint64 sys_settickets(void);`[cite: 15].
    * [cite_start]Se añadió `[SYS_settickets] sys_settickets,` al *array* `syscalls[]`[cite: 15].
* **`kernel/sysproc.c`:**
    * [cite_start]Se implementó la función `uint64 sys_settickets(void)`[cite: 14], usando `argint` para obtener el número de tickets y aplicando la validación `n < 1`.
* **`user/user.h`:**
    * [cite_start]Se añadió el prototipo `int settickets(int);` para el espacio de usuario[cite: 16].
    * Se corrigió `int pause(void);` a `int pause(int);` para que aceptara argumentos.
* **`user/usys.pl`:**
    * (Este archivo genera `usys.S`). Se añadió la línea `entry("settickets");` para que el *linker* pudiera encontrar la syscall.
* **`user/demo.c`:**
    * [cite_start]Se creó este nuevo programa de prueba que usa `fork()` para crear 10 hijos[cite: 35].
    * [cite_start]Cada hijo llama a `settickets(50 * (i + 1))` para asignarse tickets distintos (50, 100, 150...)[cite: 36].
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

## [cite_start]4. Posibles Problemas del Lottery Scheduling [cite: 49]

Aunque es un sistema justo a largo plazo, el Lottery Scheduling tiene problemas inherentes:

1.  **Varianza (Injusticia a corto plazo)**: Al ser un método probabilístico, no es determinista. Un proceso con muchos tickets puede tener "mala suerte" y no ser elegido, mientras que uno con menos tickets puede tener "buena suerte". En nuestra propia prueba, vimos que el proceso con 450 tickets (PID 12) obtuvo 457 *slices*, ¡ligeramente más que el proceso con 500 tickets (PID 13) que obtuvo 426! Esto demuestra que la proporcionalidad solo se garantiza estadísticamente a largo plazo.
2.  **Inanición (Starvation)**: Aunque es muy improbable, un proceso con 1 ticket compitiendo contra procesos que tienen millones de tickets podría, teóricamente, no ser elegido por un período muy largo.
3.  **Overhead**: Es más costoso que Round-Robin. En nuestra implementación, el *scheduler* debe recorrer la lista de procesos *dos veces* en cada ciclo: una para sumar el total y otra para encontrar al ganador.
4.  **Gestión de Tickets**: Es difícil para un usuario o administrador saber cuántos tickets asignar a un proceso para que sea "justo". Un proceso malicioso podría asignarse una cantidad enorme de tickets y monopolizar la CPU.

## 5. Análisis de Resultados Obtenidos

Para cumplir con la rúbrica, se usó el `printf` en `kexit()` para capturar los `run_slices` de cada hijo de `demo.c` al ser terminados.
