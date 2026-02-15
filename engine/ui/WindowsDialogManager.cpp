#ifdef _WIN32

#include "ui/WindowsDialogManager.h"
#include "core/Logger.h"

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <codecvt>
#include <locale>

namespace fresh
{

WindowsDialogManager::WindowsDialogManager()
    : m_initialized(false),
      m_windowHandle(nullptr),
      m_defaultDirectory("")
{
}

WindowsDialogManager::~WindowsDialogManager()
{
    shutdown();
}

bool WindowsDialogManager::initialize(void* windowHandle)
{
    if (m_initialized) {
        LOG_WARNING_C("WindowsDialogManager already initialized", "WindowsDialogManager");
        return true;
    }

    if (!windowHandle) {
        LOG_ERROR_C("Invalid window handle", "WindowsDialogManager");
        return false;
    }

    m_windowHandle = windowHandle;
    m_initialized = true;

    LOG_INFO_C("Windows Dialog Manager initialized", "WindowsDialogManager");
    return true;
}

void WindowsDialogManager::shutdown()
{
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    m_windowHandle = nullptr;
    LOG_INFO_C("Windows Dialog Manager shutdown", "WindowsDialogManager");
}

std::vector<std::string> WindowsDialogManager::showOpenFileDialog(
    const std::string& title,
    const std::vector<FileFilter>& filters,
    bool multiselect)
{
    if (!m_initialized) {
        LOG_ERROR_C("Dialog manager not initialized", "WindowsDialogManager");
        return {};
    }

    // Allocate buffer for filenames (large buffer for multiselect)
    const DWORD bufferSize = multiselect ? 32768 : MAX_PATH;
    std::vector<wchar_t> fileNameBuffer(bufferSize, 0);

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = static_cast<HWND>(m_windowHandle);
    ofn.lpstrFile = fileNameBuffer.data();
    ofn.nMaxFile = bufferSize;
    ofn.lpstrTitle = toWideString(title).c_str();
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (multiselect) {
        ofn.Flags |= OFN_ALLOWMULTISELECT;
    }

    // Set filters
    std::wstring filterStr = buildFilterString(filters);
    if (!filterStr.empty()) {
        ofn.lpstrFilter = filterStr.c_str();
        ofn.nFilterIndex = 1;
    }

    // Set initial directory
    std::wstring initialDir;
    if (!m_defaultDirectory.empty()) {
        initialDir = toWideString(m_defaultDirectory);
        ofn.lpstrInitialDir = initialDir.c_str();
    }

    std::vector<std::string> selectedFiles;

    if (GetOpenFileNameW(&ofn)) {
        if (multiselect) {
            // Parse multiselect results
            const wchar_t* p = fileNameBuffer.data();
            std::wstring directory(p);
            p += directory.length() + 1;

            if (*p == 0) {
                // Single file selected
                selectedFiles.push_back(toNarrowString(directory));
            } else {
                // Multiple files selected
                while (*p) {
                    std::wstring filename(p);
                    std::wstring fullPath = directory + L"\\" + filename;
                    selectedFiles.push_back(toNarrowString(fullPath));
                    p += filename.length() + 1;
                }
            }
        } else {
            // Single file selection
            selectedFiles.push_back(toNarrowString(fileNameBuffer.data()));
        }

        LOG_INFO_C("File(s) selected: " + std::to_string(selectedFiles.size()), "WindowsDialogManager");
    } else {
        DWORD error = CommDlgExtendedError();
        if (error != 0) {
            LOG_WARNING_C("File dialog error: " + std::to_string(error), "WindowsDialogManager");
        }
    }

    return selectedFiles;
}

std::string WindowsDialogManager::showSaveFileDialog(
    const std::string& title,
    const std::string& defaultFilename,
    const std::vector<FileFilter>& filters)
{
    if (!m_initialized) {
        LOG_ERROR_C("Dialog manager not initialized", "WindowsDialogManager");
        return "";
    }

    std::vector<wchar_t> fileNameBuffer(MAX_PATH, 0);
    
    // Set default filename if provided
    if (!defaultFilename.empty()) {
        std::wstring wFilename = toWideString(defaultFilename);
        wcsncpy_s(fileNameBuffer.data(), MAX_PATH, wFilename.c_str(), _TRUNCATE);
    }

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = static_cast<HWND>(m_windowHandle);
    ofn.lpstrFile = fileNameBuffer.data();
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = toWideString(title).c_str();
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    // Set filters
    std::wstring filterStr = buildFilterString(filters);
    if (!filterStr.empty()) {
        ofn.lpstrFilter = filterStr.c_str();
        ofn.nFilterIndex = 1;
    }

    // Set initial directory
    std::wstring initialDir;
    if (!m_defaultDirectory.empty()) {
        initialDir = toWideString(m_defaultDirectory);
        ofn.lpstrInitialDir = initialDir.c_str();
    }

    if (GetSaveFileNameW(&ofn)) {
        std::string result = toNarrowString(fileNameBuffer.data());
        LOG_INFO_C("File save location selected: " + result, "WindowsDialogManager");
        return result;
    }

    return "";
}

std::string WindowsDialogManager::showFolderBrowserDialog(const std::string& title)
{
    if (!m_initialized) {
        LOG_ERROR_C("Dialog manager not initialized", "WindowsDialogManager");
        return "";
    }

    BROWSEINFOW bi = {};
    bi.hwndOwner = static_cast<HWND>(m_windowHandle);
    bi.lpszTitle = toWideString(title).c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;

    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl != nullptr) {
        wchar_t path[MAX_PATH];
        if (SHGetPathFromIDListW(pidl, path)) {
            CoTaskMemFree(pidl);
            std::string result = toNarrowString(path);
            LOG_INFO_C("Folder selected: " + result, "WindowsDialogManager");
            return result;
        }
        CoTaskMemFree(pidl);
    }

    return "";
}

MessageBoxResult WindowsDialogManager::showMessageBox(
    const std::string& title,
    const std::string& message,
    MessageBoxButtons buttons,
    MessageBoxIcon icon)
{
    if (!m_initialized) {
        LOG_ERROR_C("Dialog manager not initialized", "WindowsDialogManager");
        return MessageBoxResult::Cancel;
    }

    UINT type = 0;

    // Set button type
    switch (buttons) {
    case MessageBoxButtons::OK:
        type |= MB_OK;
        break;
    case MessageBoxButtons::OKCancel:
        type |= MB_OKCANCEL;
        break;
    case MessageBoxButtons::YesNo:
        type |= MB_YESNO;
        break;
    case MessageBoxButtons::YesNoCancel:
        type |= MB_YESNOCANCEL;
        break;
    case MessageBoxButtons::RetryCancel:
        type |= MB_RETRYCANCEL;
        break;
    case MessageBoxButtons::AbortRetryIgnore:
        type |= MB_ABORTRETRYIGNORE;
        break;
    }

    // Set icon type
    switch (icon) {
    case MessageBoxIcon::Information:
        type |= MB_ICONINFORMATION;
        break;
    case MessageBoxIcon::Warning:
        type |= MB_ICONWARNING;
        break;
    case MessageBoxIcon::Error:
        type |= MB_ICONERROR;
        break;
    case MessageBoxIcon::Question:
        type |= MB_ICONQUESTION;
        break;
    case MessageBoxIcon::None:
    default:
        break;
    }

    int result = MessageBoxW(
        static_cast<HWND>(m_windowHandle),
        toWideString(message).c_str(),
        toWideString(title).c_str(),
        type);

    // Convert result to our enum
    switch (result) {
    case IDOK:
        return MessageBoxResult::OK;
    case IDCANCEL:
        return MessageBoxResult::Cancel;
    case IDYES:
        return MessageBoxResult::Yes;
    case IDNO:
        return MessageBoxResult::No;
    case IDRETRY:
        return MessageBoxResult::Retry;
    case IDABORT:
        return MessageBoxResult::Abort;
    case IDIGNORE:
        return MessageBoxResult::Ignore;
    default:
        return MessageBoxResult::Cancel;
    }
}

std::wstring WindowsDialogManager::toWideString(const std::string& str)
{
    if (str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string WindowsDialogManager::toNarrowString(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}

std::wstring WindowsDialogManager::buildFilterString(const std::vector<FileFilter>& filters)
{
    if (filters.empty()) {
        return L"All Files (*.*)\0*.*\0\0";
    }

    std::wstring result;
    for (const auto& filter : filters) {
        result += toWideString(filter.description);
        result += L'\0';
        result += toWideString(filter.pattern);
        result += L'\0';
    }
    result += L'\0'; // Double null terminator

    return result;
}

} // namespace fresh

#endif // _WIN32
