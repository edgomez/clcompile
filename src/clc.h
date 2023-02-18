// SPDX-License-Identifier: MIT
// Copyright 2023 Edouard Gomez

#ifndef clc_h
#define clc_h

#include <CL/cl.h>

namespace clc
{

/** Returns a zero terminated string representation of the OpenCL error code
 * @param[in] errorcode Errorcode to translate to a string
 * @return A zero terminated string representing the OpenCL error code
 */
const char *cl_error_str(cl_int errorcode);

/** compiler context */
class compiler
{
  public:
    compiler() = default;
    ~compiler();

    /** Initialize an OpenCL context
     *
     * @param[in] platform_id Platform index to create the context for
     * @param[in] platform_id Platform index to create the context for
     * @return true if succeeded, false otherwise
     */
    bool init(cl_uint platform_id, cl_uint device_id);

    /** Builds an OpenCL program
     * @param[in] src Source text
     * @return true if succeeded, false otherwise
     */
    bool build(const char *src);

  private:
    /** platform in use */
    cl_platform_id m_platform = nullptr;

    /** device in use */
    cl_device_id m_device = nullptr;

    /** opencl context */
    cl_context m_context = nullptr;
};

} // namespace clc

#endif // clc_h
