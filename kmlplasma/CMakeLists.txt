include_directories(
    ${KDE4_INCLUDES} ${KDE4_KIO_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${KMLDONKEY_BINARY_DIR} ${KMLDONKEY_SOURCE_DIR}
    ${KMLDONKEY_BINARY_DIR}/libkmldonkey ${KMLDONKEY_SOURCE_DIR}/libkmldonkey
)

set(kmlplasmaengine_SRCS kmlplasmaengine.cpp)
kde4_add_plugin(plasma_engine_kmldonkey ${kmlplasmaengine_SRCS})
target_link_libraries(
    plasma_engine_kmldonkey
    libkmldonkey
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${PLASMA_LIBS}
    ${QT_QTCORE_LIBRARY}
    )
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
install(TARGETS plasma_engine_kmldonkey DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-kmldonkey.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

set(kmlplasmaapplet_SRCS kmlplasmaapplet.cpp)
kde4_add_plugin(plasma_applet_kmldonkey ${kmlplasmaapplet_SRCS})
target_link_libraries(
    plasma_applet_kmldonkey 
    ${KDE4_KDEUI_LIBS} 
    ${KDE4_KIO_LIBS} 
    ${PLASMA_LIBS}
    )
install(TARGETS plasma_applet_kmldonkey DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-kmldonkey.desktop DESTINATION ${SERVICES_INSTALL_DIR})
