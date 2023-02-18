// SPDX-License-Identifier: MIT
// Copyright 2023 Edouard Gomez

#include "clutils.h"
#include "scope_guard.h"

#include <vector>

namespace clc
{

#define CL_ERRORCODE_STR(r)                                                                                            \
    case CL_##r:                                                                                                       \
        return "CL_" #r

const char *cl_error_str(cl_int errorcode)
{
    switch (errorcode)
    {
        CL_ERRORCODE_STR(SUCCESS);
        CL_ERRORCODE_STR(DEVICE_NOT_FOUND);
        CL_ERRORCODE_STR(DEVICE_NOT_AVAILABLE);
        CL_ERRORCODE_STR(COMPILER_NOT_AVAILABLE);
        CL_ERRORCODE_STR(MEM_OBJECT_ALLOCATION_FAILURE);
        CL_ERRORCODE_STR(OUT_OF_RESOURCES);
        CL_ERRORCODE_STR(OUT_OF_HOST_MEMORY);
        CL_ERRORCODE_STR(PROFILING_INFO_NOT_AVAILABLE);
        CL_ERRORCODE_STR(MEM_COPY_OVERLAP);
        CL_ERRORCODE_STR(IMAGE_FORMAT_MISMATCH);
        CL_ERRORCODE_STR(IMAGE_FORMAT_NOT_SUPPORTED);
        CL_ERRORCODE_STR(BUILD_PROGRAM_FAILURE);
        CL_ERRORCODE_STR(MAP_FAILURE);
#ifdef CL_VERSION_1_1
        CL_ERRORCODE_STR(MISALIGNED_SUB_BUFFER_OFFSET);
        CL_ERRORCODE_STR(EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
#endif
#ifdef CL_VERSION_1_2
        CL_ERRORCODE_STR(COMPILE_PROGRAM_FAILURE);
        CL_ERRORCODE_STR(LINKER_NOT_AVAILABLE);
        CL_ERRORCODE_STR(LINK_PROGRAM_FAILURE);
        CL_ERRORCODE_STR(DEVICE_PARTITION_FAILED);
        CL_ERRORCODE_STR(KERNEL_ARG_INFO_NOT_AVAILABLE);
#endif
        CL_ERRORCODE_STR(INVALID_VALUE);
        CL_ERRORCODE_STR(INVALID_DEVICE_TYPE);
        CL_ERRORCODE_STR(INVALID_PLATFORM);
        CL_ERRORCODE_STR(INVALID_DEVICE);
        CL_ERRORCODE_STR(INVALID_CONTEXT);
        CL_ERRORCODE_STR(INVALID_QUEUE_PROPERTIES);
        CL_ERRORCODE_STR(INVALID_COMMAND_QUEUE);
        CL_ERRORCODE_STR(INVALID_HOST_PTR);
        CL_ERRORCODE_STR(INVALID_MEM_OBJECT);
        CL_ERRORCODE_STR(INVALID_IMAGE_FORMAT_DESCRIPTOR);
        CL_ERRORCODE_STR(INVALID_IMAGE_SIZE);
        CL_ERRORCODE_STR(INVALID_SAMPLER);
        CL_ERRORCODE_STR(INVALID_BINARY);
        CL_ERRORCODE_STR(INVALID_BUILD_OPTIONS);
        CL_ERRORCODE_STR(INVALID_PROGRAM);
        CL_ERRORCODE_STR(INVALID_PROGRAM_EXECUTABLE);
        CL_ERRORCODE_STR(INVALID_KERNEL_NAME);
        CL_ERRORCODE_STR(INVALID_KERNEL_DEFINITION);
        CL_ERRORCODE_STR(INVALID_KERNEL);
        CL_ERRORCODE_STR(INVALID_ARG_INDEX);
        CL_ERRORCODE_STR(INVALID_ARG_VALUE);
        CL_ERRORCODE_STR(INVALID_ARG_SIZE);
        CL_ERRORCODE_STR(INVALID_KERNEL_ARGS);
        CL_ERRORCODE_STR(INVALID_WORK_DIMENSION);
        CL_ERRORCODE_STR(INVALID_WORK_GROUP_SIZE);
        CL_ERRORCODE_STR(INVALID_WORK_ITEM_SIZE);
        CL_ERRORCODE_STR(INVALID_GLOBAL_OFFSET);
        CL_ERRORCODE_STR(INVALID_EVENT_WAIT_LIST);
        CL_ERRORCODE_STR(INVALID_EVENT);
        CL_ERRORCODE_STR(INVALID_OPERATION);
        CL_ERRORCODE_STR(INVALID_GL_OBJECT);
        CL_ERRORCODE_STR(INVALID_BUFFER_SIZE);
        CL_ERRORCODE_STR(INVALID_MIP_LEVEL);
        CL_ERRORCODE_STR(INVALID_GLOBAL_WORK_SIZE);
#ifdef CL_VERSION_1_1
        CL_ERRORCODE_STR(INVALID_PROPERTY);
#endif
#ifdef CL_VERSION_1_2
        CL_ERRORCODE_STR(INVALID_IMAGE_DESCRIPTOR);
        CL_ERRORCODE_STR(INVALID_COMPILER_OPTIONS);
        CL_ERRORCODE_STR(INVALID_LINKER_OPTIONS);
        CL_ERRORCODE_STR(INVALID_DEVICE_PARTITION_COUNT);
#endif
#ifdef CL_VERSION_2_0
        CL_ERRORCODE_STR(INVALID_PIPE_SIZE);
        CL_ERRORCODE_STR(INVALID_DEVICE_QUEUE);
#endif
#ifdef CL_VERSION_2_2
        CL_ERRORCODE_STR(INVALID_SPEC_ID);
        CL_ERRORCODE_STR(MAX_SIZE_RESTRICTION_EXCEEDED);
#endif
    }
    return "<unknown>";
}
#undef CL_ERRORCODE_STR

compiler::~compiler()
{
    if (m_context)
    {
        clReleaseContext(m_context);
        m_context = nullptr;
    }
};

bool compiler::init(cl_uint platform_id, cl_uint device_id)
{
    cl_uint num_platforms;
    cl_int err = clGetPlatformIDs(0, nullptr, &num_platforms);
    if (err != CL_SUCCESS)
    {
        logerr("could not retrieve the number of platforms (err=%s)\n", cl_error_str(err));
        return false;
    }

    if (platform_id >= num_platforms)
    {
        logerr("the requested platform %ud cannot be found\n", platform_id);
        return false;
    }

    std::vector<cl_platform_id> platforms(static_cast<size_t>(num_platforms));
    err = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
    if (err != CL_SUCCESS)
    {
        logerr("could not retrieve the platforms IDs (err=%s)\n", cl_error_str(err));
        return false;
    }

    cl_uint num_devices;
    err = clGetDeviceIDs(platforms[platform_id], CL_DEVICE_TYPE_ALL, 0, nullptr, &num_devices);
    if (err != CL_SUCCESS)
    {
        logerr("could not retrieve the number of devices "
               "for platform=%ud (err=%s)\n",
               platform_id, cl_error_str(err));
        return false;
    }

    if (device_id >= num_devices)
    {
        logerr("no device index=%ud found for platform=%ud\n", device_id, platform_id);
        return false;
    }

    std::vector<cl_device_id> devices(num_devices);
    err = clGetDeviceIDs(platforms[platform_id], CL_DEVICE_TYPE_ALL, devices.size(), devices.data(), nullptr);
    if (err != CL_SUCCESS)
    {
        logerr("could not retrieve the devices IDs "
               "for platform=%ud (err=%s)\n",
               platform_id, cl_error_str(err));
        return false;
    }

    cl_device_id device = devices[device_id];

    size_t name_len;
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, 0, nullptr, &name_len);
    if (err != CL_SUCCESS)
    {
        logerr("could not retrieve the device name length"
               "for platform=%ud device=%ud (err=%s)\n",
               platform_id, device_id, cl_error_str(err));
        return false;
    }

    std::vector<char> name(name_len);
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, name_len, name.data(), NULL);
    if (err != CL_SUCCESS)
    {
        logerr("could not retrieve the device name length"
               "for platform=%ud device=%ud (err=%s)\n",
               platform_id, device_id, cl_error_str(err));
        return false;
    }

    loginfo("found device % s\n ", name.data());

    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    if (err != CL_SUCCESS)
    {
        logerr("failed creating context for platform=%ud device=%ud (err=%s)\n", platform_id, device_id,
               cl_error_str(err));
        return false;
    }

    m_platform = platforms[platform_id];
    m_device = devices[device_id];
    m_context = context;

    return true;
}

bool compiler::build(const char *src)
{
    cl_int err;

    cl_program program = clCreateProgramWithSource(m_context, 1, (const char **)&src, nullptr, &err);
    if (err != CL_SUCCESS)
    {
        logerr("failed creating program (err=%s)", cl_error_str(err));
        return false;
    }

    on_scope_guard([&program]() { clReleaseProgram(program); });

    err = clBuildProgram(program, 1, &m_device, "", nullptr, nullptr);
    if (err == CL_SUCCESS)
    {
        loginfo("program built successfully.\n");
        return true;
    }
    else
    {
        logerr("failed building the program (err=%s)\n", cl_error_str(err));
    }

    if (err == CL_BUILD_PROGRAM_FAILURE)
    {
        size_t sz;
        clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, 0, NULL, &sz);
        std::vector<char> log(++sz);
        clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, log.size(), log.data(), &sz);
        logerr("log length=%zd\nbuild log: \n%s\n", sz, log.data());
    }

    return false;
}

} // namespace clc