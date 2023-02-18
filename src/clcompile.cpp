// SPDX-License-Identifier: MIT
// Copyright 2023 Edouard Gomez

#include <CL/cl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "clutils.h"
#include "scope_guard.h"

namespace
{

/** Loads the content from a file
 *
 * @param[in] fn filename to load
 *
 * @return nullptr if failed, or a valid pointer to a zero terminated c string
 * containing the file's contents
 */
char *load_file(const char *fn)
{
    FILE *f = std::fopen(fn, "r");
    if (!f)
    {
        std::fprintf(stderr, "error: failed opening the file \"%s\"\n", fn);
        return nullptr;
    }
    on_scope_guard([f]() { fclose(f); });

    if (fseek(f, 0, SEEK_END) < 0)
    {
        std::fprintf(stderr, "error: could not seek to the end of the file \"%s\"\n", fn);
        return nullptr;
    };

    long flen = ftell(f);
    if (flen < 0)
    {
        std::fprintf(stderr, "error: failed determining the size of the file \"%s\"\n", fn);
        return nullptr;
    }
    if (fseek(f, 0, SEEK_SET) < 0)
    {
        std::fprintf(stderr, "error: could not seek back to the beginning of the file \"%s\"\n", fn);
        return nullptr;
    };

    char *source = new char[flen];
    if (!source)
    {
        std::fprintf(stderr, "error: failed allocating memory for reading source file \"%s\"\n", fn);
        return nullptr;
    }
    on_scope_guard_named(failedRead, [source]() { delete source; });

    if (std::fread(source, 1, flen, f) != flen)
    {
        std::fprintf(stderr, "error: failed reading the source file \"%s\"'s content\n", fn);
        return nullptr;
    }

    return source;
}

void opencl_create_context(cl_uint platform_id, cl_uint device_id)
{
    using namespace clc;

    cl_uint num_platforms;
    cl_int err = clGetPlatformIDs(0, nullptr, &num_platforms);
    if (err != CL_SUCCESS)
    {
        std::fprintf(stderr, "error: could not retrieve the number of platforms (err=%s)\n", cl_error_str(err));
        return;
    }

    if (platform_id >= num_platforms)
    {
        std::fprintf(stderr, "error: the requested platform %ud cannot be found\n", platform_id);
        return;
    }

    std::vector<cl_platform_id> platforms(static_cast<size_t>(num_platforms));
    err = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
    if (err != CL_SUCCESS)
    {
        std::fprintf(stderr, "error: could not retrieve the platforms IDs (err=%s)\n", cl_error_str(err));
        return;
    }

    cl_uint num_devices;
    err = clGetDeviceIDs(platforms[platform_id], CL_DEVICE_TYPE_ALL, 0, nullptr, &num_devices);
    if (err != CL_SUCCESS)
    {
        std::fprintf(stderr,
                     "error: could not retrieve the number of devices "
                     "for platform=%ud (err=%s)\n",
                     platform_id, cl_error_str(err));
        return;
    }

    if (device_id >= num_devices)
    {
        std::fprintf(stderr, "error: no device index=%ud found for platform=%ud\n", device_id, platform_id);
        return;
    }

    std::vector<cl_device_id> devices(num_devices);
    err = clGetDeviceIDs(platforms[platform_id], CL_DEVICE_TYPE_ALL, devices.size(), devices.data(), nullptr);
    if (err != CL_SUCCESS)
    {
        std::fprintf(stderr,
                     "error: could not retrieve the devices IDs "
                     "for platform=%ud (err=%s)\n",
                     platform_id, cl_error_str(err));
        return;
    }

    cl_device_id device = devices[device_id];

    size_t name_len;
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, 0, nullptr, &name_len);
    if (err != CL_SUCCESS)
    {
        std::fprintf(stderr,
                     "error: could not retrieve the device name length"
                     "for platform=%ud device=%ud (err=%s)\n",
                     platform_id, device_id, cl_error_str(err));
        return;
    }

    std::vector<char> name(name_len);
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, name_len, name.data(), NULL);
    if (err != CL_SUCCESS)
    {
        std::fprintf(stderr,
                     "error: could not retrieve the device name length"
                     "for platform=%ud device=%ud (err=%s)\n",
                     platform_id, device_id, cl_error_str(err));
        return;
    }

    std::printf("info: found device %s\n", name.data());
}

/** Program options structure */
struct clcompile_options
{
    /** Files to be compiled */
    std::vector<const char *> filenames;

    /** Options to pass over to teh CL compiler */
    std::vector<const char *> clargs;

    /** CL Platform ID used for the compilation */
    cl_uint platform_id = 0;

    /** CL Device used for the compilation */
    cl_uint device_id = 0;
};

/** Print the help message of the program to stdout */
void print_help()
{
    std::printf("usage: clcompile [OPTION...] <filename...> -- [CLOPTION...]\n"
                "\n"
                "OPTIONS\n"
                "\n"
                "-p, --platform-id <INTEGER> Index of the platform to target\n"
                "-d, --device-id   <INTEGER> Index of the device to target\n"
                "\n"
                "-h, --help                  Print this help message\n"
                "\n"
                "CLOPTIONS\n"
                "\n"
                "See options listed on https://man.opencl.org/clBuildProgram.html\n");
}

/** Parse the program command line arguments
 *
 * @param[in] argc Number of arguments in the @ref argv argument array
 * @param[in] argv Array of zero terminated strings
 * @param[out] exit Should the program exit according to the argument processing
 * @param[out] options Resulting options from the argument processing
 *
 * @return Return value to be used on program exit (EXIT_SUCCESS when the processing succeeded, EXIT_FAILURE when some
 * option could not be parsed)
 */
int parse_args(int argc, const char **argv, bool &exit, clcompile_options &options)
{

    if (argc < 2)
    {
        print_help();
        exit = true;
        return EXIT_FAILURE;
    }

    int i = 1;

    // process non cl options
    for (i = 1; i < argc; ++i)
    {
        if (!std::strcmp("--device-id", argv[i]) || !std::strcmp("-d", argv[i]))
        {
            if (i < argc - 1)
            {
                std::fprintf(stderr, "error: missing argument for option %s", argv[i]);
                exit = true;
                return EXIT_FAILURE;
            }
            options.device_id = std::atoi(argv[i + 1]);
            ++i;
        }
        else if (!strcmp("--platform-id", argv[i]) || !strcmp("-d", argv[i]))
        {
            if (i < argc - 1)
            {
                std::fprintf(stderr, "error: missing argument for option %s", argv[i]);
                exit = true;
                return EXIT_FAILURE;
            }
            options.platform_id = atoi(argv[i + 1]);
            ++i;
        }
        else if (!strcmp("--help", argv[i]) || !strcmp("-h", argv[i]))
        {
            print_help();
            exit = true;
            return EXIT_SUCCESS;
        }
        else if (!strcmp("--", argv[i]))
        {
            // stop processing normal arguments, let the second loop accumulate
            // the options passed to the CL compiler
            ++i;
            break;
        }
        else
        {
            options.filenames.emplace_back(argv[i]);
        }
    }
    while (i < argc)
    {
        options.clargs.push_back(argv[i]);
        ++i;
    }

    if (options.filenames.size() == 0)
    {
        print_help();
        exit = true;
        return EXIT_FAILURE;
    }

    exit = false;
    return EXIT_SUCCESS;
}

} // namespace

int main(int argc, const char **argv)
{

    clcompile_options opts;
    bool exit;

    int retval = parse_args(argc, argv, exit, opts);
    if (exit)
    {
        return retval;
    }

    for (const auto &fn : opts.filenames)
    {
        char *source = load_file(fn);
        if (!source)
        {
            return EXIT_FAILURE;
        }
        on_scope_guard([source]() { delete source; });
    }

    return EXIT_SUCCESS;
}
