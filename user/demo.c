#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Crea 10 procesos hijos
#define N_CHILDREN 10

int
main(int argc, char *argv[])
{
  printf("Iniciando prueba del planificador (Lottery Scheduling)...\n");

  for (int i = 0; i < N_CHILDREN; i++) {
    // fork() crea un proceso hijo
    int pid = fork();
    
    if (pid < 0) {
      printf("fork falló\n");
      exit(1);
    }

    if (pid == 0) { // --- Este es el código del HIJO ---
      
      // Asigna tickets: 50, 100, 150, ..., 500 [cite: 36]
      int tickets = 50 * (i + 1); 
      settickets(tickets); // Llama a tu nueva syscall

      // Bucle infinito para consumir CPU
      // El kernel lo interrumpirá cuando termine la prueba
      volatile unsigned long long counter = 0;
      while (1) {
        counter++;
      }
      
      // (El hijo nunca llegará a esta línea)
      exit(0); 
    }
  }

  // --- Este es el código del PADRE ---
  
  // El padre espera un tiempo para que los hijos corran
  // (1000 "ticks" de reloj del sistema)
  pause(1000); 

  // Matamos a todos los hijos para terminar la prueba
  for(int i = 0; i < N_CHILDREN; i++){
    // (Nota: el PID del primer hijo es el PID del padre + 1)
    // Se asume que los PIDs son secuenciales.
    kill(getpid() + i + 1);
  }

  // Espera a que todos los hijos terminen (wait)
  printf("\n--- Prueba terminada. Recolectando resultados: ---\n");
  for(int i = 0; i < N_CHILDREN; i++){
    wait(0); // Espera a un hijo
  }

  // (Nota: Para imprimir los 'run_slices' necesitarías una syscall 
  //  adicional, o imprimir desde el kernel. Por ahora, esto
  //  solo prueba que el sistema no se cae.)

  printf("Prueba finalizada.\n");
  exit(0);
}