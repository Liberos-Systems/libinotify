cpp = meson.get_compiler('cpp')
cpp20 = ['-std=c++20']

# make libinotify
shared_library('inotify',
  'libinotify.cpp',
  cpp_args : cpp20,
  dependencies : dependency('fmt', version: '>=7.1.3', method : 'pkg-config'),
  install : true,
  install_dir : '/usr/lib/libinotify'
)

install_headers('libinotify.hpp', install_dir : '/usr/include/libinotify')
