#!/bin/sh
# Downloads the Eric Skiff songs that skins.json plays into GoEoOL/data/music/ericskiff.
# Resistor Anthems by Eric Skiff, CC BY 4.0: http://ericskiff.com/music
# Run it from the repository root; songs already there are skipped.
set -eu

dest="GoEoOL/data/music/ericskiff"
base="https://ericskiff.com/music/Resistor%20Anthems"
mkdir -p "$dest"

for song in \
    "01 A Night Of Dizzy Spells" \
    "02 Underclocked (underunderclocked mix)" \
    "03 Chibi Ninja" \
    "05 Come and Find Me" \
    "06 Searching" \
    "07 We're the Resistors" \
    "10 Arpanauts" \
    "12 HHAvok-main" \
    "13 Digital Native" \
    "14 Jumpshot" \
    "15 Prologue" \
    "16 We're all under the stars"
do
    file="$dest/$song.mp3"
    if [ -s "$file" ]; then
        echo "have   $song"
        continue
    fi
    url="$base/$(printf '%s' "$song" | sed -e 's/ /%20/g' -e 's/(/%28/g' -e 's/)/%29/g').mp3"
    echo "fetch  $song"
    curl -fsSL -o "$file.part" "$url"
    mv "$file.part" "$file"
done
echo "Done. Commit them with: git add $dest"
