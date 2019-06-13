#include <stdio.h>
#include <stdlib.h>
#include <chrono>
 
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
 
#define MAX_SOURCE_SIZE (0x100000)

typedef struct {
    char c;
    std::chrono::time_point<std::chrono::high_resolution_clock>* time;
} udata;

void CL_CALLBACK pfn_event_notify(cl_event event, cl_int event_command_exec_status, void *user_data)
{
    auto ud = (udata*) user_data;
    auto now = std::chrono::high_resolution_clock::now();
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now-*(ud->time)).count();
    fprintf(stdout, "%c time: %ld\n", ud->c, microseconds);
    *(ud->time) = now;
}
 
int main(int argc, char* argv[]) {
    // Create the two input vectors
    int i;
    int LIST_SIZE = 131072;

    if (argc > 1) {
        LIST_SIZE = atoi(argv[1]);
    }
     
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
    cl_device_id device_id[2];   
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(2, platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id[1], CL_DEVICE_TYPE_GPU, 2, 
            device_id, &ret_num_devices);
 
    // Create an OpenCL context
    cl_context context = clCreateContext(NULL, 2, device_id, NULL, NULL, &ret);
 
    // Create a command queue
    cl_command_queue queue = clCreateCommandQueue(context, device_id[0], 0, &ret);
 
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
    ret = clBuildProgram(program, 2, device_id, NULL, NULL, NULL);
	if (ret == CL_BUILD_PROGRAM_FAILURE) {
	    size_t log_size;
	    clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	    char *log = (char *) malloc(log_size);
	    clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
	    printf("%s\n", log);
	}
 
    // Create the OpenCL kernel
    cl_kernel vecadd = clCreateKernel(program, "vecadd", &ret);
    cl_kernel init = clCreateKernel(program, "init", &ret);

    size_t global_item_size = LIST_SIZE; // Process the entire lists
    size_t local_item_size = 64; // Divide work items into groups of 64
    int a = 0;

    auto start = std::chrono::high_resolution_clock::now();
    auto first = std::chrono::high_resolution_clock::now();

    // Initialize buffers
    ret = clSetKernelArg(init, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    a = 10;
    ret = clSetKernelArg(init, 1, sizeof(int), (void *)&a);
    cl_event init_a;
    ret = clEnqueueNDRangeKernel(queue, init, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, &init_a);
    udata ud_init_a = {'a', &start};
    ret = clSetEventCallback(init_a, CL_COMPLETE, pfn_event_notify, &ud_init_a);

    ret = clSetKernelArg(init, 0, sizeof(cl_mem), (void *)&b_mem_obj);
    a = 20;
    ret = clSetKernelArg(init, 1, sizeof(int), (void *)&a);
    cl_event init_b;
    ret = clEnqueueNDRangeKernel(queue, init, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, &init_b);
    udata ud_init_b = {'b', &start};
    ret = clSetEventCallback(init_b, CL_COMPLETE, pfn_event_notify, &ud_init_b);

    // Run vecadds
    ret = clSetKernelArg(vecadd, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(vecadd, 1, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(vecadd, 2, sizeof(cl_mem), (void *)&a_mem_obj);
    cl_event vecadd_a;
    ret = clEnqueueNDRangeKernel(queue, vecadd, 1, NULL, 
            &global_item_size, &local_item_size, 1, &init_a, &vecadd_a);
    udata ud_vecadd_a = {'c', &start};
    ret = clSetEventCallback(vecadd_a, CL_COMPLETE, pfn_event_notify, &ud_vecadd_a);

    ret = clSetKernelArg(vecadd, 0, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(vecadd, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(vecadd, 2, sizeof(cl_mem), (void *)&b_mem_obj);
    cl_event vecadd_b;
    ret = clEnqueueNDRangeKernel(queue, vecadd, 1, NULL, 
            &global_item_size, &local_item_size, 1, &init_b, &vecadd_b);
    udata ud_vecadd_b = {'d', &start};
    ret = clSetEventCallback(vecadd_b, CL_COMPLETE, pfn_event_notify, &ud_vecadd_b);

    ret = clSetKernelArg(vecadd, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(vecadd, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(vecadd, 2, sizeof(cl_mem), (void *)&c_mem_obj);
    cl_event dep_vecadd_c[2] = {vecadd_a, vecadd_b};
    cl_event vecadd_c;
    ret = clEnqueueNDRangeKernel(queue, vecadd, 1, NULL, 
            &global_item_size, &local_item_size, 2, dep_vecadd_c, &vecadd_c);
    udata ud_vecadd_c = {'e', &start};
    ret = clSetEventCallback(vecadd_c, CL_COMPLETE, pfn_event_notify, &ud_vecadd_c);


    // Read the memory buffer C on the device to the local variable C
    int *C = (int*)malloc(sizeof(int)*LIST_SIZE);
    cl_event read_c;
    ret = clEnqueueReadBuffer(queue, c_mem_obj, CL_FALSE, 0, 
            LIST_SIZE * sizeof(int), C, 1, &vecadd_c, &read_c);
    udata ud_read_c = {'f', &start};
    ret = clSetEventCallback(read_c, CL_COMPLETE, pfn_event_notify, &ud_read_c);

    ret = clFlush(queue);
    ret = clFinish(queue);
    auto last = std::chrono::high_resolution_clock::now();
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(last-first).count();
    fprintf(stdout, "Total time: %ld\n", microseconds);

    for(int i=0; i<LIST_SIZE; i++) {
        if (C[i] != 60) {
            fprintf(stderr, "Error C[%d]=%d\n", i, C[i]);
            break;
        }
    }
 
    // Clean up
    ret = clReleaseKernel(vecadd);
    ret = clReleaseKernel(init);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseCommandQueue(queue);
    ret = clReleaseContext(context);
    free(C);
    return 0;
}
