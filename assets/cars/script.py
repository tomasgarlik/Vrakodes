from pathlib import Path
import re

# Složka, ve které se nachází tento skript
root_dir = Path(__file__).resolve().parent

# Odstraní "min_len": <číslo>, včetně čárky za ním (případně před ním, pokud je na konci objektu)
pattern_trailing_comma = re.compile(
    r'"min_len"\s*:\s*[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?\s*,\s*'
)
pattern_leading_comma = re.compile(
    r'\s*,\s*"min_len"\s*:\s*[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?'
)

modified_files = 0
removed_total = 0

for json_path in root_dir.rglob("vehicle.json"):
    with open(json_path, "r", encoding="utf-8", newline="") as f:
        content = f.read()

    new_content, count1 = pattern_trailing_comma.subn("", content)
    new_content, count2 = pattern_leading_comma.subn("", new_content)
    removed = count1 + count2

    if removed > 0:
        with open(json_path, "w", encoding="utf-8", newline="") as f:
            f.write(new_content)
        modified_files += 1
        removed_total += removed
        print(f"[OK] {json_path.relative_to(root_dir)} (-{removed})")

print(f"\nHotovo. Upraveno souborů: {modified_files}, odstraněno položek min_len: {removed_total}.")