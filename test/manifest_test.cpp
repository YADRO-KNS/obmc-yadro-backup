// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "manifest.hpp"

#include <fstream>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

/**
 * @class ManifestTest
 * @brief Tests for manifest file.
 */
class ManifestTest : public ::testing::Test
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
        dataDir /= "manifest";
        if (!fs::is_directory(dataDir))
        {
            fprintf(stderr, "Invalid path to data files: %s\n", testDataDir);
            FAIL();
        }

        fs::remove_all(tmpDir);
        fs::create_directories(tmpDir);
    }

    void TearDown() override
    {
        fs::remove_all(tmpDir);
    }

    const fs::path tmpDir = fs::temp_directory_path() / "backup_test";
    fs::path dataDir;
};

TEST_F(ManifestTest, Load)
{
    const Manifest manifest = Manifest::load(dataDir);
    EXPECT_EQ(manifest.osVersion, "v2.9.0-dev");
}

TEST_F(ManifestTest, Save)
{
    const Manifest manifest{"v2.9.0-dev"};
    manifest.save(tmpDir);

    std::ifstream f1(tmpDir / "bmc.manifest", std::ifstream::binary);
    std::ifstream f2(dataDir / "bmc.manifest", std::ifstream::binary);
    const std::string s1((std::istreambuf_iterator<char>(f1)),
                         std::istreambuf_iterator<char>());
    const std::string s2((std::istreambuf_iterator<char>(f2)),
                         std::istreambuf_iterator<char>());
    EXPECT_FALSE(s1.empty());
    EXPECT_FALSE(s2.empty());
    EXPECT_EQ(s1, s2);
}

TEST_F(ManifestTest, FromOsRelease)
{
    const Manifest m = Manifest::fromOsRelease(dataDir);
    EXPECT_EQ(m.osVersion, "v2.9.0-dev");
}
