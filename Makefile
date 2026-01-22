# ==========================================
# 🎮 Trinity Launcher - Makefile Wrapper
# ==========================================

# Variables
SCRIPT = ./build.sh
# Forzamos el uso de bash para evitar errores de sintaxis (sh/dash)
SHELL_CMD = bash

# Targets (comandos) disponibles
.PHONY: all build debug deps clean run translations help start uninstall install

# 1. Comando por defecto (solo 'make') -> Compila en modo Release
all: build

# 2. Compilación Release (Solo genera los binarios en build/)
build:
	@$(SHELL_CMD) $(SCRIPT) --release

# 3. Compilación Debug (Con símbolos para gdb/lldb)
debug:
	@$(SHELL_CMD) $(SCRIPT) --debug

# 4. Instalar Dependencias (Detecta distro y usa sudo)
deps:
	@$(SHELL_CMD) $(SCRIPT) --deps-only

# 5. Limpiar proyecto (Borra carpeta build/ y repara permisos si es necesario)
clean:
	@$(SHELL_CMD) $(SCRIPT) --clean
# 6. Ejecutar el Launcher (Compila Release + Ejecuta en LOCAL)
# Nota: Ideal para desarrollo, no pide contraseña ni instala en sistema.
run:
	@$(SHELL_CMD) $(SCRIPT) --release --run

# 7. Actualizar Traducciones (.ts)
translations:
	@$(SHELL_CMD) $(SCRIPT) --update-ts

# 8. Setup completo (Deps + Build + Run)
# Nota: Ideal para la primera vez que clonas el repo.
start:
	@$(SHELL_CMD) $(SCRIPT) --deps --release --run --detached

# 9. Instalar en el Sistema (/usr/local/bin)
# Nota: Requiere contraseña sudo. Copia los binarios para uso global.
install:
	@$(SHELL_CMD) $(SCRIPT) --release --install --detached

# 10. Desinstalar launcher (Elimina de /usr/local/bin , accesos directos y build/)
uninstall:
	@$(SHELL_CMD) $(SCRIPT) --clean-only
	@$(SHELL_CMD) $(SCRIPT) --uninstall

help:
	@echo "🛠️  Comandos disponibles en Trinity Launcher:"
	@echo ""
	@echo "  make          -> Compila el proyecto en modo Release (Igual a 'make build')"
	@echo "  make start    -> Flujo completo: Deps + Build + Run (Recomendado primera vez)"
	@echo "  make run      -> Compila y ejecuta la versión LOCAL (Rápido para desarrollo)"
	@echo "  make install  -> Compila e INSTALA en el sistema (/usr/local/bin) [Pide Sudo]"
	@echo "  make debug    -> Compila en modo Debug"
	@echo "  make deps     -> Instala dependencias del sistema"
	@echo "  make clean    -> Borra la carpeta 'build y recompilar"
	@echo "  make uninstall -> Elimina Trinity del sistema"
	@echo "  make translations -> Actualiza archivos de traducción .ts"
	@echo ""
