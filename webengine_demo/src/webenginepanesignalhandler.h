#pragma once

#include "webenginesignals.h"

class WebEnginePane;

class WebEnginePaneSignalHandler final : public WebEngineSignals
{
public:
    explicit WebEnginePaneSignalHandler(WebEnginePane *pane);

    void updateFullScreen(bool enabled);

private:
    void handleViewAttached(QWebEngineView *view);
    void handleViewDetached();
    void handleViewUrlChanged(const QUrl &url);
    void handleViewTitleChanged(const QString &title);
    void handleViewIconChanged(const QIcon &icon);
    void handleViewLoadStarted();
    void handleViewLoadProgress(int progress);
    void handleViewLoadFinished(bool ok);
    void handleViewSelectionChanged();
    void handleViewRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, int exitCode);
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    void handleViewFindTextFinished(const QWebEngineFindTextResult &result);
#endif
    void handlePageLinkHovered(const QString &url);
    void handlePageFullScreenRequested(QWebEngineFullScreenRequest request);
    void handlePageFeaturePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
    void handlePageFeaturePermissionRequestCanceled(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
    void handlePageQuotaRequested(QWebEngineQuotaRequest request);
    void handlePageSelectClientCertificate(QWebEngineClientCertificateSelection request);
    void handlePageAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator);
    void handlePageProxyAuthenticationRequired(const QUrl &requestUrl,
                                               QAuthenticator *authenticator,
                                               const QString &proxyHost);
    void handlePageRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, int exitCode);
    void handlePageWindowCloseRequested();
    void handlePageRegisterProtocolHandlerRequested(const QWebEngineRegisterProtocolHandlerRequest &request);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    void handlePageNotificationShown(QWebEngineNotification *notification);
    void handlePageNotificationClosed(QWebEngineNotification *notification);
#endif

private:
    WebEnginePane *m_pane {nullptr};
};


