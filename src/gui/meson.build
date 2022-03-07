project('gui', 'c++',
  version: '2.0',
  meson_version : '>= 0.56.0',
  default_options: [ 'buildtype=debugoptimized'],
)

fc_version = meson.project_version()
version_arr = fc_version.split('.')

cc = meson.get_compiler('c')


freetype_dep = dependency('freetype2', method: 'pkg-config', version: freetype_req, required: false)

fonts_conf = configuration_data()


if not get_option('tools').disabled()
  subdir('fc-cache')
  subdir('fc-cat')
  subdir('fc-conflist')
  subdir('fc-list')
  subdir('fc-match')
  subdir('fc-pattern')
  subdir('fc-query')
  subdir('fc-scan')
  subdir('fc-validate')
endif

if not get_option('tests').disabled()
  subdir('test')
endif

subdir('conf.d')
subdir('its')

# xgettext is optional (on Windows for instance)
if find_program('xgettext', required : get_option('nls')).found()
  subdir('po')
  subdir('po-conf')
endif

if not get_option('doc').disabled()
  subdir('doc')
endif

configure_file(output: 'config.h', configuration: conf)

configure_file(output: 'fonts.conf',
  input: 'fonts.conf.in',
  configuration: fonts_conf,
  install_dir: fc_baseconfigdir,
  install: true)

install_data('fonts.dtd',
  install_dir: join_paths(get_option('prefix'), get_option('datadir'), 'xml/fontconfig')
)

fc_headers = [
  'fontconfig/fontconfig.h',
  'fontconfig/fcfreetype.h',
  'fontconfig/fcprivate.h',
]

install_headers(fc_headers, subdir: meson.project_name())

# Summary
if meson.version().version_compare('>= 0.53')
  doc_targets = get_variable('doc_targets', [])

  summary({
         'Documentation': (doc_targets.length() > 0 ? doc_targets : false),
         'NLS': not get_option('nls').disabled(),
         'Tests': not get_option('tests').disabled(),
         'Tools': not get_option('tools').disabled(),
         }, section: 'General', bool_yn: true, list_sep: ', ')
endif
