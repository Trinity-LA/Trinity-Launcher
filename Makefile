# ==========================================
# 🎮 Trinity Launcher - Makefile Wrapper
# ==========================================

# Variables
SCRIPT = ./build.sh
# Force the use of bash to avoid syntax errors (sh/dash)
SHELL_CMD = bash

# Targets (commands) available
.PHONY: all build debug deps clean run translations help start uninstall install

# 1. Default command (just 'make') -> Compiles in Release mode
all: build

# 2. Release Compilation (Only generates binaries in build/)
build:
	@$(SHELL_CMD) $(SCRIPT) --release

# 3. Debug Compilation (With symbols for gdb/lldb)
debug:
	@$(SHELL_CMD) $(SCRIPT) --debug

# 4. Install Dependencies (Detects distro and uses sudo)
deps:
	@$(SHELL_CMD) $(SCRIPT) --deps-only

# 5. Clean project (Deletes build/ folder and repairs permissions if necessary)
clean:
	@$(SHELL_CMD) $(SCRIPT) --clean
# 6. Run the Launcher (Compiles Release + Runs LOCAL)
# Note: Ideal for development, doesn't ask for password or install in system.
run:
	@$(SHELL_CMD) $(SCRIPT) --release --run

# 7. Update Translations (.ts)
translations:
	@$(SHELL_CMD) $(SCRIPT) --update-ts

# 8. Full Setup (Deps + Build + Run)
# Note: Ideal for the first time you clone the repo.
start:
	@$(SHELL_CMD) $(SCRIPT) --deps --release --run --detached

# 9. Install to System (/usr/local/bin)
# Note: Requires sudo password. Copies binaries for global usage.
install:
	@$(SHELL_CMD) $(SCRIPT) --release --install --detached

# 10. Uninstall launcher (Removes from /usr/local/bin , shortcuts and build/)
uninstall:
	@$(SHELL_CMD) $(SCRIPT) --clean-only
	@$(SHELL_CMD) $(SCRIPT) --uninstall

help:
	@echo "🛠️  Available commands in Trinity Launcher:"
	@echo ""
	@echo "  make          -> Compiles the project in Release mode (Same as 'make build')"
	@echo "  make start    -> Full flow: Deps + Build + Run (Recommended for first time)"
	@echo "  make run      -> Compiles and runs the LOCAL version (Fast for development)"
	@echo "  make install  -> Compiles and INSTALLS to system (/usr/local/bin) [Asks for Sudo]"
	@echo "  make debug    -> Compiles in Debug mode"
	@echo "  make deps     -> Installs system dependencies"
	@echo "  make clean    -> Deletes 'build' folder to recompile"
	@echo "  make uninstall -> Removes Trinity from system"
	@echo "  make translations -> Updates .ts translation files"
	@echo ""
