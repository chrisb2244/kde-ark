set(kerfuffle_singlefile_SRCS singlefileplugin.cpp)
set(SUPPORTED_LIBSINGLEFILE_MIMETYPES "")

# This MIME type was originally set in ark.desktop but is does not
# seem to be supported anywhere.  Assuming that, if it were supported,
# it would be here.
set(SUPPORTED_LIBSINGLEFILE_MIMETYPES "${SUPPORTED_LIBSINGLEFILE_MIMETYPES}application/x-compress;")

#
# GZip files
#
find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
                       URL "http://www.zlib.net/"
                       DESCRIPTION "The Zlib compression library"
                       PURPOSE "Required for .gz format support in Ark")

if (ZLIB_FOUND)
	set(kerfuffle_libgz_SRCS gzplugin.cpp ${kerfuffle_singlefile_SRCS})
	set(SUPPORTED_LIBSINGLEFILE_MIMETYPES "${SUPPORTED_LIBSINGLEFILE_MIMETYPES}application/x-gzip;")

	add_library(kerfuffle_libgz MODULE ${kerfuffle_libgz_SRCS})

	target_link_libraries(kerfuffle_libgz KF5::Archive KF5::KIOCore kerfuffle )

	install(TARGETS kerfuffle_libgz DESTINATION ${PLUGIN_INSTALL_DIR} )

	install( FILES kerfuffle_libgz.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
endif (ZLIB_FOUND)

#
# Bzip2 files
#
find_package(BZip2)
set_package_properties(BZip2 PROPERTIES
                       URL "http://www.bzip.org/"
                       DESCRIPTION "The Bzip2 compression library"
                       PURPOSE "Required for .bz2 format support in Ark")

if (BZIP2_FOUND)
	set(kerfuffle_libbz2_SRCS bz2plugin.cpp ${kerfuffle_singlefile_SRCS})
	set(SUPPORTED_LIBSINGLEFILE_MIMETYPES "${SUPPORTED_LIBSINGLEFILE_MIMETYPES}application/x-bzip;application/x-bzip2;")

	add_library(kerfuffle_libbz2 MODULE ${kerfuffle_libbz2_SRCS})

	target_link_libraries(kerfuffle_libbz2 KF5::Archive KF5::KIOCore kerfuffle )

	install(TARGETS kerfuffle_libbz2 DESTINATION ${PLUGIN_INSTALL_DIR} )

	install( FILES kerfuffle_libbz2.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
endif (BZIP2_FOUND)

#
# LZMA files
#
find_package(LibLZMA)
set_package_properties(LibLZMA PROPERTIES
                       URL "http://tukaani.org/xz/"
                       DESCRIPTION "A LZMA compression library"
                       PURPOSE "Required for .xz and .lzma format support in Ark")

if (LIBLZMA_FOUND)
	set(kerfuffle_libxz_SRCS xzplugin.cpp ${kerfuffle_singlefile_SRCS})
	set(SUPPORTED_LIBSINGLEFILE_MIMETYPES "${SUPPORTED_LIBSINGLEFILE_MIMETYPES}application/x-lzma;application/x-xz;")

	add_library(kerfuffle_libxz MODULE ${kerfuffle_libxz_SRCS})

	target_link_libraries(kerfuffle_libxz KF5::Archive KF5::KIOCore kerfuffle )

	install(TARGETS kerfuffle_libxz DESTINATION ${PLUGIN_INSTALL_DIR} )

	install( FILES kerfuffle_libxz.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
endif (LIBLZMA_FOUND)

set(SUPPORTED_ARK_MIMETYPES "${SUPPORTED_ARK_MIMETYPES}${SUPPORTED_LIBSINGLEFILE_MIMETYPES}" PARENT_SCOPE)
