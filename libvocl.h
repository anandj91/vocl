#ifndef _VOCL_H_
# define _VOCL_H_

#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <dlfcn.h>
#include <CL/cl.h>
#include <string.h>
#include <boost/preprocessor/tuple/push_front.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>

extern "C" {
	void* __libc_dlsym (void *map, const char *name);
	void* __libc_dlopen_mode (const char* name, int mode);
}

typedef void* (*fnDlsym)(void*, const char*);

static void* real_dlsym(void *handle, const char* symbol)
{
	static fnDlsym internal_dlsym = (fnDlsym)__libc_dlsym(
			__libc_dlopen_mode("libdl.so.2", RTLD_LAZY), "dlsym");
	return (*internal_dlsym)(handle, symbol);
}

#define STRINGIFY(x) BOOST_PP_STRINGIZE(x)
#define CONCAT(x,y) x##y
#define FN_TYPE(x) CONCAT(fn,x)
#define VOCL_FN(x) CONCAT(vo,x)
#define ADD_ELEM(t, x) BOOST_PP_TUPLE_PUSH_FRONT(t, x)
#define GET_ELEM(t, i) BOOST_PP_TUPLE_ELEM(i, t)
#define FOR_EACH(macro, seq) BOOST_PP_SEQ_FOR_EACH(macro, _, seq)

#define GET_FUNC(t) GET_ELEM(t, 1)

#define WRAP_FN(ret_type, funcname, params, ...) \
    typedef ret_type (*FN_TYPE(funcname)) params; \
    ret_type VOCL_FN(funcname) ADD_ELEM(params, FN_TYPE(funcname) real_fn); \
    ret_type funcname params \
    { \
        static auto real_fn = (FN_TYPE(funcname)) real_dlsym(RTLD_NEXT, \
            STRINGIFY(funcname)); \
        ret_type ret = VOCL_FN(funcname)(real_fn, __VA_ARGS__); \
        return ret; \
    } \


#define INTERCEPT(r, data, ocl_call) \
    if (strcmp(symbol, STRINGIFY(GET_FUNC(ocl_call))) == 0) { \
        return (void*) GET_FUNC(ocl_call); \
    } else

#define WRAP(r, data, ocl_call) \
    WRAP_FN ocl_call

/*
 * List of OpenCL calls with full signature
 * ((return_type, function_name, (list_of_parameters), args...)
 */
#define OCL_CALLS \
    ((cl_int, clEnqueueNDRangeKernel, (cl_command_queue command_queue, \
                                       cl_kernel kernel, \
                                       cl_uint work_dim, \
                                       const size_t *global_work_offset, \
                                       const size_t *global_work_size, \
                                       const size_t *local_work_size, \
                                       cl_uint num_events_in_wait_list, \
                                       const cl_event *event_wait_list, \
                                       cl_event *event), \
        command_queue, kernel, work_dim, global_work_offset, global_work_size, \
        local_work_size, num_events_in_wait_list, event_wait_list, event)) \
    \
    ((cl_int, clWaitForEvents, (cl_uint num_events, \
                                const cl_event *event_list), \
        num_events, event_list)) \
    \
    ((cl_int, clSetKernelArg, (cl_kernel kernel, \
                              cl_uint arg_index, \
                              size_t arg_size, \
                              const void *arg_value), \
                              kernel, arg_index, arg_size, arg_value)) \
    \
    ((cl_mem, clCreateBuffer, (cl_context context, \
                               cl_mem_flags flags, \
                               size_t size, \
                               void *host_ptr, \
                               cl_int *errcode_ret), \
        context, flags, size, host_ptr, errcode_ret)) \
    \
    ((cl_int, clEnqueueReadBuffer, (cl_command_queue command_queue, \
                                    cl_mem buffer, \
                                    cl_bool blocking_read, \
                                    size_t offset, \
                                    size_t cb, \
                                    void *ptr, \
                                    cl_uint num_events_in_wait_list, \
                                    const cl_event *event_wait_list, \
                                    cl_event *event), \
        command_queue, buffer, blocking_read, offset, cb, ptr, \
        num_events_in_wait_list, event_wait_list, event)) \
    \
	((cl_int, clEnqueueReadBufferRect, (cl_command_queue command_queue, \
                                        cl_mem buffer, \
                                        cl_bool blocking_read, \
                                        const size_t *buffer_origin, \
                                        const size_t *host_origin, \
                                        const size_t *region, \
                                        size_t buffer_row_pitch, \
                                        size_t buffer_slice_pitch, \
                                        size_t host_row_pitch, \
                                        size_t host_slice_pitch, \
                                        void *ptr, \
                                        cl_uint num_events_in_wait_list, \
                                        const cl_event *event_wait_list, \
                                        cl_event *event), \
		command_queue, buffer, blocking_read, buffer_origin, host_origin, \
		region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, \
		host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event)) \
	\
    ((cl_int, clEnqueueWriteBuffer, (cl_command_queue command_queue, \
                                     cl_mem buffer, \
                                     cl_bool blocking_write, \
                                     size_t offset, \
                                     size_t cb, \
                                     const void *ptr, \
                                     cl_uint num_events_in_wait_list, \
                                     const cl_event *event_wait_list, \
                                     cl_event *event), \
		command_queue, buffer, blocking_write, offset, cb, ptr, \
		num_events_in_wait_list, event_wait_list, event)) \
    \
    ((cl_int, clGetPlatformIDs, (cl_uint num_entries, \
                                 cl_platform_id *platforms, \
                                 cl_uint *num_platforms), \
        num_entries, platforms, num_platforms))

/*
 * Dynamic Intercepts
 */
void* dlsym(void *handle, const char *symbol)
{
    FOR_EACH(INTERCEPT, OCL_CALLS) {
        return real_dlsym(handle, symbol);
    }
}

/*
 * Static Intercepts
 */
FOR_EACH(WRAP, OCL_CALLS)

#endif /* _VOCL_H_ */
