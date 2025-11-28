#pragma once

#include <QWidget>

class QUrl;
class QWebChannel;
class QWebEngineProfile;
class QWebEngineView;

class WebBridge;

class WebEnginePane final : public QWidget
{
    Q_OBJECT

public:
    explicit WebEnginePane(WebBridge *bridge = nullptr, QWidget *parent = nullptr);
    ~WebEnginePane() override;

    QWebEngineView *view() const;
    QWebEngineProfile *profile() const;
    WebBridge *bridge() const;

public slots:
    void load(const QUrl &url);
    void clearProfileData();
    void broadcastToPage(const QString &payload);

signals:
    void urlChanged(const QUrl &url);
    void loadFinished(bool ok);
    void messageFromJs(const QString &payload);

private:
    void configureProfile();
    void configureView();
    void setupChannel();
    void ensureBridge();

    QWebEngineView *m_view {nullptr};
    QWebEngineProfile *m_profile {nullptr};
    QWebChannel *m_channel {nullptr};
    WebBridge *m_bridge {nullptr};
};

