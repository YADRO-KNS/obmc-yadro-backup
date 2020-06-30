// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "manifest.hpp"

#include <limits.h>
#include <unistd.h>

#include <fstream>
#include <regex>

namespace fs = std::filesystem;

/** @brief Name of the manifest file. */
static const std::string manifestFile = "bmc.manifest";

/** @brief Name of OS version property. */
static const std::string osVersionProp = "VERSION";
/** @brief Name of machine (platform) name property. */
static const std::string machineNameProp = "MACHINE";
/** @brief Name of host name property. */
static const std::string hostNameProp = "HOSTNAME";

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

Manifest::Manifest(const std::filesystem::path& rootFs)
{
    const fs::path osRelease = rootFs / "etc/os-release";
    const std::map<std::string, std::string> ini = parseIni(osRelease);
    const std::map<std::string, std::string> osr{
        {"OPENBMC_TARGET_MACHINE", machineNameProp},
        {"VERSION", osVersionProp}};
    for (const auto& it : osr)
    {
        const auto val = ini.find(it.first);
        if (val == ini.end())
        {
            std::string err = "Invalid os-release file format: Property ";
            err += it.first;
            err += " not found in file ";
            err += osRelease;
            throw std::runtime_error(err);
        }
        properties.insert(std::make_pair(it.second, val->second));
    }

    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        throw std::runtime_error("Unable to get host name");
    }
    properties.insert(std::make_pair(hostNameProp, hostname));
}

Manifest Manifest::load(const fs::path& dir)
{
    Manifest manifest;

    const fs::path mnfFile = dir / manifestFile;
    const std::map<std::string, std::string> ini = parseIni(mnfFile);

    for (const auto& prop : {osVersionProp, machineNameProp, hostNameProp})
    {
        const auto val = ini.find(prop);
        if (val == ini.end())
        {
            std::string err = "Invalid manifest file format: Property ";
            err += prop;
            err += " not found in file ";
            err += mnfFile;
            throw std::runtime_error(err);
        }
        manifest.properties.insert(*val);
    }

    return manifest;
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
    for (const auto& it : properties)
    {
        file << it.first << "=\"" << it.second << '"' << std::endl;
    }
}

void Manifest::print() const
{
    for (const auto& it : properties)
    {
        printf("%-8s : %s\n", it.first.c_str(), it.second.c_str());
    }
}

const std::string& Manifest::osVersion() const
{
    return properties.find(osVersionProp)->second;
}

uint32_t Manifest::osVersionNumber() const
{
    const std::regex verRegex("v([0-9]+)\\.([0-9]+).*");
    std::smatch match;
    if (std::regex_match(osVersion(), match, verRegex))
    {
        const uint16_t major = std::stoul(match[1].str());
        const uint16_t minor = std::stoul(match[2].str());
        return major << 16 | minor;
    }
    return 0;
}

const std::string& Manifest::machineName() const
{
    return properties.find(machineNameProp)->second;
}

const std::string& Manifest::hostName() const
{
    return properties.find(hostNameProp)->second;
}
