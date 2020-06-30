// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "account_entry.hpp"
#include "account_list.hpp"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

using Entry = AccountEntry<2>;
using List = AccountList<Entry>;

TEST(AccountListTest, GetEntry)
{
    List l{{"a:1"}, {"b:2"}, {"c:3"}};
    EXPECT_TRUE(l.get("a"));
    EXPECT_EQ(l.get("a")->get(1), "1");
    EXPECT_TRUE(l.get("b"));
    EXPECT_TRUE(l.get("c"));
    EXPECT_FALSE(l.get("x"));
}

TEST(AccountListTest, RemoveExists)
{
    List l{{"a:1"}, {"b:2"}, {"c:3"}};
    l.remove(std::vector<std::string>{"a", "b"}, true);
    EXPECT_EQ(l.size(), 1);
    EXPECT_TRUE(l.get("c"));
}

TEST(AccountListTest, RemoveNotExists)
{
    List l{{"a:1"}, {"b:2"}, {"c:3"}};
    l.remove(std::vector<std::string>{"a", "b"}, false);
    EXPECT_EQ(l.size(), 2);
    EXPECT_TRUE(l.get("a"));
    EXPECT_TRUE(l.get("b"));
}

TEST(AccountListTest, Load)
{
    const fs::path inFile = fs::temp_directory_path() / "account_list_test";
    std::ofstream file(inFile);
    file << "a:1\n";
    file << "b:2\n";
    file << "c:3\n";
    file.close();

    List l;

    ASSERT_THROW(l.load("/path/not/found"), std::system_error);

    l.load(inFile);
    EXPECT_EQ(l.size(), 3);
    ASSERT_TRUE(l.get("a"));
    EXPECT_EQ(l.get("a")->get(1), "1");
    EXPECT_TRUE(l.get("b"));
    EXPECT_TRUE(l.get("c"));

    fs::remove(inFile);
}

TEST(AccountListTest, LoadEmpty)
{
    const fs::path inFile = fs::temp_directory_path() / "account_list_test";
    std::ofstream file(inFile);
    file.close();

    List l;
    l.load(inFile);
    EXPECT_TRUE(l.empty());

    fs::remove(inFile);
}

TEST(AccountListTest, Save)
{
    const fs::path outFile = fs::temp_directory_path() / "account_list_test";

    List l{{"a:1"}, {"b:2"}, {"c:3"}};

    ASSERT_THROW(l.save("/path/not/found"), std::system_error);
    ASSERT_THROW(l.save("/access_denied"), std::system_error);

    l.save(outFile);

    std::ifstream file(outFile);
    const std::string data((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    EXPECT_EQ(data, "a:1\nb:2\nc:3\n");

    fs::remove(outFile);
}
