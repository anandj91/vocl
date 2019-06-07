#include <stdio.h>
#include "libvocl.h"

cl_int voclEnqueueNDRangeKernel(fnclEnqueueNDRangeKernel real_fn,
        cl_command_queue command_queue,
        cl_kernel kernel,
        cl_uint work_dim,
        const size_t *global_work_offset,
        const size_t *global_work_size,
        const size_t *local_work_size,
        cl_uint num_events_in_wait_list,
        const cl_event *event_wait_list,
        cl_event *event)
{
    auto ret = real_fn(command_queue, kernel, work_dim,
            global_work_offset, global_work_size, local_work_size,
            num_events_in_wait_list, event_wait_list, event);
    int i = num_events_in_wait_list;
    do {
        fflush(stdout);
        fprintf(stdout, "clEnqueueNDRangeKernel %lx: %lx %lx\n", kernel,
                (i-- > 0) ? event_wait_list[i] : NULL, *event);
        fflush(stdout);
    } while (i > 0);

    return ret;
}

cl_int voclWaitForEvents(fnclWaitForEvents real_fn,
        cl_uint num_events,
        const cl_event *event_list)
{
    auto ret = real_fn(num_events, event_list);
    fprintf(stdout, "clWaitForEvents intercepted!!!\n");
    return ret;
}

cl_int voclSetKernelArg(fnclSetKernelArg real_fn,
        cl_kernel kernel,
        cl_uint arg_index,
        size_t arg_size,
        const void *arg_value)
{
    auto ret = real_fn(kernel, arg_index, arg_size, arg_value);
    fprintf(stdout, "clSetKernelArg %lx %lx\n", kernel, *(uint64_t*)arg_value);
    return ret;
}

cl_mem voclCreateBuffer(fnclCreateBuffer real_fn,
        cl_context context,
        cl_mem_flags flags,
        size_t size,
        void *host_ptr,
        cl_int *errcode_ret)
{
    auto ret = real_fn(context, flags, size, host_ptr, errcode_ret);
    fprintf(stdout, "clCreateBuffer %lx\n", ret);
    return ret;
}

cl_int voclEnqueueReadBuffer(fnclEnqueueReadBuffer real_fn,
        cl_command_queue command_queue,
        cl_mem buffer,
        cl_bool blocking_read,
        size_t offset,
        size_t cb,
        void *ptr,
        cl_uint num_events_in_wait_list,
        const cl_event *event_wait_list,
        cl_event *event)
{
    auto ret = real_fn(command_queue, buffer, blocking_read, offset, cb, ptr,
            num_events_in_wait_list, event_wait_list, event);
    fprintf(stdout, "clEnqueueReadBuffer %lx\n", buffer);
    return ret;
}

cl_int voclEnqueueReadBufferRect(fnclEnqueueReadBufferRect real_fn,
        cl_command_queue command_queue,
        cl_mem buffer,
        cl_bool blocking_read,
        const size_t *buffer_origin,
        const size_t *host_origin,
        const size_t *region,
        size_t buffer_row_pitch,
        size_t buffer_slice_pitch,
        size_t host_row_pitch,
        size_t host_slice_pitch,
        void *ptr,
        cl_uint num_events_in_wait_list,
        const cl_event *event_wait_list,
        cl_event *event)
{
    auto ret = real_fn(command_queue, buffer, blocking_read, buffer_origin,
            host_origin, region, buffer_row_pitch, buffer_slice_pitch,
            host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list,
            event_wait_list, event);
    fprintf(stdout, "clEnqueueReadBufferRect %lx\n", buffer);
    return ret;
}

cl_int voclEnqueueWriteBuffer(fnclEnqueueWriteBuffer real_fn,
        cl_command_queue command_queue,
        cl_mem buffer,
        cl_bool blocking_write,
        size_t offset,
        size_t cb,
        const void *ptr,
        cl_uint num_events_in_wait_list,
        const cl_event *event_wait_list,
        cl_event *event)
{
    auto ret = real_fn(command_queue, buffer, blocking_write, offset, cb, ptr,
            num_events_in_wait_list, event_wait_list, event);
    fprintf(stdout, "clEnqueueWriteBuffer %lx\n", buffer);
    return ret;
}

cl_int voclGetPlatformIDs(fnclGetPlatformIDs real_fn,
        cl_uint num_entries,
        cl_platform_id *platforms,
        cl_uint *num_platforms)
{
    auto ret = real_fn(num_entries, platforms, num_platforms);
    fprintf(stdout, "clGetPlatformIDs\n");
    return ret;
}
