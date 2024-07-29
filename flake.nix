{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable-small";
  };
  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system}.pkgs;
    nativeBuildInputs = with pkgs; [
      git
      cmake
      gcc-arm-embedded
      ninja
      (python3.withPackages (ps:
        with ps; [
          # From https://github.com/zmkfirmware/zephyr/blob/HEAD/scripts/requirements-base.txt
          west
          pyelftools
          pyyaml
          pykwalify
          canopen
          packaging
          progress
          psutil
          pylink-square
          pyserial
          requests
          anytree
          intelhex
        ]))
    ];
    env = {
      ZEPHYR_TOOLCHAIN_VARIANT = "gnuarmemb";
      GNUARMEMB_TOOLCHAIN_PATH = pkgs.gcc-arm-embedded;
    };
  in {
    devShells.${system}.default = pkgs.mkShell {
      inherit nativeBuildInputs env;
      name = "scylla";
      shellHook = ''
        export ZEPHYR_BASE="$PWD/zephyr";
      '';
    };
    packages.${system} = {
      init = pkgs.writeShellApplication {
        name = "init";
        runtimeInputs = nativeBuildInputs;
        runtimeEnv = env;
        text = ''
          git submodule init
          git submodule update
          west init -l config/
          west update
          west zephyr-export
        '';
      };
      patch = pkgs.writeShellApplication {
        name = "patch";
        runtimeInputs = [pkgs.git];
        text = ''
          cd zmk
          git apply --ignore-whitespace ../patches/*.patch
        '';
      };
      left = pkgs.writeShellApplication {
        name = "left";
        runtimeInputs = nativeBuildInputs;
        runtimeEnv = env;
        text = ''
          west build -s zmk/app -d build -b nice_nano_v2 -- -DZMK_CONFIG="$PWD"/config -DSHIELD=scylla_left
        '';
      };
      right = pkgs.writeShellApplication {
        name = "right";
        runtimeInputs = nativeBuildInputs;
        runtimeEnv = env;
        text = ''
          west build -s zmk/app -d build -b nice_nano_v2 -- -DZMK_CONFIG="$PWD"/config -DSHIELD=scylla_right
        '';
      };
      flash = pkgs.writeShellApplication {
        name = "flash";
        runtimeInputs = nativeBuildInputs;
        runtimeEnv = env;
        text = ''
          sleep 3
          mount --mkdir /dev/disk/by-id/usb-Adafruit_nRF_UF2_* /tmp/nicenano
          sleep 3
          west flash
          sync
          umount /tmp/nicenano
        '';
      };
      clean = pkgs.writeShellApplication {
        name = "clean";
        text = ''
          rm -rf build
        '';
      };
    };
  };
}
