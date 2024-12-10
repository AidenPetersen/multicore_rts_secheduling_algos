{ pkgs ? import <nixpkgs> {}}:

pkgs.mkShell {
  packages = [ 
    pkgs.gcc 
    pkgs.gnumake
    pkgs.python3
    pkgs.valgrind
    pkgs.gdb
  ];
}

