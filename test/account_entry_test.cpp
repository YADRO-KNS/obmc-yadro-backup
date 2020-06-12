// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "account_entry.hpp"

#include <gtest/gtest.h>

TEST(AccountEntryTest, Invalid)
{
    ASSERT_THROW(AccountEntry<2>(":d"), std::runtime_error);
    ASSERT_THROW(AccountEntry<2>("a:b:c:d"), std::runtime_error);
    ASSERT_THROW(AccountEntry<10>("a:b:c:d"), std::runtime_error);
    ASSERT_THROW(AccountEntry<2>("65536:").getNumber(0), std::out_of_range);
    ASSERT_THROW(AccountEntry<2>("nan:").getNumber(0), std::invalid_argument);
    ASSERT_THROW(AccountEntry<2>("a:").getNumber(1), std::invalid_argument);
    ASSERT_THROW(AccountEntry<2>("a:b").get(10), std::out_of_range);
    ASSERT_THROW(AccountEntry<2>("a:b").set(10, "?"), std::out_of_range);
}

TEST(AccountEntryTest, Empty)
{
    auto cfg = AccountEntry<4>("name:::");
    EXPECT_EQ(cfg.get(0), "name");
    EXPECT_EQ(cfg.get(1), "");
    EXPECT_EQ(cfg.get(2), "");
    EXPECT_EQ(cfg.get(3), "");
}

TEST(AccountEntryTest, Load)
{
    auto cfg = AccountEntry<4>("name::123:x1,y2,z3");
    EXPECT_EQ(cfg, "name");
    EXPECT_EQ(cfg.name(), "name");
    EXPECT_EQ(cfg.get(0), "name");
    EXPECT_EQ(cfg.get(1), "");
    EXPECT_EQ(cfg.getNumber(2), 123);
    EXPECT_EQ(cfg.get(3), "x1,y2,z3");
}

TEST(AccountEntryTest, Modify)
{
    auto cfg = AccountEntry<4>("name::123:x1,y2,z3");
    cfg.set(0, "name2");
    EXPECT_EQ(cfg.get(0), "name2");
}

TEST(AccountEntryTest, Save)
{
    const char* data = "name::123:x1,y2,z3";
    auto cfg = AccountEntry<4>(data);
    EXPECT_EQ(cfg.toString(), data);
}

TEST(AccountEntryTest, Group)
{
    auto cfg = GroupEntry("name:x:123:x1,y2,z3");
    EXPECT_EQ(cfg.name(), "name");
    EXPECT_EQ(cfg.gid(), 123);
    EXPECT_EQ(cfg.getMembers(), "x1,y2,z3");
    cfg.setMembers("a3,b2,c1");
    EXPECT_EQ(cfg.getMembers(), "a3,b2,c1");
}

TEST(AccountEntryTest, Passwd)
{
    auto cfg = PasswdEntry("name:x:1001:1002::/home/name:/bin/bash");
    EXPECT_EQ(cfg.name(), "name");
    EXPECT_EQ(cfg.uid(), 1001);
    EXPECT_EQ(cfg.gid(), 1002);
}

TEST(AccountEntryTest, Shadow)
{
    auto cfg = ShadowEntry("news:*:18422:0:99999:7:::");
    EXPECT_EQ(cfg.name(), "news");
}
