#ifdef _WIN32

#include "ui/WindowsJumpListManager.h"
#include "core/Logger.h"

#include <Windows.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <shlobj.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "propsys.lib")

namespace fresh
{

WindowsJumpListManager::WindowsJumpListManager()
    : m_initialized(false),
      m_customDestinationList(nullptr),
      m_comInitialized(false)
{
}

WindowsJumpListManager::~WindowsJumpListManager()
{
    cleanupCOM();
}

bool WindowsJumpListManager::initialize(const std::string& appUserModelId)
{
    if (m_initialized) {
        LOG_WARNING_C("Jump List Manager already initialized", "WindowsJumpListManager");
        return true;
    }

    // Initialize COM
    if (!initializeCOM()) {
        LOG_ERROR_C("Failed to initialize COM for Jump List Manager", "WindowsJumpListManager");
        return false;
    }

    // Set or generate App User Model ID
    if (!appUserModelId.empty()) {
        m_appUserModelId = appUserModelId;
    } else {
        // Generate default App User Model ID
        m_appUserModelId = "FreshVoxelEngine.VoxelEditor.1";
    }

    // Set the App User Model ID for this process
    HRESULT hr = SetCurrentProcessExplicitAppUserModelID(
        std::wstring(m_appUserModelId.begin(), m_appUserModelId.end()).c_str()
    );

    if (FAILED(hr)) {
        LOG_WARNING_C("Failed to set App User Model ID: " + std::to_string(hr), 
                     "WindowsJumpListManager");
    }

    m_initialized = true;
    LOG_INFO_C("Jump List Manager initialized with ID: " + m_appUserModelId, 
              "WindowsJumpListManager");
    
    // Add default tasks
    addTask("Create New Project", "--new-world", "Create a new voxel project");
    addTask("Open Recent", "--open-recent", "Open recently used world");
    updateJumpList();

    return true;
}

bool WindowsJumpListManager::addRecentWorld(const std::string& title, 
                                           const std::string& path,
                                           const std::string& description)
{
    if (!m_initialized) {
        LOG_ERROR_C("Jump List Manager not initialized", "WindowsJumpListManager");
        return false;
    }

    // Check if this world is already in the list
    for (auto it = m_recentWorlds.begin(); it != m_recentWorlds.end(); ++it) {
        if (it->path == path) {
            // Move to front
            RecentFile file = *it;
            m_recentWorlds.erase(it);
            m_recentWorlds.insert(m_recentWorlds.begin(), file);
            return updateJumpList();
        }
    }

    // Add new recent world
    RecentFile file;
    file.title = title;
    file.path = path;
    file.description = description;
    
    m_recentWorlds.insert(m_recentWorlds.begin(), file);
    
    // Limit to 10 most recent
    if (m_recentWorlds.size() > 10) {
        m_recentWorlds.pop_back();
    }

    LOG_INFO_C("Added recent world to jump list: " + title, "WindowsJumpListManager");
    
    return updateJumpList();
}

bool WindowsJumpListManager::clearRecentWorlds()
{
    if (!m_initialized) {
        return false;
    }

    m_recentWorlds.clear();
    LOG_INFO_C("Cleared recent worlds from jump list", "WindowsJumpListManager");
    
    return updateJumpList();
}

bool WindowsJumpListManager::addTask(const std::string& title,
                                    const std::string& arguments,
                                    const std::string& description)
{
    if (!m_initialized) {
        LOG_ERROR_C("Jump List Manager not initialized", "WindowsJumpListManager");
        return false;
    }

    Task task;
    task.title = title;
    task.arguments = arguments;
    task.description = description;
    
    m_tasks.push_back(task);
    
    LOG_INFO_C("Added task to jump list: " + title, "WindowsJumpListManager");
    
    return true;
}

bool WindowsJumpListManager::clearTasks()
{
    if (!m_initialized) {
        return false;
    }

    m_tasks.clear();
    LOG_INFO_C("Cleared tasks from jump list", "WindowsJumpListManager");
    
    return updateJumpList();
}

bool WindowsJumpListManager::updateJumpList()
{
    if (!m_initialized) {
        return false;
    }

    // Create Custom Destination List
    ICustomDestinationList* pDestList = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_DestinationList, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pDestList));

    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to create destination list: " + std::to_string(hr), 
                   "WindowsJumpListManager");
        return false;
    }

    // Begin list
    UINT cMinSlots;
    IObjectArray* pRemovedItems = nullptr;
    hr = pDestList->BeginList(&cMinSlots, IID_PPV_ARGS(&pRemovedItems));
    
    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to begin jump list: " + std::to_string(hr), 
                   "WindowsJumpListManager");
        pDestList->Release();
        return false;
    }

    if (pRemovedItems) {
        pRemovedItems->Release();
    }

    // Add tasks category
    if (!m_tasks.empty()) {
        IObjectCollection* pTaskCollection = nullptr;
        hr = CoCreateInstance(CLSID_EnumerableObjectCollection, nullptr, 
                             CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pTaskCollection));

        if (SUCCEEDED(hr)) {
            // Get executable path
            wchar_t exePath[MAX_PATH];
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);

            for (const auto& task : m_tasks) {
                IShellLinkW* pLink = nullptr;
                hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                                     IID_PPV_ARGS(&pLink));

                if (SUCCEEDED(hr)) {
                    pLink->SetPath(exePath);
                    pLink->SetArguments(
                        std::wstring(task.arguments.begin(), task.arguments.end()).c_str()
                    );
                    
                    if (!task.description.empty()) {
                        pLink->SetDescription(
                            std::wstring(task.description.begin(), task.description.end()).c_str()
                        );
                    }

                    IPropertyStore* pPropStore = nullptr;
                    hr = pLink->QueryInterface(IID_PPV_ARGS(&pPropStore));
                    if (SUCCEEDED(hr)) {
                        PROPVARIANT pv;
                        InitPropVariantFromString(
                            std::wstring(task.title.begin(), task.title.end()).c_str(), 
                            &pv
                        );
                        pPropStore->SetValue(PKEY_Title, pv);
                        pPropStore->Commit();
                        PropVariantClear(&pv);
                        pPropStore->Release();
                    }

                    pTaskCollection->AddObject(pLink);
                    pLink->Release();
                }
            }

            IObjectArray* pTaskArray = nullptr;
            hr = pTaskCollection->QueryInterface(IID_PPV_ARGS(&pTaskArray));
            if (SUCCEEDED(hr)) {
                pDestList->AddUserTasks(pTaskArray);
                pTaskArray->Release();
            }

            pTaskCollection->Release();
        }
    }

    // Add recent worlds category
    if (!m_recentWorlds.empty()) {
        IObjectCollection* pRecentCollection = nullptr;
        hr = CoCreateInstance(CLSID_EnumerableObjectCollection, nullptr,
                             CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pRecentCollection));

        if (SUCCEEDED(hr)) {
            wchar_t exePath[MAX_PATH];
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);

            for (const auto& world : m_recentWorlds) {
                IShellLinkW* pLink = nullptr;
                hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                                     IID_PPV_ARGS(&pLink));

                if (SUCCEEDED(hr)) {
                    pLink->SetPath(exePath);
                    std::string args = "--open \"" + world.path + "\"";
                    pLink->SetArguments(
                        std::wstring(args.begin(), args.end()).c_str()
                    );

                    if (!world.description.empty()) {
                        pLink->SetDescription(
                            std::wstring(world.description.begin(), world.description.end()).c_str()
                        );
                    }

                    IPropertyStore* pPropStore = nullptr;
                    hr = pLink->QueryInterface(IID_PPV_ARGS(&pPropStore));
                    if (SUCCEEDED(hr)) {
                        PROPVARIANT pv;
                        InitPropVariantFromString(
                            std::wstring(world.title.begin(), world.title.end()).c_str(),
                            &pv
                        );
                        pPropStore->SetValue(PKEY_Title, pv);
                        pPropStore->Commit();
                        PropVariantClear(&pv);
                        pPropStore->Release();
                    }

                    pRecentCollection->AddObject(pLink);
                    pLink->Release();
                }
            }

            IObjectArray* pRecentArray = nullptr;
            hr = pRecentCollection->QueryInterface(IID_PPV_ARGS(&pRecentArray));
            if (SUCCEEDED(hr)) {
                pDestList->AppendCategory(L"Recent Worlds", pRecentArray);
                pRecentArray->Release();
            }

            pRecentCollection->Release();
        }
    }

    // Commit the list
    hr = pDestList->CommitList();
    pDestList->Release();

    if (FAILED(hr)) {
        LOG_ERROR_C("Failed to commit jump list: " + std::to_string(hr), 
                   "WindowsJumpListManager");
        return false;
    }

    LOG_INFO_C("Jump list updated successfully", "WindowsJumpListManager");
    return true;
}

bool WindowsJumpListManager::isSupported()
{
    // Jump Lists require Windows 7 or later
    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = 6;
    osvi.dwMinorVersion = 1;

    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) != 0;
}

bool WindowsJumpListManager::initializeCOM()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    
    if (SUCCEEDED(hr)) {
        m_comInitialized = true;
        return true;
    } else if (hr == RPC_E_CHANGED_MODE) {
        // COM already initialized in a different mode
        LOG_WARNING_C("COM already initialized in different mode", "WindowsJumpListManager");
        m_comInitialized = false;
        return true;  // Continue anyway
    }
    
    LOG_ERROR_C("Failed to initialize COM: " + std::to_string(hr), 
               "WindowsJumpListManager");
    return false;
}

void WindowsJumpListManager::cleanupCOM()
{
    if (m_comInitialized) {
        CoUninitialize();
        m_comInitialized = false;
    }
}

} // namespace fresh

#endif // _WIN32
