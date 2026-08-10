#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <windows.h>
#include <commctrl.h>
#include <thread>
#include <atomic>

#pragma comment(lib, "comctl32.lib")

namespace fs = std::filesystem;

// REPOSITORY CONFIGURATION
const std::string REPO_USER = "tomasgarlik";
const std::string REPO_NAME = "Vrakodes";
const std::string CURRENT_VERSION = "v1.3.0-alpha";
const std::string MAIN_APP_EXE = "MojeAplikace.exe"; // Změň na název tvého .exe

// Globální proměnné
HWND hMainWnd = NULL;
HWND hProgressBar = NULL;
HWND hStatusText = NULL;
std::atomic<bool> g_IsRunning(true);
HANDLE g_hJob = NULL;

void initJobObject() {
    // Vytvoříme Job Object pro správu dceřiných procesů
    g_hJob = CreateJobObjectA(NULL, NULL);
    if (g_hJob) {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {0};
        jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(g_hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
    }
}

void forceKillChildProcess() {
    g_IsRunning = false;
    if (g_hJob != NULL) {
        // Okamžitě ukončí všechny procesy v Jobu (cmd i PowerShell)
        TerminateJobObject(g_hJob, 0);
        CloseHandle(g_hJob);
        g_hJob = NULL;
    }
}

void updateStatusText(const std::string& text) {
    if (hStatusText && g_IsRunning) {
        SetWindowTextA(hStatusText, text.c_str());
    }
}

void showMessage(const std::string& text, const std::string& title, UINT type = MB_OK | MB_ICONINFORMATION) {
    if (g_IsRunning) {
        MessageBoxA(hMainWnd, text.c_str(), title.c_str(), type);
    }
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
    if (!g_IsRunning) return false;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

    std::string cmd = "cmd.exe /c " + command;

    if (CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_BREAKAWAY_FROM_JOB, NULL, NULL, &si, &pi)) {
        if (g_hJob) {
            AssignProcessToJobObject(g_hJob, pi.hProcess);
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 1;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        return exitCode == 0;
    }
    return false;
}

bool downloadFile(const std::string& url, const std::string& targetPath) {
    if (!g_IsRunning) return false;

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
    if (!g_IsRunning) return false;

    std::string fullZip = fs::absolute(zipPath).string();
    std::string fullExtract = fs::absolute(extractTo).string();

    std::string psCommand = "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
                            "Expand-Archive -Path '" + fullZip + "' -DestinationPath '" + fullExtract + "' -Force\"";
    
    return executeHiddenCommand(psCommand);
}

// Logika stahování
void asyncUpdateThread() {
    Sleep(300);

    updateStatusText("Checking for updates...");

    bool allowUnofficial = checkUnofficialUpdatesSetting("settings.cfg");
    std::string downloadUrl = "";
    std::string latestVersion = "";

    if (allowUnofficial) {
        updateStatusText("Fetching latest development build...");
        downloadUrl = "https://github.com/" + REPO_USER + "/" + REPO_NAME + "/archive/refs/heads/main.zip";
    } else {
        updateStatusText("Checking GitHub for releases...");
        latestVersion = getLatestReleaseTag();

        if (!g_IsRunning) return;

        if (latestVersion.empty()) {
            showMessage("Failed to verify latest version from GitHub.", "Update Error", MB_OK | MB_ICONERROR);
            PostMessage(hMainWnd, WM_DESTROY, 0, 0);
            return;
        }

        if (latestVersion == CURRENT_VERSION) {
            updateStatusText("Already up to date!");
            Sleep(400);
            showMessage("You are running the latest version (" + CURRENT_VERSION + ").", "Up to Date", MB_OK | MB_ICONINFORMATION);
            PostMessage(hMainWnd, WM_DESTROY, 0, 0);
            return;
        }

        downloadUrl = "https://github.com/" + REPO_USER + "/" + REPO_NAME + "/releases/download/" + latestVersion + "/update.zip";
    }

    if (!g_IsRunning) return;

    updateStatusText("Downloading update package...");
    std::string tempZip = "update_temp.zip";

    if (!downloadFile(downloadUrl, tempZip)) {
        if (g_IsRunning) showMessage("Failed to download update package.", "Error", MB_OK | MB_ICONERROR);
        PostMessage(hMainWnd, WM_DESTROY, 0, 0);
        return;
    }

    if (!g_IsRunning) return;

    updateStatusText("Extracting update files...");

    if (extractZip(tempZip, "./")) {
        fs::remove(tempZip);

        if (!g_IsRunning) return;

        updateStatusText("Update complete!");
        Sleep(400);

        std::string successMsg = allowUnofficial 
            ? "App updated to latest build!" 
            : "App successfully updated to version " + latestVersion + "!";
        
        showMessage(successMsg, "Update Complete", MB_OK | MB_ICONINFORMATION);

        ShellExecuteA(NULL, "open", MAIN_APP_EXE.c_str(), NULL, NULL, SW_SHOWNORMAL);
    } else {
        fs::remove(tempZip);
        if (g_IsRunning) showMessage("An error occurred while extracting update files.", "Error", MB_OK | MB_ICONERROR);
    }

    PostMessage(hMainWnd, WM_DESTROY, 0, 0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int progressPos = 0;

    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 30, NULL);
            return 0;

        case WM_TIMER:
            if (hProgressBar && g_IsRunning) {
                progressPos = (progressPos + 3) % 100;
                SendMessage(hProgressBar, PBM_SETPOS, progressPos, 0);
            }
            return 0;

        case WM_CLOSE:
            forceKillChildProcess();
            DestroyWindow(hwnd);
            ExitProcess(0);
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            forceKillChildProcess();
            PostQuitMessage(0);
            ExitProcess(0);
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
        WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU,
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
        WS_CHILD | WS_VISIBLE,
        20, 45, 325, 25,
        hMainWnd, NULL, hInstance, NULL
    );

    SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    initJobObject();
    createUpdateWindow(hInstance);

    std::thread workerThread(asyncUpdateThread);
    workerThread.detach();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}