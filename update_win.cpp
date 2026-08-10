#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

// NASTAVENÍ TVÉHO REPOZITÁŘE
const std::string REPO_USER = "tomasgarlik";
const std::string REPO_NAME = "Vrakodes";
const std::string CURRENT_VERSION = "v1.3.0-alpha"; // Verze tvé současné aplikace

// Zobrazení pop-up okna ve Windows
void showMessage(const std::string& text, const std::string& title, UINT type = MB_OK | MB_ICONINFORMATION) {
    MessageBoxA(NULL, text.c_str(), title.c_str(), type);
}

// Přečtení settings.cfg
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

// Spuštění systémového příkazu skrytě (bez blikání konzole) a počkání na dokončení
bool executeHiddenCommand(const std::string& command) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Skryje okno
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

// Stažení souboru přes PowerShell
bool downloadFile(const std::string& url, const std::string& targetPath) {
    if (fs::exists(targetPath)) {
        fs::remove(targetPath);
    }

    // Absolutní cesta pro jistotu, že PowerShell ví, kam přesně ukládat
    std::string fullPath = fs::absolute(targetPath).string();

    std::string psCommand = "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
                            "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; "
                            "$web = New-Object System.Net.WebClient; "
                            "$web.Headers.Add('User-Agent', 'Mozilla/5.0'); "
                            "$web.DownloadFile('" + url + "', '" + fullPath + "')\"";

    executeHiddenCommand(psCommand);

    return fs::exists(targetPath) && fs::file_size(targetPath) > 0;
}

// Zjištění nejnovějšího tagu z GitHub API
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

// Rozbalení ZIPu pomocí PowerShellu
bool extractZip(const std::string& zipPath, const std::string& extractTo) {
    std::string fullZip = fs::absolute(zipPath).string();
    std::string fullExtract = fs::absolute(extractTo).string();

    std::string psCommand = "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
                            "Expand-Archive -Path '" + fullZip + "' -DestinationPath '" + fullExtract + "' -Force\"";
    
    return executeHiddenCommand(psCommand);
}

int main() {
    Sleep(1000);

    bool allowUnofficial = checkUnofficialUpdatesSetting("settings.cfg");
    std::string downloadUrl = "";
    std::string latestVersion = "";

    if (allowUnofficial) {
        downloadUrl = "https://github.com/" + REPO_USER + "/" + REPO_NAME + "/archive/refs/heads/main.zip";
    } else {
        latestVersion = getLatestReleaseTag();

        if (latestVersion.empty()) {
            showMessage("Nepadřilo se ověřit nejnovější verzi z GitHubu.\n\n(Ujisti se, že repozitář není PRIVATE a že na GitHubu existuje alespoň 1 vydaný Release).", "Chyba aktualizace", MB_OK | MB_ICONERROR);
            return 1;
        }

        if (latestVersion == CURRENT_VERSION) {
            showMessage("Máte nejnovější verzi aplikace (" + CURRENT_VERSION + ").", "Aktualizace není potřeba", MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        downloadUrl = "https://github.com/" + REPO_USER + "/" + REPO_NAME + "/releases/download/" + latestVersion + "/update.zip";
    }

    std::string tempZip = "update_temp.zip";

    if (!downloadFile(downloadUrl, tempZip)) {
        showMessage("Chyba při stahování souboru aktualizace z adrese:\n" + downloadUrl, "Chyba", MB_OK | MB_ICONERROR);
        return 1;
    }

    if (extractZip(tempZip, "./")) {
        fs::remove(tempZip);

        std::string successMsg = allowUnofficial 
            ? "Aplikace byla úspěšně aktualizována na nejnovější vývojovou verzi (main)!" 
            : "Aplikace byla úspěšně aktualizována na verzi " + latestVersion + "!";
        
        showMessage(successMsg, "Aktualizace dokončena", MB_OK | MB_ICONINFORMATION);

        ShellExecuteA(NULL, "open", "MojeAplikace.exe", NULL, NULL, SW_SHOWNORMAL);
    } else {
        fs::remove(tempZip);
        showMessage("Při rozbalování aktualizačního balíčku došlo k chybě.", "Chyba", MB_OK | MB_ICONERROR);
        return 1;
    }

    return 0;
}