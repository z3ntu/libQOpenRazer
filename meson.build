project('libopenrazer',
        'cpp',
        version : '0.3.0',
        default_options : ['cpp_std=c++17'])

qt = import('qt6')
qt_dep = dependency('qt6', modules : ['Core', 'DBus', 'Gui', 'Xml'])

if build_machine.system() == 'darwin'
  libopenrazer_data_dir = 'Contents/Resources'
else
  libopenrazer_data_dir = get_option('datadir') / 'libopenrazer'
endif

subdir('translations')

conf_data = configuration_data({
  'LIBOPENRAZER_VERSION' : '"' + meson.project_version() + '"',
  'LIBOPENRAZER_DATADIR' : '"' + get_option('prefix') / get_option('datadir') / 'libopenrazer' + '"',
})

configure_file(output : 'config.h',
               configuration : conf_data)

sources = [
    'src/dbusexception.cpp',
    'src/misc.cpp',
    'src/capability.cpp',

    'src/openrazer/device.cpp',
    'src/openrazer/led.cpp',
    'src/openrazer/manager.cpp',

    'src/razer_test/device.cpp',
    'src/razer_test/led.cpp',
    'src/razer_test/manager.cpp',
]

sources += qt.preprocess(
    moc_headers : [
        'include/libopenrazer/device.h',
        'include/libopenrazer/led.h',
        'include/libopenrazer/manager.h',
        'include/libopenrazer/openrazer.h',
    ]
)

inc = include_directories('include')
srcinc = include_directories('src')

openrazerlib = library('openrazer',
                       sources,
                       version : meson.project_version(),
                       dependencies : [qt_dep],
                       include_directories : [inc, srcinc],
                       install : not meson.is_subproject())

libopenrazer_dep = declare_dependency(
    dependencies : [qt_dep],
    include_directories : inc,
    link_with : openrazerlib
)

# Stop execution here if we are in a subproject
if meson.is_subproject()
  subdir_done()
endif

install_headers('include/libopenrazer.h')
install_headers('include/libopenrazer/dbusexception.h',
                'include/libopenrazer/device.h',
                'include/libopenrazer/led.h',
                'include/libopenrazer/manager.h',
                'include/libopenrazer/misc.h',
                'include/libopenrazer/openrazer.h',
                'include/libopenrazer/capability.h',
                subdir : 'libopenrazer')

pkg = import('pkgconfig')
pkg.generate(name : meson.project_name(),
             description : 'Qt wrapper around the D-Bus API from OpenRazer',
             libraries : openrazerlib,
             version : meson.project_version())


# Demo executable
if get_option('demo') == true
  message('Building libopenrazerdemo...')
  executable('libopenrazerdemo',
             'src/demo/libopenrazerdemo.cpp',
             dependencies : [qt_dep, libopenrazer_dep])
endif
