#include "webenginepanesignalhandler.h"

#include "connectguard.h"
#include "webenginepane.h"
#include <QWebEngineView>

#include <QAuthenticator>
#include <QDebug>
#include <QIcon>

WebEnginePaneSignalHandler::WebEnginePaneSignalHandler(WebEnginePane *pane)
    : WebEngineSignals(pane)
    , m_pane(pane)
{
    Q_ASSERT(m_pane);

    ENSURE_QT_CONNECT(this, &WebEngineSignals::viewAttached, this, &WebEnginePaneSignalHandler::handleViewAttached);
    ENSURE_QT_CONNECT(this, &WebEngineSignals::viewDetached, this, &WebEnginePaneSignalHandler::handleViewDetached);
    ENSURE_QT_CONNECT(this, &WebEngineSignals::viewUrlChanged, this, &WebEnginePaneSignalHandler::handleViewUrlChanged);
    ENSURE_QT_CONNECT(this, &WebEngineSignals::viewTitleChanged, this, &WebEnginePaneSignalHandler::handleViewTitleChanged);
    ENSURE_QT_CONNECT(this, &WebEngineSignals::viewIconChanged, this, &WebEnginePaneSignalHandler::handleViewIconChanged);
    ENSURE_QT_CONNECT(this, &WebEngineSignals::viewLoadStarted, this, &WebEnginePaneSignalHandler::handleViewLoadStarted);
    ENSURE_QT_CONNECT(this, &WebEngineSignals::viewLoadProgress, this, &WebEnginePaneSignalHandler::handleViewLoadProgress);
    ENSURE_QT_CONNECT(this, &WebEngineSignals::viewLoadFinished, this, &WebEnginePaneSignalHandler::handleViewLoadFinished);
    ENSURE_QT_CONNECT(this, &WebEngineSignals::viewSelectionChanged, this, &WebEnginePaneSignalHandler::handleViewSelectionChanged);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::viewRenderProcessTerminated,
                      this,
                      &WebEnginePaneSignalHandler::handleViewRenderProcessTerminated);
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::viewFindTextFinished,
                      this,
                      &WebEnginePaneSignalHandler::handleViewFindTextFinished);
#endif

    ENSURE_QT_CONNECT(this, &WebEngineSignals::pageLinkHovered, this, &WebEnginePaneSignalHandler::handlePageLinkHovered);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageFullScreenRequested,
                      this,
                      &WebEnginePaneSignalHandler::handlePageFullScreenRequested);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageFeaturePermissionRequested,
                      this,
                      &WebEnginePaneSignalHandler::handlePageFeaturePermissionRequested);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageFeaturePermissionRequestCanceled,
                      this,
                      &WebEnginePaneSignalHandler::handlePageFeaturePermissionRequestCanceled);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageQuotaRequested,
                      this,
                      &WebEnginePaneSignalHandler::handlePageQuotaRequested);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageSelectClientCertificate,
                      this,
                      &WebEnginePaneSignalHandler::handlePageSelectClientCertificate);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageAuthenticationRequired,
                      this,
                      &WebEnginePaneSignalHandler::handlePageAuthenticationRequired);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageProxyAuthenticationRequired,
                      this,
                      &WebEnginePaneSignalHandler::handlePageProxyAuthenticationRequired);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageRenderProcessTerminated,
                      this,
                      &WebEnginePaneSignalHandler::handlePageRenderProcessTerminated);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageWindowCloseRequested,
                      this,
                      &WebEnginePaneSignalHandler::handlePageWindowCloseRequested);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageRegisterProtocolHandlerRequested,
                      this,
                      &WebEnginePaneSignalHandler::handlePageRegisterProtocolHandlerRequested);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageNotificationShown,
                      this,
                      &WebEnginePaneSignalHandler::handlePageNotificationShown);
    ENSURE_QT_CONNECT(this,
                      &WebEngineSignals::pageNotificationClosed,
                      this,
                      &WebEnginePaneSignalHandler::handlePageNotificationClosed);
#endif
}

void WebEnginePaneSignalHandler::updateFullScreen(bool enabled)
{
    auto *window = m_pane ? m_pane->window() : nullptr;
    if (!window) {
        return;
    }
    if (enabled) {
        window->showFullScreen();
    } else {
        window->showNormal();
    }
}

void WebEnginePaneSignalHandler::handleViewAttached(QWebEngineView *view)
{
    Q_UNUSED(view);
    if (m_pane) {
        m_pane->resetLoadState();
    }
}

void WebEnginePaneSignalHandler::handleViewDetached()
{
    if (m_pane) {
        m_pane->resetLoadState();
    }
}

void WebEnginePaneSignalHandler::handleViewUrlChanged(const QUrl &url)
{
    if (!m_pane) {
        return;
    }
    Q_EMIT m_pane->urlChanged(url);
}

void WebEnginePaneSignalHandler::handleViewTitleChanged(const QString &title)
{
    auto *window = m_pane ? m_pane->window() : nullptr;
    if (window) {
        window->setWindowTitle(title);
    }
}

void WebEnginePaneSignalHandler::handleViewIconChanged(const QIcon &icon)
{
    auto *window = m_pane ? m_pane->window() : nullptr;
    if (window) {
        window->setWindowIcon(icon);
    }
}

void WebEnginePaneSignalHandler::handleViewLoadStarted()
{
    if (m_pane) {
        m_pane->handleLoadStarted();
    }
}

void WebEnginePaneSignalHandler::handleViewLoadProgress(int progress)
{
    Q_UNUSED(progress);
}

void WebEnginePaneSignalHandler::handleViewLoadFinished(bool ok)
{
    if (!m_pane) {
        return;
    }
    m_pane->m_lastLoadSucceeded = ok;
    if (m_pane->m_lastLoadSucceeded && m_pane->m_jsReady) {
        m_pane->flushPendingMessages();
    }
    Q_EMIT m_pane->loadFinished(ok);
}

void WebEnginePaneSignalHandler::handleViewSelectionChanged()
{
    // no-op hook for future use
}

void WebEnginePaneSignalHandler::handleViewRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status,
                                                                   int exitCode)
{
    qWarning() << "QWebEngineView render process terminated" << status << exitCode;
    if (m_pane) {
        m_pane->resetLoadState();
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
void WebEnginePaneSignalHandler::handleViewFindTextFinished(const QWebEngineFindTextResult &result)
{
    Q_UNUSED(result);
}
#endif

void WebEnginePaneSignalHandler::handlePageLinkHovered(const QString &url)
{
    Q_UNUSED(url);
}

void WebEnginePaneSignalHandler::handlePageFullScreenRequested(QWebEngineFullScreenRequest request)
{
    request.accept();
    updateFullScreen(request.toggleOn());
}

void WebEnginePaneSignalHandler::handlePageFeaturePermissionRequested(const QUrl &securityOrigin,
                                                                      QWebEnginePage::Feature feature)
{
    if (auto *enginePage = page()) {
        enginePage->setFeaturePermission(securityOrigin, feature, QWebEnginePage::PermissionGrantedByUser);
    }
}

void WebEnginePaneSignalHandler::handlePageFeaturePermissionRequestCanceled(const QUrl &securityOrigin,
                                                                            QWebEnginePage::Feature feature)
{
    if (auto *enginePage = page()) {
        enginePage->setFeaturePermission(securityOrigin, feature, QWebEnginePage::PermissionDeniedByUser);
    }
}

void WebEnginePaneSignalHandler::handlePageQuotaRequested(QWebEngineQuotaRequest request)
{
    request.accept();
}

void WebEnginePaneSignalHandler::handlePageSelectClientCertificate(QWebEngineClientCertificateSelection request)
{
    Q_UNUSED(request);
    qWarning() << "Client certificate selection requested but not implemented.";
}

void WebEnginePaneSignalHandler::handlePageAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator)
{
    Q_UNUSED(requestUrl);
    if (authenticator) {
        authenticator->setUser(QString());
        authenticator->setPassword(QString());
    }
}

void WebEnginePaneSignalHandler::handlePageProxyAuthenticationRequired(const QUrl &requestUrl,
                                                                       QAuthenticator *authenticator,
                                                                       const QString &proxyHost)
{
    Q_UNUSED(requestUrl);
    Q_UNUSED(proxyHost);
    if (authenticator) {
        authenticator->setUser(QString());
        authenticator->setPassword(QString());
    }
}

void WebEnginePaneSignalHandler::handlePageRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status,
                                                                   int exitCode)
{
    handleViewRenderProcessTerminated(status, exitCode);
}

void WebEnginePaneSignalHandler::handlePageWindowCloseRequested()
{
    if (auto *view = m_pane ? m_pane->view() : nullptr) {
        view->close();
    }
}

void WebEnginePaneSignalHandler::handlePageRegisterProtocolHandlerRequested(
    const QWebEngineRegisterProtocolHandlerRequest &request)
{
    auto mutableRequest = request;
    mutableRequest.accept();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
void WebEnginePaneSignalHandler::handlePageNotificationShown(QWebEngineNotification *notification)
{
    if (notification) {
        notification->show();
    }
}

void WebEnginePaneSignalHandler::handlePageNotificationClosed(QWebEngineNotification *notification)
{
    if (notification) {
        notification->close();
    }
}
#endif


