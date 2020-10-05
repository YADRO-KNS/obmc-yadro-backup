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
    "etc/ipmi_pass",
    "etc/machine-id",
};

/** @brief List of network configuration files/directories. */
static const std::vector<const char*> networkConfigs = {
    "etc/hostname",
    "etc/systemd/network",
    "var/lib/first-boot-set-hostname",
};
// clang-format on

Backup::~Backup()
{
    if (!tmpDir.empty())
    {
        fs::remove_all(tmpDir);
    }
}

void Backup::backup()
{
    if (fs::exists(archiveFile))
    {
        std::string err = "Backup file already exists: ";
        err += archiveFile;
        throw std::runtime_error(err);
    }

    tmpDir = createTempDir();

    if (handleAccounts)
    {
        Accounts acc(rootFs, tmpDir, readOnlyFs);
        acc.backup();
    }

    for (const auto& it : baseConfigs)
    {
        backupFile(it);
    }

    if (handleNetwork)
    {
        for (const auto& it : networkConfigs)
        {
            backupFile(it);
        }
    }

    const Manifest manifest(rootFs);
    manifest.save(tmpDir);

    callTar(false);
}

void Backup::restore()
{
    if (!fs::exists(archiveFile))
    {
        std::string err = "File not found: ";
        err += archiveFile;
        throw std::runtime_error(err);
    }

    tmpDir = createTempDir();

    callTar(true);

    checkManifest();

    if (handleAccounts)
    {
        Accounts acc(tmpDir, rootFs, readOnlyFs);
        acc.restore();
    }

    for (const auto& it : baseConfigs)
    {
        restoreFile(it);
    }

    if (handleNetwork)
    {
        for (const auto& it : networkConfigs)
        {
            restoreFile(it);
        }
    }
}

fs::path Backup::createTempDir() const
{
    std::string pathTemplate = fs::temp_directory_path() / "backup_XXXXXX";
    const char* path = mkdtemp(pathTemplate.data());
    if (!path)
    {
        throw std::system_error(errno, std::system_category(), pathTemplate);
    }
    return path;
}

void Backup::checkManifest() const
{
    const Manifest mnfBackup = Manifest::load(tmpDir);
    const Manifest mnfCurrent = Manifest(rootFs);

    if (mnfBackup.machineName() != mnfCurrent.machineName())
    {
        std::string err = "Target machine type mismatch: current is ";
        err += mnfCurrent.machineName();
        err += ", but backup was created for ";
        err += mnfBackup.machineName();
        throw std::runtime_error(err);
    }

    printf("Restore from backup file %s\n", archiveFile.c_str());
    mnfBackup.print();

    // Check versions
    const uint32_t bkpVer = mnfBackup.osVersionNumber();
    const uint32_t curVer = mnfCurrent.osVersionNumber();
    if (bkpVer > curVer)
    {
        fprintf(stderr, "ERROR: Backup was created for newer BMC version %s.\n",
                mnfBackup.osVersion().c_str());
        fprintf(stderr, "Current BMC version: %s.\n",
                mnfCurrent.osVersion().c_str());
        throw std::runtime_error("Downgrading configuration is not possible");
    }
    if (bkpVer < curVer)
    {
        printf("WARNING! Backup was created for older BMC version %s.\n",
               mnfBackup.osVersion().c_str());
        fprintf(stderr, "Current BMC version: %s.\n",
                mnfCurrent.osVersion().c_str());
        puts(
            "Restoring from this backup may cause the BMC to become unstable!");
    }

    if (!unattendedMode)
    {
        printf("Do you want to continue? [y/N]: ");
        char answer[2]; // first char with \0
        if (!fgets(answer, sizeof(answer), stdin) ||
            (*answer != 'y' && *answer != 'Y'))
        {
            throw std::runtime_error("Aborted by user");
        }
    }
}

void Backup::backupFile(const char* path) const
{
    fs::path src = rootFs / path;
    if (!fs::exists(src))
    {
        // try to get the file from RO
        src = readOnlyFs / path;
        if (!fs::exists(src))
        {
            return;
        }
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

    const fs::path dst = rootFs / path;

    // restore permissions
    fs::path permsFile = dst;
    if (!fs::exists(permsFile))
    {
        // try to get permissions from RO
        permsFile = readOnlyFs / path;
    }
    if (fs::exists(permsFile))
    {
        const fs::perms p = fs::status(permsFile).permissions();
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
    cmd += archiveFile;
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
        err += archiveFile;
        throw std::runtime_error(err);
    }
}
