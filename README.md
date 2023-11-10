# libinotify
This is a library for monitoring file system events, written in C++23. It enables you to watch individual files and directories, including recursively, and allows you to select the type of event that triggers an action.

An example of how to use this library can be found in the 'examples' directory. When building the library, you can also build the example by setting the 'BUILD_EXAMPLE' variable to true. **Please note that by default, this is set to false.**

## Build, Compile and Install Commands
Please execute these commands in the root directory of the project.
```bash
rm -rf build/
meson build
meson compile -C build
meson install -C build
```

### Dependencies
```
A C++ compiler with support for C++23
GLib, version: >=2.5
Linux kernel version 3.19 or higher
```

### Licence
GNU GPL, see [LICENSE](./LICENSE) file

### Author
Written by Kacper Paczos (kacper-paczos@linux.pl)