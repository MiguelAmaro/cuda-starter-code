#include <stdio.h>
extern "C" __global__
void CudaKernel(void)
{
  printf("hello world from cuda <%d,%d>\n",blockIdx.x, threadIdx.x);
  return;
}
extern "C" void CudaRunCodeFromC(void)
{
  CudaKernel<<<10, 1>>>();
  cudaDeviceSynchronize();
  return;
}
// int main(void) { return; } not needed