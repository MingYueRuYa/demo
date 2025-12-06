#pragma once

#include <QString>

// 简单的配置单例，负责读取可执行目录下的 config.json，
// 目前仅暴露 remoteDebugPort 设置以便按需开启远程调试。
class ConfigManager final
{
public:
    static ConfigManager &instance();

    void initialize(const QString &baseDir);
    void reload();

    int remoteDebugPort() const;
    QString configFilePath() const;

    void applyWebEngineRemoteDebugging() const;

private:
    ConfigManager() = default;

    void ensureInitialized() const;
    void loadConfig();

    QString m_baseDir;
    int m_remoteDebugPort {0};
    mutable bool m_initialized {false};
};




