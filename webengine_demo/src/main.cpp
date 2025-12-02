#include "browserwindow.h"
#include "configmanager.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    qputenv("QTWEBENGINE_DISABLE_SANDBOX", "1");
    auto &config = ConfigManager::instance();
    const QString executableDir = QFileInfo(QString::fromLocal8Bit(argv[0])).absolutePath();
    config.initialize(executableDir);
    config.applyWebEngineRemoteDebugging();
#if !defined(NDEBUG)
    if (config.remoteDebugPort() <= 0) {
        qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "9223");
    }
#endif
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

