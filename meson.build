project('libinotify', ['cpp', 'c'],
  version : '0.1.0',
  meson_version: '>= 0.61.0',
  default_options : ['warning_level=3',
                     'cpp_std=c++14'])

glib_dep = dependency('glib-2.0', version: '>=2.5')
linux_kernel_dep = dependency('linux', version: '>=3.19')

exe = executable('libinotify', 'libinotify.cpp',
  dependencies : [glib_dep, linux_kernel_dep],
  install : true)

test('basic', exe)
