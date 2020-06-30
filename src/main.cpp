// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "backup.hpp"
#include "version.hpp"

#include <getopt.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace fs = std::filesystem;

/**
 * @class Operation
 * @brief Operation types.
 */
enum class Operation
{
    backup,
    restore
};

/**
 * @brief Print help usage info.
 *
 * @param[in] app application's file name
 */
static void printHelp(const char* app)
{
    puts("OpenBMC backup tool.");
    puts("Copyright (c) 2020 YADRO.");
    puts("Version " VERSION);
    printf("Usage: %s [OPTION...] {backup|restore} FILE\n", app);
    puts("  -a, --skip-accounts  Skip accounts data");
    puts("  -n, --skip-network   Skip network configuration");
    puts("  -y, --yes            Do not ask for confirmation");
    puts("  -h, --help           Print this help and exit");
}

/** @brief Application entry point. */
int main(int argc, char* argv[])
{
    Configuration config;

    // clang-format off
    const struct option longOpts[] = {
        {"skip-accounts", no_argument,       nullptr, 'a'},
        {"skip-network",  no_argument,       nullptr, 'n'},
        {"yes",           no_argument,       nullptr, 'y'},
        {"help",          no_argument,       nullptr, 'h'},
        {nullptr,         0,                 nullptr,  0 }
    };
    // clang-format on
    const char* shortOpts = "anyh";

    opterr = 0; // prevent native error messages

    // parse arguments
    int val;
    while ((val = getopt_long(argc, argv, shortOpts, longOpts, nullptr)) != -1)
    {
        switch (val)
        {
            case 'a':
                config.handleAccounts = false;
                break;
            case 'n':
                config.handleNetwork = false;
                break;
            case 'y':
                config.unattendedMode = true;
                break;
            case 'h':
                printHelp(argv[0]);
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Invalid option: %s\n", argv[optind - 1]);
                return EXIT_FAILURE;
        }
    }

    // there are must be exactly 2 positional arguments (operation + file name)
    if (optind + 2 > argc)
    {
        fprintf(stderr,
                "Invalid arguments: expected \"backup|restore FILE\"\n");
        return EXIT_FAILURE;
    }
    else if (optind + 2 < argc)
    {
        fprintf(stderr, "Unexpected argument: %s\n", argv[optind + 2]);
        return EXIT_FAILURE;
    }

    // get operation type from positional argument
    Operation operation;
    if (strcmp(argv[optind], "backup") == 0)
    {
        operation = Operation::backup;
    }
    else if (strcmp(argv[optind], "restore") == 0)
    {
        operation = Operation::restore;
    }
    else
    {
        fprintf(stderr,
                "Invalid argument: %s, expected \"backup\" or \"restore\"\n",
                argv[optind]);
        return EXIT_FAILURE;
    }
    ++optind;

    // get file name from positional argument
    config.backupFile = argv[optind];
    if (operation == Operation::backup && fs::exists(config.backupFile))
    {
        fprintf(stderr, "Backup file already exists: %s\n",
                config.backupFile.c_str());
        return EXIT_FAILURE;
    }
    if (operation == Operation::restore && !fs::exists(config.backupFile))
    {
        fprintf(stderr, "Backup file not found: %s\n",
                config.backupFile.c_str());
        return EXIT_FAILURE;
    }

    try
    {
        Backup bk(config);
        if (operation == Operation::backup)
        {
            bk.backup();
            printf("Backup created: %s\n", config.backupFile.c_str());
        }
        else
        {
            bk.restore();
            puts("Configuration was restored.");
            puts("Please reboot the BMC to apply changes.");
        }
    }
    catch (std::exception& ex)
    {
        fprintf(stderr, "%s\n", ex.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
