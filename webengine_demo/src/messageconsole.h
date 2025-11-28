#pragma once

#include <QWidget>
#include <QString>

class QPlainTextEdit;
class QLineEdit;
class WebBridge;

class MessageConsole final : public QWidget
{
    Q_OBJECT

public:
    explicit MessageConsole(QWidget *parent = nullptr);

    void attachBridge(WebBridge *bridge);
    void focusInput();

signals:
    void messageSent(const QString &payload);
    void messageFromWeb(const QString &payload);

private slots:
    void handleSendClicked();
    void handleIncomingMessage(const QString &payload);

private:
    void appendEntry(const QString &direction, const QString &payload);
    void appendSystemMessage(const QString &payload);

    WebBridge *m_bridge {nullptr};
    QPlainTextEdit *m_log {nullptr};
    QLineEdit *m_input {nullptr};
};

