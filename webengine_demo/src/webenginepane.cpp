#include "webenginepane.h"

#include "connectguard.h"
#include "webbridge.h"
#include "webenginesignals.h"

#include <QAction>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QMenu>
#include <QNetworkCookie>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEngineContextMenuData>
#include <QWebEngineCookieStore>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

namespace {

constexpr auto kZhihuHost = "www.zhihu.com";
constexpr auto kZhihuHostNoWWW = "zhihu.com";
const QUrl kRedirectTarget(QStringLiteral("https://baidu.com"));

bool shouldRedirectZhihu(const QUrl &url)
{
    if (!url.isValid()) {
        return false;
    }
    const QString host = url.host().toLower();
    if (host == QLatin1String(kZhihuHost) || host == QLatin1String(kZhihuHostNoWWW)) {
        const QString scheme = url.scheme().toLower();
        return scheme == QLatin1String("http") || scheme == QLatin1String("https");
    }
    return false;
}

class InterceptingPage final : public QWebEnginePage
{
public:
    explicit InterceptingPage(QWebEngineProfile *profile, QObject *parent = nullptr)
        : QWebEnginePage(profile, parent)
    {
    }

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override
    {
        if (isMainFrame && shouldRedirectZhihu(url)) {
            const QUrl target(kRedirectTarget);
            QTimer::singleShot(0, this, [this, target]() {
                this->load(target);
            });
            return false;
        }
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }
};

} // namespace

WebEnginePane::WebEnginePane(WebBridge *bridge, QWidget *parent)
    : QWidget(parent)
    , m_bridge(bridge)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_view = new QWebEngineView(this);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_view);

    configureProfile();
    configureView();
    ensureBridge();
    setupChannel();
    resetLoadState();
    m_signalHub = new WebEngineSignals(this);
    m_signalHub->bind(m_view);

    ENSURE_QT_CONNECT(m_view, &QWebEngineView::urlChanged, this, &WebEnginePane::urlChanged);
    ENSURE_QT_CONNECT(m_view, &QWebEngineView::loadFinished, this, &WebEnginePane::loadFinished);
    ENSURE_QT_CONNECT(m_bridge, &WebBridge::messageFromJs, this, &WebEnginePane::messageFromJs);
    ENSURE_QT_CONNECT(m_view, &QWidget::customContextMenuRequested, this, &WebEnginePane::showCustomContextMenu);
    ENSURE_QT_CONNECT(m_view, &QWebEngineView::loadStarted, this, &WebEnginePane::handleLoadStarted);
    ENSURE_QT_CONNECT(m_bridge, &WebBridge::pageReady, this, &WebEnginePane::handlePageReady);
    ENSURE_QT_CONNECT(m_view,
                      &QWebEngineView::loadFinished,
                      this,
                      [this](bool ok) {
                          m_lastLoadSucceeded = ok;
                          if (m_lastLoadSucceeded && m_jsReady) {
                              flushPendingMessages();
                          }
                      });
}

WebEnginePane::~WebEnginePane() = default;

QWebEngineView *WebEnginePane::view() const
{
    return m_view;
}

QWebEngineProfile *WebEnginePane::profile() const
{
    return m_profile;
}

WebBridge *WebEnginePane::bridge() const
{
    return m_bridge;
}

WebEngineSignals *WebEnginePane::signalHub() const
{
    return m_signalHub;
}

void WebEnginePane::setUserAgent(const QString &ua)
{
    if (!m_profile) {
        return;
    }

    const QString trimmed = ua.trimmed();
    if (trimmed.isEmpty()) {
        if (!m_defaultUserAgent.isEmpty()) {
            m_profile->setHttpUserAgent(m_defaultUserAgent);
        }
        return;
    }

    m_profile->setHttpUserAgent(trimmed);
}

QString WebEnginePane::currentUserAgent() const
{
    if (!m_profile) {
        return {};
    }
    return m_profile->httpUserAgent();
}

bool WebEnginePane::setCookieForCurrentPage(const QString &cookieLine)
{
    if (!m_profile || !m_view) {
        return false;
    }

    auto *store = m_profile->cookieStore();
    if (!store) {
        return false;
    }

    const QUrl currentUrl = m_view->url();
    if (!currentUrl.isValid() || currentUrl.host().isEmpty()) {
        return false;
    }

    const QString trimmed = cookieLine.trimmed();
    const int separator = trimmed.indexOf('=');
    if (separator <= 0) {
        return false;
    }

    const QString name = trimmed.left(separator).trimmed();
    const QString value = trimmed.mid(separator + 1).trimmed();
    if (name.isEmpty()) {
        return false;
    }

    QNetworkCookie cookie;
    cookie.setName(name.toUtf8());
    cookie.setValue(value.toUtf8());
    cookie.setDomain(currentUrl.host());
    cookie.setPath(QStringLiteral("/"));
    cookie.setSecure(currentUrl.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0);
    cookie.setExpirationDate(QDateTime::currentDateTimeUtc().addYears(1));

    store->setCookie(cookie, currentUrl);
    return true;
}

void WebEnginePane::dumpDocumentCookies()
{
    if (!m_view || !m_view->page()) {
        emit cookiesDumped(tr("当前页面尚未加载，无法读取 Cookie"));
        return;
    }

    auto *page = m_view->page();
    page->runJavaScript(QStringLiteral("document.cookie"), [this](const QVariant &value) {
        if (!value.isValid()) {
            emit cookiesDumped(tr("未能获取 document.cookie"));
            return;
        }
        const QString cookies = value.toString();
        emit cookiesDumped(cookies);
    });
}

void WebEnginePane::load(const QUrl &url)
{
    if (!m_view || !url.isValid()) {
        return;
    }
    resetLoadState();
    m_view->setUrl(url);
}

void WebEnginePane::clearProfileData()
{
    if (!m_profile) {
        return;
    }

    m_profile->clearAllVisitedLinks();
    m_profile->clearHttpCache();
    if (auto *store = m_profile->cookieStore()) {
        store->deleteAllCookies();
    }
}

void WebEnginePane::broadcastToPage(const QString &payload)
{
    if (!m_bridge) {
        return;
    }

    const QString trimmed = payload.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }

    if (m_lastLoadSucceeded && m_jsReady) {
        m_bridge->dispatchToWeb(trimmed);
        return;
    }

    m_pendingPayloads.append(trimmed);
}

void WebEnginePane::configureProfile()
{
    m_profile = new QWebEngineProfile("DemoProfile", this);
    m_profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    m_profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    m_profile->setSpellCheckEnabled(false);
    m_defaultUserAgent = m_profile->httpUserAgent();

    const QString storageRoot = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (!storageRoot.isEmpty()) {
        m_profile->setCachePath(storageRoot + "/cache");
        m_profile->setPersistentStoragePath(storageRoot + "/storage");
        m_profile->setDownloadPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    }
}

void WebEnginePane::configureView()
{
    if (!m_view) {
        return;
    }
    auto *page = new InterceptingPage(m_profile, m_view);
    m_view->setPage(page);
    auto *settings = page->settings();
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::SpatialNavigationEnabled, true);
    settings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
    settings->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
    settings->setDefaultTextEncoding("utf-8");

    m_view->setZoomFactor(1.0);
    //m_view->setBackgroundColor(Qt::transparent);
    page->setBackgroundColor(Qt::transparent);
}

void WebEnginePane::showCustomContextMenu(const QPoint &pos)
{
    if (!m_view) {
        return;
    }

    auto *page = m_view->page();
    if (!page) {
        return;
    }

    const QWebEngineContextMenuData data = page->contextMenuData();
    const bool hasLinkTarget = data.linkUrl().isValid();
    const QUrl targetUrl = hasLinkTarget ? data.linkUrl() : m_view->url();

    QMenu menu(m_view);

    auto *openExternal = menu.addAction(hasLinkTarget
                                            ? tr("在系统浏览器打开此链接")
                                            : tr("在系统浏览器打开当前页面"));
    openExternal->setEnabled(targetUrl.isValid());
    QObject::connect(openExternal, &QAction::triggered, this, [targetUrl]() {
        if (targetUrl.isValid()) {
            QDesktopServices::openUrl(targetUrl);
        }
    });

    auto *copyUrl = menu.addAction(hasLinkTarget ? tr("复制链接地址")
                                                 : tr("复制当前页面 URL"));
    copyUrl->setEnabled(targetUrl.isValid());
    QObject::connect(copyUrl, &QAction::triggered, this, [targetUrl]() {
        if (!targetUrl.isValid()) {
            return;
        }
        if (auto *clipboard = QGuiApplication::clipboard()) {
            clipboard->setText(targetUrl.toString());
        }
    });

    if (m_bridge) {
        menu.addSeparator();
        auto *notifyPage = menu.addAction(tr("向网页广播右键事件"));
        const QString selected = data.selectedText();
        QObject::connect(notifyPage, &QAction::triggered, this, [this, selected]() {
            QString payload = tr("来自本地菜单的事件");
            if (!selected.isEmpty()) {
                payload = tr("本地菜单传递选中文本：%1").arg(selected);
            }
            broadcastToPage(payload);
        });
    }

    menu.exec(m_view->mapToGlobal(pos));
}

void WebEnginePane::setupChannel()
{
    if (!m_channel) {
        m_channel = new QWebChannel(this);
    }

    if (!m_bridge) {
        return;
    }

    m_channel->registerObject(QStringLiteral("bridge"), m_bridge);
    if (m_view && m_view->page()) {
        m_view->page()->setWebChannel(m_channel);
    }
}

void WebEnginePane::ensureBridge()
{
    if (!m_bridge) {
        m_bridge = new BasicBridge(this);
    } else if (!m_bridge->parent()) {
        m_bridge->setParent(this);
    }
}

void WebEnginePane::handleLoadStarted()
{
    m_lastLoadSucceeded = false;
    m_jsReady = false;
}

void WebEnginePane::handlePageReady()
{
    m_jsReady = true;
    if (m_lastLoadSucceeded) {
        flushPendingMessages();
    }
}

void WebEnginePane::resetLoadState()
{
    m_lastLoadSucceeded = false;
    m_jsReady = false;
}

void WebEnginePane::flushPendingMessages()
{
    if (!m_bridge || m_pendingPayloads.isEmpty()) {
        return;
    }

    const auto pending = m_pendingPayloads;
    m_pendingPayloads.clear();
    for (const QString &message : pending) {
        if (!message.isEmpty()) {
            m_bridge->dispatchToWeb(message);
        }
    }
}

