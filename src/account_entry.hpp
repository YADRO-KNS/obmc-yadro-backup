// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @class AccountEntry
 * @brief Single account entry.
 */
template <size_t N>
class AccountEntry : std::array<std::string, N>
{
  public:
    /** @brief Delimiter between fields in a line. */
    static constexpr char fieldDelimiter = ':';

    /**
     * @brief Constructor.
     *
     * @param[in] line line from a configuration file
     *
     * @throw std::runtime_exception if line has invalid format
     */
    AccountEntry(const std::string& line)
    {
        static_assert(N);

        size_t index = 0;
        size_t begin = 0;
        size_t end = 0;
        while (end != std::string::npos)
        {
            end = line.find(fieldDelimiter, begin);
            this->at(index) = line.substr(begin, end - begin);
            if (++index == N)
            {
                break;
            }
            begin = end + 1;
        }
        if (index != N || end != std::string::npos || this->at(0).empty())
        {
            throw std::runtime_error("Invalid format");
        }
    }

    virtual ~AccountEntry() = default;

    bool operator==(const std::string& entryName) const
    {
        return name() == entryName;
    }

    /**
     * @brief Get entry name (always first field).
     *
     * @return entry name
     */
    const std::string& name() const
    {
        return get(0);
    }

    /**
     * @brief Set field value.
     *
     * @param[in] index field index
     * @param[in] value new value
     */
    void set(size_t index, const std::string& value)
    {
        this->at(index) = value;
    }

    /**
     * @brief Get field value.
     *
     * @param[in] index field index
     *
     * @return field value
     */
    const std::string& get(size_t index) const
    {
        return this->at(index);
    }

    /**
     * @brief Get field value as number.
     *
     * @param[in] index field index
     *
     * @return field value
     */
    uint16_t getNumber(size_t index) const
    {
        const std::string& txt = get(index);
        const unsigned long num = stoul(txt);
        if (num > std::numeric_limits<uint16_t>::max())
        {
            throw std::out_of_range("Invalid numeric value");
        }
        return static_cast<uint16_t>(num);
    }

    /**
     * @brief Serialize fields to string.
     *
     * @return config line
     */
    std::string toString() const
    {
        std::string text;
        for (const auto& entry : *this)
        {
            if (!text.empty())
            {
                text += fieldDelimiter;
            }
            text += entry;
        }
        return text;
    }
};

/**
 * @class GroupEntry
 * @brief Single entry from file /etc/group (4 fields in a row).
 */
class GroupEntry : public AccountEntry<4>
{
  public:
    using AccountEntry::AccountEntry;

    /** @brief Sequence number of a GID field. */
    static constexpr size_t fieldGid = 2;
    /** @brief Sequence number of a member list field. */
    static constexpr size_t fieldMembers = 3;

    /**
     * @brief Get group Id.
     *
     * @return group Id
     */
    uint16_t gid() const
    {
        return getNumber(fieldGid);
    }

    /**
     * @brief Get group member list.
     *
     * @return group member list
     */
    const std::string& getMembers() const
    {
        return get(fieldMembers);
    }

    /**
     * @brief Set group member list.
     *
     * @param[in] members plain list of group members
     */
    void setMembers(const std::string& members)
    {
        set(fieldMembers, members);
    }
};

/**
 * @class PasswdEntry
 * @brief Single entry from file /etc/passwd (7 fields in a row).
 */
class PasswdEntry : public AccountEntry<7>
{
  public:
    using AccountEntry::AccountEntry;

    /** @brief Sequence number of an UID field. */
    static constexpr size_t fieldUid = 2;
    /** @brief Sequence number of a primary GID field. */
    static constexpr size_t fieldGid = 3;

    /**
     * @brief Get user Id.
     *
     * @return user Id
     */
    uint16_t uid() const
    {
        return getNumber(fieldUid);
    }

    /**
     * @brief Get primary group Id.
     *
     * @return primary group Id
     */
    uint16_t gid() const
    {
        return getNumber(fieldGid);
    }
};

/** @brief Single entry from file /etc/shadow (9 fields in a row). */
using ShadowEntry = AccountEntry<9>;
