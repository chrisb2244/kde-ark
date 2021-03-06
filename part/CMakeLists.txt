include_directories(${CMAKE_SOURCE_DIR}/app)

set(arkpart_PART_SRCS
	part.cpp
	infopanel.cpp
	arkviewer.cpp
	archivemodel.cpp
	archiveview.cpp
	jobtracker.cpp
    )

qt5_add_dbus_adaptor(arkpart_PART_SRCS dnddbusinterface.xml part.h Ark::Part)

ki18n_wrap_ui(arkpart_PART_SRCS infopanel.ui )
ki18n_wrap_ui(arkpart_PART_SRCS jobtracker.ui )

add_library(arkpart MODULE ${arkpart_PART_SRCS})

target_link_libraries(arkpart kerfuffle KF5::KHtml KF5::KIOFileWidgets)

configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/ark_part.desktop.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/ark_part.desktop
)

install(TARGETS arkpart DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/ark_part.desktop     DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES ark_part.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ark )
