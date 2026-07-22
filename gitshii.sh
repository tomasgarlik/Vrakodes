#!/bin/bash

# Kontrola, zda byl zadán argument (commit message)
if [ -z "$1" ]; then
    echo "❌ Chyba: Musíš zadat zprávu pro commit!"
    echo "Použití: ./quickpush.sh \"Tvoje commit zpráva\""
    exit 1
fi

# Vykonání Git příkazů
echo "➕ Přidávám změny (git add .)..."
git add .

echo "📝 Vytvářím commit s hláškou: \"$1\"..."
git commit -m "$1"

echo "🚀 Odesílám na GitHub (git push -u origin main)..."
git push -u origin main

echo "✅ Hotovo! Vrakodes je aktualizovaný."