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

#define ADD_ELEM(t, x) BOOST_PP_TUPLE_PUSH_FRONT(t, x)
#define GET_ELEM(t, i) BOOST_PP_TUPLE_ELEM(i, t)
#define FOR_EACH(macro, seq) BOOST_PP_SEQ_FOR_EACH(macro, _, seq)

#define FN_TYPE(x) CONCAT(x, _t)
#define OCL_FN(x) CONCAT(cl, x)
#define VOCL_FN(x) x

#define FUNC(t) GET_ELEM(t, 1)

#define BUILD_FN_TYPE_HELPER(ret_type, funcname, params, ...) \
    typedef ret_type (*FN_TYPE(funcname)) params;

#define BUILD_OCL_HELPER(ret_type, funcname, params, ...) \
    ret_type OCL_FN(funcname) params \
    { \
        static auto real_fn = (FN_TYPE(funcname)) real_dlsym(RTLD_NEXT, \
            STRINGIFY(OCL_FN(funcname))); \
        ret_type ret = vocl->VOCL_FN(funcname)(real_fn, __VA_ARGS__); \
        return ret; \
    } \


#define BUILD_VOCL_HELPER(ret_type, funcname, params, ...) \
    virtual ret_type funcname ADD_ELEM(params, FN_TYPE(funcname) real_fn) \
    { \
        auto ret = real_fn(__VA_ARGS__); \
        fprintf(stdout, "%s intercepted\n", STRINGIFY(funcname)); \
        return ret; \
    } \


#define INTERCEPT(r, data, ocl_call) \
    if (strcmp(symbol, STRINGIFY(OCL_FN(FUNC(ocl_call)))) == 0) { \
        return (void*) OCL_FN(FUNC(ocl_call)); \
    } else

#define BUILD_OCL(r, data, ocl_call) \
    BUILD_OCL_HELPER ocl_call

#define BUILD_VOCL(r, data, ocl_call) \
    BUILD_VOCL_HELPER ocl_call

#define BUILD_FN_TYPE(r, data, ocl_call) \
    BUILD_FN_TYPE_HELPER ocl_call

/*
 * List of OpenCL calls with full signature
 * ((return_type, function_name, (list_of_parameters), args...)
 */
#define CL_CALLS \
    ((cl_int, EnqueueNDRangeKernel, (cl_command_queue command_queue, \
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
    ((cl_int, WaitForEvents, (cl_uint num_events, \
                              const cl_event *event_list), \
        num_events, event_list)) \
    \
    ((cl_int, SetKernelArg, (cl_kernel kernel, \
                             cl_uint arg_index, \
                             size_t arg_size, \
                             const void *arg_value), \
                             kernel, arg_index, arg_size, arg_value)) \
    \
    ((cl_mem, CreateBuffer, (cl_context context, \
                             cl_mem_flags flags, \
                             size_t size, \
                             void *host_ptr, \
                             cl_int *errcode_ret), \
        context, flags, size, host_ptr, errcode_ret)) \
    \
    ((cl_int, EnqueueReadBuffer, (cl_command_queue command_queue, \
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
    ((cl_int, EnqueueReadBufferRect, (cl_command_queue command_queue, \
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
    ((cl_int, EnqueueWriteBuffer, (cl_command_queue command_queue, \
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
    ((cl_int, GetPlatformIDs, (cl_uint num_entries, \
                               cl_platform_id *platforms, \
                               cl_uint *num_platforms), \
        num_entries, platforms, num_platforms)) \
    \
    ((cl_int, GetPlatformInfo, (cl_platform_id platform, \
                                cl_platform_info param_name, \
                                size_t param_value_size, \
                                void *param_value, \
                                size_t *param_value_size_ret), \
		platform, param_name, param_value_size, param_value, \
		param_value_size_ret)) \
    \
    ((cl_int, GetDeviceIDs, (cl_platform_id platform, \
                             cl_device_type device_type, \
                             cl_uint num_entries, \
                             cl_device_id *devices, \
                             cl_uint *num_devices), \
		platform, device_type, num_entries, devices, num_devices)) \
    \
    ((cl_int, GetDeviceInfo, (cl_device_id device, \
                              cl_device_info param_name, \
                              size_t param_value_size, \
                              void *param_value, \
                              size_t *param_value_size_ret), \
		device, param_name, param_value_size, param_value, \
		param_value_size_ret)) \
	\
    ((cl_int, EnqueueFillBuffer, (cl_command_queue command_queue, \
                                  cl_mem buffer, \
                                  const void *pattern, \
                                  size_t pattern_size, \
                                  size_t offset, \
                                  size_t size, \
                                  cl_uint num_events_in_wait_list, \
                                  const cl_event *event_wait_list, \
                                  cl_event *event), \
		command_queue, buffer, pattern, pattern_size, offset, size, \
		num_events_in_wait_list, event_wait_list, event)) \
	\
    ((cl_int, EnqueueCopyBuffer, (cl_command_queue command_queue, \
                                  cl_mem src_buffer, \
                                  cl_mem dst_buffer, \
                                  size_t src_offset, \
                                  size_t dst_offset, \
                                  size_t cb, \
                                  cl_uint num_events_in_wait_list, \
                                  const cl_event *event_wait_list, \
                                  cl_event *event), \
		command_queue, src_buffer, dst_buffer, src_offset, dst_offset, \
		cb, num_events_in_wait_list, event_wait_list, event)) \
	\
    ((void*, EnqueueMapBuffer, (cl_command_queue command_queue, \
                                cl_mem buffer, \
                                cl_bool blocking_map, \
                                cl_map_flags map_flags, \
                                size_t offset, \
                                size_t cb, \
                                cl_uint num_events_in_wait_list, \
                                const cl_event *event_wait_list, \
                                cl_event *event, \
                                cl_int *errcode_ret), \
		command_queue, buffer, blocking_map, map_flags, offset, cb, \
		num_events_in_wait_list, event_wait_list, event, errcode_ret)) \
	\
	((cl_int, RetainMemObject, (cl_mem memobj), memobj)) \
	\
	((cl_int, ReleaseMemObject, (cl_mem memobj), memobj)) \
	\
    ((cl_int, EnqueueUnmapMemObject, (cl_command_queue command_queue, \
                                     cl_mem memobj, \
                                     void *mapped_ptr, \
                                     cl_uint num_events_in_wait_list, \
                                     const cl_event *event_wait_list, \
                                     cl_event *event), \
		command_queue, memobj, mapped_ptr, num_events_in_wait_list, \
		event_wait_list, event)) \
	\
    ((cl_program, CreateProgramWithSource, (cl_context context, \
                                            cl_uint count, \
                                            const char **strings, \
                                            const size_t *lengths, \
                                            cl_int *errcode_ret), \
		context, count, strings, lengths, errcode_ret)) \
	\
    ((cl_program, CreateProgramWithBinary, (cl_context context, \
                                            cl_uint num_devices, \
                                            const cl_device_id *device_list, \
                                            const size_t *lengths, \
                                            const unsigned char **binaries, \
                                            cl_int *binary_status, \
                                            cl_int *errcode_ret), \
		context, num_devices, device_list, lengths, binaries, \
		binary_status, errcode_ret)) \
	\
	((cl_int, ReleaseProgram, (cl_program program), program)) \
	\
    ((cl_int, BuildProgram ,(cl_program program, \
                             cl_uint num_devices, \
                             const cl_device_id *device_list, \
                             const char *options, \
                             void (*pfn_notify)(cl_program, void *user_data), \
                             void *user_data), \
		program, num_devices, device_list, options, pfn_notify, user_data)) \
	\
    ((cl_int, CompileProgram, (cl_program program, \
                               cl_uint num_devices, \
                               const cl_device_id *device_list, \
                               const char *options, \
                               cl_uint num_input_headers, \
                               const cl_program *input_headers, \
                               const char **header_include_names, \
                               void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), \
                               void *user_data), \
		program, num_devices, device_list, options, num_input_headers, \
		input_headers, header_include_names, pfn_notify, user_data)) \
	\
    ((cl_kernel, CreateKernel, (cl_program  program, \
                                const char *kernel_name, \
                                cl_int *errcode_ret), \
		program, kernel_name, errcode_ret)) \
	\
    ((cl_int, CreateKernelsInProgram, (cl_program program, \
                                       cl_uint num_kernels, \
                                       cl_kernel *kernels, \
                                       cl_uint *num_kernels_ret), \
		program, num_kernels, kernels, num_kernels_ret)) \
	\
	((cl_int, ReleaseKernel, (cl_kernel kernel), kernel)) \
	\
    ((cl_int, EnqueueTask, (cl_command_queue command_queue, \
                            cl_kernel kernel, \
                            cl_uint num_events_in_wait_list, \
                            const cl_event *event_wait_list, \
                            cl_event *event), \
		command_queue, kernel, num_events_in_wait_list, event_wait_list, \
		event)) \
	\
    ((cl_int, SetEventCallback, (cl_event event, \
                                 cl_int command_exec_callback_type, \
                                 void (CL_CALLBACK  *pfn_event_notify)(cl_event event, cl_int event_command_exec_status, void *user_data), \
                                 void *user_data), \
		event, command_exec_callback_type, pfn_event_notify, user_data)) \
	\
	((cl_int, Flush, (cl_command_queue command_queue), command_queue)) \
	\
	((cl_int, Finish, (cl_command_queue command_queue), command_queue)) \
    \
    ((cl_int, EnqueueMarkerWithWaitList, (cl_command_queue command_queue, \
                                          cl_uint num_events_in_wait_list, \
                                          const cl_event *event_wait_list, \
                                          cl_event *event), \
         command_queue, num_events_in_wait_list, event_wait_list, event))

/**
 * OpenCL function types
 */
FOR_EACH(BUILD_FN_TYPE, CL_CALLS)

/**
 * vocl class
 */
class Vocl {
  public:
    Vocl() {
    }

    FOR_EACH(BUILD_VOCL, CL_CALLS)

};

#endif /* _VOCL_H_ */
