#pragma once

#include <QMainWindow>
#include <QUrl>

class QLineEdit;
class QSlider;
class QAction;
class WebEnginePane;
class MessageConsole;

class BrowserWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);
    ~BrowserWindow() override;

private slots:
    void loadRequestedUrl();
    void navigateHome();
    void clearProfileData();
    void showMessageConsole();
    void handleMessageFromPage(const QString &payload);
    void handleLoadFinished(bool ok);
    void applyOpacity(int sliderValue);
    void handleTransparencyToggle(bool enabled);
    void applyCustomUserAgent();
    void applyRedirectTarget();

private:
    void buildUi();
    void buildToolbar();
    void updateStatus(const QString &text, int timeoutMs = 5000);
    void updateWindowTransparency(bool transparent);
    [[nodiscard]] QUrl homeUrl() const;

    WebEnginePane *m_engine {nullptr};
    MessageConsole *m_console {nullptr};
    QLineEdit *m_addressBar {nullptr};
    QLineEdit *m_userAgentInput {nullptr};
    QLineEdit *m_redirectInput {nullptr};
    QSlider *m_opacitySlider {nullptr};
    QAction *m_transparentAction {nullptr};
};

