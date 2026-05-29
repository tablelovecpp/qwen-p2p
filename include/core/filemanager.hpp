#ifndef P2PTRANSFER_CORE_FILEMANAGER_HPP
#define P2PTRANSFER_CORE_FILEMANAGER_HPP

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QVector>
#include <QMutex>
#include <memory>
#include <filesystem>

namespace p2p {

/**
 * @brief Information about a file to be transferred
 */
struct FileInfo {
    QString id;
    QString name;
    QString path;
    qint64 size;
    QString hash;
    bool isDirectory;
    qint64 transferredBytes;
    bool isComplete;
    
    FileInfo() 
        : size(0)
        , isDirectory(false)
        , transferredBytes(0)
        , isComplete(false) {}
};

/**
 * @brief Manages file operations for P2P transfers
 * 
 * Handles file discovery, hashing, and transfer progress tracking
 * using C++23 filesystem features for optimal performance.
 */
class FileManager : public QObject {
    Q_OBJECT
    
public:
    explicit FileManager(QObject* parent = nullptr);
    ~FileManager() override = default;
    
    /**
     * @brief Scan a directory for files to share
     * @param path Directory path to scan
     * @param recursive Whether to scan subdirectories
     * @return List of file information
     */
    QVector<FileInfo> scanDirectory(const QString& path, bool recursive = true);
    
    /**
     * @brief Get file information for a single file
     * @param filePath Path to the file
     * @return File information
     */
    FileInfo getFileInfo(const QString& filePath);
    
    /**
     * @brief Calculate SHA-256 hash of a file
     * @param filePath Path to the file
     * @return Hash string
     */
    QString calculateHash(const QString& filePath);
    
    /**
     * @brief Prepare a file for transfer
     * @param fileInfo Information about the file
     * @return True if preparation successful
     */
    bool prepareForTransfer(const FileInfo& fileInfo);
    
    /**
     * @brief Save received file data
     * @param fileInfo Information about the file
     * @param data File data chunk
     * @param offset Offset in the file
     * @return True if save successful
     */
    bool saveFileChunk(const FileInfo& fileInfo, const QByteArray& data, qint64 offset);
    
    /**
     * @brief Get transfer progress for a file
     * @param fileId File ID
     * @return Progress as percentage (0.0 - 1.0)
     */
    double getTransferProgress(const QString& fileId) const;
    
    /**
     * @brief Cancel an ongoing transfer
     * @param fileId File ID to cancel
     */
    void cancelTransfer(const QString& fileId);
    
    /**
     * @brief Get default download directory
     * @return Default download directory path
     */
    static QString getDefaultDownloadDirectory();
    
signals:
    void scanCompleted(const QVector<FileInfo>& files);
    void scanError(const QString& error);
    void transferProgress(const QString& fileId, double progress, qint64 bytesTransferred);
    void transferCompleted(const QString& fileId, const QString& savedPath);
    void transferError(const QString& fileId, const QString& error);
    
private:
    struct TransferState {
        QFile file;
        qint64 totalSize;
        qint64 transferredBytes;
        bool isActive;
    };
    
    QMap<QString, std::unique_ptr<TransferState>> m_activeTransfers;
    mutable QMutex m_mutex;
    
    QString generateFileId(const QString& filePath);
    
    /**
     * @brief Validate file path to prevent directory traversal attacks
     * @param path The path to validate
     * @param baseDirectory The allowed base directory
     * @return True if path is valid and within base directory
     */
    bool isValidPath(const QString& path, const QString& baseDirectory);
};

} // namespace p2p

#endif // P2PTRANSFER_CORE_FILEMANAGER_HPP
