{
  "targets": [
    {
      "target_name": "hotkey_listener",
      "sources": ["hotkey_listener.cpp"],
    "libraries": ["Dwmapi.lib"],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "<!@(node -p \"require('napi-thread-safe-callback').include\")"
      ],
      "dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],
      "cflags_cc": ["-std=c++17", "-D NAPI_CPP_EXCEPTIONS"],
      "cflags!": [ "-fno-exceptions" ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7",
      },
        "msvs_settings": {
          "VCCLCompilerTool": { "ExceptionHandling": 1 },
        },
      "conditions": [
        ["OS=='win'", { "defines": [ "_HAS_EXCEPTIONS=1" ] }]
      ],
      "defines": ["NAPI_CPP_EXCEPTIONS"],
    }
  ]
}