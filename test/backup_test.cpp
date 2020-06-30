// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "backup.hpp"
#include "manifest.hpp"

#include <fstream>
#include <set>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

/**
 * @class BackupTest
 * @brief Tests for full backup/restore.
 */
class BackupTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        const char* testDataDir = std::getenv("TEST_DATA_DIR");
        if (!testDataDir)
        {
            testDataDir = TEST_DATA_DIR;
        }

        dataDir = testDataDir;
        dataDir /= "full";
        if (!fs::is_directory(dataDir))
        {
            fprintf(stderr, "Invalid path to data files: %s\n", testDataDir);
            FAIL();
        }

        rwRoot = dataDir / "rw";
        roRoot = dataDir / "ro";

        fs::remove_all(tmpDir);
        fs::create_directories(tmpDir);
    }

    void TearDown() override
    {
        fs::remove_all(tmpDir);
    }

    std::set<std::string> fileList(const fs::path& archive)
    {
        std::set<std::string> files;

        std::string cmd = "tar tf ";
        cmd += archive;

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }
        char buf[64];
        while (fgets(buf, sizeof(buf), pipe))
        {
            const size_t len = strlen(buf);
            files.insert(std::string(buf + 1, buf + len - 1));
        }
        pclose(pipe);
        return files;
    }

    const fs::path tmpDir = fs::temp_directory_path() / "full_backup_test";
    fs::path dataDir;
    fs::path rwRoot; // root FS with new users
    fs::path roRoot; // RO partition (/run/initramfs/ro)
};

TEST_F(BackupTest, BackupFull)
{
    const fs::path arc = tmpDir / "backup.tar.gz";

    Configuration conf;
    conf.unattendedMode = true;
    conf.backupFile = arc;
    conf.rootFs = rwRoot;
    conf.readOnlyFs = roRoot;

    Backup bk(conf);
    bk.backup();

    const std::set<std::string> real = fileList(arc);
    const std::set<std::string> expect = {
        "/",
        "/bmc.manifest",
        "/var/",
        "/var/lib/",
        "/var/lib/first-boot-set-hostname",
        "/etc/",
        "/etc/systemd/",
        "/etc/systemd/network/",
        "/etc/systemd/network/00-bmc-eth0.network",
        "/etc/hostname",
        "/etc/machine-id",
        "/etc/dropbear/",
        "/etc/dropbear/dropbear_rsa_host_key",
        "/etc/shadow",
        "/etc/passwd",
        "/etc/group"};
    EXPECT_EQ(real, expect);
}

TEST_F(BackupTest, BackupNoAcc)
{
    const fs::path arc = tmpDir / "backup.tar.gz";

    Configuration conf;
    conf.unattendedMode = true;
    conf.backupFile = arc;
    conf.handleAccounts = false;
    conf.rootFs = rwRoot;
    conf.readOnlyFs = roRoot;

    Backup bk(conf);
    bk.backup();

    const std::set<std::string> real = fileList(arc);
    const std::set<std::string> expect = {
        "/",
        "/bmc.manifest",
        "/var/",
        "/var/lib/",
        "/var/lib/first-boot-set-hostname",
        "/etc/",
        "/etc/systemd/",
        "/etc/systemd/network/",
        "/etc/systemd/network/00-bmc-eth0.network",
        "/etc/hostname",
        "/etc/machine-id",
        "/etc/dropbear/",
        "/etc/dropbear/dropbear_rsa_host_key",
    };
    EXPECT_EQ(real, expect);
}

TEST_F(BackupTest, BackupNoAccNoNet)
{
    const fs::path arc = tmpDir / "backup.tar.gz";

    Configuration conf;
    conf.unattendedMode = true;
    conf.backupFile = arc;
    conf.handleAccounts = false;
    conf.handleNetwork = false;
    conf.rootFs = rwRoot;
    conf.readOnlyFs = roRoot;

    Backup bk(conf);
    bk.backup();

    const std::set<std::string> real = fileList(arc);
    const std::set<std::string> expect = {
        "/",
        "/bmc.manifest",
        "/etc/",
        "/etc/machine-id",
        "/etc/dropbear/",
        "/etc/dropbear/dropbear_rsa_host_key",
    };
    EXPECT_EQ(real, expect);
}

TEST_F(BackupTest, Restore)
{
    const fs::path arc = tmpDir / "backup.tar.gz";

    Configuration conf;
    conf.unattendedMode = true;
    conf.backupFile = arc;
    conf.rootFs = rwRoot;
    conf.readOnlyFs = roRoot;

    Backup bk(conf);
    bk.backup();

    conf.rootFs = tmpDir;
    fs::create_directory(tmpDir / "etc");
    fs::create_symlink(rwRoot / "etc/os-release", tmpDir / "etc/os-release");
    bk.restore();

    fs::remove(arc);
    for (const auto& p : fs::recursive_directory_iterator(tmpDir))
    {
        const fs::path expect = rwRoot / fs::relative(p.path(), tmpDir);
        if (expect.filename() != "os-release" && !fs::exists(expect))
        {
            fprintf(stderr, "File %s not found\n", expect.c_str());
            FAIL();
        }
    }
}
