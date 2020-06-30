// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include <filesystem>

/**
 * @class Accounts
 * @brief Backup and restore accounts: users, groups and passwords.
 */
class Accounts
{
  public:
    /**
     * @brief Constructor.
     *
     * @param[in] srcRoot source path to root FS
     * @param[in] dstRoot destination path to root FS
     * @param[in] roRoot path to RO root FS (usually "/run/initramfs/ro")
     *
     * @throw std::exception in case of errors
     */
    Accounts(const std::filesystem::path& srcRoot,
             const std::filesystem::path& dstRoot,
             const std::filesystem::path& roRoot);

    /**
     * @brief Backup accounts files.
     *
     * @throw std::exception in case of errors
     */
    void backup();

    /**
     * @brief Restore accounts files.
     *
     * @throw std::exception in case of errors
     */
    void restore();

  private:
    /**
     * @brief Backup groups.
     *
     * @throw std::exception in case of errors
     */
    void backupGroup();

    /**
     * @brief Backup users.
     *
     * @throw std::exception in case of errors
     */
    void backupPasswd();

    /**
     * @brief Backup passwords.
     *
     * @throw std::exception in case of errors
     */
    void backupShadow();

    /**
     * @brief Restore groups.
     *
     * @throw std::exception in case of errors
     */
    void restoreGroup();

    /**
     * @brief Restore users.
     *
     * @throw std::exception in case of errors
     */
    void restorePasswd();

    /**
     * @brief Restore passwords.
     *
     * @throw std::exception in case of errors
     */
    void restoreShadow();

  private:
    /** @brief Source directory. */
    const std::filesystem::path srcDir;
    /** @brief Destination directory. */
    const std::filesystem::path dstDir;
    /** @brief RO directory. */
    const std::filesystem::path roDir;
};
