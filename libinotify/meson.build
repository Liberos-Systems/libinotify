cpp = meson.get_compiler('cpp')
cpp23 = ['-std=c++23']

# make libinotify
shared_library('inotify',
  'libinotify.cpp',
  cpp_args : cpp23,
  install : true,
  install_dir : '/usr/lib/libinotify'
)

install_headers('libinotify.hpp', install_dir : '/usr/include/libinotify')
