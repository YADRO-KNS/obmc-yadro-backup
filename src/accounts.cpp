// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "account_entry.hpp"
#include "account_list.hpp"
#include "accounts.hpp"

#include <set>
#include <string>

namespace fs = std::filesystem;

/** @brief Directory name for accounts files. */
static const char* accountsDir = "etc";
/** @brief Name of the group file. */
static const char* groupFile = "group";
/** @brief Name of the passwd file. */
static const char* passwdFile = "passwd";
/** @brief Name of the shadow file. */
static const char* shadowFile = "shadow";

/** @brief Minimal value for UID. Most of the Linux systems (and OpenBMC too)
 *         have defined it to 1000, see UID_MIN from /etc/login.defs.
 */
static constexpr uint16_t minUserId = 1000;

// clang-format off
/** @brief List of users, that are created by OpenBMC but can be changed by an
 *         end user.
 */
static const std::set<const char*> allowedUsers = {
    "admin"
};
/** @brief List of groups to which an end user may add or remove members. */
static const std::set<const char*> allowedGroups = {
    "priv-admin",
    "priv-operator",
    "priv-user",
    "ipmi",
    "redfish",
    "web",
};
// clang-format on

using Groups = AccountList<GroupEntry>;
using Passwd = AccountList<PasswdEntry>;
using Shadow = AccountList<ShadowEntry>;

Accounts::Accounts(const fs::path& srcRoot, const fs::path& dstRoot,
                   const fs::path& roRoot) :
    srcDir(srcRoot / accountsDir),
    dstDir(dstRoot / accountsDir), roDir(roRoot / accountsDir)
{
    fs::create_directories(dstDir);
}

void Accounts::backup()
{
    backupGroup();
    backupPasswd();
    backupShadow();
}

void Accounts::restore()
{
    restoreGroup();
    restorePasswd();
    restoreShadow();
}

void Accounts::backupGroup()
{
    Groups bk;
    bk.load(srcDir / groupFile);

    // Remove groups that are not in the white list
    bk.remove(allowedGroups, false);

    bk.save(dstDir / groupFile);
}

void Accounts::backupPasswd()
{
    Passwd bk;
    bk.load(srcDir / passwdFile);

    Passwd ro;
    ro.load(roDir / passwdFile);
    ro.remove(allowedUsers, true); // Exception for modifiable user accounts

    // Remove build-in accounts
    bk.remove(ro, true);

    bk.save(dstDir / passwdFile);
}

void Accounts::backupShadow()
{
    Shadow bk;
    bk.load(srcDir / shadowFile);

    Shadow ro;
    ro.load(roDir / shadowFile);
    ro.remove(allowedUsers, true); // Exception for modifiable user accounts

    // Remove build-in accounts
    bk.remove(ro, true);

    bk.save(dstDir / shadowFile);
}

void Accounts::restoreGroup()
{
    Groups bk;
    bk.load(srcDir / groupFile);

    Groups rst;
    rst.load(roDir / groupFile);

    for (const auto& it : allowedGroups)
    {
        GroupEntry* r = rst.get(it);
        const GroupEntry* b = bk.get(it);
        if (r && b)
        {
            // Copy membership information from backup
            r->setMembers(b->getMembers());
        }
    }

    const fs::path outFile = dstDir / groupFile;
    rst.save(outFile);

    fs::permissions(outFile,
                    fs::perms::owner_read | fs::perms::owner_write |
                        fs::perms::group_read | fs::perms::others_read,
                    fs::perm_options::replace);
}

void Accounts::restorePasswd()
{
    // Create a set with valid GIDs (groups which can be primary for a user)
    std::set<uint16_t> validGids;
    Groups groups;
    groups.load(roDir / groupFile);
    for (const auto& name : allowedGroups)
    {
        const GroupEntry* grp = groups.get(name);
        if (grp)
        {
            validGids.insert(grp->gid());
        }
    }

    Passwd bk;
    bk.load(srcDir / passwdFile);

    Passwd rst;
    rst.load(roDir / passwdFile);
    rst.remove(allowedUsers, true); // Exception for modifiable user accounts

    for (const auto& user : bk)
    {
        const std::string& name = user.name();
        if (rst.get(name))
        {
            continue; // skip build-in accounts
        }
        if (user.uid() < minUserId)
        {
            std::string err = "Ignore user account (bad UID): ";
            err += name;
            throw std::runtime_error(err);
        }
        if (validGids.find(user.gid()) == validGids.end())
        {
            std::string err = "Ignore user account (bad GID): ";
            err += name;
            throw std::runtime_error(err);
        }

        rst.push_back(user);
    }

    const fs::path outFile = dstDir / passwdFile;
    rst.save(outFile);

    fs::permissions(outFile,
                    fs::perms::owner_read | fs::perms::owner_write |
                        fs::perms::group_read | fs::perms::others_read,
                    fs::perm_options::replace);
}

void Accounts::restoreShadow()
{
    Shadow bk;
    bk.load(srcDir / shadowFile);

    Shadow rst;
    rst.load(roDir / shadowFile);
    rst.remove(allowedUsers, true); // Exception for modifiable user accounts

    for (const auto& user : bk)
    {
        const std::string& name = user.name();
        if (rst.get(name))
        {
            continue; // skip build-in accounts
        }
        rst.push_back(user);
    }

    const fs::path outFile = dstDir / shadowFile;
    rst.save(outFile);

    fs::permissions(outFile, fs::perms::owner_read | fs::perms::owner_write,
                    fs::perm_options::replace);
}
