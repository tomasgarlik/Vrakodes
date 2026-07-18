#include <iostream>
#include <cstdlib> // Knihovna pro funkci system()

int main() {
    std::cout << "Spoustim git pull origin main...\n";
    
    // Funkce system() spustí příkaz v příkazové řádce (PowerShellu/CMD)
    // Vrátí 0, pokud vše proběhlo v pořádku
    int vysledek = std::system("git pull origin main");
    
    if (vysledek == 0) {
        std::cout << "Hotovo. Repositoar je aktualni.\n";
    } else {
        std::cerr << "Chyba: Git pull selhal.\n";
    }
    
    return vysledek;
}