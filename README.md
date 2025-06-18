# 3D Reverb

This is a 3D reverb plugin made with the JUCE DSP module.

## Building

```
$ git clone --recurse-submodules https://github.com/Kontses/3D-Reverb.git
$ cd 3D-Reverb
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
$ cmake --build build --config Release
```

## VTS3 file

The VST3 file is located in the `\out\build\x64-Debug\3d_reverb_artefacts\Debug\VST3\3d_reverb.vst3\Contents\x86_64-win` directory.
You can copy it to your VST3 plugins directory.