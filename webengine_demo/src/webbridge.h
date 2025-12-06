#pragma once

#include <QObject>

class WebBridge : public QObject
{
    Q_OBJECT

public:
    explicit WebBridge(QObject *parent = nullptr);
    ~WebBridge() override = default;

    Q_INVOKABLE void sendToCpp(const QString &payload);
    Q_INVOKABLE QString applicationVersion() const;
    Q_INVOKABLE void notifyPageReady();

public slots:
    void dispatchToWeb(const QString &payload);

signals:
    void messageFromJs(const QString &payload);
    void messageFromCpp(const QString &payload);
    void pageReady();

protected:
    virtual void onMessageFromWeb(const QString &payload) = 0;
    virtual void onMessageFromCpp(const QString &payload) = 0;
};

class WebBridge;

class BasicBridge final : public WebBridge
{
    Q_OBJECT

public:
    explicit BasicBridge(QObject *parent = nullptr);

protected:
    void onMessageFromWeb(const QString &payload) override;
    void onMessageFromCpp(const QString &payload) override;
};

