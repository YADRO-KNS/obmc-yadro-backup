// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "backup.hpp"
#include "version.hpp"

#include <getopt.h>

#include <cstdio>
#include <cstdlib>
#include <optional>
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
    printf("Usage: %s [OPTION...]\n", app);
    puts("  -b, --backup=FILE    Backup current configuration to file");
    puts("  -r, --restore=FILE   Restore configuration from backup file");
    puts("  -a, --skip-accounts  Skip accounts data");
    puts("  -n, --skip-network   Skip network configuration");
    puts("  -h, --help           Print this help and exit");
}

/** @brief Application entry point. */
int main(int argc, char* argv[])
{
    Configuration config;
    std::optional<Operation> operation = std::nullopt;

    // clang-format off
    const struct option longOpts[] = {
        {"backup",        required_argument, nullptr, 'b'},
        {"restore",       required_argument, nullptr, 'r'},
        {"skip-accounts", no_argument,       nullptr, 'a'},
        {"skip-network",  no_argument,       nullptr, 'n'},
        {"help",          no_argument,       nullptr, 'h'},
        {nullptr,         0,                 nullptr,  0 }
    };
    // clang-format on
    const char* shortOpts = "b:r:anh";

    opterr = 0; // prevent native error messages

    // parse arguments
    int val;
    while ((val = getopt_long(argc, argv, shortOpts, longOpts, nullptr)) != -1)
    {
        switch (val)
        {
            case 'b':
                operation = Operation::backup;
                config.backupFile = optarg;
                break;
            case 'r':
                operation = Operation::restore;
                config.backupFile = optarg;
                break;
            case 'a':
                config.handleAccounts = false;
                break;
            case 'n':
                config.handleNetwork = false;
                break;
            case 'h':
                printHelp(argv[0]);
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Invalid option: %s\n", argv[optind - 1]);
                return EXIT_FAILURE;
        }
    }
    if (optind < argc)
    {
        fprintf(stderr, "Unexpected option: %s\n", argv[optind]);
        return EXIT_FAILURE;
    }
    if (!operation.has_value())
    {
        fprintf(stderr, "Unknown operation, expected backup or restore\n");
        return EXIT_FAILURE;
    }
    if (config.backupFile.empty())
    {
        fprintf(stderr, "Backup file name can not be empty\n");
        return EXIT_FAILURE;
    }
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
