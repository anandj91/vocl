#ifndef _VOCL_H_
#define _VOCL_H_

#include <iostream>
#include "base.h"

/**
 * OpenCL function types
 */
FOR_EACH(BUILD_FN_TYPE, CL_CALLS)

/**
 * vocl class
 */
class Vocl {
  public:
    Vocl()
    {
        //init_ctx();
    }

    FOR_EACH(BUILD_VOCL, CL_CALLS)

  protected:
    void init_ctx()
    {
        cl_platform_id platform_id = NULL;
        cl_device_id device_id = NULL;
        cl_uint ret_num_platforms;
        cl_uint ret_num_devices;

		cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
		ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, 
				&device_id, &ret_num_devices);
        cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    }

  private:
};

#endif /* _VOCL_H_ */
