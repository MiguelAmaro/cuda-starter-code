#include <stdio.h>
#include "mycuda.h"
int main()
{
  printf("hello from c\n");
  CudaRunCodeFromC();
  return;
}