cairo_boilerplate_sources = [
  'cairo-boilerplate-getopt.c',
  'cairo-boilerplate-system.c',
  'cairo-boilerplate.c',
]

cairo_boilerplate_feature_sources = {
  'cairo-xlib': ['cairo-boilerplate-xlib.c'],
  'cairo-qt': ['cairo-boilerplate-qt.cpp'],
  'cairo-quartz': ['cairo-boilerplate-quartz.c'],
  'cairo-xcb': ['cairo-boilerplate-xcb.c'],
  'cairo-win32': ['cairo-boilerplate-win32.c', 'cairo-boilerplate-win32-printing.c'],
  'cairo-drm': ['cairo-boilerplate-drm.c'],
  'cairo-cogl': ['cairo-boilerplate-cogl.c'],
  'cairo-directfb': ['cairo-boilerplate-directfb.c'],
  'cairo-pdf': ['cairo-boilerplate-pdf.c'],
  'cairo-ps': ['cairo-boilerplate-ps.c'],
  'cairo-svg': ['cairo-boilerplate-svg.c'],
  'cairo-vg': ['cairo-boilerplate-vg.c'],
  'cairo-script': ['cairo-boilerplate-script.c'],
  # All tests crash with FPE
  # 'cairo-egl': ['cairo-boilerplate-egl.c'],
  # cairo-glx: ['cairo-boilerplate-glx.c'],
  'cairo-wgl': ['cairo-boilerplate-wgl.c'],
}

foreach feature: built_features
  source_key = feature.get('source-key', feature.get('name'))
  cairo_boilerplate_sources += cairo_boilerplate_feature_sources.get(source_key, [])
endforeach

cairo_boilerplate_constructors = custom_target('cairo-boilerplate-constructors.c',
  input: files(cairo_boilerplate_sources),
  output: 'cairo-boilerplate-constructors.c',
  command: [python3, files('make-cairo-boilerplate-constructors.py')[0], '@OUTPUT@', '@INPUT@'])

libcairoboilerplate = static_library('cairoboilerplate', cairo_boilerplate_sources + [cairo_boilerplate_constructors],
  c_args: ['-DHAVE_CONFIG_H'],
  cpp_args: ['-DHAVE_CONFIG_H'],
  include_directories: [incbase, incsrc],
  dependencies: deps,
  link_with: [libcairo],
)
