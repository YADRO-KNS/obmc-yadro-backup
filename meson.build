# Rules for building with Meson

project(
  'backup',
  'cpp',
  default_options: [
   'warning_level=3',
   'werror=true',
   'cpp_std=c++17',
  ],
  license: 'Apache-2.0',
)

version = vcs_tag(command: [ 'git', 'describe', '--always', '--dirty', '--long' ],
                  input: 'src/version.hpp.in',
                  output: 'version.hpp')

build_tests = get_option('tests')
subdir('test')

executable(
  'backup',
  [
    version,
    'src/accounts.cpp',
    'src/backup.cpp',
    'src/main.cpp',
    'src/manifest.cpp',
  ],
  install: true
)
