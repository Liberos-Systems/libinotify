cpp = meson.get_compiler('cpp')
libinotify = cpp.find_library('libinotify', dirs : '/usr/lib/libinotify')
fmt = dependency('fmt', version: '>=7.1.3', method : 'pkg-config')

executable('libinotify_example', 'main.cpp', dependencies : [libinotify, fmt], cpp_args : '-std=c++20', install : true, install_dir : '/usr/bin')