#pragma once

#include <string>
#include <unordered_map>

namespace oka
{

class SettingsManager
{
private:
    /* data */
    std::unordered_map<std::string, std::string> mMap;

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
        return mMap[name];
    }

    template <>
    uint32_t getAs(const char* name)
    {
        return atoi(mMap[name].c_str());
    }
};

} // namespace oka
