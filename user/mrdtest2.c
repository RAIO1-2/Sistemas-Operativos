#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"

int main() {
  printf("Iniciando prueba de ciclo completo (mrdprotect -> munrdprotect)...\n");
  
  char *addr = sbrk(4096);
  if (addr == (char*)-1) {
    printf("sbrk falló\n");
    exit(1);
  }

  // 1. Escritura inicial
  addr[0] = 'Z';
  printf("Escritura inicial 'Z' exitosa.\n");

  // 2. Proteger
  if (mrdprotect(addr, 1) < 0) {
    printf("mrdprotect falló\n");
    exit(1);
  }
  printf("Página protegida. (Ahora no se puede leer NI escribir).\n");

  // 3. Desproteger INMEDIATAMENTE
  if (munrdprotect(addr, 1) < 0) {
    printf("munrdprotect falló\n");
    exit(1);
  }
  printf("Protección revertida (munrdprotect exitoso).\n");

  // 4. Ahora SÍ probamos la escritura y lectura
  
  // Prueba de escritura post-desprotección
  addr[0] = 'A';
  printf("Escritura 'A' post-desprotección exitosa.\n");

  // Prueba de lectura post-desprotección
  char c = addr[0];
  printf("Lectura post-desprotección exitosa.\n");

  // Si el programa llega aquí, ¡ambas funciones fueron probadas!
  printf("----------------------------------------\n");
  printf("¡ÉXITO! Valor final leído: %c\n", c);
  printf("----------------------------------------\n");
  
  exit(0);
}