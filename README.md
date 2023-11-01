# libinotify
This is a library for monitoring file system events, written in C++23. It enables you to watch individual files and directories, including recursively, and allows you to select the type of event that triggers an action.

An example of how to use this library can be found in the 'examples' directory. When building the library, you can also build the example by setting the 'BUILD_EXAMPLE' variable to true. **Please note that by default, this is set to false.**

## Build, Compile and Install Commands
```bash
rm -rf build/
meson build
meson compile -C build
meson install
```

