#include "browserwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    qputenv("QTWEBENGINE_DISABLE_SANDBOX", "1");
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Qt WebEngine Demo"));
    QApplication::setOrganizationName(QStringLiteral("DemoOrg"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    BrowserWindow window;
    window.show();
    return QApplication::instance()->exec();
}

