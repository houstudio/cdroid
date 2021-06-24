set(sub_dir_name src)

set(PUBLIC_FILES 
    ${PROJECT_SOURCE_DIR}/fontconfig/fontconfig.h
    ${PROJECT_SOURCE_DIR}/src/fcdeprecate.h
    ${PROJECT_SOURCE_DIR}/fontconfig/fcprivate.h)

add_custom_command(OUTPUT fcalias.h fcaliastail.h
    COMMAND sh ${PROJECT_SOURCE_DIR}/src/makealias ${PROJECT_SOURCE_DIR}/src fcalias.h fcaliastail.h ${PUBLIC_FILES}
    DEPENDS ${PROJECT_SOURCE_DIR}/src/fcdeprecate.h ${PROJECT_SOURCE_DIR}/fontconfig/fontconfig.h  ${PROJECT_SOURCE_DIR}/fontconfig/fcprivate.h 
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

set(PUBLIC_FT_FILES ${PROJECT_SOURCE_DIR}/fontconfig/fcfreetype.h)
add_custom_command(OUTPUT fcftalias.h fcftaliastail.h
    COMMAND sh ${PROJECT_SOURCE_DIR}/src/makealias ${PROJECT_SOURCE_DIR}/src fcftalias.h fcftaliastail.h ${PUBLIC_FT_FILES}
    DEPENDS ${PROJECT_SOURCE_DIR}/fontconfig/fcfreetype.h
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/src)

set( fontconfig_SRCS
    fcarch.c
    fcatomic.c
    # fcblanks.c
    fccache.c
    fccfg.c
    fccharset.c
    fccompat.c
    fcdbg.c
    fcdefault.c
    fcdir.c
    fcformat.c
    fcfreetype.c
    fcfs.c
    fcinit.c
    fclang.c
    fclist.c
    fcmatch.c
    fcmatrix.c
    fcname.c
    fcobjs.c
    fcpat.c
    fcrange.c
    fcserialize.c
    fcstat.c
    fcstr.c
    fcweight
    fcxml.c
    ftglue.c
    fcptrlist.c
    fchash.c
    # generated
    fcaliastail.h
    fcalias.h
    fcftaliastail.h
    fcftalias.h
)

include_directories(
 ${PROJECT_SOURCE_DIR}
 ${PROJECT_SOURCE_DIR}/fontconfig
 ${PROJECT_BINARY_DIR}/src
 ${PROJECT_BINARY_DIR}/fc-lang
 ${CMAKE_BINARY_DIR}/include

 ${PROJECT_SOURCE_DIR}/../
 ${CMAKE_BINARY_DIR}/include/
)

add_definitions(-DXML_STATIC -DHAVE_CONFIG_H)
add_library( fontconfig STATIC ${fontconfig_SRCS} )
set_target_properties(fontconfig PROPERTIES OUTPUT_NAME fontconfig )

target_link_libraries( fontconfig ${FREETYPE_LIBRARIES} ${EXPAT_LIBRARY} uuid )

install( TARGETS fontconfig RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib )

