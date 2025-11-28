#include "browserwindow.h"

#include "messageconsole.h"
#include "webenginepane.h"
#include "webbridge.h"

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QSlider>
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebEngineView>
#include <QWidgetAction>
#include <QWidget>

namespace {
constexpr int kOpacityMin = 40;
constexpr int kOpacityMax = 100;
constexpr int kOpacityDefault = 95;
} // namespace

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Qt WebEngine 综合 Demo"));
    resize(1200, 800);
    setAttribute(Qt::WA_TranslucentBackground, true);
    statusBar()->setSizeGripEnabled(true);

    buildUi();
    buildToolbar();

    if (m_console) {
        connect(m_console, &MessageConsole::messageSent, this, [this](const QString &payload) {
            updateStatus(tr("已向网页发送：%1").arg(payload));
        });
        connect(m_console, &MessageConsole::messageFromWeb, this, &BrowserWindow::handleMessageFromPage);
    }

    if (m_engine) {
        m_engine->load(homeUrl());
        connect(m_engine, &WebEnginePane::urlChanged, this, [this](const QUrl &url) {
            if (!url.isEmpty()) {
                m_addressBar->setText(url.toString());
            }
        });
        connect(m_engine, &WebEnginePane::loadFinished, this, &BrowserWindow::handleLoadFinished);
    }

    m_addressBar->setText(homeUrl().toString());
    applyOpacity(kOpacityDefault);
}

BrowserWindow::~BrowserWindow() = default;

void BrowserWindow::buildUi()
{
    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");
    central->setStyleSheet("#centralWidget { background-color: rgba(255, 255, 255, 255); }");
    setCentralWidget(central);

    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    m_engine = new WebEnginePane(new BasicBridge, this);
    layout->addWidget(m_engine, 1);

    m_console = new MessageConsole(this);
    layout->addWidget(m_console);
    if (m_engine) {
        m_console->attachBridge(m_engine->bridge());
    }
}

void BrowserWindow::buildToolbar()
{
    auto *toolbar = addToolBar(tr("导航"));
    toolbar->setMovable(false);
    toolbar->setStyleSheet("background-color:rgb(255,255,255)");

    if (m_engine && m_engine->view()) {
        toolbar->addAction(tr("后退"), m_engine->view(), &QWebEngineView::back);
        toolbar->addAction(tr("前进"), m_engine->view(), &QWebEngineView::forward);
        toolbar->addAction(tr("刷新"), m_engine->view(), &QWebEngineView::reload);
    }
    auto *homeAction = toolbar->addAction(tr("主页"));
    connect(homeAction, &QAction::triggered, this, &BrowserWindow::navigateHome);

    m_addressBar = new QLineEdit(this);
    m_addressBar->setPlaceholderText(tr("输入 URL 或按 Enter 加载"));
    m_addressBar->setClearButtonEnabled(true);
    connect(m_addressBar, &QLineEdit::returnPressed, this, &BrowserWindow::loadRequestedUrl);
    toolbar->addWidget(m_addressBar);

    toolbar->addSeparator();
    auto *clearAction = toolbar->addAction(tr("清空缓存"));
    connect(clearAction, &QAction::triggered, this, &BrowserWindow::clearProfileData);
    auto *sendAction = toolbar->addAction(tr("消息面板"));
    connect(sendAction, &QAction::triggered, this, &BrowserWindow::showMessageConsole);

    m_transparentAction = toolbar->addAction(tr("透明模式"));
    m_transparentAction->setCheckable(true);
    connect(m_transparentAction, &QAction::toggled, this, &BrowserWindow::handleTransparencyToggle);

    auto *sliderAction = new QWidgetAction(this);
    m_opacitySlider = new QSlider(Qt::Horizontal, toolbar);
    m_opacitySlider->setRange(kOpacityMin, kOpacityMax);
    m_opacitySlider->setValue(kOpacityDefault);
    connect(m_opacitySlider, &QSlider::valueChanged, this, &BrowserWindow::applyOpacity);

    sliderAction->setDefaultWidget(m_opacitySlider);
    toolbar->addAction(sliderAction);
}

void BrowserWindow::loadRequestedUrl()
{
    const QString text = m_addressBar->text().trimmed();
    if (text.isEmpty()) {
        navigateHome();
        return;
    }

    QUrl url = QUrl::fromUserInput(text);
    if (m_engine) {
        m_engine->load(url);
    }
    updateStatus(tr("加载 %1").arg(url.toString()));
}

void BrowserWindow::navigateHome()
{
    if (m_engine) {
        m_engine->load(homeUrl());
    }
}

void BrowserWindow::clearProfileData()
{
    if (m_engine) {
        m_engine->clearProfileData();
    }
    updateStatus(tr("缓存与 Cookie 清理完成"));
}

void BrowserWindow::showMessageConsole()
{
    if (!m_console) {
        return;
    }
    m_console->setVisible(true);
    m_console->focusInput();
}

void BrowserWindow::handleMessageFromPage(const QString &payload)
{
    updateStatus(tr("网页发来信息：%1").arg(payload));
    QMessageBox::information(this, tr("来自网页"), payload);
}

void BrowserWindow::handleLoadFinished(bool ok)
{
    if (ok) {
        updateStatus(tr("页面加载完成"));
        if (m_engine) {
            const QString info = tr("C++ 已完成加载，时间戳 %1")
                                     .arg(QDateTime::currentDateTime().toString(Qt::ISODate));
            m_engine->broadcastToPage(info);
        }
    } else {
        updateStatus(tr("页面加载失败"), 8000);
    }
}

void BrowserWindow::applyOpacity(int sliderValue)
{
    const auto opacity = static_cast<double>(sliderValue) / 100.0;
    setWindowOpacity(opacity);
}

void BrowserWindow::handleTransparencyToggle(bool enabled)
{
    updateWindowTransparency(enabled);
}

void BrowserWindow::updateStatus(const QString &text, int timeoutMs)
{
    statusBar()->showMessage(text, timeoutMs);
}

void BrowserWindow::updateWindowTransparency(bool transparent)
{
    auto *view = m_engine ? m_engine->view() : nullptr;
    if (!view) {
        return;
    }

    if (transparent) {
        view->setAttribute(Qt::WA_TranslucentBackground, true);
        view->setStyleSheet("background: transparent;");
        if (m_engine) {
            m_engine->setStyleSheet("background: transparent;");
        }
        setStyleSheet("QMainWindow { background: transparent; }");
        setWindowOpacity(static_cast<double>(m_opacitySlider->value()) / 100.0);
    } else {
        view->setAttribute(Qt::WA_NoSystemBackground, false);
        view->setStyleSheet({});
        if (m_engine) {
            m_engine->setStyleSheet({});
        }
        setStyleSheet({});
        setWindowOpacity(1.0);
        if (m_opacitySlider) {
            m_opacitySlider->setValue(kOpacityMax);
        }
    }
}

QUrl BrowserWindow::homeUrl() const
{
    return QUrl(QStringLiteral("qrc:/web/index.html"));
}

