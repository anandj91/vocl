#include <stdio.h>
#include "libvocl.h"

/*
 ** Interposed Functions
 */
void* dlsym(void *handle, const char *symbol)
{
    if (strcmp(symbol, "clWaitForEvents") == 0) {
        return (void*)clWaitForEvents;
    } else if (strcmp(symbol, "clEnqueueNDRangeKernel") == 0) {
        return (void*)clEnqueueNDRangeKernel;
    } else if (strcmp(symbol, "clSetKernelArg") == 0) {
        return (void*)clSetKernelArg;
    } else {
        return real_dlsym(handle, symbol);
    }
}

cl_int clEnqueueNDRangeKernel(cl_command_queue command_queue,
        cl_kernel kernel,
        cl_uint work_dim,
        const size_t *global_work_offset,
        const size_t *global_work_size,
        const size_t *local_work_size,
        cl_uint num_events_in_wait_list,
        const cl_event *event_wait_list,
        cl_event *event)
{
    static fnClEnqueueNDRangeKernel real_fn = (fnClEnqueueNDRangeKernel)real_dlsym(RTLD_NEXT, "clEnqueueNDRangeKernel");
    cl_int ret = real_fn(command_queue, kernel, work_dim,
            global_work_offset, global_work_size, local_work_size,
            num_events_in_wait_list, event_wait_list, event);
    int i = num_events_in_wait_list;
    do {
        fflush(stdout);
        fprintf(stdout, "clEnqueueNDRangeKernel %lx: %lx %lx\n", kernel, (i-->0)?event_wait_list[i]:NULL, *event);
        fflush(stdout);
    } while (i>0);
}

cl_int clWaitForEvents(cl_uint num_events, const cl_event *event_list)
{
    fprintf(stdout, "clWaitForEvents intercepted!!!\n");
    static fnClWaitForEvents real_fn = (fnClWaitForEvents)real_dlsym(RTLD_NEXT, "clWaitForEvents");
    return real_fn(num_events, event_list);
}

cl_int clSetKernelArg(cl_kernel kernel,
        cl_uint arg_index,
        size_t arg_size,
        const void *arg_value)
{
    fprintf(stdout, "clSetKernelArg %lx %lx\n", kernel, *(uint64_t*)arg_value);
    static fnClSetKernelArg real_fn = (fnClSetKernelArg)real_dlsym(RTLD_NEXT, "clSetKernelArg");
    return real_fn(kernel, arg_index, arg_size, arg_value);
}
