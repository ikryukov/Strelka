#pragma once

#include <mi/base/handle.h>
#include <mi/neuraylib/ineuray.h>

namespace nevk
{
class MdlNeurayLoader
{
public:
    MdlNeurayLoader();
    ~MdlNeurayLoader();

public:
    bool init(const char* resourcePath, const char* imagePluginPath);

    mi::base::Handle<mi::neuraylib::INeuray> getNeuray() const;

private:
    bool loadDso(const char* resourcePath);
    bool loadNeuray();
    bool loadPlugin(const char* imagePluginPath);
    void unloadDso();

private:
    void* mDsoHandle;
    mi::base::Handle<mi::neuraylib::INeuray> mNeuray;
};
}
