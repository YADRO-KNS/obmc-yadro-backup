// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include <filesystem>

/**
 * @struct Manifest
 * @brief Manifest file.
 */
struct Manifest
{
    /** @brief Version of OS. */
    std::string osVersion;

    /**
     * @brief Create manifest from os-release file.
     *
     * @param[in] rootFs path to the root FS
     *
     * @throw std::runtime_error in case of errors
     *
     * @return manifest instance
     */
    static Manifest fromOsRelease(const std::filesystem::path& rootFs);

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
};
