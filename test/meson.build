# Rules for building tests

test(
  'backup',
  executable(
    'backup_test',
    [
      'account_entry_test.cpp',
      'account_list_test.cpp',
      'accounts_test.cpp',
      'backup_test.cpp',
      'manifest_test.cpp',
      '../src/accounts.cpp',
      '../src/backup.cpp',
      '../src/manifest.cpp',
    ],
    dependencies: [
      dependency('gtest', main: true, disabler: true, required: build_tests),
    ],
    include_directories: '../src',
    cpp_args : '-DTEST_DATA_DIR="' + meson.current_source_dir() + '/data"',
  )
)
