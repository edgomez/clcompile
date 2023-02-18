// SPDX-License-Identifier: MIT
// Copyright 2023 Edouard Gomez

#include "clc.h"
#include "log.h"
#include "scope_guard.h"

#include <CL/cl.h>
#include <cstdlib>
#include <cstring>
#include <vector>

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
        logerr("failed opening the file \"%s\"\n", fn);
        return nullptr;
    }
    on_scope_guard([f]() { fclose(f); });

    if (fseek(f, 0, SEEK_END) < 0)
    {
        logerr("could not seek to the end of the file \"%s\"\n", fn);
        return nullptr;
    };

    long flen = ftell(f);
    if (flen < 0)
    {
        logerr("failed determining the size of the file \"%s\"\n", fn);
        return nullptr;
    }
    if (fseek(f, 0, SEEK_SET) < 0)
    {
        logerr("could not seek back to the beginning of the file \"%s\"\n", fn);
        return nullptr;
    };

    char *source = new char[flen];
    if (!source)
    {
        logerr("failed allocating memory for reading source file \"%s\"\n", fn);
        return nullptr;
    }
    on_scope_guard_named(failedRead, [source]() { delete[] source; });

    if (std::fread(source, 1, flen, f) != flen)
    {
        logerr("failed reading the source file \"%s\"'s content\n", fn);
        return nullptr;
    }

    failedRead.dismiss();

    return source;
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
                "-v, --version               Print the program's version\n"
                "\n"
                "CLOPTIONS\n"
                "\n"
                "See options listed on https://man.opencl.org/clBuildProgram.html\n");
}

#define CLC_STRINGIFY_(a) #a
#define CLC_STRINGIFY(a) CLC_STRINGIFY_(a)

/** Print the version message of the program to stdout */
void print_version()
{
    std::printf("0.1 (cl_target_opencl_version:%s)\n", CLC_STRINGIFY(CL_TARGET_OPENCL_VERSION));
}

/** Parse the program command line arguments
 *
 * @param[in] argc Number of arguments in the @ref argv argument array
 * @param[in] argv Array of zero terminated strings
 * @param[out] exit Should the program exit according to the argument processing
 * @param[out] options Resulting options from the argument processing
 *
 * @return Return value to be used on program exit (EXIT_SUCCESS when the processing succeeded, EXIT_FAILURE when
 * some option could not be parsed)
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
                logerr("missing argument for option %s", argv[i]);
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
                logerr("missing argument for option %s", argv[i]);
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
        else if (!strcmp("--version", argv[i]) || !strcmp("-v", argv[i]))
        {
            print_version();
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

    clc::compiler c;
    if (!c.init(opts.platform_id, opts.device_id))
    {
        return EXIT_FAILURE;
    }

    for (const auto &fn : opts.filenames)
    {
        char *source = load_file(fn);
        if (!source)
        {
            return EXIT_FAILURE;
        }
        on_scope_guard([source]() { delete[] source; });
        c.build(source);
    }

    return EXIT_SUCCESS;
}
