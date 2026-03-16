{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs =
    inputs:
    let
      system = "x86_64-linux";
      pkgs = inputs.nixpkgs.legacyPackages.${system};
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          (raylib.overrideAttrs (prev: {
            cmakeFlags = prev.cmakeFlags ++ [ "-DGLFW_BUILD_WAYLAND=ON" ];
          }))
        ];
        shellHook = ''
          exec fish
        '';
      };

    };
}
