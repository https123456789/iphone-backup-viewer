project('plist', 'c', version: '2.0')

mod = import('unstable-external_project')

run_command('./autogen.sh', env: {'NOCONFIGURE': '1'}, check: true)

p = mod.add_project('configure',
  configure_options: ['--prefix=@PREFIX@'],
  verbose: true
)

plist_dep = p.dependency('plist-2.0')
meson.override_dependency('plist', plist_dep)
