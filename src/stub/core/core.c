/* #include <machine/arch/machine.h> */
#include <machine/target/target.h>
#include <core/ops.h>

struct ops g_ops;

/* extern unsigned long __GOT_START; */
/* extern unsigned long __GOT_END; */
/* extern unsigned long __START; */
extern unsigned long __GOT_OFFSET;
extern unsigned long __GOT_LENGTH;

void fix_got();

__attribute__((externally_visible,used)) void _start(void *args) {
  fix_got();
  return;
  target_init(args);
  g_ops.log("123asd", 6);
  return;
}

void fix_got() {
  unsigned long start = (unsigned long)&_start;
  /* unsigned long __GOT_OFFSET = (unsigned long)(&__GOT_START - &__START); */
  /* unsigned long __GOT_LENGTH = (unsigned long)(&__GOT_END - &__GOT_START); */
  unsigned long *got = (unsigned long *)(start + (unsigned long)&__GOT_OFFSET);
  for (unsigned int i = 0; i < ((unsigned long)&__GOT_LENGTH) / sizeof(*got); ++i) {
    if (i == 10)
      break;
    got[i] += start;
  }
}

