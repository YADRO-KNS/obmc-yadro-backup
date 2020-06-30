// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include <filesystem>

/**
 * @class Backup
 * @brief Backup/restore OpenBMC configuration.
 */
class Backup
{
  public:
    ~Backup();

    /**
     * @brief Backup OpenBMC configuration.
     *
     * @throw std::exception in case of errors
     */
    void backup();

    /**
     * @brief Restore OpenBMC configuration.
     *
     * @throw std::exception in case of errors
     */
    void restore();

  private:
    /**
     * @brief Create temp directory.
     *
     * @throw std::system_error in case of other errors
     *
     * @return path to the created dir
     */
    std::filesystem::path createTempDir() const;

    /**
     * @brief Check manifest of early created backup.
     *
     * @throw std::runtime_error in case of errors
     */
    void checkManifest() const;

    /**
     * @brief Backup single file or directory.
     *
     * @param[in] path relative path to the file
     *
     * @throw std::runtime_error in case of errors
     */
    void backupFile(const char* path) const;

    /**
     * @brief Restore single file or directory.
     *
     * @param[in] path relative path to the file
     *
     * @throw std::runtime_error in case of errors
     */
    void restoreFile(const char* path) const;

    /**
     * @brief Call tar for creating or extracting archive.
     *
     * @param[in] extract operation type: true=extract, false=create
     *
     * @throw std::runtime_error in case of errors
     */
    void callTar(bool extract) const;

  public:
    /** @brief Unattended mode (enable/disable flag). */
    bool unattendedMode = false;
    /** @brief Handle accounts data (enable/disable flag). */
    bool handleAccounts = true;
    /** @brief Handle network configuration (enable/disable flag). */
    bool handleNetwork = true;
    /** @brief Path to the backup archive file. */
    std::filesystem::path archiveFile;
    /** @brief Path to the root file system. */
    std::filesystem::path rootFs = "/";
    /** @brief Path to the read only file system. */
    std::filesystem::path readOnlyFs = "/run/initramfs/ro";

  private:
    /** @brief Temporary directory used for unpacked data. */
    std::filesystem::path tmpDir;
};
