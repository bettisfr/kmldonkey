set(kmldonkeyapp_KDEINIT_SRCS
    abstractinfomodel.cpp
    addserver.cpp
    availability.cpp
    clientlist.cpp
    consolepage.cpp
    downloadpage.cpp
    downloadrenamer.cpp
    friendpage.cpp
    infodialog.cpp
    infolist.cpp
    kmldonkey.cpp
    kmldview.cpp
    listeditor.cpp
    mlconfig.cpp
    optioneditor.cpp
    renamedialog.cpp
    searchcore.cpp
    searchcorewidgets.cpp
    search.cpp
    searchtabresult.cpp
    serverpage.cpp
    shareddirs.cpp
    sharedfilemodel.cpp
    sharepage.cpp
    statspageclients.cpp
    statspage.cpp
    statspagegnutella.cpp
    statspageovernet.cpp
    statspagerates.cpp
    submitdialog.cpp
    systemtray.cpp
    webbrowser.cpp
    webservices.cpp
    tools/clipboard.cpp
    tools/fileselector.cpp
    tools/fontselector.cpp
    tools/imageloader.cpp
    tools/mldplugin.cpp
    prefs/prefs.cpp
    uploadersmodel.cpp
)

add_library(kmldonkey_ui STATIC ${kmldonkeyapp_KDEINIT_SRCS} searchtabiface.h prefs/generalpage.ui prefs/listspage.ui prefs/searchpage.ui)
target_include_directories(kmldonkey_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tools ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/kmldonkey_ui_autogen/include)
target_link_libraries(kmldonkey_ui PUBLIC libkmldonkey Qt6::Gui KF6::XmlGui KF6::DBusAddons KF6::Codecs KF6::Completion KF6::KIOWidgets KF6::Notifications KF6::IconThemes)
add_executable(kmldonkey main.cpp)
target_link_libraries(kmldonkey PRIVATE kmldonkey_ui)
install(TARGETS kmldonkey DESTINATION ${KDE_INSTALL_BINDIR})
install(FILES kmldonkey.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES kmldonkeyui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/kmldonkey)
install(FILES kmldonkey.notifyrc DESTINATION ${KDE_INSTALL_DATADIR}/knotifications6)

qt_add_resources(kmldonkey "ui_resources" PREFIX "/kxmlgui5/kmldonkey" BASE "${CMAKE_CURRENT_SOURCE_DIR}" FILES kmldonkeyui.rc)

configure_file(kmldonkey.notifyrc ${CMAKE_BINARY_DIR}/share/knotifications6/kmldonkey.notifyrc COPYONLY)

if(UNIX AND NOT APPLE)
    file(RELATIVE_PATH kmldonkey_relative_libdir "${KDE_INSTALL_FULL_BINDIR}" "${KDE_INSTALL_FULL_LIBDIR}")
    set_target_properties(kmldonkey PROPERTIES INSTALL_RPATH "$ORIGIN/${kmldonkey_relative_libdir}")
endif()
# The historical filenames use "app"; the freedesktop icon context is "apps".
file(GLOB app_icons CONFIGURE_DEPENDS images/hi*-app-*.png images/icons/hi*-app-*.png)
foreach(icon IN LISTS app_icons)
    get_filename_component(icon_name "${icon}" NAME)
    if(icon_name MATCHES "^hi([0-9]+)-app-(.+)\\.png$")
        install(FILES "${icon}"
            DESTINATION "${KDE_INSTALL_ICONDIR}/hicolor/${CMAKE_MATCH_1}x${CMAKE_MATCH_1}/apps"
            RENAME "${CMAKE_MATCH_2}.png")
    endif()
endforeach()
