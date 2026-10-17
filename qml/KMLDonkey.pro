# Add more folders to ship with the application, here
folder_01.source = qml/KMLDonkey
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

RESOURCES += KMLDonkey.qrc

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

QT += core network

DEFINES += LIBKMLDONKEY_NO_KDE

INCLUDEPATH += ../libkmldonkey/
DEPENDPATH += ../libkmldonkey/ qml/KMLDonkey/

SOURCES += \
    main.cpp \
    ../libkmldonkey/clientinfo.cpp \
    ../libkmldonkey/donkeyhost.cpp \
    ../libkmldonkey/donkeymessage.cpp \
    ../libkmldonkey/donkeyprotocol.cpp \
    ../libkmldonkey/donkeysocket.cpp \
    ../libkmldonkey/ed2kurl.cpp \
    ../libkmldonkey/emulecollection.cpp \
    ../libkmldonkey/fileinfo.cpp \
    ../libkmldonkey/hostmanager.cpp \
    ../libkmldonkey/network.cpp \
    ../libkmldonkey/options.cpp \
    ../libkmldonkey/protocoliface.cpp \
    ../libkmldonkey/roominfo.cpp \
    ../libkmldonkey/searchinfo.cpp \
    ../libkmldonkey/searchquery.cpp \
    ../libkmldonkey/serverinfo.cpp \
    ../libkmldonkey/shareinfo.cpp \
    ../libkmldonkey/torrenthost.cpp \
    FilesModel.cpp \
    Manager.cpp \
    SearchesModel.cpp \
    BaseModel.cpp \
    SearchResultModel.cpp \
    SearchResultsModel.cpp \
    SearchQueriesModel.cpp \
    HostModel.cpp

HEADERS += \
    ../libkmldonkey/clientinfo.h \
    ../libkmldonkey/donkeyhost.h \
    ../libkmldonkey/donkeymessage.h \
    ../libkmldonkey/donkeyprotocol.h \
    ../libkmldonkey/donkeysocket.h \
    ../libkmldonkey/ed2kurl.h \
    ../libkmldonkey/emulecollection.h \
    ../libkmldonkey/fileinfo.h \
    ../libkmldonkey/hostmanager.h \
    ../libkmldonkey/network.h \
    ../libkmldonkey/options.h \
    ../libkmldonkey/protocoliface.h \
    ../libkmldonkey/roominfo.h \
    ../libkmldonkey/searchinfo.h \
    ../libkmldonkey/searchquery.h \
    ../libkmldonkey/serverinfo.h \
    ../libkmldonkey/shareinfo.h \
    ../libkmldonkey/torrenthost.h \
    FilesModel.h \
    Manager.h \
    SearchesModel.h \
    BaseModel.h \
    SearchResultModel.h \
    SearchResultsModel.h \
    SearchQueriesModel.h \
    HostModel.h

symbian {
    TARGET.UID3 = 0xE8C56D9C

    # Smart Installer package's UID
    # This UID is from the protected range and therefore the package will
    # fail to install if self-signed. By default qmake uses the unprotected
    # range value if unprotected UID is defined for the application and
    # 0x2002CCCF value if protected UID is given to the application
    #DEPLOYMENT.installer_header = 0x2002CCCF

    # Allow network access on Symbian
    TARGET.CAPABILITY += NetworkServices

    # Add dependency to Symbian components
    #CONFIG += qt-components
}

contains(MEEGO_EDITION,harmattan) {
    # Speed up launching on MeeGo/Harmattan when using applauncherd daemon
    CONFIG += qdeclarative-boostable

    OTHER_FILES += \
        qtc_packaging/debian_harmattan/rules \
        qtc_packaging/debian_harmattan/README \
        qtc_packaging/debian_harmattan/manifest.aegis \
        qtc_packaging/debian_harmattan/copyright \
        qtc_packaging/debian_harmattan/control \
        qtc_packaging/debian_harmattan/compat \
        qtc_packaging/debian_harmattan/changelog
}

android {
    OTHER_FILES += \
        android/src/org/kde/necessitas/origo/QtApplication.java \
        android/src/org/kde/necessitas/origo/QtActivity.java \
        android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
        android/src/org/kde/necessitas/ministro/IMinistro.aidl \
        android/res/values-de/strings.xml \
        android/res/values-zh-rCN/strings.xml \
        android/res/values-fa/strings.xml \
        android/res/values-es/strings.xml \
        android/res/values-nl/strings.xml \
        android/res/values-it/strings.xml \
        android/res/values-el/strings.xml \
        android/res/values-fr/strings.xml \
        android/res/values-pl/strings.xml \
        android/res/values-ro/strings.xml \
        android/res/values-nb/strings.xml \
        android/res/values-et/strings.xml \
        android/res/values-rs/strings.xml \
        android/res/values-ja/strings.xml \
        android/res/values-id/strings.xml \
        android/res/values/strings.xml \
        android/res/values-pt-rBR/strings.xml \
        android/res/values-zh-rTW/strings.xml \
        android/res/values-ru/strings.xml \
        android/res/values-ms/strings.xml \
        #android/res/layout/splash.xml \
        android/version.xml \
        android/res/drawable-ldpi/icon.png \
        android/res/drawable/icon.png \
        #android/res/drawable/logo.png \
        android/res/drawable-hdpi/icon.png \
        android/res/values/libs.xml \
        android/res/drawable-mdpi/icon.png \
        android/AndroidManifest.xml
}

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()
