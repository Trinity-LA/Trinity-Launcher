{
  description = "Entorno de desarrollo y compilación para Trinity Launcher";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        projectDeps = with pkgs; [
          qt6.qtbase qt6.qtdeclarative qt6.qtwebengine qt6.qtsvg qt6.qttools qt6.qttranslations
          libzip libpng libunwind libusb1 libevdev libpulseaudio alsa-lib pipewire libjack2 sndio
          libx11 libxi libxext libxfixes libxcursor libxrandr libxscrnsaver libxtst
          mesa libGL libdrm vulkan-loader vulkan-headers vulkan-validation-layers wayland libdecor libxkbcommon
          dbus bluez ibus
        ];
      in
      {
        packages.default = pkgs.clangStdenv.mkDerivation {
          pname = "trinity-launcher";
          version = "1.0.0";
          src = ./.;

          nativeBuildInputs = with pkgs; [ pkg-config cmake ninja git curl qt6.wrapQtAppsHook ];
          buildInputs = projectDeps;
          
          installPhase = ''
            mkdir -p $out/bin
            cp app/trinity $out/bin/
          '';
        };

        devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          nativeBuildInputs = with pkgs; [ pkg-config cmake ninja git curl qt6.wrapQtAppsHook ];
          buildInputs = projectDeps;
          shellHook = ''
            export QT_QPA_PLATFORM="wayland;xcb"
            export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath (projectDeps ++ [ pkgs.stdenv.cc.cc.lib ])}:$LD_LIBRARY_PATH"
          '';
        };
      }
    );
}