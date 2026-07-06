#!/usr/bin/env bash
set -e

REPO_URL="https://github.com/bokshi-gh/text-editor.git"
TARGET="micro"

TMP_DIR=$(mktemp -d)

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
RESET='\033[0m'
BOLD='\033[1m'

echo -e "${BOLD}${GREEN}╔═══════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${GREEN}║   ${TARGET} - Text Editor Installer   ║${RESET}"
echo -e "${BOLD}${GREEN}╚═══════════════════════════════════════╝${RESET}"
echo

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo -e "${RED}Error: git is not installed. Please install git first.${RESET}"
    exit 1
fi

# Check if make is installed
if ! command -v make &> /dev/null; then
    echo -e "${RED}Error: make is not installed. Please install make first.${RESET}"
    exit 1
fi

# Check if g++ is installed
if ! command -v g++ &> /dev/null; then
    echo -e "${RED}Error: g++ is not installed. Please install g++ first.${RESET}"
    exit 1
fi

echo -e "${YELLOW}Cloning repository...${RESET}"
git clone --depth 1 "$REPO_URL" "$TMP_DIR" 2>/dev/null || {
    echo -e "${RED}Failed to clone repository. Please check the URL and your internet connection.${RESET}"
    exit 1
}

cd "$TMP_DIR"

# Check if repository has the expected structure
if [[ ! -f "Makefile" ]]; then
    echo -e "${RED}Error: Makefile not found in the repository.${RESET}"
    exit 1
fi

if [[ ! -d "src" ]] || [[ ! -d "include" ]]; then
    echo -e "${RED}Error: Invalid repository structure (src/ or include/ missing).${RESET}"
    exit 1
fi

echo
echo -e "${YELLOW}Building ${TARGET}...${RESET}"
if ! make clean && make; then
    echo -e "${RED}Build failed. Please check the error messages above.${RESET}"
    exit 1
fi

# Check if binary was created
if [[ ! -f "bin/$TARGET" ]] && [[ ! -f "$TARGET" ]]; then
    echo -e "${RED}Error: Binary not found after build.${RESET}"
    exit 1
fi

echo
echo -e "${YELLOW}Installing ${TARGET} system-wide...${RESET}"
if ! sudo make install; then
    echo -e "${RED}Installation failed. Please check permissions.${RESET}"
    exit 1
fi

cd /
rm -rf "$TMP_DIR"

echo
echo -e "${BOLD}${GREEN}╔═══════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${GREEN}║     Installation Complete! 🎉      ║${RESET}"
echo -e "${BOLD}${GREEN}╚═══════════════════════════════════════╝${RESET}"
echo
echo -e "${BOLD}${GREEN}${TARGET}${RESET} has been installed successfully!"
echo
echo -e "Quick start:"
echo -e "  ${BOLD}${TARGET} --help${RESET}        Show help"
echo -e "  ${BOLD}${TARGET} file.txt${RESET}     Open a file"
echo -e "  ${BOLD}${TARGET} -v${RESET}           Show version"
echo
echo -e "Keybindings:"
echo -e "  ${YELLOW}Ctrl-Q${RESET}    Quit editor"
echo -e "  ${YELLOW}Ctrl-S${RESET}    Save file"
echo -e "  ${YELLOW}Ctrl-O${RESET}    Open file"
echo -e "  ${YELLOW}Arrow Keys${RESET} Navigate"
echo -e "  ${YELLOW}Tab${RESET}       Insert tab (${BOLD}4 spaces${RESET})"
echo
echo -e "To uninstall: ${YELLOW}sudo make uninstall${RESET} from the build directory"
echo -e "or manually remove: ${YELLOW}sudo rm /usr/local/bin/${TARGET}${RESET}"
