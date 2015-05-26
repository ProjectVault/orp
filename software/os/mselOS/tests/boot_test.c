#include <stdint.h>
#include <stddef.h>
#include <msel.h>

void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;
}

int checker = 2;

void should_never_get_called() { while(1) { } }
void should_always_get_called() { }

void safepoint() { }

#define base ((int *)0x200000)
#define MEM_SIZE    (128 * 1024)
#define BASE_SIZE   (MEM_SIZE / sizeof(int))

int main() {
/*
    if (1 == checker) should_never_get_called();
    if (2 == checker) should_always_get_called();

    safepoint();
*/

    base[0] = 128 * 1024;
    base[1] = 3;
    base[2] = 0xffffffff;

    while (base[0] > 0) {
      for (base[1] = 3; base[1] < BASE_SIZE; base[1]++) {
        base[base[1]] = base[1] * base[0];
      }

      for (base[1] = 3; base[1] < BASE_SIZE; base[1]++) {
        if (base[base[1]] != base[1] * base[0])
          base[1]--;
      }

      base[0]--;
    }

    safepoint();

    for (base[1] = 3; base[1] < BASE_SIZE; base[1]++)
      if (base[1] == BASE_SIZE - 1)
        base[1] = 3;

    msel_init();
    msel_start();


    while(1);

    /* never reached */
    return 0;
}
