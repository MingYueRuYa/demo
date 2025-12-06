#pragma once

#include <QString>
#include <QStringList>
#include <QWidget>

class QUrl;
class QWebChannel;
class QWebEngineProfile;
class QWebEngineView;
class QPoint;

class WebBridge;
class WebEngineSignals;

class WebEnginePane final : public QWidget
{
    Q_OBJECT

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

private:
    void configureProfile();
    void configureView();
    void setupChannel();
    void ensureBridge();
    void resetLoadState();
    void flushPendingMessages();

    QWebEngineView *m_view {nullptr};
    QWebEngineProfile *m_profile {nullptr};
    QWebChannel *m_channel {nullptr};
    WebBridge *m_bridge {nullptr};
    QString m_defaultUserAgent;
    bool m_lastLoadSucceeded {false};
    bool m_jsReady {false};
    QStringList m_pendingPayloads;
    WebEngineSignals *m_signalHub {nullptr};
};

