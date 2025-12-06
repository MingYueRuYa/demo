#pragma once

#include <QDebug>
#include <QMetaObject>
#include <QObject>
#include <QtGlobal>

class ConnectGuard final
{
public:
    ConnectGuard() = delete;

    static void verify(const QMetaObject::Connection &connection,
                       const char *expression,
                       const char *file,
                       int line)
    {
        if (connection) {
            return;
        }
        logFailure(expression, file, line);
    }

    static void verify(bool success,
                       const char *expression,
                       const char *file,
                       int line)
    {
        if (success) {
            return;
        }
        logFailure(expression, file, line);
    }

private:
    static void logFailure(const char *expression,
                           const char *file,
                           int line)
    {
        qCritical().nospace() << "QObject::connect failed at " << file << ':' << line
                              << " -> " << expression;
        Q_ASSERT_X(false, "QObject::connect", expression);
    }
};

#define ENSURE_QT_CONNECT(...) \
    ConnectGuard::verify(QObject::connect(__VA_ARGS__), #__VA_ARGS__, __FILE__, __LINE__)


