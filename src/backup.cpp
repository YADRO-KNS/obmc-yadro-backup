// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "accounts.hpp"
#include "backup.hpp"
#include "manifest.hpp"

#include <vector>

namespace fs = std::filesystem;

// clang-format off
/** @brief List of base configuration files/directories. */
static const std::vector<const char*> baseConfigs = {
    "etc/dropbear/dropbear_rsa_host_key",
    "etc/machine-id",
};

/** @brief List of network configuration files/directories. */
static const std::vector<const char*> networkConfigs = {
    "etc/hostname",
    "etc/systemd/network",
    "var/lib/first-boot-set-hostname",
};
// clang-format on

Backup::Backup(const Configuration& cfg) : config(cfg)
{
    std::string tmpPathTemplate = fs::temp_directory_path() / "backup_XXXXXX";
    tmpDir = mkdtemp(tmpPathTemplate.data());
}

Backup::~Backup()
{
    fs::remove_all(tmpDir);
}

void Backup::backup() const
{
    if (config.handleAccounts)
    {
        Accounts acc(config.rootFs, tmpDir, config.readOnlyFs);
        acc.backup();
    }

    for (const auto& it : baseConfigs)
    {
        backupFile(it);
    }

    if (config.handleNetwork)
    {
        for (const auto& it : networkConfigs)
        {
            backupFile(it);
        }
    }

    const Manifest manifest = Manifest::fromOsRelease(config.rootFs);
    manifest.save(tmpDir);

    callTar(false);
}

void Backup::restore() const
{
    callTar(true);

    const Manifest osRelease = Manifest::fromOsRelease(config.rootFs);
    const Manifest manifest = Manifest::load(tmpDir);
    if (osRelease.osVersion != manifest.osVersion)
    {
        std::string err = "OS version mismatch: current is ";
        err += osRelease.osVersion;
        err += ", but backup was created for ";
        err += manifest.osVersion;
        throw std::runtime_error(err);
    }

    if (config.handleAccounts)
    {
        Accounts acc(tmpDir, config.rootFs, config.readOnlyFs);
        acc.restore();
    }

    for (const auto& it : baseConfigs)
    {
        restoreFile(it);
    }

    if (config.handleNetwork)
    {
        for (const auto& it : networkConfigs)
        {
            restoreFile(it);
        }
    }
}

void Backup::backupFile(const char* path) const
{
    const fs::path src = config.rootFs / path;
    if (!fs::exists(src))
    {
        return;
    }

    const fs::path dst = tmpDir / path;
    fs::create_directories(dst.parent_path());
    fs::copy(src, dst,
             fs::copy_options::recursive | fs::copy_options::copy_symlinks);
}

void Backup::restoreFile(const char* path) const
{
    const fs::path src = tmpDir / path;
    if (!fs::exists(src))
    {
        return;
    }

    const fs::path dst = config.rootFs / path;

    // restore permissions
    if (fs::exists(dst))
    {
        const fs::perms p = fs::status(dst).permissions();
        fs::permissions(src, p, fs::perm_options::replace);
    }

    // copy file
    fs::create_directories(dst.parent_path());
    fs::copy(src, dst,
             fs::copy_options::overwrite_existing |
                 fs::copy_options::recursive | fs::copy_options::copy_symlinks);
}

void Backup::callTar(bool extract) const
{
    std::string cmd = "tar ";
    cmd += extract ? "xf" : "czf";
    cmd += " '";
    cmd += config.backupFile;
    cmd += "' -C '";
    cmd += tmpDir;
    cmd += '\'';
    if (!extract)
    {
        cmd += " .";
    }

    const int rc = system(cmd.c_str());
    if (rc)
    {
        std::string err = "Tar error: ";
        err += config.backupFile;
        throw std::runtime_error(err);
    }
}