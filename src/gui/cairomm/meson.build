# cairomm

# Input: cairomm_build_dep, cairomm_pcname, cairomm_libversion, cairomm_rc,
#        cairomm_libname, macos_darwin_versions, cairomm_extra_libs,
#        cairomm_api_version, cairomm_pc_requires, cairo_dep
# Output: source_h_files, cairomm_own_dep

# There are no built source files in cairomm.

cairomm_cc = [
  'context.cc',
  'context_surface_quartz.cc',
  'context_surface_win32.cc',
  'context_surface_xlib.cc',
  'device.cc',
  'exception.cc',
  'fontface.cc',
  'fontoptions.cc',
  'matrix.cc',
  'mesh_pattern.cc',
  'path.cc',
  'pattern.cc',
  'private.cc',
  'quartz_font.cc',
  'quartz_surface.cc',
  'region.cc',
  'scaledfont.cc',
  'script.cc',
  'script_surface.cc',
  'surface.cc',
  'win32_font.cc',
  'win32_surface.cc',
  'xcb_device.cc',
  'xcb_surface.cc',
  'xlib_device.cc',
  'xlib_surface.cc',
]

cairomm_public_h = [
  'cairomm.h',
  'context.h',
  'device.h',
  'enums.h',
  'exception.h',
  'fontface.h',
  'fontoptions.h',
  'matrix.h',
  'mesh_pattern.h',
  'path.h',
  'pattern.h',
  'quartz_font.h',
  'quartz_surface.h',
  'refptr.h',
  'region.h',
  'scaledfont.h',
  'script.h',
  'script_surface.h',
  'surface.h',
  'types.h',
  'win32_font.h',
  'win32_surface.h',
  'xcb_device.h',
  'xcb_surface.h',
  'xlib_device.h',
  'xlib_surface.h',
]

cairomm_private_h = [
  'context_private.h',
  'private.h',
]

# docs/reference/meson.build needs this.
source_h_files = cairomm_public_h

install_headers(cairomm_public_h, subdir: cairomm_pcname / 'cairomm')

# Make sure we are exporting the symbols from the DLL
cairomm_cpp_args = ['-DCAIROMM_BUILD=1']

# Build the .rc file for Windows builds and link to it
extra_cairomm_objects = []
if host_machine.system() == 'windows'
  windows = import('windows')
  cairomm_res = windows.compile_resources(cairomm_rc)
  extra_cairomm_objects += cairomm_res
endif

extra_include_dirs = ['..']

cairomm_library = library(cairomm_libname,
  cairomm_cc, extra_cairomm_objects,
  version: cairomm_libversion,
  darwin_versions: macos_darwin_versions,
  implicit_include_directories: false,
  include_directories: extra_include_dirs,
  cpp_args: cairomm_cpp_args,
  dependencies: cairomm_build_dep,
  install: true,
)

# Generate .pc files, used by pkg-config.
pkg_config = import('pkgconfig')
pc_common_variables = [
  'doxytagfile=${docdir}/reference/' + cairomm_pcname + '.tag',
  'htmlrefdir=${docdir}/reference/html',
  'htmlrefpub=https://cairo.pages.freedesktop.org/' + meson.project_name(),
]
pc_variables = [
  'exec_prefix=${prefix}',
  'datarootdir=${datadir}',
  'docdir=${datadir}/doc/' + cairomm_pcname,
] + pc_common_variables
pc_uninstalled_variables = [
  'docdir=${prefix}/docs',
] + pc_common_variables

pkg_config.generate(cairomm_library,
  filebase: cairomm_pcname,
  variables: pc_variables,
  uninstalled_variables: pc_uninstalled_variables,
  name: meson.project_name(),
  description: 'C++ binding for the cairo graphics library',
  url: 'https://www.cairographics.org/cairomm/',
  requires: cairomm_pc_requires,
  libraries: cairomm_extra_libs,
  subdirs: [cairomm_pcname],
  extra_cflags: ['-I${libdir}/' + cairomm_pcname + '/include'],
)

cairomm_mods = [
# [name, description(, function, prefix)]
  ['fc', 'Fontconfig font backend'],
  ['ft', 'FreeType font backend'],
#  ['glitz', 'Glitz surface backend'], # Glitz support has been removed from cairo 1.18
  ['pdf', 'PDF surface backend'],
  ['png', 'PNG functions',
    'cairo_surface_write_to_png', '#include <cairo.h>'],
  ['ps', 'PostScript surface backend'],
  ['quartz', 'Quartz surface backend'],
  ['quartz-font','Quartz font backend',
    'cairo_quartz_font_face_create_for_cgfont', '#include <cairo-quartz.h>'],
  ['quartz-image', 'Quartz Image surface backend'],
  ['script', 'Script surface backend'],
  ['svg', 'SVG surface backend'],
  ['win32', 'Microsoft Windows surface backend'],
  ['win32-font', 'Microsoft Windows font backend',
    'cairo_win32_font_face_create_for_hfont', '#include <cairo-win32.h>'],
  ['xcb', 'XCB surface backend'],
  ['xlib', 'Xlib surface backend'],
  ['xlib-xrender', 'Xlib Xrender surface backend'],
]

foreach mod : cairomm_mods
  mod_name = mod[0]
  mod_desc = mod[1]
  cairo_mod_name = 'cairo-' + mod_name
  cairomm_mod_name = meson.project_name() + '-' + mod_name

  # Generate and install the module only if there is a corresponding cairo module.
  if cairo_dep.type_name() in ['pkgconfig', 'internal']
    do_install = dependency(cairo_mod_name, required: false).found()
  elif mod.length() >= 4
    do_install = cpp_compiler.has_function(mod[2],
                                           prefix: mod[3],
                                           dependencies: cairo_dep)
  else
    do_install = cpp_compiler.has_header(cairo_mod_name + '.h')
  endif
  if do_install
    pkg_config.generate(
      filebase: cairomm_mod_name + '-' + cairomm_api_version,
      name: cairomm_mod_name,
      description: 'C++ wrapper for cairo - ' + mod_desc,
      requires: [cairomm_pcname, cairo_mod_name],
    )
  endif
endforeach

# This is used when building example programs and test programs.
# It's also a part of cairomm_dep, when cairomm is a subproject.
cairomm_own_dep = declare_dependency(
  link_with: cairomm_library,
  include_directories: extra_include_dirs,
  dependencies: cairomm_build_dep
)
