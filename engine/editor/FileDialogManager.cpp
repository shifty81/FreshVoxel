#include "editor/FileDialogManager.h"
#include "core/Logger.h"

#ifdef FRESH_NFD_AVAILABLE
#include <nfd.h>
#endif

namespace fresh {

bool FileDialogManager::isAvailable() {
#ifdef FRESH_NFD_AVAILABLE
    return true;
#else
    return false;
#endif
}

std::string FileDialogManager::openFile(const std::vector<Filter>& filters,
                                       const std::string& defaultPath) {
#ifdef FRESH_NFD_AVAILABLE
    NFD_Init();
    
    nfdchar_t* outPath = nullptr;
    nfdfilteritem_t* nfdFilters = nullptr;
    nfdfiltersize_t filterCount = 0;
    
    // Convert filters to NFD format
    if (!filters.empty()) {
        nfdFilters = new nfdfilteritem_t[filters.size()];
        for (size_t i = 0; i < filters.size(); ++i) {
            nfdFilters[i].name = filters[i].name.c_str();
            nfdFilters[i].spec = filters[i].extensions.c_str();
        }
        filterCount = static_cast<nfdfiltersize_t>(filters.size());
    }
    
    nfdresult_t result = NFD_OpenDialog(&outPath,
                                       nfdFilters,
                                       filterCount,
                                       defaultPath.empty() ? nullptr : defaultPath.c_str());
    
    std::string selectedPath;
    if (result == NFD_OKAY) {
        selectedPath = outPath;
        NFD_FreePath(outPath);
        Logger::getInstance().info("File selected: " + selectedPath, "FileDialogManager");
    } else if (result == NFD_CANCEL) {
        Logger::getInstance().info("File dialog canceled", "FileDialogManager");
    } else {
        Logger::getInstance().error("File dialog error: " + std::string(NFD_GetError()), 
                                   "FileDialogManager");
    }
    
    delete[] nfdFilters;
    NFD_Quit();
    
    return selectedPath;
#else
    Logger::getInstance().warning("File dialogs not available (NFD not compiled in)", 
                                 "FileDialogManager");
    return "";
#endif
}

std::string FileDialogManager::saveFile(const std::vector<Filter>& filters,
                                       const std::string& defaultPath,
                                       const std::string& defaultName) {
#ifdef FRESH_NFD_AVAILABLE
    NFD_Init();
    
    nfdchar_t* outPath = nullptr;
    nfdfilteritem_t* nfdFilters = nullptr;
    nfdfiltersize_t filterCount = 0;
    
    // Convert filters to NFD format
    if (!filters.empty()) {
        nfdFilters = new nfdfilteritem_t[filters.size()];
        for (size_t i = 0; i < filters.size(); ++i) {
            nfdFilters[i].name = filters[i].name.c_str();
            nfdFilters[i].spec = filters[i].extensions.c_str();
        }
        filterCount = static_cast<nfdfiltersize_t>(filters.size());
    }
    
    nfdresult_t result = NFD_SaveDialog(&outPath,
                                       nfdFilters,
                                       filterCount,
                                       defaultPath.empty() ? nullptr : defaultPath.c_str(),
                                       defaultName.empty() ? nullptr : defaultName.c_str());
    
    std::string selectedPath;
    if (result == NFD_OKAY) {
        selectedPath = outPath;
        NFD_FreePath(outPath);
        Logger::getInstance().info("Save path selected: " + selectedPath, "FileDialogManager");
    } else if (result == NFD_CANCEL) {
        Logger::getInstance().info("Save dialog canceled", "FileDialogManager");
    } else {
        Logger::getInstance().error("Save dialog error: " + std::string(NFD_GetError()), 
                                   "FileDialogManager");
    }
    
    delete[] nfdFilters;
    NFD_Quit();
    
    return selectedPath;
#else
    Logger::getInstance().warning("File dialogs not available (NFD not compiled in)", 
                                 "FileDialogManager");
    return "";
#endif
}

std::vector<std::string> FileDialogManager::openMultipleFiles(const std::vector<Filter>& filters,
                                                              const std::string& defaultPath) {
#ifdef FRESH_NFD_AVAILABLE
    NFD_Init();
    
    const nfdpathset_t* outPaths = nullptr;
    nfdfilteritem_t* nfdFilters = nullptr;
    nfdfiltersize_t filterCount = 0;
    
    // Convert filters to NFD format
    if (!filters.empty()) {
        nfdFilters = new nfdfilteritem_t[filters.size()];
        for (size_t i = 0; i < filters.size(); ++i) {
            nfdFilters[i].name = filters[i].name.c_str();
            nfdFilters[i].spec = filters[i].extensions.c_str();
        }
        filterCount = static_cast<nfdfiltersize_t>(filters.size());
    }
    
    nfdresult_t result = NFD_OpenDialogMultiple(&outPaths,
                                                nfdFilters,
                                                filterCount,
                                                defaultPath.empty() ? nullptr : defaultPath.c_str());
    
    std::vector<std::string> selectedPaths;
    if (result == NFD_OKAY) {
        nfdpathsetsize_t numPaths;
        NFD_PathSet_GetCount(outPaths, &numPaths);
        
        for (nfdpathsetsize_t i = 0; i < numPaths; ++i) {
            nfdchar_t* path;
            NFD_PathSet_GetPath(outPaths, i, &path);
            selectedPaths.push_back(path);
            NFD_PathSet_FreePath(path);
        }
        
        NFD_PathSet_Free(outPaths);
        Logger::getInstance().info("Selected " + std::to_string(selectedPaths.size()) + " files",
                                 "FileDialogManager");
    } else if (result == NFD_CANCEL) {
        Logger::getInstance().info("Multiple file dialog canceled", "FileDialogManager");
    } else {
        Logger::getInstance().error("Multiple file dialog error: " + std::string(NFD_GetError()),
                                   "FileDialogManager");
    }
    
    delete[] nfdFilters;
    NFD_Quit();
    
    return selectedPaths;
#else
    Logger::getInstance().warning("File dialogs not available (NFD not compiled in)",
                                 "FileDialogManager");
    return {};
#endif
}

std::string FileDialogManager::pickFolder(const std::string& defaultPath) {
#ifdef FRESH_NFD_AVAILABLE
    NFD_Init();
    
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_PickFolder(&outPath,
                                       defaultPath.empty() ? nullptr : defaultPath.c_str());
    
    std::string selectedPath;
    if (result == NFD_OKAY) {
        selectedPath = outPath;
        NFD_FreePath(outPath);
        Logger::getInstance().info("Folder selected: " + selectedPath, "FileDialogManager");
    } else if (result == NFD_CANCEL) {
        Logger::getInstance().info("Folder dialog canceled", "FileDialogManager");
    } else {
        Logger::getInstance().error("Folder dialog error: " + std::string(NFD_GetError()),
                                   "FileDialogManager");
    }
    
    NFD_Quit();
    
    return selectedPath;
#else
    Logger::getInstance().warning("File dialogs not available (NFD not compiled in)",
                                 "FileDialogManager");
    return "";
#endif
}

} // namespace fresh
