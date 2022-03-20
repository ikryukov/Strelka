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
    void setAs(std::string& name, T& value)
    {
        mMap[name] = std::to_string(value);
    }

    template <typename T>
    T getAs(std::string& name)
    {
        return mMap[name];
    }
};

} // namespace oka
