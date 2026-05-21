#include "ui/mainview.hpp"
#include "core/application.hpp"
#include <QFileDialog>
#include <QClipboard>
#include <QGuiApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QStyleHints>

namespace p2p {

MainView::MainView(Application* app, QObject* parent)
    : QObject(parent)
    , m_app(app)
{
}

MainView::~MainView() = default;

QString MainView::formatFileSize(qint64 bytes)
{
    const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < units.size() - 1) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(size, 0, 'f', unitIndex == 0 ? 0 : 1).arg(units[unitIndex]);
}

QString MainView::formatSpeed(qint64 bytesPerSecond)
{
    if (bytesPerSecond <= 0) {
        return "0 B/s";
    }
    
    const QStringList units = {"B/s", "KB/s", "MB/s", "GB/s"};
    int unitIndex = 0;
    double speed = static_cast<double>(bytesPerSecond);
    
    while (speed >= 1024.0 && unitIndex < units.size() - 1) {
        speed /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(speed, 0, 'f', 1).arg(units[unitIndex]);
}

QString MainView::formatDuration(int seconds)
{
    if (seconds < 0) {
        return "--:--";
    }
    
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    if (hours > 0) {
        return QString("%1h %2m").arg(hours).arg(minutes);
    } else if (minutes > 0) {
        return QString("%1m %2s").arg(minutes).arg(secs);
    } else {
        return QString("%1s").arg(secs);
    }
}

QVariantList MainView::openFileDialog()
{
    QVariantList result;
    
    QStringList files = QFileDialog::getOpenFileNames(
        nullptr,
        tr("Select Files to Send"),
        QDir::homePath(),
        tr("All Files (*.*)")
    );
    
    for (const QString& file : files) {
        result.append(file);
    }
    
    return result;
}

QString MainView::openFolderDialog()
{
    return QFileDialog::getExistingDirectory(
        nullptr,
        tr("Select Folder to Share"),
        QDir::homePath()
    );
}

void MainView::showNotification(const QString& title, const QString& message, 
                                 const QString& type)
{
    // In a real implementation, this would trigger a QML notification
    // For now, we just log it
    qDebug() << "Notification [" << type << "]:" << title << "-" << message;
    
    // Emit a signal that QML can listen to
    emit statusMessageChanged();
}

void MainView::copyToClipboard(const QString& text)
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
}

void MainView::openUrl(const QString& url)
{
    QDesktopServices::openUrl(QUrl(url));
}

QString MainView::getSystemTheme()
{
    auto* app = QGuiApplication::instance();
    if (!app) {
        return "system";
    }
    
    Qt::ColorScheme scheme = app->styleHints()->colorScheme();
    
    switch (scheme) {
        case Qt::ColorScheme::Dark:
            return "dark";
        case Qt::ColorScheme::Light:
            return "light";
        default:
            return "system";
    }
}

void MainView::setStatusMessage(const QString& message)
{
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged();
    }
}

void MainView::setBusy(bool busy)
{
    if (m_isBusy != busy) {
        m_isBusy = busy;
        emit busyChanged();
    }
}

} // namespace p2p
