#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <windows.h>
#include <commctrl.h>
#include <thread>

#pragma comment(lib, "comctl32.lib")

namespace fs = std::filesystem;

// REPOSITORY CONFIGURATION
const std::string REPO_USER = "tomasgarlik";
const std::string REPO_NAME = "Vrakodes";
const std::string CURRENT_VERSION = "v1.3.0-alpha";
const std::string MAIN_APP_EXE = "MojeAplikace.exe"; // Změň na název tvého .exe

// GUI Globals
HWND hMainWnd = NULL;
HWND hProgressBar = NULL;
HWND hStatusText = NULL;

void updateStatusText(const std::string& text) {
    if (hStatusText) {
        SetWindowTextA(hStatusText, text.c_str());
    }
}

// Přepínání mezi neustálým pohybem (Marquee) a pevnými procenty
void setMarqueeMode(bool enable) {
    if (!hProgressBar) return;
    LONG_PTR style = GetWindowLongPtr(hProgressBar, GWL_STYLE);
    if (enable) {
        SetWindowLongPtr(hProgressBar, GWL_STYLE, style | PBS_MARQUEE);
        SendMessage(hProgressBar, PBM_SETMARQUEE, TRUE, 30); // Číslo 30 určuje rychlost animace
    } else {
        SendMessage(hProgressBar, PBM_SETMARQUEE, FALSE, 0);
        SetWindowLongPtr(hProgressBar, GWL_STYLE, style & ~PBS_MARQUEE);
    }
}

void setProgressPercent(int percentage) {
    setMarqueeMode(false);
    if (hProgressBar) {
        SendMessage(hProgressBar, PBM_SETPOS, percentage, 0);
    }
}

void showMessage(const std::string& text, const std::string& title, UINT type = MB_OK | MB_ICONINFORMATION) {
    MessageBoxA(hMainWnd, text.c_str(), title.c_str(), type);
}

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

bool extractZip(const std::string& zipPath, const std::string& extractTo) {
    std::string fullZip = fs::absolute(zipPath).string();
    std::string fullExtract = fs::absolute(extractTo).string();

    std::string psCommand = "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
                            "Expand-Archive -Path '" + fullZip + "' -DestinationPath '" + fullExtract + "' -Force\"";
    
    return executeHiddenCommand(psCommand);
}

// Tuto funkci spouštíme ve vedlejším vlákně
void asyncUpdateThread() {
    Sleep(500); // Čas na inicializaci okna

    updateStatusText("Checking for updates...");
    setMarqueeMode(true); // Zapne animovaného jezzdce

    bool allowUnofficial = checkUnofficialUpdatesSetting("settings.cfg");
    std::string downloadUrl = "";
    std::string latestVersion = "";

    if (allowUnofficial) {
        updateStatusText("Fetching latest development build...");
        downloadUrl = "https://github.com/" + REPO_USER + "/" + REPO_NAME + "/archive/refs/heads/main.zip";
    } else {
        updateStatusText("Checking GitHub for releases...");
        latestVersion = getLatestReleaseTag();

        if (latestVersion.empty()) {
            showMessage("Failed to verify latest version from GitHub.", "Update Error", MB_OK | MB_ICONERROR);
            PostMessage(hMainWnd, WM_CLOSE, 0, 0);
            return;
        }

        if (latestVersion == CURRENT_VERSION) {
            setProgressPercent(100);
            updateStatusText("Already up to date!");
            Sleep(300);
            showMessage("You are running the latest version (" + CURRENT_VERSION + ").", "Up to Date", MB_OK | MB_ICONINFORMATION);
            PostMessage(hMainWnd, WM_CLOSE, 0, 0);
            return;
        }

        downloadUrl = "https://github.com/" + REPO_USER + "/" + REPO_NAME + "/releases/download/" + latestVersion + "/update.zip";
    }

    updateStatusText("Downloading update package from GitHub...");
    setMarqueeMode(true); // Během stahování se lišta neustále hýbe!
    
    std::string tempZip = "update_temp.zip";

    if (!downloadFile(downloadUrl, tempZip)) {
        showMessage("Failed to download update package.", "Error", MB_OK | MB_ICONERROR);
        PostMessage(hMainWnd, WM_CLOSE, 0, 0);
        return;
    }

    updateStatusText("Extracting update files...");
    setMarqueeMode(true); // Během rozbalování se lišta stále hýbe

    if (extractZip(tempZip, "./")) {
        fs::remove(tempZip);

        setProgressPercent(100);
        updateStatusText("Update complete!");
        Sleep(500);

        std::string successMsg = allowUnofficial 
            ? "App updated to the latest development build (main)!" 
            : "App successfully updated to version " + latestVersion + "!";
        
        showMessage(successMsg, "Update Complete", MB_OK | MB_ICONINFORMATION);

        ShellExecuteA(NULL, "open", MAIN_APP_EXE.c_str(), NULL, NULL, SW_SHOWNORMAL);
    } else {
        fs::remove(tempZip);
        showMessage("An error occurred while extracting update files.", "Error", MB_OK | MB_ICONERROR);
    }

    PostMessage(hMainWnd, WM_CLOSE, 0, 0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

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

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int winW = 380, winH = 150;
    int posX = (screenW - winW) / 2;
    int posY = (screenH - winH) / 2;

    hMainWnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        "UpdaterWindowClass",
        "Software Updater",
        WS_VISIBLE | WS_POPUP | WS_CAPTION,
        posX, posY, winW, winH,
        NULL, NULL, hInstance, NULL
    );

    hStatusText = CreateWindowExA(
        0, "STATIC", "Initializing...",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 15, 330, 20,
        hMainWnd, NULL, hInstance, NULL
    );

    hProgressBar = CreateWindowExA(
        0, PROGRESS_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        20, 45, 325, 25,
        hMainWnd, NULL, hInstance, NULL
    );
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    createUpdateWindow(hInstance);

    // Spustíme logiku aktualizace v odděleném vlákně
    std::thread workerThread(asyncUpdateThread);
    workerThread.detach();

    // Hlavní vlákno starající se POUZE o plynulost okna (Message Loop)
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}