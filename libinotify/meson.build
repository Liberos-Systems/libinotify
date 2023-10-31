cpp = meson.get_compiler('cpp')
cpp14 = ['-std=c++20']

shared_library('libinotify',
  'libinotify.cpp',
  cpp_args : cpp14,
  install : true
)

install_headers('./libinotify.hpp')