#include <stdio.h>
#include <stdlib.h>
 
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
 
#define MAX_SOURCE_SIZE (0x100000)
 
int main(void) {
    // Create the two input vectors
    int i;
    const int LIST_SIZE = 102400;
     
    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;
 
    fp = fopen("test/kernels.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );
 
    // Get platform and device information
    cl_platform_id platform_id[2];
    cl_device_id device_id = NULL;   
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(2, platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id[1], CL_DEVICE_TYPE_DEFAULT, 1, 
            &device_id, &ret_num_devices);
 
    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
 
    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
 
    // Create memory buffers on the device for each vector 
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, 
            LIST_SIZE * sizeof(int), NULL, &ret);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
            LIST_SIZE * sizeof(int), NULL, &ret);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, 
            LIST_SIZE * sizeof(int), NULL, &ret);
 
    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, 
            (const char **)&source_str, (const size_t *)&source_size, &ret);
 
    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	if (ret == CL_BUILD_PROGRAM_FAILURE) {
	    size_t log_size;
	    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	    char *log = (char *) malloc(log_size);
	    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
	    printf("%s\n", log);
	}
 
    // Create the OpenCL kernel
    cl_kernel vecadd = clCreateKernel(program, "vecadd", &ret);
    cl_kernel init = clCreateKernel(program, "init", &ret);

    size_t global_item_size = LIST_SIZE; // Process the entire lists
    size_t local_item_size = 64; // Divide work items into groups of 64
    int a = 0;

    // Initialize buffers
    ret = clSetKernelArg(init, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    a = 10;
    ret = clSetKernelArg(init, 1, sizeof(int), (void *)&a);
    ret = clEnqueueNDRangeKernel(command_queue, init, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);

    ret = clSetKernelArg(init, 0, sizeof(cl_mem), (void *)&b_mem_obj);
    a = 20;
    ret = clSetKernelArg(init, 1, sizeof(int), (void *)&a);
    ret = clEnqueueNDRangeKernel(command_queue, init, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);

    // Run vecadds
    ret = clSetKernelArg(vecadd, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(vecadd, 1, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(vecadd, 2, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clEnqueueNDRangeKernel(command_queue, vecadd, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);

    ret = clSetKernelArg(vecadd, 0, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(vecadd, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(vecadd, 2, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clEnqueueNDRangeKernel(command_queue, vecadd, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);

    ret = clSetKernelArg(vecadd, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(vecadd, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(vecadd, 2, sizeof(cl_mem), (void *)&c_mem_obj);
    ret = clEnqueueNDRangeKernel(command_queue, vecadd, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read the memory buffer C on the device to the local variable C
    int *C = (int*)malloc(sizeof(int)*LIST_SIZE);
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, 
            LIST_SIZE * sizeof(int), C, 0, NULL, NULL);
    for(int i=0; i<LIST_SIZE; i++) {
        if (C[i] != 60) {
            fprintf(stderr, "Error C[%d]=%d\n", i, C[i]);
            break;
        }
    }
 
    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(vecadd);
    ret = clReleaseKernel(init);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
    return 0;
}
