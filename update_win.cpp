#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <windows.h>
#include <urlmon.h>
#include <wininet.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")
namespace fs = std::filesystem;

// NASTAVENÍ TVÉHO REPOZITÁŘE
const std::string REPO_USER = "TVOJE_JMENO";
const std::string REPO_NAME = "TVUJ_REPOZITAR";
const std::string CURRENT_VERSION = "v1.0.0"; // Verze tvé současné aplikace

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

// Stažení souboru přes Windows API
bool downloadFile(const std::string& url, const std::string& targetPath) {
    DeleteUrlCacheEntryA(url.c_str());

    // Nastavení User-Agent pro GitHub
    HRESULT hr = URLDownloadToFileA(
        NULL, 
        url.c_str(), 
        targetPath.c_str(), 
        0, 
        NULL
    );

    return hr == S_OK;
}

// Zjištění náspledného tagu (verze) z GitHub API přes jednoduché stažení JSONu
std::string getLatestReleaseTag() {
    std::string apiUrl = "https://api.github.com/repos/" + REPO_USER + "/" + REPO_NAME + "/releases/latest";
    std::string jsonPath = "release_info.json";
    
    if (!downloadFile(apiUrl, jsonPath)) {
        return "";
    }

    std::ifstream file(jsonPath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    fs::remove(jsonPath); // Úklid dočasného souboru

    // Jednoduché vyhledání "tag_name": "vX.X.X" v JSONu bez externích knihoven
    size_t tagPos = content.find("\"tag_name\":");
    if (tagPos == std::string::npos) return "";

    size_t start = content.find("\"", tagPos + 11) + 1;
    size_t end = content.find("\"", start);
    
    return content.substr(start, end - start);
}

// Rozbalení ZIPu pomocí vestavěného PowerShellu
bool extractZip(const std::string& zipPath, const std::string& extractTo) {
    std::string command = "powershell -Command \"Expand-Archive -Path '" + 
                          zipPath + "' -DestinationPath '" + extractTo + "' -Force\"";
    int result = system(command.c_str());
    return result == 0;
}

int main() {
    // Krátká pauza, aby se hlavní .exe stihl zavřít, pokud updater spouští přímo on
    Sleep(1000);

    bool allowUnofficial = checkUnofficialUpdatesSetting("settings.cfg");
    std::string downloadUrl = "";
    std::string latestVersion = "";

    if (allowUnofficial) {
        // UNOFFICIAL MÓD: Stahuje vždy nejnovější main větev
        downloadUrl = "https://github.com/" + REPO_USER + "/" + REPO_NAME + "/archive/refs/heads/main.zip";
    } else {
        // OFFICIAL MÓD: Kontrola verzí přes GitHub API
        latestVersion = getLatestReleaseTag();

        if (latestVersion.empty()) {
            showMessage("Nepadřilo se ověřit nejnovější verzi z GitHubu.", "Chyba aktualizace", MB_OK | MB_ICONERROR);
            return 1;
        }

        if (latestVersion == CURRENT_VERSION) {
            showMessage("Máte nejnovější verzi aplikace (" + CURRENT_VERSION + ").", "Aktualizace není potřeba", MB_OK | MB_ICONINFORMATION);
            return 0; // Konec, nic se nestahuje
        }

        // Pokud je novější verze, stáhne se asset z release
        downloadUrl = "https://github.com/" + REPO_USER + "/" + REPO_NAME + "/releases/download/" + latestVersion + "/update.zip";
    }

    // Stažení a rozbalení aktualizace
    std::string tempZip = "update_temp.zip";

    if (!downloadFile(downloadUrl, tempZip)) {
        showMessage("Chyba při stahování souboru aktualizace.", "Chyba", MB_OK | MB_ICONERROR);
        return 1;
    }

    if (extractZip(tempZip, "./")) {
        fs::remove(tempZip); // Úklid ZIPu

        std::string successMsg = allowUnofficial 
            ? "Aplikace byla úspěšně aktualizována na nejnovější vývojovou verzi (main)!" 
            : "Aplikace byla úspěšně aktualizována na verzi " + latestVersion + "!";
        
        showMessage(successMsg, "Aktualizace dokončena", MB_OK | MB_ICONINFORMATION);

        // Znovu spustíme hlavní aplikaci
        ShellExecuteA(NULL, "open", "MojeAplikace.exe", NULL, NULL, SW_SHOWNORMAL);
    } else {
        fs::remove(tempZip);
        showMessage("Při rozbalování aktualizačního balíčku došlo k chybě.", "Chyba", MB_OK | MB_ICONERROR);
        return 1;
    }

    return 0;
}