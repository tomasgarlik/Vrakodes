#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

namespace fs = std::filesystem;

// REPOSITORY CONFIGURATION
const std::string REPO_USER = "tomasgarlik";
const std::string REPO_NAME = "Vrakodes";
const std::string CURRENT_VERSION = "v1.3.0-alpha"; // Your current app version

// GUI Globals
HWND hMainWnd = NULL;
HWND hProgressBar = NULL;
HWND hStatusText = NULL;

// Helper to update GUI status and progress percentage (0 - 100)
void updateProgress(const std::string& text, int percentage) {
    if (hStatusText) SetWindowTextA(hStatusText, text.c_str());
    if (hProgressBar) SendMessage(hProgressBar, PBM_SETPOS, percentage, 0);
    
    // Process GUI messages to keep window responsive
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void showMessage(const std::string& text, const std::string& title, UINT type = MB_OK | MB_ICONINFORMATION) {
    MessageBoxA(hMainWnd, text.c_str(), title.c_str(), type);
}

// Check settings.cfg
bool checkUnofficialUpdatesSetting(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) return false;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("UNOFFICIAL_UPDATES=true") != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Execute command silently
bool executeHiddenCommand(const std::string& command) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

    std::string cmd = "cmd.exe /c " + command;

    if (CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return exitCode == 0;
    }
    return false;
}

// Download file via PowerShell
bool downloadFile(const std::string& url, const std::string& targetPath) {
    if (fs::exists(targetPath)) {
        fs::remove(targetPath);
    }

    std::string fullPath = fs::absolute(targetPath).string();

    std::string psCommand = "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
                            "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; "
                            "$web = New-Object System.Net.WebClient; "
                            "$web.Headers.Add('User-Agent', 'Mozilla/5.0'); "
                            "$web.DownloadFile('" + url + "', '" + fullPath + "')\"";

    executeHiddenCommand(psCommand);

    return fs::exists(targetPath) && fs::file_size(targetPath) > 0;
}

// Get tag from GitHub API
std::string getLatestReleaseTag() {
    std::string apiUrl = "https://api.github.com/repos/" + REPO_USER + "/" + REPO_NAME + "/releases/latest";
    std::string jsonPath = "release_info.json";
    
    if (!downloadFile(apiUrl, jsonPath)) {
        return "";
    }

    std::ifstream file(jsonPath);
    if (!file.is_open()) return "";

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    fs::remove(jsonPath);

    size_t tagPos = content.find("\"tag_name\":");
    if (tagPos == std::string::npos) return "";

    size_t start = content.find("\"", tagPos + 11) + 1;
    size_t end = content.find("\"", start);
    
    return content.substr(start, end - start);
}

// Extract ZIP using PowerShell
bool extractZip(const std::string& zipPath, const std::string& extractTo) {
    std::string fullZip = fs::absolute(zipPath).string();
    std::string fullExtract = fs::absolute(extractTo).string();

    std::string psCommand = "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
                            "Expand-Archive -Path '" + fullZip + "' -DestinationPath '" + fullExtract + "' -Force\"";
    
    return executeHiddenCommand(psCommand);
}

// Basic Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Create native Windows progress GUI
void createUpdateWindow(HINSTANCE hInstance) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "UpdaterWindowClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassExA(&wc);

    // Center window on screen
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int winW = 380, winH = 150;
    int posX = (screenW - winW) / 2;
    int posY = (screenH - winH) / 2;

    hMainWnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        "UpdaterWindowClass",
        "Software Updater",
        WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU,
        posX, posY, winW, winH,
        NULL, NULL, hInstance, NULL
    );

    hStatusText = CreateWindowExA(
        0, "STATIC", "Initializing updater...",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 15, 320, 20,
        hMainWnd, NULL, hInstance, NULL
    );

    hProgressBar = CreateWindowExA(
        0, PROGRESS_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        20, 45, 325, 25,
        hMainWnd, NULL, hInstance, NULL
    );

    SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
}

// Main logic running in worker loop
void runUpdaterLogic() {
    updateProgress("Checking for updates... (10%)", 10);
    Sleep(800);

    bool allowUnofficial = checkUnofficialUpdatesSetting("settings.cfg");
    std::string downloadUrl = "";
    std::string latestVersion = "";

    if (allowUnofficial) {
        updateProgress("Fetching latest development build... (25%)", 25);
        downloadUrl = "https://github.com/" + REPO_USER + "/" + REPO_NAME + "/archive/refs/heads/main.zip";
    } else {
        updateProgress("Checking GitHub for releases... (25%)", 25);
        latestVersion = getLatestReleaseTag();

        if (latestVersion.empty()) {
            showMessage("Failed to verify latest version from GitHub.\nCheck your internet connection or release settings.", "Update Error", MB_OK | MB_ICONERROR);
            DestroyWindow(hMainWnd);
            return;
        }

        if (latestVersion == CURRENT_VERSION) {
            updateProgress("Already up to date! (100%)", 100);
            Sleep(500);
            showMessage("You are running the latest version (" + CURRENT_VERSION + ").", "Up to Date", MB_OK | MB_ICONINFORMATION);
            DestroyWindow(hMainWnd);
            return;
        }

        downloadUrl = "https://github.com/" + REPO_USER + "/" + REPO_NAME + "/releases/download/" + latestVersion + "/update.zip";
    }

    updateProgress("Downloading update package... (50%)", 50);
    std::string tempZip = "update_temp.zip";

    if (!downloadFile(downloadUrl, tempZip)) {
        showMessage("Failed to download update package.", "Error", MB_OK | MB_ICONERROR);
        DestroyWindow(hMainWnd);
        return;
    }

    updateProgress("Extracting files... (80%)", 80);
    if (extractZip(tempZip, "./")) {
        fs::remove(tempZip);

        updateProgress("Update complete! (100%)", 100);
        Sleep(500);

        std::string successMsg = allowUnofficial 
            ? "App updated to the latest development build (main)!" 
            : "App successfully updated to version " + latestVersion + "!";
        
        showMessage(successMsg, "Update Complete", MB_OK | MB_ICONINFORMATION);

        // Restart main application
        ShellExecuteA(NULL, "open", "MojeAplikace.exe", NULL, NULL, SW_SHOWNORMAL);
    } else {
        fs::remove(tempZip);
        showMessage("An error occurred while extracting update files.", "Error", MB_OK | MB_ICONERROR);
    }

    DestroyWindow(hMainWnd);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    createUpdateWindow(hInstance);
    
    // Give window time to render properly
    updateProgress("Starting updater...", 0);
    Sleep(500);

    // Run update logic
    runUpdaterLogic();

    return 0;
}