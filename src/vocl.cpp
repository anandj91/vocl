#include <stdio.h>
#include "vocl.h"


class VoclImpl : public Vocl {
  public:
	cl_int EnqueueNDRangeKernel(EnqueueNDRangeKernel_t real_fn,
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
	        fprintf(stdout, "EnqueueNDRangeKernel %lx: %lx %lx\n", kernel,
	                (i-- > 0) ? event_wait_list[i] : NULL, *event);
	        fflush(stdout);
	    } while (i > 0);
	
	    return ret;
	}
	
	cl_int SetKernelArg(SetKernelArg_t real_fn,
	        cl_kernel kernel,
	        cl_uint arg_index,
	        size_t arg_size,
	        const void *arg_value)
	{
	    auto ret = real_fn(kernel, arg_index, arg_size, arg_value);
	    fprintf(stdout, "SetKernelArg %lx %lx\n", kernel, *(uint64_t*)arg_value);
	    return ret;
	}
	
	cl_mem CreateBuffer(CreateBuffer_t real_fn,
	        cl_context context,
	        cl_mem_flags flags,
	        size_t size,
	        void *host_ptr,
	        cl_int *errcode_ret)
	{
	    auto ret = real_fn(context, flags, size, host_ptr, errcode_ret);
	    fprintf(stdout, "CreateBuffer %lx\n", ret);
	    return ret;
	}

	void* EnqueueMapBuffer(EnqueueMapBuffer_t real_fn,
			cl_command_queue command_queue,
			cl_mem buffer,
			cl_bool blocking_map,
			cl_map_flags map_flags,
			size_t offset,
			size_t cb,
			cl_uint num_events_in_wait_list,
			const cl_event *event_wait_list,
			cl_event *event,
			cl_int *errcode_ret)
	{
        auto ret = real_fn(command_queue, buffer, blocking_map, map_flags,
                offset, cb, num_events_in_wait_list,
                event_wait_list, event, errcode_ret);
        fprintf(stdout, "EnqueueMapBuffer %lx\n", buffer);
        return ret;
	}
};

static Vocl* vocl = new VoclImpl();

/**
 * Dynamic Intercepts
 */
void* dlsym(void *handle, const char *symbol)
{
    FOR_EACH(INTERCEPT, CL_CALLS) {
        return real_dlsym(handle, symbol);
    }
}

/**
 * Static intercepts
 */
FOR_EACH(BUILD_OCL, CL_CALLS)
