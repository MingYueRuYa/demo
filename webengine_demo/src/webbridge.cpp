#include "webbridge.h"

#include <QCoreApplication>

WebBridge::WebBridge(QObject *parent)
    : QObject(parent)
{
}

void WebBridge::sendToCpp(const QString &payload)
{
    emit messageFromJs(payload);
    onMessageFromWeb(payload);
}

QString WebBridge::applicationVersion() const
{
    const QString version = QCoreApplication::applicationVersion();
    if (!version.isEmpty()) {
        return version;
    }
    return QStringLiteral("dev-build");
}

void WebBridge::dispatchToWeb(const QString &payload)
{
    emit messageFromCpp(payload);
    onMessageFromCpp(payload);
}

void WebBridge::onMessageFromWeb(const QString &payload)
{
    Q_UNUSED(payload);
}

void WebBridge::onMessageFromCpp(const QString &payload)
{
    Q_UNUSED(payload);
}

BasicBridge::BasicBridge(QObject *parent)
    : WebBridge(parent)
{
}

void BasicBridge::onMessageFromWeb(const QString &payload)
{
    Q_UNUSED(payload);
}

