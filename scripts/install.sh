#!/bin/sh

mkdir -p $HOME/.local/bin
mkdir -p $HOME/.local/share/pongc
mkdir -p $HOME/.local/share/pongc/scripts

echo "[*] Installing binary to $HOME/.local/bin"
mv build/pongc $HOME/.local/bin

echo "[*] Installing shared between architecture files to $HOME/.local/share/pongc"
for f in docs/*; do
  cp "$f" $HOME/.local/share/pongc
done

cp "LICENSE.txt" "$HOME/.local/share/pongc"

echo "[*] Installing pongc scripts"
cp uninstall.sh $HOME/.local/share/pongc/scripts

echo "[*] Make sure you have $HOME/.local/bin in your \$PATH in order to run pongc without full path to executable"
