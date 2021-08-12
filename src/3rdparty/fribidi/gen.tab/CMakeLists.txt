
message(STATUS "${PROJECT_SOURCE_DIR}")


ADD_EXECUTABLE(gen-unicode-version gen-unicode-version.c)
ADD_EXECUTABLE(gen-bidi-type-tab gen-bidi-type-tab.c packtab.c)
ADD_EXECUTABLE(gen-joining-type-tab gen-joining-type-tab.c packtab.c)
ADD_EXECUTABLE(gen-arabic-shaping-tab gen-arabic-shaping-tab.c)
ADD_EXECUTABLE(gen-mirroring-tab gen-mirroring-tab.c packtab.c)

ADD_EXECUTABLE(gen-brackets-tab gen-brackets-tab.c packtab.c packtab.h)
ADD_EXECUTABLE(gen_brackets_type_tab gen-brackets-type-tab.c packtab.c packtab.h)

ADD_DEPENDENCIES(gen-bidi-type-tab gen-unicode-version)
ADD_DEPENDENCIES(gen-joining-type-tab gen-unicode-version)
ADD_DEPENDENCIES(gen-arabic-shaping-tab gen-unicode-version)
ADD_DEPENDENCIES(gen-mirroring-tab gen-unicode-version)

ADD_DEPENDENCIES(gen-brackets-tab gen-unicode-version)
ADD_DEPENDENCIES(gen_brackets_type_tab gen-unicode-version)

IF(MSVC)
  ADD_DEFINITIONS(-Dpow=s_pow) #redefinition
ENDIF()
# TODO: use OUTPUT instead of TARGET
add_custom_command(TARGET gen-unicode-version POST_BUILD
  COMMAND gen-unicode-version unidata/ReadMe.txt unidata/BidiMirroring.txt gen-unicode-version.c >fribidi-unicode-version.h
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_custom_command(TARGET gen-bidi-type-tab
  #COMMAND gen-bidi-type-tab unidata/DerivedBidiClass.txt fribidi-unicode-version.h gen-bidi-type-tab.c packtab.c packtab.h > derived_bidi-type.tab.i
  COMMAND gen-bidi-type-tab 2 unidata/UnicodeData.txt fribidi-unicode-version.h gen-bidi-type-tab.c packtab.c packtab.h > bidi-type.tab.i
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_custom_command(TARGET gen-joining-type-tab
  COMMAND gen-joining-type-tab 2 unidata/UnicodeData.txt unidata/ArabicShaping.txt fribidi-unicode-version.h gen-joining-type-tab.c packtab.c packtab.h >joining-type.tab.i
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_custom_command(TARGET gen-arabic-shaping-tab
  COMMAND gen-arabic-shaping-tab 2 unidata/UnicodeData.txt fribidi-unicode-version.h gen-arabic-shaping-tab.c >arabic-shaping.tab.i
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_custom_command(TARGET gen-mirroring-tab
  COMMAND gen-mirroring-tab 2 unidata/BidiMirroring.txt fribidi-unicode-version.h gen-mirroring-tab.c packtab.c packtab.h >mirroring.tab.i
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_custom_command(TARGET gen-brackets-tab
  COMMAND gen-brackets-tab 2 unidata/BidiBrackets.txt unidata/UnicodeData.txt fribidi-unicode-version.h gen-brackets-tab.c packtab.c packtab.h >brackets.tab.i
 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_custom_command(TARGET gen_brackets_type_tab
  COMMAND gen_brackets_type_tab 2 unidata/BidiBrackets.txt unidata/UnicodeData.txt fribidi-unicode-version.h gen-brackets-type-tab.c packtab.c packtab.h >brackets-type.tab.i
 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
) 

add_custom_target(gen_tab
  DEPENDS gen-unicode-version gen-bidi-type-tab gen-joining-type-tab gen-arabic-shaping-tab gen-mirroring-tab gen-brackets-tab gen_brackets_type_tab
)
