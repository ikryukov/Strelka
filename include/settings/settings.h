#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

namespace oka
{

class SettingsManager
{
private:
    /* data */
    std::unordered_map<std::string, std::string> mMap;

    void isNameValid(const char* name)
    {
        if (mMap.find(name) == mMap.end())
        {
            std::cerr << "The setting " << name << " does not exist" << std::endl;
            assert(0);
        }
    }

public:
    SettingsManager(/* args */);
    ~SettingsManager();

    template <typename T>
    void setAs(const char* name, const T& value)
    {
        mMap[name] = std::to_string(value);
    }
    template <typename T>
    T getAs(const char* name)
    {
        isNameValid(name);
        return mMap[name];
    }

    template <>
    bool getAs(const char* name)
    {
        isNameValid(name);
        return (bool)atoi(mMap[name].c_str());
    }

    template <>
    float getAs(const char* name)
    {
        isNameValid(name);
        return atof(mMap[name].c_str());
    }

    template <>
    uint32_t getAs(const char* name)
    {
        isNameValid(name);
        return atoi(mMap[name].c_str());
    }
};

} // namespace oka
