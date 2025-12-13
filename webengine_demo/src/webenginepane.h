#pragma once

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QWidget>
#include <QWebEnginePage>

class QUrl;
class QWebChannel;
class QWebEngineProfile;
class QWebEngineView;
class QPoint;

class WebBridge;
class WebEngineSignals;
class WebEnginePaneSignalHandler;
class QWebEnginePage;

//namespace {

class InterceptingPage final : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit InterceptingPage(QWebEngineProfile* profile, QObject* parent = nullptr);
    void setRedirectTarget(const QUrl& target);

protected:
    virtual bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame);

private:
    QUrl m_redirectTarget{ QUrl(QStringLiteral("https://baidu.com")) };
};
//}

class WebEnginePane final : public QWidget
{
    Q_OBJECT
    friend class WebEnginePaneSignalHandler;

public:
    explicit WebEnginePane(WebBridge *bridge = nullptr, QWidget *parent = nullptr);
    ~WebEnginePane() override;

    QWebEngineView *view() const;
    QWebEngineProfile *profile() const;
    WebBridge *bridge() const;
    void setUserAgent(const QString &ua);
    QString currentUserAgent() const;
    WebEngineSignals *signalHub() const;
    bool setCookieForCurrentPage(const QString& cookieLine);
    void dumpDocumentCookies();
    void setRedirectTarget(const QUrl &url);
    QUrl redirectTarget() const;

public slots:
    void load(const QUrl &url);
    void clearProfileData();
    void broadcastToPage(const QString &payload);

signals:
    void urlChanged(const QUrl &url);
    void loadFinished(bool ok);
    void messageFromJs(const QString &payload);
    void cookiesDumped(const QString &cookies);

private slots:
    void showCustomContextMenu(const QPoint &pos);
    void handleLoadStarted();
    void handlePageReady();
    void resetLoadState();

private:
    void configureProfile();
    void configureView();
    void setupChannel();
    void ensureBridge();
    void flushPendingMessages();

private:
    QWebEngineView *m_view {nullptr};
    QWebEngineProfile *m_profile {nullptr};
    QWebChannel *m_channel {nullptr};
    WebBridge *m_bridge {nullptr};
    QString m_defaultUserAgent;
    bool m_lastLoadSucceeded {false};
    bool m_jsReady {false};
    QStringList m_pendingPayloads;
    WebEngineSignals *m_signalHub {nullptr};
    QUrl m_redirectTarget = QStringLiteral("https://baidu.com");
};

