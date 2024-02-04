project('iphone-backup-viewer', 'c', version : '0.0.0')

warning_level = 3

c_log = dependency('c-log')
argparse = dependency('argparse')
plist = dependency('plist')
sqlite3 = dependency('sqlite3')

include_dirs = include_directories('include')

subdir('src')

executable(
  'iphone-backup-viewer',
  sources,
  dependencies: [c_log, argparse, plist, sqlite3],
  include_directories: include_dirs
)
