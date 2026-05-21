#ifndef P2PTRANSFER_UI_MAINVIEW_HPP
#define P2PTRANSFER_UI_MAINVIEW_HPP

#include <QObject>
#include <QQmlEngine>

namespace p2p {

class Application;

/**
 * @brief UI controller for the main view
 * 
 * Bridges the application backend with QML UI, providing
 * a clean interface for all UI operations.
 */
class MainView : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyChanged)
    
public:
    explicit MainView(Application* app, QObject* parent = nullptr);
    ~MainView() override;
    
    /**
     * @brief Get current status message
     * @return Status message string
     */
    QString statusMessage() const { return m_statusMessage; }
    
    /**
     * @brief Check if UI is in busy state
     * @return True if busy
     */
    bool isBusy() const { return m_isBusy; }
    
    /**
     * @brief Format file size for display
     * @param bytes Size in bytes
     * @return Formatted string (e.g., "1.5 MB")
     */
    Q_INVOKABLE static QString formatFileSize(qint64 bytes);
    
    /**
     * @brief Format transfer speed for display
     * @param bytesPerSecond Speed in bytes/second
     * @return Formatted string (e.g., "2.3 MB/s")
     */
    Q_INVOKABLE static QString formatSpeed(qint64 bytesPerSecond);
    
    /**
     * @brief Format time duration for display
     * @param seconds Duration in seconds
     * @return Formatted string (e.g., "5m 30s")
     */
    Q_INVOKABLE static QString formatDuration(int seconds);
    
    /**
     * @brief Open file dialog to select files
     * @return List of selected file paths
     */
    Q_INVOKABLE QVariantList openFileDialog();
    
    /**
     * @brief Open folder dialog to select directory
     * @return Selected folder path
     */
    Q_INVOKABLE QString openFolderDialog();
    
    /**
     * @brief Show notification to user
     * @param title Notification title
     * @param message Notification message
     * @param type Notification type (info, warning, error, success)
     */
    Q_INVOKABLE void showNotification(const QString& title, const QString& message, 
                                      const QString& type = "info");
    
    /**
     * @brief Copy text to clipboard
     * @param text Text to copy
     */
    Q_INVOKABLE void copyToClipboard(const QString& text);
    
    /**
     * @brief Open external URL in browser
     * @param url URL to open
     */
    Q_INVOKABLE void openUrl(const QString& url);
    
    /**
     * @brief Get system theme preference
     * @return "light", "dark", or "system"
     */
    Q_INVOKABLE static QString getSystemTheme();
    
signals:
    void statusMessageChanged();
    void busyChanged();
    void requestShowFileLocation(const QString& filePath);
    
public slots:
    /**
     * @brief Update status message
     * @param message New status message
     */
    void setStatusMessage(const QString& message);
    
    /**
     * @brief Set busy state
     * @param busy Whether UI is busy
     */
    void setBusy(bool busy);
    
private:
    Application* m_app;
    QString m_statusMessage;
    bool m_isBusy = false;
};

} // namespace p2p

#endif // P2PTRANSFER_UI_MAINVIEW_HPP
