// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include <filesystem>
#include <map>

/**
 * @struct Manifest
 * @brief Manifest file.
 */
class Manifest
{
  private:
    Manifest() = default;

  public:
    /**
     * @brief Constructor - create manifest for current system.
     *
     * @param[in] rootFs path to the root FS
     *
     * @throw std::runtime_error in case of errors
     */
    Manifest(const std::filesystem::path& rootFs);

    /**
     * @brief Load manifest from file.
     *
     * @param[in] dir path to the directory with manifest file
     *
     * @throw std::runtime_error in case of errors
     *
     * @return manifest instance
     */
    static Manifest load(const std::filesystem::path& dir);

    /**
     * @brief Save manifest to a file.
     *
     * @param[in] dir path to the directory for the manifest file
     *
     * @throw std::runtime_error in case of errors
     */
    void save(const std::filesystem::path& dir) const;

    /**
     * @brief Print manifest data to stdout.
     */
    void print() const;

    /** @brief Get OS version. */
    const std::string& osVersion() const;
    /** @brief Get OS version as numeric format for campare. */
    uint32_t osVersionNumber() const;
    /** @brief Get machine name. */
    const std::string& machineName() const;
    /** @brief Get host name. */
    const std::string& hostName() const;

  private:
    /** @brief Properties. */
    std::map<std::string, std::string> properties;
};
