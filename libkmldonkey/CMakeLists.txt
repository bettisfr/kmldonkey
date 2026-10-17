set(libkmldonkey_LIB_SRCS
    torrenthost.cpp
    emulecollection.cpp
    clientinfo.cpp
    donkeyhost.cpp
    donkeymessage.cpp
    donkeyprotocol.cpp
    donkeysocket.cpp
    ed2kurl.cpp
    fileinfo.cpp
    hostmanager.cpp
    hostselectaction.cpp
    hostdialog.cpp
    network.cpp
    options.cpp
    protocoliface.cpp
    roominfo.cpp
    searchinfo.cpp
    searchquery.cpp
    serverinfo.cpp
    shareinfo.cpp
)

add_library(libkmldonkey SHARED ${libkmldonkey_LIB_SRCS} ../compat/porteddialog.h ../compat/portedpagedialog.h)
target_include_directories(libkmldonkey PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(libkmldonkey PRIVATE MAKE_LIBKMLDONKEY_LIB)
target_link_libraries(libkmldonkey PUBLIC Qt6::Core Qt6::Widgets Qt6::Network Qt6::Xml Qt6::Core5Compat KF6::ConfigCore KF6::I18n KF6::WidgetsAddons KF6::XmlGui KF6::KIOCore KF6::Completion)
set_target_properties(libkmldonkey PROPERTIES VERSION 6.0.0 SOVERSION 6)
install(TARGETS libkmldonkey LIBRARY DESTINATION ${KDE_INSTALL_LIBDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kmldonkey FILES_MATCHING PATTERN "*.h")
