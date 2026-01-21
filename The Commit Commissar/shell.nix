{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    cmake
    pkg-config
    ninja
  ];

  buildInputs = with pkgs; [
    gcc
    gdb
    gtk3
    gdk-pixbuf
    libappindicator-gtk3
    glib
    pkg-config
    git
  ];

  # This helps pkg-config find the .pc files in your nix-shell
  shellHook = ''
    export PKG_CONFIG_PATH="${pkgs.gtk3.dev}/lib/pkgconfig:$PKG_CONFIG_PATH"
  '';
}
