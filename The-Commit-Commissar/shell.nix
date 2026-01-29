{ pkgs ? import <nixpkgs> {
    config.allowUnfree = true;
    config.android_sdk.accept_license = true;
  } 
}:

let
  androidEnv = pkgs.androidenv.composeAndroidPackages {
    includeNDK = true;
    ndkVersion = "25.1.8937393";
    platformVersions = [ "33" "34" "35" ];
    buildToolsVersions = [ "33.0.0" "35.0.0"];
    includeCmake = true;
    includeEmulator = true;
    includeSystemImages = true;
    systemImageTypes = [ "google_apis_playstore" ];
  };
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    cmake
    pkg-config
    ninja
    git
    androidEnv.androidsdk
    jdk17
  ];

  buildInputs = with pkgs; [
    gcc
    gdb
    gdk-pixbuf
    libappindicator-gtk3
    
    vulkan-headers
    vulkan-loader
    glfw
    shaderc
    
    # Ultralight dependencies
    gtk3
    cairo
    pango
    glib
    fontconfig
    harfbuzz
    atk
    xorg.libX11
    xorg.libXrandr
    xorg.libXcursor
    xorg.libXi
    xorg.libXrender
    xorg.libXext
    bzip2
    fontconfig
    freetype
    
    # Android emulator dependencies
    libGL
    vulkan-loader
  ];

  shellHook = ''
    export REAL_ANDROID_HOME="${androidEnv.androidsdk}/libexec/android-sdk"
    export ANDROID_HOME="$PWD/.android_sdk_mirror"
    
    mkdir -p "$ANDROID_HOME/ndk"
    
    # Symlink required components from Nix Store to our local writable mirror
    ln -sfn "$REAL_ANDROID_HOME/platform-tools" "$ANDROID_HOME/platform-tools"
    ln -sfn "$REAL_ANDROID_HOME/build-tools" "$ANDROID_HOME/build-tools"
    ln -sfn "$REAL_ANDROID_HOME/platforms" "$ANDROID_HOME/platforms"
    ln -sfn "$REAL_ANDROID_HOME/licenses" "$ANDROID_HOME/licenses"
    ln -sfn "$REAL_ANDROID_HOME/emulator" "$ANDROID_HOME/emulator"
    ln -sfn "$REAL_ANDROID_HOME/system-images" "$ANDROID_HOME/system-images"
    
    # Fix the NDK structure specifically for modern Gradle (versioned folder)
    ln -sfn "$REAL_ANDROID_HOME/ndk-bundle" "$ANDROID_HOME/ndk/25.1.8937393"
    
    export ANDROID_NDK_ROOT="$ANDROID_HOME/ndk/25.1.8937393"
    export PATH="$ANDROID_NDK_ROOT:$ANDROID_HOME/tools/emulator:$ANDROID_HOME/platform-tools:$PATH"
    export ANDROID_SDK_ROOT="$ANDROID_HOME"
    
    export SHADERC_LIB_DIR="${pkgs.shaderc}/lib"
    export SHADERC_INC_DIR="${pkgs.shaderc.dev}/include"
    
    export ANDROID_EMULATOR_USE_SYSTEM_LIBS=1

    # Important: Tell CMake to look at Nix paths
    export CMAKE_PREFIX_PATH="${pkgs.shaderc}:${pkgs.shaderc.dev}:$CMAKE_PREFIX_PATH"

    # 2. Java and Tooling Setup
    export JAVA_HOME="${pkgs.jdk17.home}"
    export SHADERC_INC="${pkgs.shaderc.dev}/include"
    export CPATH="${pkgs.shaderc.dev}/include:$CPATH"
    
    export WOLF_VARS="${pkgs.lib.makeLibraryPath [ 
      pkgs.vulkan-loader 
      pkgs.libGL
      pkgs.glfw 
      pkgs.bzip2 
      pkgs.fontconfig 
      pkgs.gtk3 
      pkgs.atk
      pkgs.pango
      pkgs.gdk-pixbuf
      pkgs.cairo
      pkgs.glib
    ]}:$PWD/../ThirdParty/UltraLight/linux/bin"

    # Fix for the bzip2 version mismatch
    mkdir -p ./.lib_links
    ln -sf ${pkgs.bzip2.out}/lib/libbz2.so.1 ./.lib_links/libbz2.so.1.0
    export WOLF_VARS="$PWD/.lib_links:$WOLF_VARS"
    
    export VK_LAYER_PATH="${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d"
    export LD_LIBRARY_PATH="${pkgs.vulkan-validation-layers}/lib:$LD_LIBRARY_PATH"
    export VK_ICD_FILENAMES="/run/opengl-driver/share/vulkan/icd.d/nvidia_icd.x86_64.json:/run/opengl-driver-32/share/vulkan/icd.d/nvidia_icd.i686.json"
    
    echo "--- Wolf engine environment loaded ---"
    echo "Use LD_LIBRARY_PATH=\$WOLF_VARS"
  '';
}
