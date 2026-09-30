#!/bin/sh
# Builds the puppet master voice lines: every controller kind says "controller enabled"
# in its own language, in a S.A.M.-like robot voice.
#
# Needs espeak-ng (the speech) and ffmpeg (the robot filter and the Ogg encoding).
# Run it from the repository root; it writes GoEoOL/data/sounds/voice-controller-*.ogg.
# The generated audio is ours to ship: espeak-ng's GPL covers the program, not what it says.
set -eu

out=GoEoOL/data/sounds
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

# 8-bit crunch at a low sample rate (like SAM on a C64), a slight ring modulation,
# and a short metallic echo; then loudness to match the other effects
robot='aresample=11025,acrusher=bits=8:mode=lin:mix=0.6,aresample=22050,'\
'aeval=val(0)*(0.65+0.35*sin(2*PI*55*t)):c=same,'\
'aecho=0.7:0.6:7|13:0.35|0.2,'\
'loudnorm=I=-16:TP=-1.5,aresample=22050'

# kind, espeak-ng voice, words
while IFS='|' read -r kind voice words; do
    espeak-ng -v "$voice+robosoft" -s 125 -p 30 -a 160 -w "$tmp/$kind.wav" "$words"
    ffmpeg -nostdin -loglevel error -y -i "$tmp/$kind.wav" -af "$robot" -ac 1 \
        -c:a libvorbis -q:a 4 "$out/voice-controller-$kind.ogg"
    echo "$out/voice-controller-$kind.ogg: $words"
done <<LINES
patrol|en-gb|Controller enabled.
collector|pl|Kontroler włączony.
hunter|de|Steuerung aktiviert.
wallfollower|fr|Contrôleur activé.
guardian|ru|Контроллер включён.
hound|la|Moderator activatus.
LINES
