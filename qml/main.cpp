#include <QtGui/QApplication>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDir>

#include "qmlapplicationviewer.h"

#include "Manager.h"
#include "FilesModel.h"
#include "SearchesModel.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    Manager donkeyManager;

    FilesModel filesModel;
    filesModel.setManager(&donkeyManager);

    SearchesModel searchesModel;
    searchesModel.setManager(&donkeyManager);

    QmlApplicationViewer viewer;
    QDeclarativeContext *ctxt = viewer.rootContext();

    ctxt->setContextProperty("Manager", &donkeyManager);
    ctxt->setContextProperty("FilesModel", &filesModel);
    ctxt->setContextProperty("SearchesModel", &searchesModel);

//#ifdef Q_OS_ANDROID
//    viewer.addImportPath("/imports/");
//    //viewer.engine()->addPluginPath(QDir::homePath()+"/../lib");
//    viewer.engine()->addPluginPath(QApplication::applicationDirPath()+"/lib");
//#endif

    viewer.addImportPath("qml/KMLDonkey/components");
    viewer.addImportPath("/home/snoopy/src/kmldonkey/qml/qml/KMLDonkey/components");

    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/KMLDonkey/main.qml"));

    viewer.showExpanded();
    //viewer.showFullScreen();

    return app->exec();
}
