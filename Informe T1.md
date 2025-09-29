Informe 2

En esta tarea se implementaron dos nuevas llamadas al sistema en xv6: getppid y getancestor.
La primera retorna el PID del proceso padre, mientras que la segunda permite obtener el PID de un ancestro enésimo (por ejemplo, el abuelo de un proceso).

Para lograrlo se modificaron los archivos principales del kernel (sysproc.c, syscall.c, syscall.h) y los de usuario (user.h, usys.pl). Luego se creó el programa de prueba yosoytupadre.c, que ejecuta ambas llamadas y muestra los resultados por pantalla.

El Makefile también fue ajustado para incluir este nuevo programa dentro de los binarios de usuario, de manera que se cargue en el sistema de archivos (fs.img).
Adicionalmente, se editó init.c para que yosoytupadre se ejecute automáticamente al iniciar xv6, permitiendo validar el correcto funcionamiento de las llamadas antes de lanzar el shell.

Los resultados fueron correctos. Al ejecutar desde init, el padre corresponde al proceso init (PID 1) y no existe abuelo, por lo que retorna -1. En cambio, al ejecutar manualmente desde el shell, el padre es el proceso sh (PID 3) y el abuelo corresponde al proceso init (PID 1).

En conclusión, la implementación de ambas llamadas fue exitosa. El programa de usuario confirma el comportamiento esperado tanto en ejecución automática como manual, cumpliendo con los objetivos de la tarea.
