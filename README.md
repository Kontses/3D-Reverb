# Simple Reverb

This is a simple reverb plugin made with the JUCE DSP module.

<img alt="Screenshot 2024-04-02 at 17 33 04" src="">

## UI Manual

- value changes: dragging or arrow keys
- fine mode: shift + dragging or shift + arrow keys
- edit mode: 0-9
- undo: cmd + z
- redo: cmd + shift + z
- reset: double click

## Building

```
$ git clone --recurse-submodules https://github.com/Kontses/3D-Reverb.git
$ cd 3D-Reverb
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
$ cmake --build build --config Release
```
