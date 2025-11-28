#include "messageconsole.h"

#include "webbridge.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace {
QString timestamp()
{
    return QDateTime::currentDateTime().toString("hh:mm:ss");
}
} // namespace

MessageConsole::MessageConsole(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    m_log = new QPlainTextEdit(this);
    m_log->setReadOnly(true);
    m_log->setMaximumBlockCount(1000);
    layout->addWidget(m_log, 1);

    auto *inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(6);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(tr("输入内容并发送到网页..."));
    inputLayout->addWidget(m_input, 1);

    layout->addLayout(inputLayout);

    connect(m_input, &QLineEdit::returnPressed, this, &MessageConsole::handleSendClicked);
}

void MessageConsole::attachBridge(WebBridge *bridge)
{
    if (m_bridge == bridge) {
        return;
    }
    if (m_bridge) {
        disconnect(m_bridge, nullptr, this, nullptr);
    }
    m_bridge = bridge;
    if (m_bridge) {
        connect(m_bridge, &WebBridge::messageFromJs, this, &MessageConsole::handleIncomingMessage);
        connect(m_bridge, &WebBridge::messageFromCpp, this, [this](const QString &payload) {
            appendEntry(tr("C++ -> Web"), payload);
        });
        appendSystemMessage(tr("消息通道已连接 "));
    } else {
        appendSystemMessage(tr("消息通道未连接 "));
    }
}

void MessageConsole::focusInput()
{
    if (m_input) {
        m_input->setFocus();
        m_input->selectAll();
    }
}

void MessageConsole::handleSendClicked()
{
    if (!m_input) {
        return;
    }
    const QString payload = m_input->text().trimmed();
    if (payload.isEmpty()) {
        return;
    }

    if (!m_bridge) {
        appendSystemMessage(tr("无法发送：Web 引擎尚未准备"));
        return;
    }

    m_bridge->dispatchToWeb(payload);
    emit messageSent(payload);
    m_input->clear();
}

void MessageConsole::handleIncomingMessage(const QString &payload)
{
    appendEntry(tr("Web -> C++"), payload);
    emit messageFromWeb(payload);
}

void MessageConsole::appendEntry(const QString &direction, const QString &payload)
{
    if (!m_log) {
        return;
    }
    m_log->appendPlainText(QString("[%1] %2 %3").arg(timestamp(), direction, payload));
}

void MessageConsole::appendSystemMessage(const QString &payload)
{
    appendEntry(tr("系统"), payload);
}

