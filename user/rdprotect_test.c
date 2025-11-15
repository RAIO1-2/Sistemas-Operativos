#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
  // Pide una página de memoria al heap
  char *addr = sbrk(4096);

  // Escribir valor inicial (debería funcionar)
  addr[0] = 'Z';
  printf("Escritura inicial 'Z' exitosa.\n");

  // Proteger contra lectura
  if (mrdprotect(addr, 1) < 0) {
    printf("mrdprotect falló\n");
    exit(1);
  }
  printf("Página protegida contra lectura.\n");

  // Escritura aún permitida
  addr[0] = 'A';
  printf("Escritura 'A' sobre página protegida exitosa.\n");

  // Intento de lectura debería provocar fallo
  printf("Intentando leer... (esto debería fallar y el OS matar el programa)\n");
  
  // Esta es la línea que debe fallar:
  char c = addr[0];
  
  // Si el programa llega aquí, la protección NO funcionó.
  printf("¡ERROR! Valor leído: %c (esto NO debería imprimirse)\n", c);

  // Revertir protección (este código no debería alcanzarse si la Tarea 3 funciona)
  if (munrdprotect(addr, 1) < 0) {
    printf("munrdprotect falló\n");
    exit(1);
  }

  printf("Protección revertida correctamente.\n");
  exit(0);
}