#!/usr/bin/env bash
set -e

TARGET="micro"
BINDIR="/usr/local/bin"

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
RESET='\033[0m'
BOLD='\033[1m'

echo -e "${BOLD}${YELLOW}╔═══════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${YELLOW}║   ${TARGET} - Uninstaller            ║${RESET}"
echo -e "${BOLD}${YELLOW}╚═══════════════════════════════════════╝${RESET}"
echo

if [[ -f "$BINDIR/$TARGET" ]]; then
    echo -e "${YELLOW}Removing ${TARGET} from $BINDIR...${RESET}"
    sudo rm -f "$BINDIR/$TARGET"
    echo -e "${GREEN}✓${RESET} ${TARGET} has been uninstalled successfully."
else
    echo -e "${RED}✗${RESET} ${TARGET} is not installed in $BINDIR"
fi

echo
echo -e "${GREEN}Done!${RESET}"
