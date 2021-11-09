#include "materialmanager.h"
#include <dlfcn.h>

namespace nevk
{
bool MaterialManager::initMDL(const char* resourcePath)
{
    if (!loadDso(resourcePath))
    {
        return false;
    }
    if (!loadNeuray())
    {
        return false;
    }
    return m_neuray->start() == 0;
}
uint32_t MaterialManager::loadMaterial(const char* fileName){
}

uint32_t MaterialManager::destroyMaterial(uint32_t matId)
{
}

bool MaterialManager::loadDso(const char* resourcePath)
{
    std::string dsoFilename = std::string(resourcePath) + std::string("/libmdl_sdk" MI_BASE_DLL_FILE_EXT);

#ifdef MI_PLATFORM_WINDOWS
    HMODULE handle = LoadLibraryA(dsoFilename.c_str());
    if (!handle)
    {
      LPTSTR buffer = NULL;
      LPCTSTR message = TEXT("unknown failure");
      DWORD error_code = GetLastError();
      if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS, 0, error_code,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0, 0))
      {
        message = buffer;
      }
      fprintf(stderr, "Failed to load library (%u): %s", error_code, message);
      if (buffer)
      {
        LocalFree(buffer);
      }
      return false;
    }
#else
    void* handle = dlopen(dsoFilename.c_str(), RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, "%s\n", dlerror());
        return false;
    }
#endif

    m_dsoHandle = handle;
    return true;
}

bool MaterialManager::loadNeuray()
{
#ifdef MI_PLATFORM_WINDOWS
    void* symbol = GetProcAddress(reinterpret_cast<HMODULE>(m_dsoHandle), "mi_factory");
    if (!symbol)
    {
      LPTSTR buffer = NULL;
      LPCTSTR message = TEXT("unknown failure");
      DWORD error_code = GetLastError();
      if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS, 0, error_code,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0, 0))
      {
        message = buffer;
      }
      fprintf(stderr, "GetProcAddress error (%u): %s", error_code, message);
      if (buffer)
      {
        LocalFree(buffer);
      }
      return false;
    }
#else
    void* symbol = dlsym(m_dsoHandle, "mi_factory");
    if (!symbol)
    {
        fprintf(stderr, "%s\n", dlerror());
        return false;
    }
#endif

    m_neuray = mi::base::Handle<mi::neuraylib::INeuray>(mi::neuraylib::mi_factory<mi::neuraylib::INeuray>(symbol));
    if (m_neuray.is_valid_interface())
    {
        return true;
    }

    mi::base::Handle<mi::neuraylib::IVersion> version(mi::neuraylib::mi_factory<mi::neuraylib::IVersion>(symbol));
    if (!version)
    {
        fprintf(stderr, "Error: Incompatible library.\n");
    }
    else
    {
        fprintf(stderr, "Error: Library version %s does not match header version %s.\n", version->get_product_version(), MI_NEURAYLIB_PRODUCT_VERSION_STRING);
    }

    return false;
}

void MaterialManager::unloadDso()
{
#ifdef MI_PLATFORM_WINDOWS
    if (FreeLibrary(reinterpret_cast<HMODULE>(m_dsoHandle)))
    {
      return;
    }
    LPTSTR buffer = 0;
    LPCTSTR message = TEXT("unknown failure");
    DWORD error_code = GetLastError();
    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS, 0, error_code,
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &buffer, 0, 0))
    {
        message = buffer;
    }
    fprintf(stderr, "Failed to unload library (%u): %s", error_code, message);
    if (buffer)
    {
      LocalFree(buffer);
    }
#else
    if (dlclose(m_dsoHandle) != 0)
    {
        printf( "%s\n", dlerror());
    }
#endif
}

}
