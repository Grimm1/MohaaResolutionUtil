#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <shlobj.h>
#include "resource.h"

// Common screen resolutions
const std::vector<std::pair<int, int>> commonResolutions = {
    {800, 600}, {1024, 768}, {1280, 720}, {1920, 1080}, {2560, 1440}, {3840, 2160}
};

// Configuration file settings
std::wstring configFileName = L"unnamedsoldier.cfg";
std::wstring gameDir, virtualStoreDir;
bool firstRun = true;

// Reads game installation path from registry
bool ReadRegistry(std::wstring& gameDir) {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\2015\\MOHAA", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return false;

    wchar_t buffer[MAX_PATH];
    DWORD size = sizeof(buffer);
    if (RegQueryValueExW(hKey, L"basepath", nullptr, nullptr, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
        gameDir = buffer;
        if (!gameDir.empty() && gameDir.back() != L'\\') gameDir += L'\\';
    }
    RegCloseKey(hKey);
    return !gameDir.empty();
}

// Retrieves VirtualStore path for configuration files
std::wstring GetVirtualStorePath() {
    wchar_t* appData;
    SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appData);
    std::wstring path = appData;
    CoTaskMemFree(appData);
    path += L"\\VirtualStore\\Program Files (x86)\\EA GAMES\\MOHAA\\";
    return path;
}

// Checks if a file exists at the specified path
bool FileExists(const std::wstring& path) {
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

// Retrieves all configuration file paths (base game and expansions)
std::vector<std::wstring> GetConfigPaths() {
    std::vector<std::wstring> paths;
    // Base game paths
    std::wstring baseMain = gameDir + L"main\\configs\\" + configFileName;
    std::wstring vsMain = virtualStoreDir + L"main\\configs\\" + configFileName;
    if (FileExists(baseMain)) paths.push_back(baseMain);
    if (FileExists(vsMain)) paths.push_back(vsMain);
    // Spearhead expansion paths
    std::wstring baseMainta = gameDir + L"mainta\\configs\\" + configFileName;
    std::wstring vsMainta = virtualStoreDir + L"mainta\\configs\\" + configFileName;
    if (FileExists(baseMainta)) paths.push_back(baseMainta);
    if (FileExists(vsMainta)) paths.push_back(vsMainta);
    // Breakthrough expansion paths
    std::wstring baseMaintt = gameDir + L"maintt\\configs\\" + configFileName;
    std::wstring vsMaintt = virtualStoreDir + L"maintt\\configs\\" + configFileName;
    if (FileExists(baseMaintt)) paths.push_back(baseMaintt);
    if (FileExists(vsMaintt)) paths.push_back(vsMaintt);
    return paths;
}

// Retrieves configuration paths for expansions only
std::vector<std::wstring> GetExpansionConfigPaths() {
    std::vector<std::wstring> paths;
    // Spearhead expansion paths
    std::wstring baseMainta = gameDir + L"mainta\\configs\\" + configFileName;
    std::wstring vsMainta = virtualStoreDir + L"mainta\\configs\\" + configFileName;
    if (FileExists(baseMainta)) paths.push_back(baseMainta);
    if (FileExists(vsMainta)) paths.push_back(vsMainta);
    // Breakthrough expansion paths
    std::wstring baseMaintt = gameDir + L"maintt\\configs\\" + configFileName;
    std::wstring vsMaintt = virtualStoreDir + L"maintt\\configs\\" + configFileName;
    if (FileExists(baseMaintt)) paths.push_back(baseMaintt);
    if (FileExists(vsMaintt)) paths.push_back(vsMaintt);
    return paths;
}

// Reads configuration file content into a vector of strings
std::vector<std::string> ReadConfig(const std::wstring& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    }
    return lines;
}

// Writes configuration content to file
void WriteConfig(const std::wstring& path, const std::vector<std::string>& lines) {
    std::ofstream file(path, std::ios::trunc);
    if (file.is_open()) {
        for (const auto& line : lines) {
            file << line << "\n";
        }
        file.close();
    }
}

// Updates or adds a setting in the configuration
void UpdateSetting(std::vector<std::string>& lines, const std::string& key, const std::string& value) {
    std::string newLine = "seta " + key + " \"" + value + "\"";
    bool found = false;
    for (auto& line : lines) {
        if (line.find("seta " + key) == 0) {
            line = newLine;
            found = true;
            break;
        }
    }
    if (!found) {
        lines.push_back(newLine);
    }
}

// Verifies if resolution is in common resolutions list
bool IsCommonResolution(int width, int height) {
    for (const auto& res : commonResolutions) {
        if (res.first == width && res.second == height)
            return true;
    }
    return false;
}

// Applies resolution settings to multiple configuration files
void ApplyResolutionSettings(HWND hwnd, const std::vector<std::wstring>& paths, int width, int height) {
    for (const auto& path : paths) {
        std::vector<std::string> lines = ReadConfig(path);
        UpdateSetting(lines, "r_customwidth", std::to_string(width));
        UpdateSetting(lines, "r_customheight", std::to_string(height));
        UpdateSetting(lines, "r_mode", "-1");
        UpdateSetting(lines, "r_forceClampToEdge", "1");
        WriteConfig(path, lines);
    }
}

// Applies maximum quality settings to multiple configuration files
void ApplyMaxQualitySettings(const std::vector<std::wstring>& paths) {
    for (const auto& path : paths) {
        std::vector<std::string> lines = ReadConfig(path);
        UpdateSetting(lines, "r_textureMode", "GL_LINEAR_MIPMAP_LINEAR");
        UpdateSetting(lines, "r_gamma", "1.000000");
        UpdateSetting(lines, "r_intensity", "1.2");
        UpdateSetting(lines, "r_finish", "1");
        UpdateSetting(lines, "r_ext_compressed_textures", "0");
        UpdateSetting(lines, "r_texturebits", "32");
        UpdateSetting(lines, "r_colorbits", "32");
        UpdateSetting(lines, "r_picmip", "0");
        UpdateSetting(lines, "cg_shadows", "2");
        UpdateSetting(lines, "vss_maxcount", "10");
        UpdateSetting(lines, "cg_effectdetail", "1.0");
        UpdateSetting(lines, "r_lodviewmodelcap", "1.0");
        UpdateSetting(lines, "r_lodcap", "1.0");
        UpdateSetting(lines, "r_lodscale", "1.1");
        UpdateSetting(lines, "r_subdivisions", "3");
        WriteConfig(path, lines);
    }
}

// Dialog procedure for expansions dialog
INT_PTR CALLBACK ExpansionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        SetDlgItemTextW(hwnd, IDC_EXPANSIONS_TEXT, L"Expansion configs found for Spearhead and/or Breakthrough. Apply settings to these as well?");
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_YES_BUTTON) {
            EndDialog(hwnd, IDOK);
        }
        else if (LOWORD(wParam) == IDC_NO_BUTTON) {
            EndDialog(hwnd, IDCANCEL);
        }
        return TRUE;
    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

// Dialog procedure for first-run dialog
INT_PTR CALLBACK FirstRunDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        SetDlgItemTextW(hwnd, IDC_FIRSTRUN_TEXT, L"Have you run Medal of Honor: Allied Assault and its expansions at least once?");
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_YES_BUTTON) {
            std::ofstream config("MohaaResolutionUtil.cfg");
            config << "firstRun=0\n";
            config.close();
            firstRun = false;
            EndDialog(hwnd, IDOK);
        }
        else if (LOWORD(wParam) == IDC_NO_BUTTON) {
            MessageBoxW(hwnd, L"Please run Medal of Honor and its expansions first, then re-run this program.", L"Info", MB_OK | MB_ICONINFORMATION);
            EndDialog(hwnd, IDCANCEL);
        }
        return TRUE;
    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

// Dialog procedure for main dialog
INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HFONT hLinkFont;
    static HCURSOR hHandCursor;

    switch (msg) {
    case WM_INITDIALOG: {
        // Initialize default resolution values
        SetDlgItemTextW(hwnd, IDC_WIDTH_EDIT, L"1920");
        SetDlgItemTextW(hwnd, IDC_HEIGHT_EDIT, L"1080");

        // Create underlined font for hyperlink
        HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
        LOGFONT lf;
        GetObject(hFont, sizeof(LOGFONT), &lf);
        lf.lfUnderline = TRUE;
        hLinkFont = CreateFontIndirect(&lf);

        // Set hyperlink font
        SendDlgItemMessageW(hwnd, IDC_HYPERLINK, WM_SETFONT, (WPARAM)hLinkFont, TRUE);

        // Load hand cursor
        hHandCursor = LoadCursor(nullptr, IDC_HAND);

        // Load bitmap for display
        HBITMAP hBitmap = (HBITMAP)LoadBitmapW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(100));
        if (hBitmap) {
            SendDlgItemMessageW(hwnd, IDC_IMAGE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
        }
        return TRUE;
    }
    case WM_SETCURSOR: {
        // Set hand cursor for hyperlink
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);
        HWND hwndLink = GetDlgItem(hwnd, IDC_HYPERLINK);
        RECT rect;
        GetClientRect(hwndLink, &rect);
        MapWindowPoints(hwndLink, hwnd, (LPPOINT)&rect, 2);
        if (PtInRect(&rect, pt)) {
            SetCursor(hHandCursor);
            return TRUE;
        }
        break;
    }
    case WM_NOTIFY: {
        // Handle hyperlink clicks
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->idFrom == IDC_HYPERLINK && (pnmh->code == NM_CLICK || pnmh->code == NM_RETURN)) {
            ShellExecuteW(nullptr, L"open", L"https://grimms3dworlds.ddns.net", nullptr, nullptr, SW_SHOWNORMAL);
            return TRUE;
        }
        break;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_HYPERLINK && HIWORD(wParam) == STN_CLICKED) {
            ShellExecuteW(nullptr, L"open", L"https://grimms3dworlds.ddns.net", nullptr, nullptr, SW_SHOWNORMAL);
            return TRUE;
        }
        if (LOWORD(wParam) == IDC_APPLY_BUTTON) {
            // Apply custom resolution
            wchar_t widthStr[10], heightStr[10];
            GetDlgItemTextW(hwnd, IDC_WIDTH_EDIT, widthStr, 10);
            GetDlgItemTextW(hwnd, IDC_HEIGHT_EDIT, heightStr, 10);
            int width = _wtoi(widthStr);
            int height = _wtoi(heightStr);

            if (width <= 0 || height <= 0) {
                MessageBoxW(hwnd, L"Invalid resolution values.", L"Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            if (!IsCommonResolution(width, height)) {
                if (MessageBoxW(hwnd, L"The resolution is not common. Are you sure you want to apply it?", L"Confirm", MB_YESNO | MB_ICONQUESTION) != IDYES)
                    return TRUE;
            }

            // Get base game configuration paths
            std::vector<std::wstring> basePaths;
            std::wstring baseMain = gameDir + L"main\\configs\\" + configFileName;
            std::wstring vsMain = virtualStoreDir + L"main\\configs\\" + configFileName;
            if (FileExists(baseMain)) basePaths.push_back(baseMain);
            if (FileExists(vsMain)) basePaths.push_back(vsMain);

            // Handle expansion configurations
            std::vector<std::wstring> expansionPaths = GetExpansionConfigPaths();
            std::vector<std::wstring> allPaths = basePaths;
            if (!expansionPaths.empty()) {
                if (DialogBoxW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDD_EXPANSIONS_DIALOG), hwnd, ExpansionsDlgProc) == IDOK) {
                    allPaths.insert(allPaths.end(), expansionPaths.begin(), expansionPaths.end());
                }
            }

            ApplyResolutionSettings(hwnd, allPaths, width, height);
            MessageBoxW(hwnd, L"Resolution settings applied successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
        }
        else if (LOWORD(wParam) == IDC_MAX_QUALITY_BUTTON) {
            // Apply maximum quality settings
            std::vector<std::wstring> basePaths;
            std::wstring baseMain = gameDir + L"main\\configs\\" + configFileName;
            std::wstring vsMain = virtualStoreDir + L"main\\configs\\" + configFileName;
            if (FileExists(baseMain)) basePaths.push_back(baseMain);
            if (FileExists(vsMain)) basePaths.push_back(vsMain);

            std::vector<std::wstring> expansionPaths = GetExpansionConfigPaths();
            std::vector<std::wstring> allPaths = basePaths;
            if (!expansionPaths.empty()) {
                if (DialogBoxW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDD_EXPANSIONS_DIALOG), hwnd, ExpansionsDlgProc) == IDOK) {
                    allPaths.insert(allPaths.end(), expansionPaths.begin(), expansionPaths.end());
                }
            }

            ApplyMaxQualitySettings(allPaths);
            MessageBoxW(hwnd, L"Max quality settings applied successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
        }
        return TRUE;
    case WM_DESTROY:
        // Clean up resources
        if (hLinkFont) {
            DeleteObject(hLinkFont);
        }
        return TRUE;
    case WM_CLOSE:
        EndDialog(hwnd, 0);
        return TRUE;
    }
    return FALSE;
}

// Program entry point
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
) {
    // Check first-run status
    std::ifstream config("MohaaResolutionUtil.cfg");
    if (config.is_open()) {
        std::string line;
        std::getline(config, line);
        if (line.find("firstRun=0") != std::string::npos)
            firstRun = false;
        config.close();
    }

    // Show first-run dialog if necessary
    if (firstRun) {
        if (DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_FIRSTRUN_DIALOG), nullptr, FirstRunDlgProc) != IDOK) {
            return 0;
        }
    }

    // Read game installation path from registry
    if (!ReadRegistry(gameDir)) {
        MessageBoxW(nullptr, L"Could not find MOHAA installation in registry.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    virtualStoreDir = GetVirtualStorePath();

    // Verify base game configuration exists
    std::wstring baseMain = gameDir + L"main\\configs\\" + configFileName;
    std::wstring vsMain = virtualStoreDir + L"main\\configs\\" + configFileName;
    if (!FileExists(baseMain) && !FileExists(vsMain)) {
        MessageBoxW(nullptr, L"Configuration file not found for base game. Please run the game first.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Show main dialog
    DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_MAIN_DIALOG), nullptr, MainDlgProc);

    return 0;
}