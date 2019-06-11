#include <stdio.h>
#include <mutex>
#include "vocl.h"

class VoclImpl : public Vocl {
  public:
    cl_int EnqueueNDRangeKernel(cl_command_queue command_queue,
            cl_kernel kernel,
            cl_uint work_dim,
            const size_t *global_work_offset,
            const size_t *global_work_size,
            const size_t *local_work_size,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event)
    {
        auto ret = real_EnqueueNDRangeKernel(command_queue, kernel, work_dim,
                global_work_offset, global_work_size, local_work_size,
                num_events_in_wait_list, event_wait_list, event);
        print_events("EnqueueNDRangeKernel", (void*)kernel,
                num_events_in_wait_list, event_wait_list, event);
        return ret;
    }

    cl_int SetKernelArg(cl_kernel kernel,
            cl_uint arg_index,
            size_t arg_size,
            const void *arg_value)
    {
        auto ret = real_SetKernelArg(kernel, arg_index, arg_size, arg_value);
        fprintf(stdout, "SetKernelArg %lx %lx\n", kernel, *(uint64_t*)arg_value);
        fflush(stdout);
        return ret;
    }

    cl_mem CreateBuffer(cl_context context,
            cl_mem_flags flags,
            size_t size,
            void *host_ptr,
            cl_int *errcode_ret)
    {
        auto ret = real_CreateBuffer(context, flags, size, host_ptr, errcode_ret);
        fprintf(stdout, "CreateBuffer %lx\n", ret);
        fflush(stdout);
        return ret;
    }

    void* EnqueueMapBuffer(cl_command_queue command_queue,
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
        auto ret = real_EnqueueMapBuffer(command_queue, buffer, blocking_map,
                map_flags, offset, cb, num_events_in_wait_list,
                event_wait_list, event, errcode_ret);
        print_events("EnqueueMapBuffer", buffer, num_events_in_wait_list,
                event_wait_list, event);
        return ret;
    }

    cl_int EnqueueUnmapMemObject(cl_command_queue command_queues,
            cl_mem memobj,
            void *mapped_ptr,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event)
    {
        auto ret = real_EnqueueUnmapMemObject(command_queues, memobj,
                mapped_ptr, num_events_in_wait_list, event_wait_list, event);
        print_events("EnqueueUnmapMemObject", memobj, num_events_in_wait_list,
                event_wait_list, event);
        return ret;
    }

    cl_int SetEventCallback(cl_event event,
            cl_int command_exec_callback_type,
            void (CL_CALLBACK *pfn_event_notify)(cl_event, cl_int, void*),
            void *user_data)
    {
        auto ret = real_SetEventCallback(event, command_exec_callback_type,
                pfn_event_notify, user_data);
        fprintf(stdout, "SetEventCallback %lx\n", event);
        fflush(stdout);
        return ret;
    }

    cl_int EnqueueMarkerWithWaitList(cl_command_queue command_queue,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event)
    {
        auto ret = real_EnqueueMarkerWithWaitList(command_queue,
                num_events_in_wait_list, event_wait_list, event);
        print_events("EnqueueMarkerWithWaitList", NULL, num_events_in_wait_list,
                event_wait_list, event);
        return ret;
    }

  private:
    void print_events(const char* tag,
            void* ref,
            int num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event)
    {
        int i = num_events_in_wait_list;
        do {
            fprintf(stdout, "%s %lx - in_event: %lx out_event: %lx\n", tag, ref,
                    (i-- > 0) ? event_wait_list[i] : NULL,
                    event ? *event : NULL);
            fflush(stdout);
        } while (i > 0);
    }


};

Vocl* VoclFactory::Get()
{
    static Vocl* vocl = NULL;
    static std::mutex mu_;
    if (!vocl) {
        mu_.lock();
        if (!vocl) {
            vocl = new VoclImpl();
            vocl->init();
        }
        mu_.unlock();
    }
    return vocl;
}

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
