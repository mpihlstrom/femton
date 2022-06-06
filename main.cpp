#include <QApplication>
//#include <QDesktopWidget>
#include <QSurfaceFormat>

#include "common/vector.h"
#include "globals.h"
#include "complex/complex.h"
#include "json/json.h"
#include "util.h"
#include "init.h"

#include "window.h"

void myMessageOutput(QtMsgType, const QMessageLogContext&, const QString&) {}

int main(int argc, char **argv) {

    pre_init();

    #ifdef QT_NO_DEBUG
    //qInstallMessageHandler(myMessageOutput);
    #endif
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseDesktopOpenGL);
    QSurfaceFormat fmt;
    fmt.setVersion(4, 2);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

    QMainWindow main_window;
    window = new Window();

    main_window.setWindowTitle("qualia painter");
    main_window.setCentralWidget(window);
    main_window.resize(main_window.sizeHint());
    main_window.setMouseTracking(true);
    main_window.show();

    post_init();

    return app.exec();
}

