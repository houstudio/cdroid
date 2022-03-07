project('freetype2', 'c',
  meson_version: '>= 0.47.0',
  version: '22.1.16',
  default_options: [ 'buildtype=debugoptimized'])

pkgmod = import('pkgconfig')

cc = meson.get_compiler('c')

base_sources = [
  'src/autofit/autofit.c',
  'src/base/ftbase.c',
  'src/base/ftbbox.c',
  'src/base/ftbdf.c',
  'src/base/ftbitmap.c',
  'src/base/ftcid.c',
  'src/base/ftfstype.c',
  'src/base/ftgasp.c',
  'src/base/ftglyph.c',
  'src/base/ftgxval.c',
  'src/base/ftinit.c',
  'src/base/ftmm.c',
  'src/base/ftotval.c',
  'src/base/ftpatent.c',
  'src/base/ftpfr.c',
  'src/base/ftstroke.c',
  'src/base/ftsynth.c',
  'src/base/ftsystem.c',
  'src/base/fttype1.c',
  'src/base/ftwinfnt.c',
  'src/bdf/bdf.c',
  'src/bzip2/ftbzip2.c',
  'src/cache/ftcache.c',
  'src/cff/cff.c',
  'src/cid/type1cid.c',
  'src/gzip/ftgzip.c',
  'src/lzw/ftlzw.c',
  'src/pcf/pcf.c',
  'src/pfr/pfr.c',
  'src/psaux/psaux.c',
  'src/pshinter/pshinter.c',
  'src/psnames/psnames.c',
  'src/raster/raster.c',
  'src/sfnt/sfnt.c',
  'src/smooth/smooth.c',
  'src/truetype/truetype.c',
  'src/type1/type1.c',
  'src/type42/type42.c',
  'src/winfonts/winfnt.c',
]

if host_machine.system() == 'windows'
  winmod = import('windows')
  base_sources += [
    'builds/windows/ftdebug.c',
    winmod.compile_resources('src/base/ftver.rc'),
  ]
else
  base_sources += [
    'src/base/ftdebug.c',
  ]
endif

c_args = [
  '-DFT2_BUILD_LIBRARY',
  '-DFT_CONFIG_CONFIG_H=<ftconfig.h>',
  '-DFT_CONFIG_OPTIONS_H=<ftoption.h>'
]

if host_machine.system() == 'windows'
  c_args += [
    '-DDLL_EXPORT',
    '-D_CRT_SECURE_NO_WARNINGS',
    '-D_CRT_NONSTDC_NO_WARNINGS',
  ]
endif

check_headers = []

if ['linux', 'darwin', 'cygwin'].contains(host_machine.system())
  check_headers += [
    ['unistd.h'],
    ['fcntl.h'],
    ['stdint.h'],
  ]
  ftconfig_h_in = files('builds/unix/ftconfig.h.in')
else
  ftconfig_h_in = files('include/freetype/config/ftconfig.h')
endif

conf = configuration_data()
deps = []
incbase = include_directories(['include'])

# headers
foreach check : check_headers
  name = check[0]

  if cc.has_header(name)
    conf.set('HAVE_@0@'.format(name.to_upper().underscorify()), 1)
  endif
endforeach

# optional dependencies

# bzip2
bzip2_dep = cc.find_library('bz2', required: get_option('bzip2'))
if bzip2_dep.found() and cc.has_header_symbol('bzlib.h', 'BZ2_bzlibVersion')
  conf.set('FT_CONFIG_OPTION_USE_BZIP2', 1)
  deps += [bzip2_dep]
endif

# harfbuzz
harfbuzz_dep = dependency('harfbuzz', version: '>= 1.3.0',
                          required: get_option('harfbuzz'),
                          fallback: ['harfbuzz', 'libharfbuzz_dep'],
                          default_options : ['freetype=disabled',
                                             'fontconfig=disabled'])
if harfbuzz_dep.found()
  conf.set('FT_CONFIG_OPTION_USE_HARFBUZZ', 1)
  deps += [harfbuzz_dep]
endif

# zlib
zlib_dep = dependency('zlib', required: get_option('zlib'),
  fallback: ['zlib', 'zlib_dep'])
if zlib_dep.found()
  conf.set('FT_CONFIG_OPTION_SYSTEM_ZLIB', 1)
  deps += [zlib_dep]
endif

# png
png_dep = dependency('libpng', required: get_option('png'),
  fallback: ['libpng', 'libpng_dep'])
if png_dep.found()
  conf.set('FT_CONFIG_OPTION_USE_PNG', 1)
  deps += [png_dep]
endif

includedir = join_paths(get_option('includedir'), 'freetype2')
ftincludedir = join_paths(includedir, 'freetype')
configincludedir = join_paths(ftincludedir, 'config')

# Configure and install headers
configure_file(input: ftconfig_h_in,
  output: 'ftconfig.h',
  configuration: conf,
  install_dir: configincludedir)

# Copy the original ftoption.h and add some #mesondefines to it to override the
# existing configuration. This saves some maintenance burden over maintaining
# a copy of that file.
ftoption_h_in = configure_file(input: 'include/freetype/config/ftoption.h',
  output: 'ftoption.h.in',
  command: [find_program('include/configure-ftoption_h.py'), '@INPUT@', '@OUTPUT@'])

configure_file(input: ftoption_h_in,
  output: 'ftoption.h',
  configuration: conf,
  install_dir: configincludedir)

libfreetype = library('freetype', base_sources,
  include_directories: incbase,
  dependencies: deps,
  c_args: c_args,
  install: true,
  version: '6.16.0',
  soversion: '6')

freetype_dep = declare_dependency(link_with: libfreetype,
  include_directories : incbase,
  dependencies: deps
)

install_headers('include/ft2build.h', subdir: 'freetype2')
# Install the include subdirs as-is so we don't need to maintain an installed
# headers list.
install_subdir('include/freetype', install_dir: includedir,
               exclude_directories: ['config', 'internal'])
install_subdir('include/freetype/config', install_dir: ftincludedir,
               exclude_files: ['ftoption.h', 'ftconfig.h'])

pkgmod.generate(libfreetype,
  name: 'freetype2',
  description: 'The FreeType font rendering library.',
  subdirs: ['freetype2'],
  version: meson.project_version())

