import re
import sys
import os

def clamp_attribute():
    # Kontrola argumentů: soubor, min_hodnota, max_hodnota, název atributu
    if len(sys.argv) < 5:
        print("Použití: python3 clamp.py <soubor> <min> <max> <atribut>")
        print("Příklad: python3 clamp.py assets/cars/felda.txt 100 5000 stiffness")
        return

    file_path = sys.argv[1]
    
    try:
        min_val = float(sys.argv[2])
        max_val = float(sys.argv[3])
    except ValueError:
        print("Chyba: Min a Max musí být čísla (např. 0.5 nebo 100).")
        return

    if min_val > max_val:
        print("Chyba: Min hodnota nemůže být větší než Max hodnota!")
        return

    attribute_name = sys.argv[4]

    if not os.path.exists(file_path):
        print(f"Chyba: Soubor '{file_path}' neexistuje.")
        return

    # Regex najde "atribut": hodnota (bere v potaz i záporná čísla a desatinné tečky)
    pattern = rf'"{attribute_name}"\s*:\s*(\-?\d+\.?\d*)'

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # Pomocná proměnná, abychom věděli, jestli jsme vůbec něco změnili
        changes = {"count": 0}

        def replace_func(match):
            old_value = float(match.group(1))
            
            # Samotný clamp: udrží hodnotu mezi min_val a max_val
            new_value = max(min_val, min(old_value, max_val))
            
            if old_value != new_value:
                changes["count"] += 1
                
            return f'"{attribute_name}":{new_value:.6f}'

        # Provedeme nahrazení
        new_content = re.sub(pattern, replace_func, content)

        # Zapíšeme změny zpět do souboru
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        
        print(f"Hotovo! Atribut '{attribute_name}' byl v souboru '{file_path}' omezen na rozsah {min_val} až {max_val}.")
        print(f"Upraveno hodnot: {changes['count']}")

    except Exception as e:
        print(f"Nastala chyba: {e}")

if __name__ == "__main__":
    clamp_attribute()