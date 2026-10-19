{
  description = "opengl flake for you bebz";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
  in {
    devShells.${system}.default = pkgs.mkShell {
      nativeBuildInputs = [
        pkgs.cmake
        pkgs.ninja
        pkgs.pkg-config
      ];

      buildInputs = [
        pkgs.glfw
        pkgs.glew
        pkgs.freeglut
        pkgs.libGL
        pkgs.libGLU
        pkgs.xorg.libX11
        pkgs.xorg.libXrandr
        pkgs.xorg.libXinerama
        pkgs.xorg.libXcursor
        pkgs.xorg.libXi
        pkgs.xorg.libXext
        pkgs.xorg.libXxf86vm
      ];

      shellHook = ''
        export CMAKE_POLICY_VERSION_MINIMUM=3.5
        export LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath [
          pkgs.libGL
          pkgs.xorg.libX11
          pkgs.xorg.libXrandr
          pkgs.xorg.libXinerama
          pkgs.xorg.libXcursor
          pkgs.xorg.libXi
          pkgs.xorg.libXext
        ]}:/run/opengl-driver/lib:$LD_LIBRARY_PATH
      '';
    };
  };
}
