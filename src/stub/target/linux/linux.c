#include <machine/target/target.h>
#include <machine/target/libc.h>
#include <core/target_ops.h>
#include <libc/malloc.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096

struct ops *target_init(void *args) {
  char *free_space = (char *)mmap2(0, PAGE_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
  /* free_space[PAGE_SIZE * 2] = 0; */
  malloc_init();
  set_malloc_locking(NULL, NULL);
  add_malloc_block(free_space, PAGE_SIZE);
  char *test = (char *)malloc(10);
  *test = 12;

  return 0;
}
