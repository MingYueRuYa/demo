#include "webenginepane.h"

#include "connectguard.h"
#include "webbridge.h"

#include <QStandardPaths>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEngineCookieStore>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

WebEnginePane::WebEnginePane(WebBridge *bridge, QWidget *parent)
    : QWidget(parent)
    , m_bridge(bridge)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_view = new QWebEngineView(this);
    m_view->setContextMenuPolicy(Qt::DefaultContextMenu);
    layout->addWidget(m_view);

    configureProfile();
    configureView();
    ensureBridge();
    setupChannel();

    ENSURE_QT_CONNECT(m_view, &QWebEngineView::urlChanged, this, &WebEnginePane::urlChanged);
    ENSURE_QT_CONNECT(m_view, &QWebEngineView::loadFinished, this, &WebEnginePane::loadFinished);
    ENSURE_QT_CONNECT(m_bridge, &WebBridge::messageFromJs, this, &WebEnginePane::messageFromJs);
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

void WebEnginePane::load(const QUrl &url)
{
    if (!m_view || !url.isValid()) {
        return;
    }
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
    if (!m_bridge || payload.trimmed().isEmpty()) {
        return;
    }
    m_bridge->dispatchToWeb(payload);
}

void WebEnginePane::configureProfile()
{
    m_profile = new QWebEngineProfile("DemoProfile", this);
    m_profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    m_profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    m_profile->setSpellCheckEnabled(false);

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
    auto *page = new QWebEnginePage(m_profile, m_view);
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

