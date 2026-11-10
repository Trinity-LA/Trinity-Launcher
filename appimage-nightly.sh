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
#   MCPE_NX        URL/repo del engine (por defecto manifest publico, rama qt6)
#   CHANNEL        canal de release para el hook self-updater: latest|nightly
#   UPINFO         update-information explicito
#   OUTPATH        destino del AppImage (por defecto ./dist)
#   GLIBC_TARGET   version de glibc objetivo (por defecto 2.28 = Ubuntu 18.04,
#                  compatible con Linux Mint 19/20/21 y Debian 10+)
#   COMPRESSION    compresor SquashFS (xz|gzip|zstd), solo PACKAGER=squashfs

set -eux

ARCH="$(uname -m)"
ROOT="$PWD"
OUTPATH="${OUTPATH:-$ROOT/dist}"
CHANNEL="${CHANNEL:-latest}"
OUTNAME="${OUTNAME:-Trinity_Launcher-$ARCH.AppImage}"
if [ -z "${UPINFO:-}" ]; then
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
	base-devel git curl wget cmake clang ninja patchelf zsync ccache perl \
	xorg-server-xvfb pciutils hwdata dbus \
	qt6-base qt6-declarative qt6-webengine qt6-svg qt6-tools qt6-translations \
	gtk3 webkit2gtk-4.1 \
	libzip libpng zlib libpulse alsa-lib pipewire jack2 sndio \
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
	ENGINE_URL="${MCPE_NX:-https://github.com/minecraft-linux/mcpelauncher-manifest.git}"
	set +x
	git clone "$ENGINE_URL" mcpe-nx > /dev/null 2>&1
	if [ -z "${MCPE_NX:-}" ]; then
		git -C mcpe-nx checkout qt6 > /dev/null 2>&1 || true
		git -C mcpe-nx submodule update --init --recursive > /dev/null 2>&1 || true
	fi
	set -x
fi
if [ -f patches/libc-shim-fcntl-getlk.patch ] && [ -d mcpe-nx/libc-shim ]; then
	(cd mcpe-nx && patch -p1 --forward < ../patches/libc-shim-fcntl-getlk.patch) || true
fi
if [ ! -d tapk-extract ]; then
	git clone https://gitlab.com/javiercplus/tapk-extract.git tapk-extract
fi
if [ ! -d linux-bin ] && [ ! -d mcpe-nx/mcpelauncher-linux-bin ]; then
	git clone https://github.com/minecraft-linux/mcpelauncher-linux-bin.git linux-bin
fi
if [ "$ARCH" = "x86_64" ] && [ ! -d 32bitmcpe ]; then
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

# Wrappers ejecutables (path unico, sin `;` ni espacios, para que
# ExternalProject_Add(curl_ext ...) los propague intactos al subproyecto).
# -idirafter /usr/include: headers no-libc (zlib, openssl, libpng) del host.
# -L/usr/lib -L/lib:       librerias de sistema que Zig no empaqueta.
# -fno-sanitize=all:       libcurl se compila con UBSan en ReleaseFast; sin
#                          esto el linker final pide __ubsan_handle_* que no
#                          estan enlazados.
# -Wl,--allow-shlib-undefined: libssl/libpng de Arch piden GLIBC_2.29+;
#                          el binario se enlaza contra 2.28 y en runtime
#                          sharun resuelve contra su glibc reciente.
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
export CMAKE_C_COMPILER="$CC"
export CMAKE_CXX_COMPILER="$CXX"

CCACHE_LAUNCHER=""
if command -v ccache >/dev/null 2>&1; then
	CCACHE_DIR="${CCACHE_DIR:-$ROOT/.ccache}"
	export CCACHE_DIR
	mkdir -p "$CCACHE_DIR"
	CCACHE_LAUNCHER="-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
fi

# --- 4a. Engine mcpelauncher (sin GUI, SDL3) ---
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

# --- 4b. tapk-extract (Zig build, target glibc 2.28) ---
ZIG_CACHE_DIR="$ROOT/.zig-cache"
mkdir -p "$ZIG_CACHE_DIR/tapk-local" "$ZIG_CACHE_DIR/webview-local" "$ZIG_CACHE_DIR/global"
(cd tapk-extract && zig build --release=fast \
	-Dtarget="$TARGET_TRIPLE" -Dcpu=baseline \
	--cache-dir "$ZIG_CACHE_DIR/tapk-local" \
	--global-cache-dir "$ZIG_CACHE_DIR/global")

# --- 4c. lite-webview (backend de login Xbox, opcional) ---
# Con -target, zig build-exe aisla el sysroot y no busca librerias en /usr/lib,
# asi que webkit2gtk/gtk-3/etc no se encuentran aunque pkg-config de sus -I.
# Se parchea build.zig para que anada addLibraryPath(/usr/lib, /lib) y ademas
# se pasa --search-prefix como refuerzo. Si aun asi falla, se omite (login cae
# al backend Qt compilado con XAL_WEBVIEW_USE_QT=ON en el engine).
LW_DIR="mcpe-nx/lite-webview"
LW_SKIP=0
if [ -d "$LW_DIR" ] && [ -f "$LW_DIR/build.zig" ]; then
	if ! grep -q 'addLibraryPath.*"/usr/lib"' "$LW_DIR/build.zig"; then
		echo "Parcheando $LW_DIR/build.zig con rutas de librerias del sistema..."
		# Inserta addLibraryPath justo despues de exe.linkLibC() / linkLibCpp().
		perl -i -pe 's|^(\s*)(exe\.linkLibC(?:pp)?\(\);)|$1$2\n$1exe.addLibraryPath(.{ .cwd_relative = "/usr/lib" });\n$1exe.addLibraryPath(.{ .cwd_relative = "/lib" });\n$1exe.addLibraryPath(.{ .cwd_relative = "/usr/local/lib" });|g' "$LW_DIR/build.zig"
	fi
	if ! grep -q 'addLibraryPath.*"/usr/lib"' "$LW_DIR/build.zig"; then
		# Fallback: insertar antes del primer linkSystemLibrary
		perl -i -pe 's|^(\s*)(exe\.linkSystemLibrary)|\1exe.addLibraryPath(.{ .cwd_relative = "/usr/lib" });\n\1exe.addLibraryPath(.{ .cwd_relative = "/lib" });\n\1\2|' "$LW_DIR/build.zig"
	fi
	if grep -q 'addLibraryPath.*"/usr/lib"' "$LW_DIR/build.zig"; then
		echo "OK: build.zig de lite-webview parcheado"
	else
		echo "WARN: no se pudo parchear build.zig de lite-webview; se intentara sin patch"
	fi

	if ! (cd "$LW_DIR" && zig build --release=fast \
		-Dtarget="$TARGET_TRIPLE" -Dcpu=baseline \
		--search-prefix /usr --search-prefix /lib \
		--cache-dir "$ZIG_CACHE_DIR/webview-local" \
		--global-cache-dir "$ZIG_CACHE_DIR/global"); then
		echo "WARN: lite-webview fallo al compilar; se omite (login usara Qt fallback)"
		LW_SKIP=1
	fi
fi

# --- 4d. Limpieza de flags x86 SIMD en ARM (SSE/AVX rompen el build aarch64) ---
if [ "$ARCH" != "x86_64" ]; then
	find . -maxdepth 3 -name "CMakeLists.txt" -exec \
		sed -i 's/-msse3\b//g;s/-msse4[^ ]*//g;s/-mavx[^ ]*//g' {} \;
	sed -i 's/-msse3\b//g;s/-msse4[^ ]*//g;s/-mavx[^ ]*//g' build.sh 2>/dev/null || true
fi

# --- 4e. Launcher Trinity (proyecto raiz, GUI Qt6) ---
if [ ! -f CMakeLists.txt ]; then
	echo "FATAL: no hay CMakeLists.txt en $PWD (¿clone incompleto de Trinity?)" >&2
	exit 1
fi
# shellcheck disable=SC2086
cmake -S . -B build -G Ninja \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER="$CC" \
	-DCMAKE_CXX_COMPILER="$CXX" \
	$CCACHE_LAUNCHER \
	-Wno-dev
cmake --build build --parallel "$(nproc)"

# Descubrimiento dinamico del binario (el layout puede cambiar entre
# versiones de Trinity, y con zig-cc puede no ser exactamente
# build/app/trinity).
TRINITY_BIN="$(find build -maxdepth 4 -type f \
	\( -name 'trinity' -o -name 'Trinity' -o -name 'trinity-launcher' \) \
	-perm -u+x -print -quit 2>/dev/null || true)"
if [ -z "$TRINITY_BIN" ]; then
	echo "FATAL: no se genero el binario 'trinity' bajo build/" >&2
	find build -maxdepth 4 -type f -perm -u+x 2>/dev/null | head -30 >&2
	exit 1
fi
echo "Binario Trinity: $TRINITY_BIN"

echo "=== 5/7 Instalar todo en /usr (requisito de quick-sharun) ==="
$SUDO install -Dm755 "$TRINITY_BIN" /usr/bin/trinity
$SUDO install -Dm755 mcpe-nx/build/mcpelauncher-client/mcpelauncher-client /usr/bin/mcpelauncher-client
$SUDO install -Dm755 tapk-extract/zig-out/bin/tapk-extract /usr/bin/mcpelauncher-extract
if [ "$LW_SKIP" = "0" ] && [ -x "$LW_DIR/zig-out/bin/lite-webview" ]; then
	$SUDO install -Dm755 "$LW_DIR/zig-out/bin/lite-webview" /usr/bin/mcpelauncher-webview
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
export STRACE_MODE=1
export ANYLINUX_LIB=0
export GTK_CLASS_FIX=1
export GTK_WINDOW_CLASS=com.trench.trinity.launcher

BINS="/usr/bin/trinity /usr/bin/mcpelauncher-client /usr/bin/mcpelauncher-extract /usr/bin/lspci"
if [ -x /usr/bin/mcpelauncher-webview ]; then
	BINS="$BINS /usr/bin/mcpelauncher-webview"
fi
for helper in msa-daemon mcpelauncher-error; do
	if [ -x "/usr/bin/$helper" ]; then
		BINS="$BINS /usr/bin/$helper"
	fi
done
if [ -f /usr/lib/libasound.so.2 ]; then
	BINS="$BINS /usr/lib/libasound.so.2"
fi
# shellcheck disable=SC2086
./quick-sharun $BINS
rm -f AppDir/bin/bwrap AppDir/bin/xdg-dbus-proxy \
	AppDir/shared/bin/bwrap AppDir/shared/bin/xdg-dbus-proxy

echo "=== 6b/7 Datos extra + sidecar 32-bit (x86_64) ==="
mkdir -p AppDir/share/mcpelauncher
cp -rn /usr/share/mcpelauncher/. AppDir/share/mcpelauncher/
if [ -f /usr/share/hwdata/pci.ids ]; then
	mkdir -p AppDir/share/hwdata
	cp -n /usr/share/hwdata/pci.ids AppDir/share/hwdata/ 2>/dev/null || true
fi
if [ -d /usr/share/alsa ]; then
	mkdir -p AppDir/share
	cp -rn /usr/share/alsa AppDir/share/ 2>/dev/null || true
fi
if [ -d /usr/share/pipewire ]; then
	mkdir -p AppDir/share
	cp -rn /usr/share/pipewire AppDir/share/ 2>/dev/null || true
fi
echo 'MCPELAUNCHER_DATA_DIR=${SHARUN_DIR}/share/mcpelauncher' >> AppDir/.env
echo 'PCI_IDS=${SHARUN_DIR}/share/hwdata/pci.ids' >> AppDir/.env
echo 'GDK_BACKEND=x11' >> AppDir/.env
echo 'QT_QPA_PLATFORM=xcb' >> AppDir/.env
echo 'DISABLE_WAYLAND=1' >> AppDir/.env
echo 'WEBKIT_EXEC_PATH=${SHARUN_DIR}/lib/webkit2gtk-4.1' >> AppDir/.env
echo 'QT_QPA_PLATFORMTHEME=fusion' >> AppDir/.env
echo 'GIO_MODULE_DIR=${SHARUN_DIR}/lib/gio/modules' >> AppDir/.env
echo 'GIO_USE_VFS=local' >> AppDir/.env

if [ "$ARCH" = "x86_64" ] && [ -f 32bitmcpe/bin/mcpelauncher-client86 ]; then
	mkdir -p AppDir/lib32 AppDir/bin
	cp -f 32bitmcpe/bin/mcpelauncher-client86 AppDir/bin/.mcpelauncher-client86.real
	chmod +x AppDir/bin/.mcpelauncher-client86.real
	cp -f 32bitmcpe/lib32/*.so* AppDir/lib32/ 2>/dev/null || true
	LD32="$(find 32bitmcpe -name 'ld-linux.so*' -type f -print 2>/dev/null | head -n 1)"
	if [ -n "$LD32" ]; then
		cp -f "$LD32" AppDir/lib32/ld-linux.so.2
	elif [ -f /usr/lib32/ld-linux.so.2 ]; then
		cp -f /usr/lib32/ld-linux.so.2 AppDir/lib32/
	fi
	rm -f AppDir/lib32/libGLdispatch.so* AppDir/lib32/libX*.so* AppDir/lib32/libxcb*.so*
	cat > AppDir/bin/mcpelauncher-client86 <<'EOF'
#!/bin/sh
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
_QS_TMPDIR="${TMPDIR:-/tmp}"
_QS_REFDIR="$(mktemp -d)"
tar -xf "$_QS_TMPDIR/sharun+helper-libs-$ARCH.tar" -C "$_QS_REFDIR" sharun
if ! cmp -s "$_QS_REFDIR/sharun" AppDir/sharun; then
	echo "FATAL: AppDir/sharun difiere del loader original (¿sobrescritura via hardlink?)" >&2
	exit 1
fi
rm -rf "$_QS_REFDIR"
echo "sharun integro."

# --- Verificacion glibc de los ELF del bundle ---
echo "=== glibc maxima referenciada por binario ==="
for b in AppDir/bin/trinity AppDir/bin/mcpelauncher-client AppDir/bin/mcpelauncher-extract AppDir/bin/mcpelauncher-webview; do
	[ -x "$b" ] || continue
	MAX_GLIBC="$(objdump -T "$b" 2>/dev/null \
		| grep -oE 'GLIBC_[0-9]+\.[0-9]+' | sort -uV | tail -n1 || true)"
	echo "  $b -> ${MAX_GLIBC:-sin simbolos versionados}"
done

PACKAGER="${PACKAGER:-appimage}"

echo "=== 7/7 Empaquetar (PACKAGER=$PACKAGER) y test ==="
export OUTPATH OUTNAME UPINFO
if [ "$PACKAGER" = "squashfs" ] ; then
	d="$(find AppDir -name 'com.trench.trinity.launcher.desktop' -type f | head -n 1)"
	i="$(find AppDir -name 'com.trench.trinity.launcher.svg' -type f | head -n 1)"
	[ -n "$d" ] || { echo "FATAL: .desktop no desplegado" >&2; exit 1; }
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
	mkdir -p "$OUTPATH"
	rm -f "$OUTPATH"/*.AppImage "$OUTPATH"/*.zsync
	rm -f AppDir/lib/libVkLayer_* \
		AppDir/share/vulkan/explicit_layer.d/VkLayer_khronos_validation.json
	COMPRESSION="${COMPRESSION:-}"
	if [ -z "$COMPRESSION" ]; then
		if [ "$CHANNEL" = "latest" ]; then
			COMPRESSION="xz"
		else
			COMPRESSION="gzip"
		fi
	fi
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
	AI_ARCH="$ARCH"
	if [ "$ARCH" = "aarch64" ]; then
		AI_ARCH="arm_aarch64"
	fi
	APPIMAGE_EXTRACT_AND_RUN=1 ARCH="$AI_ARCH" ./appimagetool-classic \
		--mksquashfs-opt -b --mksquashfs-opt 1M \
		--runtime-file ./runtime-classic \
		-u "$UPINFO" AppDir "$OUTPATH/$OUTNAME"
	for z in ./*.zsync; do
		[ -e "$z" ] || continue
		mv -f "$z" "$OUTPATH/"
	done
else
	./quick-sharun --make-appimage
fi
export APPIMAGE_EXTRACT_AND_RUN=1
xvfb-run -a ./quick-sharun --simple-test "$OUTPATH"/*.AppImage

echo "Listo: $OUTPATH/$OUTNAME"
