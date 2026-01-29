{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = [
    pkgs.python3
  ];

  shellHook = ''
    echo "Starting Python HTTP server on port 9000..."
    python3 -m http.server 9000 --bind 0.0.0.0
  '';
}
