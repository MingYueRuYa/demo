#include "configmanager.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {
QString resolveBaseDir(const QString &preferredDir)
{
    if (!preferredDir.isEmpty()) {
        return QDir(preferredDir).absolutePath();
    }
    if (QCoreApplication::instance()) {
        return QCoreApplication::applicationDirPath();
    }
    return QDir::currentPath();
}

bool isValidPort(int port)
{
    return port > 0 && port <= 65535;
}
} // namespace

ConfigManager &ConfigManager::instance()
{
    static ConfigManager s_instance;
    return s_instance;
}

void ConfigManager::initialize(const QString &baseDir)
{
    const QString resolvedDir = resolveBaseDir(baseDir);
    if (m_initialized && resolvedDir == m_baseDir) {
        return;
    }
    m_baseDir = resolvedDir;
    loadConfig();
    m_initialized = true;
}

void ConfigManager::reload()
{
    ensureInitialized();
    loadConfig();
}

int ConfigManager::remoteDebugPort() const
{
    return m_remoteDebugPort;
}

QString ConfigManager::configFilePath() const
{
    if (m_baseDir.isEmpty()) {
        return {};
    }
    return QDir(m_baseDir).filePath(QStringLiteral("config.json"));
}

void ConfigManager::applyWebEngineRemoteDebugging() const
{
    ensureInitialized();
    if (!isValidPort(m_remoteDebugPort)) {
        return;
    }
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", QByteArray::number(m_remoteDebugPort));
}

void ConfigManager::ensureInitialized() const
{
    if (m_initialized) {
        return;
    }
    const_cast<ConfigManager *>(this)->initialize(QString());
}

void ConfigManager::loadConfig()
{
    m_remoteDebugPort = 0;

    const QString path = configFilePath();
    if (path.isEmpty()) {
        qWarning() << "ConfigManager: config path unresolved";
        return;
    }

    QFile file(path);
    if (!file.exists()) {
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ConfigManager: cannot open" << path << file.errorString();
        return;
    }

    const QByteArray data = file.readAll();
    QJsonParseError error {};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "ConfigManager: invalid JSON" << path << error.errorString();
        return;
    }

    const QJsonObject root = doc.object();
    const int portCandidate = root.value(QStringLiteral("remoteDebugPort")).toInt(0);
    if (isValidPort(portCandidate)) {
        m_remoteDebugPort = portCandidate;
        qInfo() << "ConfigManager: enable remote debug port" << m_remoteDebugPort;
    } else {
        m_remoteDebugPort = 0;
    }
}


