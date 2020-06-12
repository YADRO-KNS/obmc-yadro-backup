// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "accounts.hpp"

#include <fstream>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

/**
 * @class AccountsTest
 * @brief Tests for backup/restore accounts files.
 */
class AccountsTest : public ::testing::Test
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
        dataDir /= "accounts";
        if (!fs::is_directory(dataDir))
        {
            fprintf(stderr, "Invalid path to data files: %s\n", testDataDir);
            FAIL();
        }

        rwRoot = dataDir / "rw";
        roRoot = dataDir / "ro";

        fs::remove_all(tmpDir);
    }

    void TearDown() override
    {
        fs::remove_all(tmpDir);
    }

    void compareFiles(const fs::path& c1, const fs::path& c2) const
    {
        std::ifstream f1(c1, std::ifstream::binary);
        std::ifstream f2(c2, std::ifstream::binary);
        const std::string s1((std::istreambuf_iterator<char>(f1)),
                             std::istreambuf_iterator<char>());
        const std::string s2((std::istreambuf_iterator<char>(f2)),
                             std::istreambuf_iterator<char>());
        EXPECT_FALSE(s1.empty());
        EXPECT_FALSE(s2.empty());
        EXPECT_EQ(s1, s2);
    }

    void compareConfigs(const fs::path& d1, const fs::path& d2) const
    {
        const fs::path etc1 = d1 / "etc";
        const fs::path etc2 = d2 / "etc";
        compareFiles(etc1 / "group", etc2 / "group");
        compareFiles(etc1 / "passwd", etc2 / "passwd");
        compareFiles(etc1 / "shadow", etc2 / "shadow");
    }

    const fs::path tmpDir = fs::temp_directory_path() / "account_backup_test";
    fs::path dataDir;
    fs::path rwRoot; // root FS with new users
    fs::path roRoot; // RO partition (/run/initramfs/ro)
};

TEST_F(AccountsTest, Backup)
{
    Accounts acc(rwRoot, tmpDir, roRoot);
    acc.backup();
    compareConfigs(tmpDir, dataDir / "backup_good");
}

TEST_F(AccountsTest, RestoreGood)
{
    Accounts acc(dataDir / "backup_good", tmpDir, roRoot);
    acc.restore();
    compareConfigs(tmpDir, rwRoot);

    const fs::perms p = fs::status(tmpDir / "etc/shadow").permissions();
    EXPECT_EQ(p, fs::perms::owner_read | fs::perms::owner_write);
}

TEST_F(AccountsTest, RestoreExceeded)
{
    Accounts acc(dataDir / "backup_exceeded", tmpDir, roRoot);
    acc.restore();
    compareConfigs(tmpDir, rwRoot);
}

TEST_F(AccountsTest, RestoreBadUid)
{
    Accounts acc(dataDir / "backup_baduid", tmpDir, roRoot);
    ASSERT_THROW(acc.restore(), std::runtime_error);
}

TEST_F(AccountsTest, RestoreBadGid)
{
    Accounts acc(dataDir / "backup_badgid", tmpDir, roRoot);
    ASSERT_THROW(acc.restore(), std::runtime_error);
}
