// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "manifest.hpp"

#include <fstream>
#include <map>
#include <regex>

namespace fs = std::filesystem;

/** @brief Name of the manifest file. */
static const char* manifestFile = "bmc.manifest";

/** @brief Name of OS version property. */
static const char* osVersionProp = "VERSION";

/**
 * @brief Parse ini file.
 *
 * @param[in] iniFile path to the ini file to parse
 *
 * @throw std::runtime_error in case of errors
 *
 * @return map of ini values
 */
static std::map<std::string, std::string> parseIni(const fs::path& iniFile)
{
    std::map<std::string, std::string> data;
    const std::regex iniRegex("([^= ]+)\\s*=\\s*\"?([^\"]+)\"?");

    std::ifstream file(iniFile);
    if (!file)
    {
        std::string err = "Error opening file ";
        err += iniFile;
        throw std::runtime_error(err);
    }
    std::string line;
    while (std::getline(file, line))
    {

        std::smatch match;
        if (std::regex_match(line, match, iniRegex))
        {
            const auto& name = match[1].str();
            const auto& value = match[2].str();
            data.insert(std::make_pair(name, value));
        }
    }

    return data;
}

/**
 * @brief Create manifest from any ini file.
 *
 * @param[in] iniFile path to the ini file to parse
 *
 * @throw std::runtime_error in case of errors
 *
 * @return manifest instance
 */
static Manifest fromIni(const std::filesystem::path& iniFile)
{
    const std::map<std::string, std::string> ini = parseIni(iniFile);

    const auto ver = ini.find(osVersionProp);
    if (ver == ini.end())
    {
        std::string err = "Invalid file format (VERSION field not found): ";
        err += iniFile;
        throw std::runtime_error(err);
    }

    Manifest manifest{ver->second};
    return manifest;
}

Manifest Manifest::fromOsRelease(const std::filesystem::path& rootFs)
{
    return fromIni(rootFs / "etc/os-release");
}

Manifest Manifest::load(const fs::path& dir)
{
    return fromIni(dir / manifestFile);
}

void Manifest::save(const fs::path& dir) const
{
    const fs::path iniFile = dir / manifestFile;
    std::ofstream file(iniFile);
    if (!file)
    {
        std::string err = "Error creating file ";
        err += iniFile;
        throw std::runtime_error(err);
    }
    file << osVersionProp << "=\"" << osVersion << '"' << std::endl;
}
