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
#define REAL_FN(x) CONCAT(real_, x)
#define OCL_FN(x) CONCAT(cl, x)
#define VOCL_FN(x) x

#define FUNC(t) GET_ELEM(t, 1)

#define BUILD_REAL_HELPER(ret_type, funcname, params, ...) \
    typedef ret_type (*FN_TYPE(funcname)) params; \
    ret_type REAL_FN(funcname) params \
    { \
        static auto real_fn = (FN_TYPE(funcname))real_dlsym(RTLD_NEXT, \
                STRINGIFY(OCL_FN(funcname))); \
        auto ret = real_fn(__VA_ARGS__); \
        return ret; \
    } \

#define BUILD_OCL_HELPER(ret_type, funcname, params, ...) \
    ret_type OCL_FN(funcname) params \
    { \
        ret_type ret = VoclFactory::Get()->VOCL_FN(funcname)(__VA_ARGS__); \
        return ret; \
    } \


#define BUILD_VOCL_HELPER(ret_type, funcname, params, ...) \
    virtual ret_type funcname params \
    { \
        auto ret = REAL_FN(funcname)(__VA_ARGS__); \
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

#define BUILD_REAL(r, data, ocl_call) \
    BUILD_REAL_HELPER ocl_call

