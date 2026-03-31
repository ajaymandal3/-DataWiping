{
  "targets": [
    {
      "target_name": "securewipe_native",
      "sources": [
        "native/addon.cpp",
        "native/securewipe_core.c",
        "native/methods/method_standard.c",
        "native/methods/method_crypto.c",
        "native/methods/method_storage.c",
        "native/methods/method_raid.c",
        "native/methods/method_dispatcher.c"
      ],
      "defines": [],
      "include_dirs": [
        "."
      ]
    }
  ]
}
