{
    "targets": [{
        "target_name": "detector_addon",
        "cflags": [ '-std=c++17' ],
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],
        "cflags_cc": ['-std=c++17' ],
        "sources": [
            "main.cpp",
            "cppsrc/test.cpp",
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")"
        ],
        "libraries": [
            "/usr/local/lib/libfvad.so",
            "/usr/lib/x86_64-linux-gnu/libsndfile.so",
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }]
}
