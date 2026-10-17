# Runtime dependencies are derived by dpkg-shlibdeps on the build host.
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME kmldonkey)
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}+qt6.11")
set(CPACK_PACKAGE_CONTACT "KMLDonkey fork maintainers")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/bettisfr/kmldonkey")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MLDonkey frontend for KDE Plasma 6")
set(CPACK_DEBIAN_PACKAGE_SECTION "kde")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# A core can run on another machine, so MLDonkey is not a hard dependency.
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "mldonkey")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
install(FILES
    "${PROJECT_SOURCE_DIR}/COPYING"
    "${PROJECT_SOURCE_DIR}/AUTHORS"
    "${PROJECT_SOURCE_DIR}/README.md"
    "${PROJECT_SOURCE_DIR}/BUILDING.md"
    DESTINATION "${KDE_INSTALL_DATAROOTDIR}/doc/kmldonkey")
include(CPack)
