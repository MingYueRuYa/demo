#pragma once

#include <QObject>
#include <QPointer>
#include <QIcon>
#include <QPoint>
#include <QString>
#include <QUrl>

#include <QWebEngineCertificateError>
#include <QWebEngineFindTextResult>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineNotification>
#include <QWebEnginePage>
#include <QWebEngineQuotaRequest>
#include <QWebEngineRegisterProtocolHandlerRequest>
//#include <QWebEngineLoadingInfo>
//#include <QWebEngineSelectClientCertificateRequest>

class QAuthenticator;
class QPoint;
class QWebEngineDownloadRequest;
class QWebEngineNewWindowRequest;
class QWebEngineProfile;
class QWebEngineView;

// WebEngineSignals 负责集中连接 QWebEngineView/QWebEnginePage 的常用信号，
// 方便子类继承后一次性获得所有 WebEngine 相关事件。
class WebEngineSignals : public QObject
{
    Q_OBJECT

public:
    explicit WebEngineSignals(QObject *parent = nullptr);

    void bind(QWebEngineView *view);
    void unbind();

    QWebEngineView *view() const;
    QWebEnginePage *page() const;

signals:
    void viewAttached(QWebEngineView *view);
    void viewDetached();

    void viewUrlChanged(const QUrl &url);
    void viewTitleChanged(const QString &title);
    void viewIconChanged(const QIcon &icon);
    void viewLoadStarted();
    void viewLoadProgress(int progress);
    void viewLoadFinished(bool ok);
    //void viewLoadingChanged(const QWebEngineLoadingInfo &info);
    void viewSelectionChanged();
    void viewRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, int exitCode);
    void viewZoomFactorChanged(qreal factor);
    void viewFindTextFinished(const QWebEngineFindTextResult &result);

    void pageLinkHovered(const QString &url);
    void pageFullScreenRequested(const QWebEngineFullScreenRequest &request);
    void pageFeaturePermissionRequested(const QUrl &url, QWebEnginePage::Feature feature);
    void pageFeaturePermissionRequestCanceled(const QUrl &url, QWebEnginePage::Feature feature);
    void pageQuotaRequested(const QWebEngineQuotaRequest &request);
    void pageCertificateError(const QWebEngineCertificateError &error);
    //void pageSelectClientCertificate(QWebEngineSelectClientCertificateRequest &request);
    void pageAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator, const QString &proxyHost);
    void pageProxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator, const QString &proxyHost);
    void pageRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, int exitCode);
    void pageWindowCloseRequested();
    void pageProfileChanged(QWebEngineProfile *profile);
    void pageNewWindowRequested(const QWebEngineNewWindowRequest &request);
    void pageRegisterProtocolHandlerRequested(const QWebEngineRegisterProtocolHandlerRequest &request);
    void pageDownloadRequested(QWebEngineDownloadRequest *download);
    void pageNotificationShown(QWebEngineNotification *notification);
    void pageNotificationClosed(QWebEngineNotification *notification);

private slots:
    void handleViewDestroyed();
    void handlePageChanged(QWebEnginePage *page);

private:
    void connectViewSignals(QWebEngineView *view);
    void connectPageSignals(QWebEnginePage *page);
    void disconnectViewSignals();
    void disconnectPageSignals();

    QPointer<QWebEngineView> m_view;
    QPointer<QWebEnginePage> m_page;
};


