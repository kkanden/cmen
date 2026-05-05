{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in
    {

      packages.${system}.default = pkgs.stdenv.mkDerivation (finalAtrrs: {
        pname = "libjson";
        version = "0.1.0";

        src = ./.;

        buildFlags = [
          "RELEASE=1"
          "lib"
          "solib"
        ];

        installPhase = ''
          runHook preInstall
          mkdir -p $out/include/libjson
          mkdir -p $out/lib
          cp include/* $out/include/libjson
          cp build/libjson.{so,a} $out/lib
          runHook postInstall
        '';
      });

    };
}
