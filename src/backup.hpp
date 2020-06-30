// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include "configuration.hpp"

/**
 * @class Backup
 * @brief Backup/restore OpenBMC configuration.
 */
class Backup
{
  public:
    /**
     * @brief Constructor.
     *
     * @param[in] cfg configuration of the backup/restore procedure
     *
     * @throw std::exception in case of errors
     */
    Backup(const Configuration& cfg);

    ~Backup();

    /**
     * @brief Backup OpenBMC configuration.
     *
     * @throw std::exception in case of errors
     */
    void backup() const;

    /**
     * @brief Restore OpenBMC configuration.
     *
     * @throw std::exception in case of errors
     */
    void restore() const;

  private:
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

  private:
    /** @brief Configuration of the backup/restore procedure. */
    const Configuration& config;
    /** @brief Temporary directory used for unpacked data. */
    std::filesystem::path tmpDir;
};
