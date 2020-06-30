// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @class AccountList
 * @brief List of accounts entries.
 */
template <class T>
class AccountList : public std::vector<T>
{
  public:
    using std::vector<T>::vector;

    /**
     * @brief Load list from file.
     *
     * @param[in] path path to the file to load
     *
     * @throw std::exception in case of error
     */
    void load(const std::string& path)
    {
        try
        {
            std::ifstream file(path);
            if (!file)
            {
                throw std::system_error(errno, std::system_category(), path);
            }
            std::string line;
            while (std::getline(file, line))
            {
                this->emplace_back(T(line));
            }
        }
        catch (const std::system_error&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            std::string err = "Failed to read file ";
            err += path;
            err += ": ";
            err += ex.what();
            throw std::runtime_error(err);
        }
    }

    /**
     * @brief Save list to file.
     *
     * @param[in] path path to the file to write
     *
     * @throw std::exception in case of error
     */
    void save(const std::string& path) const
    {
        std::ofstream file(path);
        if (!file)
        {
            throw std::system_error(errno, std::system_category(), path);
        }
        for (const auto& entry : *this)
        {
            file << entry.toString() << std::endl;
        }
    }

    /**
     * @brief Get entry by name.
     *
     * @param[in] name entry name
     *
     * @return pointer to the entry or nullptr if not found
     */
    T* get(const std::string& name)
    {
        auto it = std::find(this->begin(), this->end(), name);
        return it == this->end() ? nullptr : &*it;
    }

    /**
     * @brief Remove difference or intersection.
     *
     * @param[in] filter array with entry names
     * @param[in] ifExists operation type:
     *                     true: all entries that exists in filter must be
     *                           removed, This[a,b,c] - Filter[a] = This[b,c]
     *                     false: all entries that not exists in filter must
     *                           be removed, This[a,b,c] - Filter[a] = This[a]
     */
    template <class C>
    void remove(const C& filter, bool ifExists)
    {
        auto pred = [&filter, ifExists](const auto& entry) {
            const bool doesExist = std::find(filter.begin(), filter.end(),
                                             entry.name()) != filter.end();
            return doesExist == ifExists;
        };
        this->erase(std::remove_if(this->begin(), this->end(), pred),
                    this->end());
    }
};
