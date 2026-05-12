#!/usr/bin/env bash
set -e

REPO_URL="https://github.com/bokshi-gh/micro.git"
TARGET="micro"

TMP_DIR=$(mktemp -d)

GREEN='\033[0;32m'
RESET='\033[0m'

echo -e "${GREEN}Cloning repository...${RESET}"
git clone --depth 1 "$REPO_URL" "$TMP_DIR"

cd "$TMP_DIR"

echo
echo -e "${GREEN}Building ${TARGET}...${RESET}"
make

echo
echo -e "${GREEN}Installing ${TARGET} system-wide...${RESET}"
sudo make install

cd /
rm -rf "$TMP_DIR"

echo
echo -e "${GREEN}Installation complete!${RESET}"
echo -e "Run '${TARGET} -help' to get started."
