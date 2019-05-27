#ifndef _VOCL_H_
# define _VOCL_H_

#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <dlfcn.h>
#include <CL/cl.h>
#include <string.h>

#define STRINGIFY(x) #x

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

typedef cl_int (*fnClEnqueueNDRangeKernel)(cl_command_queue,
		cl_kernel, cl_uint, const size_t*, const size_t*, const size_t*,
		cl_uint, const cl_event*, cl_event*);

typedef cl_int (*fnClWaitForEvents)(cl_uint, const cl_event*);


#endif /* _VOCL_H_ */
