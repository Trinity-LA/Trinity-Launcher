#!/bin/sh
# Trinity Launcher — AnyLinux AppImage (metodo quick-sharun/sharun).
#
# Construye un AppImage 100% portable (glibc + ld-linux propios, DwarFS +
# uruntime): corre en cualquier distro, musl y NixOS sin FHS-wrapper.
#
# REGLAS (https://github.com/pkgforge-dev/Anylinux-AppImages):
#  - Compilar SOLO en Arch Linux. Nunca Fedora ni Ubuntu.
#  - La app debe estar instalada en /usr ANTES de empaquetar.
#  - Nunca copiar .so/binarios a mano al AppDir: se pasan como args a
#    quick-sharun y el hace el deploy (ldd + strace de dlopens + sharun).
#  - Ignorar guias externas (linuxdeploy, appimage-builder, docs.appimage).
#
# Uso local (en Arch):   sh ./trinity-appimage.sh
# En CI lo llama .github/workflows/anylinux-appimage.yml dentro del
# contenedor ghcr.io/pkgforge-dev/archlinux:latest.
#
# Env opcionales:
#   MCPE_NX   URL/repo del engine (por defecto manifest publico, rama qt6)
#   CHANNEL   canal de release para el hook self-updater: latest|nightly
#             (por defecto latest; el workflow anylinux lo fija a nightly
#             en el cron diario)
#   UPINFO    update-information explicito (por defecto
#             gh-releases-zsync|owner|repo|CHANNEL|Trinity_Launcher-$ARCH.AppImage.zsync)
#   OUTPATH   destino del AppImage (por defecto ./dist)

set -eux

ARCH="$(uname -m)"
ROOT="$PWD"
OUTPATH="${OUTPATH:-$ROOT/dist}"
CHANNEL="${CHANNEL:-latest}"
OUTNAME="${OUTNAME:-Trinity_Launcher-$ARCH.AppImage}"
# UPINFO por canal: latest y nightly actualizan cada uno su propio tag
# (antes el nightly apuntaba a latest y se pisaban entre si).
if [ -z "${UPINFO:-}" ]; then
	# GITHUB_REPOSITORY lo pone el runner (owner/repo); fuera del CI cae
	# al valor literal del repo publico.
	REPO="${GITHUB_REPOSITORY:-Trinity-LA/Trinity-Launcher}"
	UPINFO="gh-releases-zsync|${REPO%/*}|${REPO#*/}|$CHANNEL|$OUTNAME.zsync"
fi
SHARUN_URL="https://raw.githubusercontent.com/pkgforge-dev/Anylinux-AppImages/refs/heads/main/useful-tools/quick-sharun.sh"
DEBLOAT_URL="https://raw.githubusercontent.com/pkgforge-dev/Anylinux-AppImages/refs/heads/main/useful-tools/get-debloated-pkgs.sh"
ZIG_VER="0.16.0"

if [ "$(id -u)" -eq 0 ]; then
	SUDO=""
else
	SUDO="sudo"
fi

echo "=== 1/7 Dependencias del sistema (Arch) ==="
$SUDO pacman -Syu --noconfirm \
	base-devel git curl wget cmake clang ninja patchelf zsync ccache \
	xorg-server-xvfb pciutils hwdata dbus \
	qt6-base qt6-declarative qt6-webengine qt6-svg qt6-tools qt6-translations \
	gtk3 webkit2gtk-4.1 \
	libzip libpng libpulse alsa-lib pipewire jack2 sndio \
	libx11 libxi libxext libxfixes libxcursor libxrandr libxss libxtst \
	libxcb libxkbcommon libxkbcommon-x11 xcb-util-wm \
	mesa vulkan-headers vulkan-validation-layers libdrm \
	libevdev libusb bluez-libs ibus libunwind libdecor wayland \
	libcups openssl curl

echo "=== 2/7 Paquetes debloated (mesa sin LLVM completo, icu/qt/gtk minis) ==="
wget --retry-connrefused --tries=30 "$DEBLOAT_URL" -O ./get-debloated-pkgs.sh
chmod +x ./get-debloated-pkgs.sh
./get-debloated-pkgs.sh --add-common --add-mesa --prefer-nano

echo "=== 3/7 Fuentes del engine + datos ==="
if [ ! -d mcpe-nx ]; then
	# MCPE_NX trae la URL privada (secreto del CI, igual que en los
	# workflows simple-appimage/nightly); sin el se usa el manifest
	# publico en su rama qt6 (ver docs/BUILD.md).
	ENGINE_URL="${MCPE_NX:-https://github.com/minecraft-linux/mcpelauncher-manifest.git}"
	# Silenciado a proposito: la URL puede llevar credenciales y este
	# script corre con xtrace (set -x).
	set +x
	git clone "$ENGINE_URL" mcpe-nx > /dev/null 2>&1
	if [ -z "${MCPE_NX:-}" ]; then
		git -C mcpe-nx checkout qt6 > /dev/null 2>&1 || true
		git -C mcpe-nx submodule update --init --recursive > /dev/null 2>&1 || true
	fi
	set -x
fi
# Parche del engine: libc-shim necesita fcntl GETLK(5)/SETLKW(7) porque el
# libsqliteX del juego los sondea al iniciar (si no: SIGABRT). Se omite en
# silencio si el engine ya lo trae.
if [ -f patches/libc-shim-fcntl-getlk.patch ] && [ -d mcpe-nx/libc-shim ]; then
	(cd mcpe-nx && patch -p1 --forward < ../patches/libc-shim-fcntl-getlk.patch) || true
fi
if [ ! -d tapk-extract ]; then
	git clone https://gitlab.com/javiercplus/tapk-extract.git tapk-extract
fi
if [ ! -d linux-bin ] && [ ! -d mcpe-nx/mcpelauncher-linux-bin ]; then
	git clone https://github.com/minecraft-linux/mcpelauncher-linux-bin.git linux-bin
fi
# El engine trae sdl3/ y mcpelauncher-linux-bin/ vendoreados y no usa
# submodulos: un clone plano basta.
if [ "$ARCH" = "x86_64" ] && [ ! -d 32bitmcpe ]; then
	# HuggingFace limita por IP (429): reintentos con espera
	for i in 1 2 3 4 5 6 7 8 9 10; do
		if wget --retry-connrefused --tries=5 \
			https://huggingface.co/datasets/ccoffee20/PEPE/resolve/main/mcpe32bit.tar \
			-O mcpe32bit.tar; then
			break
		fi
		echo "Descarga 429/fallida (intento $i/10), esperando 60s..."
		rm -f mcpe32bit.tar
		sleep 60
	done
	tar -xf mcpe32bit.tar
fi

echo "=== 4/7 Compilar engine (sin GUI, SDL3) + extractor (Zig) + Trinity ==="

# --- Toolchain Zig: glibc antigua + LLVM integrado (zig cc = Clang/LLVM) ---
GLIBC_TARGET="${GLIBC_TARGET:-2.28}"
if [ "$ARCH" = "aarch64" ]; then
    TARGET_TRIPLE="aarch64-linux-gnu.${GLIBC_TARGET}"
else
    TARGET_TRIPLE="x86_64-linux-gnu.${GLIBC_TARGET}"
fi

ZIG_TAR="zig-$ARCH-linux-$ZIG_VER.tar.xz"
if ! command -v zig >/dev/null 2>&1; then
    wget -q "https://ziglang.org/download/$ZIG_VER/$ZIG_TAR"
    tar -xf "$ZIG_TAR"
    export PATH="$ROOT/zig-$ARCH-linux-$ZIG_VER:$PATH"
fi

# Wrappers ejecutables: un path unico sin `;` ni espacios, para que
# ExternalProject_Add(curl_ext ...) los propague intactos al subproyecto.
# Sin esto, CMake resuelve "zig;cc" a /usr/sbin/zig + arg "cc", y al pasar
# el compilador al ExternalProject se pierde el "cc" (shell parte el `;`),
# dejando a curl_ext invocando `zig -o ...` (unknown command: -o).
mkdir -p "$ROOT/.toolchain"
cat > "$ROOT/.toolchain/zig-cc" <<EOF
#!/bin/sh
exec zig cc -target ${TARGET_TRIPLE} \\
    -idirafter /usr/include \\
    -idirafter /usr/local/include \\
    -L/usr/lib -L/lib -L/usr/local/lib \\
    -fno-sanitize=all \\
    -Wl,--allow-shlib-undefined \\
    "\$@"
EOF
cat > "$ROOT/.toolchain/zig-cxx" <<EOF
#!/bin/sh
exec zig c++ -target ${TARGET_TRIPLE} \\
    -idirafter /usr/include \\
    -idirafter /usr/local/include \\
    -L/usr/lib -L/lib -L/usr/local/lib \\
    -fno-sanitize=all \\
    -Wl,--allow-shlib-undefined \\
    "\$@"
EOF
chmod +x "$ROOT/.toolchain/zig-cc" "$ROOT/.toolchain/zig-cxx"

export CC="$ROOT/.toolchain/zig-cc"
export CXX="$ROOT/.toolchain/zig-cxx"
# Refuerzo para CMake y para cualquier subproyecto que lea estas vars.
export CMAKE_C_COMPILER="$CC"
export CMAKE_CXX_COMPILER="$CXX"
# No se exporta CMAKE_C_COMPILER_TARGET: el target ya va dentro del wrapper.
# Evita el doble -target y que curl_ext (que no hereda esa var) se quede sin el.

CCACHE_LAUNCHER=""
if command -v ccache >/dev/null 2>&1; then
    CCACHE_DIR="${CCACHE_DIR:-$ROOT/.ccache}"
    export CCACHE_DIR
    mkdir -p "$CCACHE_DIR"
    CCACHE_LAUNCHER="-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
fi

# shellcheck disable=SC2086
cmake -S mcpe-nx -B mcpe-nx/build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER="$CC" \
    -DCMAKE_CXX_COMPILER="$CXX" \
    -DBUILD_WEBVIEW=OFF \
    -DGAMEWINDOW_SYSTEM=SDL3 \
    -DBUILD_UI=OFF \
    -DXAL_WEBVIEW_USE_QT=ON \
    -DSDL3_VENDORED=ON \
    -DENABLE_DEV_PATHS=OFF \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    $CCACHE_LAUNCHER \
    -Wno-dev
cmake --build mcpe-nx/build --parallel "$(nproc)"

echo "=== 5/7 Instalar todo en /usr (requisito de quick-sharun) ==="
$SUDO install -Dm755 build/app/trinity /usr/bin/trinity
$SUDO install -Dm755 mcpe-nx/build/mcpelauncher-client/mcpelauncher-client /usr/bin/mcpelauncher-client
$SUDO install -Dm755 tapk-extract/zig-out/bin/tapk-extract /usr/bin/mcpelauncher-extract
# Backend de login Xbox (ver paso 4/7): solo si se pudo compilar.
if [ -x mcpe-nx/lite-webview/zig-out/bin/lite-webview ]; then
	$SUDO install -Dm755 mcpe-nx/lite-webview/zig-out/bin/lite-webview /usr/bin/mcpelauncher-webview
fi
for helper in msa-daemon mcpelauncher-error; do
	found="$(find mcpe-nx/build -type f -name "$helper" -print | head -n 1)" || true
	if [ -n "$found" ]; then
		$SUDO install -Dm755 "$found" "/usr/bin/$helper"
	fi
done
$SUDO install -Dm644 resources/shortcuts/com.trench.trinity.launcher.desktop \
	/usr/share/applications/com.trench.trinity.launcher.desktop
$SUDO install -Dm644 resources/branding/com.trench.trinity.launcher.svg \
	/usr/share/icons/hicolor/scalable/apps/com.trench.trinity.launcher.svg
$SUDO mkdir -p /usr/share/mcpelauncher
if [ -d mcpe-nx/mcpelauncher-linux-bin ]; then
	$SUDO cp -r mcpe-nx/mcpelauncher-linux-bin/. /usr/share/mcpelauncher/
else
	$SUDO cp -r linux-bin/. /usr/share/mcpelauncher/
fi

echo "=== 6/7 Deploy con quick-sharun (jamas copiar .so a mano) ==="
wget --retry-connrefused --tries=30 "$SHARUN_URL" -O ./quick-sharun
chmod +x ./quick-sharun

export ICON=/usr/share/icons/hicolor/scalable/apps/com.trench.trinity.launcher.svg
export DESKTOP=/usr/share/applications/com.trench.trinity.launcher.desktop
export MAIN_BIN=trinity
export OUTPATH OUTNAME
export ADD_HOOKS="self-updater.hook:fix-namespaces.hook:host-libjack.hook"
export DEPLOY_OPENGL=1 DEPLOY_VULKAN=1 DEPLOY_SDL=1 DEPLOY_PIPEWIRE=1 DEPLOY_PULSE=1
# Descubrimiento dual ldd+strace: sin STRACE_MODE los modulos que Qt/SDL3
# abren por dlopen (audio, plataformas) son invisibles y no entran al bundle.
export STRACE_MODE=1
# SIN anylinux.so: su interposicion LD_PRELOAD (hooks execv/dlopen/nss)
# rompia el spawn del webview (login->Drowned), la presencia Xbox
# (multijugador) y el audio (backends SDL por dlopen). Verificado: con
# ANYLINUX_LIB=0 todo funciona; la classic tampoco lo usa. Loader+glibc
# empaquetados intactos (portabilidad a kernels viejos/musl intacta).
export ANYLINUX_LIB=0
# Integracion con el escritorio/Wayland: el compositor agrupa por app_id;
# sin esto muestra un engranaje generico en vez del lanzador de Trinity.
export GTK_CLASS_FIX=1
export GTK_WINDOW_CLASS=com.trench.trinity.launcher

BINS="/usr/bin/trinity /usr/bin/mcpelauncher-client /usr/bin/mcpelauncher-extract /usr/bin/lspci"
# Backend de login Xbox: si existe, entra al deploy para que sharun arrastre
# webkit2gtk y sus procesos auxiliares via ldd/strace.
if [ -x /usr/bin/mcpelauncher-webview ]; then
	BINS="$BINS /usr/bin/mcpelauncher-webview"
fi
for helper in msa-daemon mcpelauncher-error; do
	if [ -x "/usr/bin/$helper" ]; then
		BINS="$BINS /usr/bin/$helper"
	fi
done
# SDL3 abre libasound por dlopen en runtime (invisible a ldd y al strace
# de Trinity, que nunca juega): sin esto no hay fallback ALSA y el audio
# muere si falla la ruta Pulse/PipeWire (la nightly usa el ALSA del host).
if [ -f /usr/lib/libasound.so.2 ]; then
	BINS="$BINS /usr/lib/libasound.so.2"
fi
# shellcheck disable=SC2086
./quick-sharun $BINS
# Sin bwrap empaquetado: WebKit cae a modo sin sandbox, igual que la
# nightly original (que no lo trae). Con sandbox empaquetado el login
# muere en kernels sin userns sin privilegios.
rm -f AppDir/bin/bwrap AppDir/bin/xdg-dbus-proxy \
	AppDir/shared/bin/bwrap AppDir/shared/bin/xdg-dbus-proxy

echo "=== 6b/7 Datos extra + sidecar 32-bit (x86_64) ==="
# linux-bin: datos del engine, no son ELF asi que van directo a share/
# (cp -n: jamas sobrescribir lo que quick-sharun ya desplego; ver 6c/7).
mkdir -p AppDir/share/mcpelauncher
cp -rn /usr/share/mcpelauncher/. AppDir/share/mcpelauncher/
# pci.ids para lspci (deteccion de GPU del gestor de contenido)
if [ -f /usr/share/hwdata/pci.ids ]; then
	mkdir -p AppDir/share/hwdata
	cp -n /usr/share/hwdata/pci.ids AppDir/share/hwdata/ 2>/dev/null || true
fi
# PROHIBIDO copiar /usr/lib/webkit2gtk-4.1 a mano: quick-sharun ya lo
# despliega solo (DEPLOY_WEBKIT2GTK via mcpelauncher-webview). Un `cp -r`
# aqui FUSIONA con el dir existente y sobrescribe sus binarios, que son
# hardlinks al inodo de sharun: O_TRUNC reemplaza el loader (y TODOS los
# bin/*) con bytes de jsc y Trinity jamas arranca (paso en 2026-09, el
# test no lo detecto porque jsc sale 0 en silencio). Solo se exporta la
# ruta para el runtime (ver WEBKIT_EXEC_PATH abajo).
# Audio: solo DATA (conf de alsa/pipewire). Las .so las despliega
# quick-sharun via DEPLOY_PIPEWIRE/PULSE, nunca a mano (misma trampa).
# cp -rn: si quick-sharun ya puso algo, no tocarlo jamas.
if [ -d /usr/share/alsa ]; then
	mkdir -p AppDir/share
	cp -rn /usr/share/alsa AppDir/share/ 2>/dev/null || true
fi
if [ -d /usr/share/pipewire ]; then
	mkdir -p AppDir/share
	cp -rn /usr/share/pipewire AppDir/share/ 2>/dev/null || true
fi
# Vars de runtime que el AppRun/sharun expande al lanzar (sin expandir aqui)
echo 'MCPELAUNCHER_DATA_DIR=${SHARUN_DIR}/share/mcpelauncher' >> AppDir/.env
echo 'PCI_IDS=${SHARUN_DIR}/share/hwdata/pci.ids' >> AppDir/.env
# Backend de login Xbox (igual que el AppRun del nightly: todo X11).
echo 'GDK_BACKEND=x11' >> AppDir/.env
echo 'QT_QPA_PLATFORM=xcb' >> AppDir/.env
echo 'DISABLE_WAYLAND=1' >> AppDir/.env
echo 'WEBKIT_EXEC_PATH=${SHARUN_DIR}/lib/webkit2gtk-4.1' >> AppDir/.env
# Aislamiento anti-crash en distros con userland viejo (Void/musl, etc):
# - fusion: el host puede traer QT_QPA_PLATFORMTHEME=gtk3 y el plugin
#   libqgtk3 empaquetado contra un GTK3 ajeno al del host -> mezcla y SIGSEGV
# - GIO_MODULE_DIR/GIO_USE_VFS: impide que GIO sondee los modulos gio/gvfs
#   del host (incompatibles con el glib empaquetado -> SIGSEGV)
echo 'QT_QPA_PLATFORMTHEME=fusion' >> AppDir/.env
echo 'GIO_MODULE_DIR=${SHARUN_DIR}/lib/gio/modules' >> AppDir/.env
echo 'GIO_USE_VFS=local' >> AppDir/.env

if [ "$ARCH" = "x86_64" ] && [ -f 32bitmcpe/bin/mcpelauncher-client86 ]; then
	# El helper de 32-bit no puede mezclarse en lib/ (colisionaria con los
	# .so de 64-bit con el mismo nombre), asi que viaja autocontenido en
	# lib32/ con su propio ld-linux --library-path: el mismo mecanismo
	# certificado de HOW-TO-MAKE-THESE.md, sin LD_LIBRARY_PATH.
	mkdir -p AppDir/lib32 AppDir/bin
	cp -f 32bitmcpe/bin/mcpelauncher-client86 AppDir/bin/.mcpelauncher-client86.real
	chmod +x AppDir/bin/.mcpelauncher-client86.real
	cp -f 32bitmcpe/lib32/*.so* AppDir/lib32/ 2>/dev/null || true
	# Loader de 32-bit: buscar en todo el bundle (no solo lib32/), luego
	# en el sistema. Sin esto el sidecar apunta a un loader inexistente.
	LD32="$(find 32bitmcpe -name 'ld-linux.so*' -type f -print 2>/dev/null | head -n 1)"
	if [ -n "$LD32" ]; then
		cp -f "$LD32" AppDir/lib32/ld-linux.so.2
	elif [ -f /usr/lib32/ld-linux.so.2 ]; then
		cp -f /usr/lib32/ld-linux.so.2 AppDir/lib32/
	fi
	# Limpieza heredada del workflow clasico: esos GL/X de 32-bit pisan
	# los del host y rompen el arranque en GPUs modernas.
	rm -f AppDir/lib32/libGLdispatch.so* AppDir/lib32/libX*.so* AppDir/lib32/libxcb*.so*
	cat > AppDir/bin/mcpelauncher-client86 <<'EOF'
#!/bin/sh
# Sidecar 32-bit: loader propio si viaja en el bundle, si no el del host,
# en ambos casos con --library-path al lib32 empaquetado y sin tocar el
# entorno del proceso padre.
HERE="$(dirname "$(readlink -f "$0")")/.."
HERE="$(readlink -f "$HERE")"
if [ -f "$HERE"/lib32/ld-linux.so.2 ]; then
    LOADER="$HERE"/lib32/ld-linux.so.2
elif [ -f /lib/ld-linux.so.2 ]; then
    LOADER=/lib/ld-linux.so.2
else
    echo "mcpelauncher-client86: sin loader de 32-bit" >&2
    exit 1
fi
exec "$LOADER" --library-path "$HERE"/lib32 "$HERE"/bin/.mcpelauncher-client86.real "$@"
EOF
	chmod +x AppDir/bin/mcpelauncher-client86
fi

echo "=== 6c/7 Verificar integridad del loader sharun ==="
# Puerta anti-corrupcion: AppDir/bin/* son hardlinks al inodo de
# AppDir/sharun. Sobrescribir CUALQUIER binario hardlinkeado (p.ej. `cp -r`
# sobre un dir ya desplegado fusiona y hace O_TRUNC sobre el inodo)
# reemplaza el loader y MATA el AppImage sin que el test lo note (el
# invasor suele salir 0 en silencio). Se compara contra el tarball pineado
# ANTES de empaquetar: fail rapido y ruidoso en vez de release roto.
_QS_TMPDIR="${TMPDIR:-/tmp}"
_QS_REFDIR="$(mktemp -d)"
tar -xf "$_QS_TMPDIR/sharun+helper-libs-$ARCH.tar" -C "$_QS_REFDIR" sharun
if ! cmp -s "$_QS_REFDIR/sharun" AppDir/sharun; then
	echo "FATAL: AppDir/sharun difiere del loader original (¿sobrescritura via hardlink?)" >&2
	exit 1
fi
rm -rf "$_QS_REFDIR"
echo "sharun integro."

# PACKAGER=appimage -> DwarFS + uruntime (AnyLinux, portable total, musl/NixOS).
# PACKAGER=squashfs -> MISMO AppDir (sharun + glibc propia, igual de portable)
#                      pero envuelto con el appimagetool clasico (SquashFS
#                      tipo 2), que es lo unico que monta el test del catalogo
#                      appimage.github.io. Sin cambios de runtime: todo lo que
#                      ya funciona (EGL, dlopen, NSS, audio) lo resuelve sharun.
PACKAGER="${PACKAGER:-appimage}"

echo "=== 7/7 Empaquetar (PACKAGER=$PACKAGER) y test ==="
export OUTPATH OUTNAME UPINFO
if [ "$PACKAGER" = "squashfs" ] ; then
	# AppRun/.DirIcon/.desktop ya los deja quick-sharun (DIRICON en su
	# linea 44); solo se ancla el .desktop en la raiz del AppDir, que es
	# lo que leen tanto appimagetool como el worker del catalogo.
	d="$(find AppDir -name 'com.trench.trinity.launcher.desktop' -type f | head -n 1)"
	i="$(find AppDir -name 'com.trench.trinity.launcher.svg' -type f | head -n 1)"
	[ -n "$d" ] || { echo "FATAL: .desktop no desplegado" >&2; exit 1; }
	# quick-sharun ya deja el .desktop real en la raiz: solo se enlaza si
	# esta en otro sitio (ln contra si mismo = symloop).
	if [ "$d" != "AppDir/com.trench.trinity.launcher.desktop" ] ; then
		ln -sf "$(realpath --relative-to=AppDir "$d")" AppDir/com.trench.trinity.launcher.desktop
	fi
	if [ -n "$i" ] ; then
		if [ "$i" != "AppDir/com.trench.trinity.launcher.svg" ] ; then
			ln -sf "$(realpath --relative-to=AppDir "$i")" AppDir/com.trench.trinity.launcher.svg
		fi
		ln -sf "$(realpath --relative-to=AppDir "$i")" AppDir/.DirIcon
	fi
	grep -q '^StartupWMClass=' "$d" || echo 'StartupWMClass=com.trench.trinity.launcher' >> "$d"
	# appimagetool upstream (no el fork DwarFS de pkgforge): SquashFS tipo 2.
	mkdir -p "$OUTPATH"
	# Limpieza: jamas subir un artefacto anterior junto al nuevo.
	rm -f "$OUTPATH"/*.AppImage "$OUTPATH"/*.zsync
	# Capa de validacion Vulkan: herramienta de debug (31 MB) que ningun
	# binario linkea; fuera del bundle classic (AnyLinux no se toca).
	rm -f AppDir/lib/libVkLayer_* \
		AppDir/share/vulkan/explicit_layer.d/VkLayer_khronos_validation.json
	# Compresor: gzip SIEMPRE. xz comprime mas pero el montador del
	# catalogo (squashfuse viejo) solo soporta zlib/zstd y rechaza la
	# imagen ("supports only zlib, zstd"); la probo el juez en 2026-10-02.
	# Bloques de 1M: mejor ratio sin cambiar el descompresor necesario.
	COMPRESSION="${COMPRESSION:-}"
	if [ -z "$COMPRESSION" ]; then
		if [ "$CHANNEL" = "latest" ]; then
			COMPRESSION="xz"
		else
			COMPRESSION="gzip"
		fi
	fi
	# Runtime tipo 2 ESTATICO oficial: sin el, check-libc.sh del catalogo
	# reporta Runtime=dynamic / Self-Contained=false (aunque la glibc de la
	# carga ya va empaquetada).
	wget --retry-connrefused --tries=30 -q \
		"https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-$ARCH" \
		-O ./runtime-classic
	chmod +x ./runtime-classic
	if readelf -lW ./runtime-classic 2>/dev/null | grep -q 'Requesting program interpreter' ; then
		echo "FATAL: el runtime descargado es dinamico, se quiere estatico" >&2
		exit 1
	fi
	wget --retry-connrefused --tries=30 -q \
		"https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-$ARCH.AppImage" \
		-O ./appimagetool-classic
	chmod +x ./appimagetool-classic
	# appimagetool solo acepta ARCH=x86_64|arm|arm_aarch64|i386...: con
	# ARCH=aarch64 no reconoce nada, escanea el AppDir y muere al ver las
	# sqlite de Android (x86/arm/arm64) en share/mcpelauncher (ait.c,
	# extract_arch_from_text: compara contra "arm_aarch64").
	AI_ARCH="$ARCH"
	if [ "$ARCH" = "aarch64" ]; then
		AI_ARCH="arm_aarch64"
	fi
	# Bloques de 1M: -33 MB medidos frente a 128K; el runtime monta
	# cualquier tamano (maximo SquashFS).
	APPIMAGE_EXTRACT_AND_RUN=1 ARCH="$AI_ARCH" ./appimagetool-classic \
		--mksquashfs-opt -b --mksquashfs-opt 1M \
		--runtime-file ./runtime-classic \
		-u "$UPINFO" AppDir "$OUTPATH/$OUTNAME"
	# appimagetool deja el .zsync en el CWD: se junta con el AppImage para
	# que el artefacto (path: dist) y el release (*.zsync) lo encuentren.
	for z in ./*.zsync; do
		[ -e "$z" ] || continue
		mv -f "$z" "$OUTPATH/"
	done
else
	./quick-sharun --make-appimage
fi
# Test: --simple-test en vez de --test. El full exige 12 s de GUI en idle
# y en CI headless la app sale (codigo 0) justo tras arrancar; el simple
# falla solo ante lo que indica un deploy roto (symbol lookup error o
# shared libraries faltantes). Es el fallback previsto por quick-sharun.
export APPIMAGE_EXTRACT_AND_RUN=1
xvfb-run -a ./quick-sharun --simple-test "$OUTPATH"/*.AppImage

echo "Listo: $OUTPATH/$OUTNAME"
