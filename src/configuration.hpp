// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include <filesystem>

/**
 * @struct Configuration
 * @brief Configuration of the backup/restore procedure.
 */
struct Configuration
{
    /** @brief Path to the backup file. */
    std::filesystem::path backupFile;

    /** @brief Handle accounts data (enable/disable flag). */
    bool handleAccounts = true;

    /** @brief Handle network configuration (enable/disable flag). */
    bool handleNetwork = true;

    /** @brief Path to the root file system. */
    std::filesystem::path rootFs = "/";

    /** @brief Path to the read only file system. */
    std::filesystem::path readOnlyFs = "/run/initramfs/ro";
};
