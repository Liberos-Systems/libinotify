project('libinotify', ['cpp','c'],
        version : '0.2.0',
        meson_version: '>= 0.61.0',
        default_options : ['warning_level=3',
                           'cpp_std=c++20'])

BUILD_EXAMPLE=true
compiler = meson.get_compiler('cpp')
libc6_check_code = '''
                    #include <stdio.h>
                    int main() {
                      printf("Hello, World!\n");
                      return 0;
                    }
                  '''
if not compiler.compiles(libc6_check_code)
  error('libc6 not found')
endif

kernel_version = run_command('uname', '-r', check : true).stdout().strip()
if kernel_version.version_compare('>=3.19')
  compiler = meson.get_compiler('cpp')
  code ='''
            #include<iostream>
            int main() {
              std::cout << "Compile and link me.\n";
              return 0;
            }
            '''

  result = compiler.compiles(code, name : 'compile check')
  link_result = compiler.links(code, name : 'link check')

  if result and link_result
    glib_dep = dependency('glib-2.0', version: '>=2.5')

    subdir('libinotify')

    if BUILD_EXAMPLE == true
      subdir('example')
    endif
  else
    error('Compile or link check failed.')
  endif
else
  error('Kernel version is too low. Required >=3.19, found ' + kernel_version)
endif