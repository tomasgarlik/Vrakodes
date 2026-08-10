import re
import sys
import os

def add_to_attribute():
    # Kontrola argumentů: soubor, hodnota k přičtení, název atributu
    if len(sys.argv) < 4:
        print("Usage: python3 pricitac.py <soubor> <hodnota_k_pricteni> <atribut>")
        print("Example: python3 pricitac.py assets/cars/tricycle.txt 50.0 mass")
        return

    file_path = sys.argv[1]
    
    try:
        value_to_add = float(sys.argv[2])
    except ValueError:
        print("Chyba: Hodnota k přičtení musí být číslo (např. 50.0 nebo -10)")
        return

    attribute_name = sys.argv[3]

    if not os.path.exists(file_path):
        print(f"Chyba: Soubor '{file_path}' neexistuje.")
        return

    # REGEX: Najde "atribut": hodnota
    pattern = rf'"{attribute_name}"\s*:\s*(\-?\d+\.?\d*)'

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        def replace_func(match):
            old_value = float(match.group(1))
            # TADY JE ZMĚNA: Sčítání místo násobení
            new_value = old_value + value_to_add
            # Vrací zpět formát "atribut": nová_hodnota
            return f'"{attribute_name}":{new_value:.6f}'

        # Provedeme nahrazení
        new_content = re.sub(pattern, replace_func, content)

        # Zapíšeme změny zpět do souboru
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        
        print(f"Hotovo! K atributu '{attribute_name}' bylo v souboru '{file_path}' přičteno {value_to_add}.")

    except Exception as e:
        print(f"Nastala chyba: {e}")

if __name__ == "__main__":
    add_to_attribute()