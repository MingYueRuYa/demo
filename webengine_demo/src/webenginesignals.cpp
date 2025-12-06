#include "webenginesignals.h"

#include "connectguard.h"

#include <QAuthenticator>
#include <QtGlobal>
//#include <QWebEngineDownloadRequest>
//#include <QWebEngineNewWindowRequest>
#include <QWebEngineProfile>
#include <QWebEngineView>

WebEngineSignals::WebEngineSignals(QObject *parent)
    : QObject(parent)
{
}

void WebEngineSignals::bind(QWebEngineView *view)
{
    if (m_view == view) {
        if (m_page != (view ? view->page() : nullptr)) {
            handlePageChanged(view ? view->page() : nullptr);
        }
        return;
    }

    disconnectViewSignals();
    disconnectPageSignals();

    m_view = view;
    if (!m_view.isNull()) {
        connectViewSignals(m_view.data());
        handlePageChanged(m_view->page());
        //ENSURE_QT_CONNECT(m_view.data(), &QObject::destroyed, this, &WebEngineSignals::handleViewDestroyed);
        //ENSURE_QT_CONNECT(m_view.data(), &QWebEngineView::pageChanged, this, &WebEngineSignals::handlePageChanged);
        emit viewAttached(m_view.data());
    } else {
        emit viewDetached();
    }
}

void WebEngineSignals::unbind()
{
    if (m_view.isNull() && m_page.isNull()) {
        return;
    }
    disconnectViewSignals();
    disconnectPageSignals();
    m_view = nullptr;
    m_page = nullptr;
    emit viewDetached();
}

QWebEngineView *WebEngineSignals::view() const
{
    return m_view.data();
}

QWebEnginePage *WebEngineSignals::page() const
{
    return m_page.data();
}

void WebEngineSignals::handleViewDestroyed()
{
    disconnectViewSignals();
    disconnectPageSignals();
    m_view = nullptr;
    m_page = nullptr;
    emit viewDetached();
}

void WebEngineSignals::handlePageChanged(QWebEnginePage *page)
{
    if (m_page == page) {
        return;
    }
    disconnectPageSignals();
    m_page = page;
    if (m_page) {
        connectPageSignals(m_page.data());
    }
}

void WebEngineSignals::connectViewSignals(QWebEngineView *view)
{
    ENSURE_QT_CONNECT(view, &QWebEngineView::urlChanged, this, &WebEngineSignals::viewUrlChanged);
    ENSURE_QT_CONNECT(view, &QWebEngineView::titleChanged, this, &WebEngineSignals::viewTitleChanged);
    ENSURE_QT_CONNECT(view, &QWebEngineView::iconChanged, this, &WebEngineSignals::viewIconChanged);
    ENSURE_QT_CONNECT(view, &QWebEngineView::loadStarted, this, &WebEngineSignals::viewLoadStarted);
    ENSURE_QT_CONNECT(view, &QWebEngineView::loadProgress, this, &WebEngineSignals::viewLoadProgress);
    ENSURE_QT_CONNECT(view, &QWebEngineView::loadFinished, this, &WebEngineSignals::viewLoadFinished);
    //ENSURE_QT_CONNECT(view, &QWebEngineView::loadingChanged, this, &WebEngineSignals::viewLoadingChanged);
    ENSURE_QT_CONNECT(view, &QWebEngineView::selectionChanged, this, &WebEngineSignals::viewSelectionChanged);
    ENSURE_QT_CONNECT(view,
                      &QWebEngineView::renderProcessTerminated,
                      this,
                      &WebEngineSignals::viewRenderProcessTerminated);
    //ENSURE_QT_CONNECT(view, &QWebEngineView::zoomFactorChanged, this, &WebEngineSignals::viewZoomFactorChanged);
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    ENSURE_QT_CONNECT(view, &QWebEngineView::findTextFinished, this, &WebEngineSignals::viewFindTextFinished);
#endif
}

void WebEngineSignals::connectPageSignals(QWebEnginePage *page)
{
    ENSURE_QT_CONNECT(page, &QWebEnginePage::linkHovered, this, &WebEngineSignals::pageLinkHovered);
    //ENSURE_QT_CONNECT(page, &QWebEnginePage::fullScreenRequested, this, &WebEngineSignals::pageFullScreenRequested);
    //ENSURE_QT_CONNECT(page,
    //                  &QWebEnginePage::featurePermissionRequested,
    //                  this,
    //                  &WebEngineSignals::pageFeaturePermissionRequested);
    //ENSURE_QT_CONNECT(page,
    //                  &QWebEnginePage::featurePermissionRequestCanceled,
    //                  this,
    //                  &WebEngineSignals::pageFeaturePermissionRequestCanceled);
    //ENSURE_QT_CONNECT(page, &QWebEnginePage::quotaRequested, this, &WebEngineSignals::pageQuotaRequested);
    //ENSURE_QT_CONNECT(page, &QWebEnginePage::certificateError, this, &WebEngineSignals::pageCertificateError);
    //ENSURE_QT_CONNECT(page,
    //                  &QWebEnginePage::selectClientCertificate,
    //                  this,
    //                  &WebEngineSignals::pageSelectClientCertificate);
    //ENSURE_QT_CONNECT(page,
    //                  &QWebEnginePage::authenticationRequired,
    //                  this,
    //                  &WebEngineSignals::pageAuthenticationRequired);
    //ENSURE_QT_CONNECT(page,
    //                  &QWebEnginePage::proxyAuthenticationRequired,
    //                  this,
    //                  &WebEngineSignals::pageProxyAuthenticationRequired);
    //ENSURE_QT_CONNECT(page,
    //                  &QWebEnginePage::renderProcessTerminated,
    //                  this,
    //                  &WebEngineSignals::pageRenderProcessTerminated);
//    ENSURE_QT_CONNECT(page, &QWebEnginePage::windowCloseRequested, this, &WebEngineSignals::pageWindowCloseRequested);
//    //ENSURE_QT_CONNECT(page, &QWebEnginePage::profileChanged, this, &WebEngineSignals::pageProfileChanged);
//    //ENSURE_QT_CONNECT(page,
//    //                  &QWebEnginePage::newWindowRequested,
//    //                  this,
//    //                  &WebEngineSignals::pageNewWindowRequested);
//    ENSURE_QT_CONNECT(page,
//                      &QWebEnginePage::registerProtocolHandlerRequested,
//                      this,
//                      &WebEngineSignals::pageRegisterProtocolHandlerRequested);
//    //ENSURE_QT_CONNECT(page,
//    //                  &QWebEnginePage::downloadRequested,
//    //                  this,
//    //                  &WebEngineSignals::pageDownloadRequested);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    ENSURE_QT_CONNECT(page,
                      &QWebEnginePage::notificationShown,
                      this,
                      &WebEngineSignals::pageNotificationShown);
    ENSURE_QT_CONNECT(page,
                      &QWebEnginePage::notificationClosed,
                      this,
                      &WebEngineSignals::pageNotificationClosed);
#endif
}

void WebEngineSignals::disconnectViewSignals()
{
    if (!m_view.isNull()) {
        disconnect(m_view.data(), nullptr, this, nullptr);
    }
}

void WebEngineSignals::disconnectPageSignals()
{
    if (!m_page.isNull()) {
        disconnect(m_page.data(), nullptr, this, nullptr);
    }
}


