ADD_DEFINITIONS(-D_LARGEFILE64_SOURCE)

set(libark_common_SRCS ../archiveformatinfo.cpp )
kde4_add_kcfg_files(libark_common_SRCS ../settings.kcfgc)

########### next target ###############

set(kerfuffle_SRCS ${libark_common_SRCS} 
    arch.cpp 
    archiveinterface.cpp
    libarchivehandler.cpp
    jobs.cpp
    archivebase.cpp
   )

kde4_add_library(kerfuffle SHARED ${kerfuffle_SRCS})

target_link_libraries(kerfuffle ${KDE4_KFILE_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${LIBARCHIVE_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KPARTS_LIBS})

set_target_properties(kerfuffle PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kerfuffle DESTINATION ${LIB_INSTALL_DIR} )