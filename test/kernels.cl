__kernel void init(__global int *A, int a) {
    int i = get_global_id(0);
    A[i] = a;
}

__kernel void vecadd(__global int *A, __global int *B, __global int *C) {
    int i = get_global_id(0);
    C[i] = A[i] + B[i];
}
