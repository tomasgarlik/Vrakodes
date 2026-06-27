import re
import sys
import os

def multiply_attribute():
    # Kontrola argumentů: soubor, násobič, název atributu
    if len(sys.argv) < 4:
        print("Použití: python3 nasobic.py <soubor> <nasobic> <atribut>")
        print("Příklad: python3 nasobic.py assets/cars/tricycle.txt 1.5 mass")
        return

    file_path = sys.argv[1]
    
    try:
        multiplier = float(sys.argv[2])
    except ValueError:
        print("Chyba: Násobič musí být číslo (např. 1.5)")
        return

    attribute_name = sys.argv[3]

    if not os.path.exists(file_path):
        print(f"Chyba: Soubor '{file_path}' neexistuje.")
        return

    # NOVÝ REGEX: Najde "atribut": hodnota
    # Hledá: "název": (nepovinné mínus a číslice)
    pattern = rf'"{attribute_name}"\s*:\s*(\-?\d+\.?\d*)'

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        def replace_func(match):
            old_value = float(match.group(1))
            new_value = old_value * multiplier
            # Vrací zpět formát "atribut": nová_hodnota
            return f'"{attribute_name}":{new_value:.6f}'

        # Provedeme nahrazení
        new_content = re.sub(pattern, replace_func, content)

        # Zapíšeme změny zpět do souboru
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        
        print(f"Hotovo! Atribut '{attribute_name}' byl v souboru '{file_path}' vynásoben {multiplier}x.")

    except Exception as e:
        print(f"Nastala chyba: {e}")

if __name__ == "__main__":
    multiply_attribute()