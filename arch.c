#include <stddef.h>
#include <stdio.h>
int main() {
#if defined(__x86_64__) || defined(_M_X64) || defined(i386) || \
    defined(__i386__) || defined(__i386) || defined(_M_IX86)
  size_t ptr_size = sizeof(int*);
  if (4 == ptr_size)
    puts("i686-w64-mingw32");
  else if (8 == ptr_size)
    puts("x86_64-w64-mingw32");
  else
    puts("Unknown Architecture");
#else
  puts("Unknown Architecture");
#endif
  return 0;
}
