/* #include <machine/arch/machine.h> */
#include <machine/target/target.h>

void _start(void *args) {
  target_init(args);
  return;
}
