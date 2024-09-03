# cairomm

# Input: cairomm_build_dep, cairomm_pcname, cairomm_libversion, cairomm_rc,
#        is_msvc, cairomm_libname, macos_darwin_versions
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

# This is used when building example programs and test programs.
# It's also a part of cairomm_dep, when cairomm is a subproject.
cairomm_own_dep = declare_dependency(
  link_with: cairomm_library,
  include_directories: extra_include_dirs,
  dependencies: cairomm_build_dep
)
