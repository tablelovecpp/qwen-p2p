#include "core/filemanager.hpp"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QMutexLocker>
#include <filesystem>
#include <fstream>
#include <QSaveFile>

namespace p2p {

FileManager::FileManager(QObject* parent)
    : QObject(parent)
{
}

// Helper function to validate and sanitize file paths
bool FileManager::isValidPath(const QString& path, const QString& baseDirectory)
{
    if (path.isEmpty() || path.contains("..")) {
        return false;
    }
    
    // Resolve to canonical path
    QFileInfo fi(path);
    QString canonicalPath = fi.canonicalFilePath();
    QString canonicalBase = QFileInfo(baseDirectory).canonicalFilePath();
    
    // Ensure the path is within the base directory
    if (!canonicalPath.startsWith(canonicalBase + "/") && 
        canonicalPath != canonicalBase) {
        return false;
    }
    
    return true;
}

QVector<FileInfo> FileManager::scanDirectory(const QString& path, bool recursive)
{
    QVector<FileInfo> files;
    
    try {
        std::filesystem::path fsPath(path.toStdString());
        
        if (!std::filesystem::exists(fsPath)) {
            emit scanError(tr("Path does not exist: %1").arg(path));
            return files;
        }
        
        auto options = recursive 
            ? std::filesystem::recursive_directory_iterator::options::skip_permission_denied
            : std::filesystem::directory_options::none;
            
        for (const auto& entry : std::filesystem::directory_iterator(fsPath, options)) {
            if (entry.is_regular_file()) {
                FileInfo info;
                info.path = QString::fromStdString(entry.path().string());
                info.name = QString::fromStdString(entry.path().filename().string());
                info.id = generateFileId(info.path);
                info.size = static_cast<qint64>(entry.file_size());
                info.isDirectory = false;
                info.transferredBytes = 0;
                info.isComplete = false;
                
                // Calculate hash asynchronously for large files
                if (info.size < 100 * 1024 * 1024) {  // Only hash files < 100MB
                    info.hash = calculateHash(info.path);
                }
                
                files.append(info);
            }
        }
        
        emit scanCompleted(files);
    } catch (const std::filesystem::filesystem_error& e) {
        emit scanError(tr("Filesystem error: %1").arg(QString::fromStdString(e.what())));
    } catch (const std::exception& e) {
        emit scanError(tr("Error: %1").arg(QString::fromStdString(e.what())));
    }
    
    return files;
}

FileInfo FileManager::getFileInfo(const QString& filePath)
{
    FileInfo info;
    
    QFileInfo fi(filePath);
    if (!fi.exists() || !fi.isFile()) {
        return info;
    }
    
    info.path = filePath;
    info.name = fi.fileName();
    info.id = generateFileId(filePath);
    info.size = fi.size();
    info.isDirectory = fi.isDir();
    info.transferredBytes = 0;
    info.isComplete = false;
    
    return info;
}

QString FileManager::calculateHash(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    
    // Read in chunks for memory efficiency
    constexpr qint64 CHUNK_SIZE = 1024 * 1024;  // 1MB chunks
    while (!file.atEnd()) {
        QByteArray chunk = file.read(CHUNK_SIZE);
        hash.addData(chunk);
    }
    
    return QString::fromLatin1(hash.result().toHex());
}

bool FileManager::prepareForTransfer(const FileInfo& fileInfo)
{
    QMutexLocker locker(&m_mutex);
    
    QFile file(fileInfo.path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    auto state = std::make_unique<TransferState>();
    state->totalSize = fileInfo.size;
    state->transferredBytes = 0;
    state->isActive = true;
    
    m_activeTransfers[fileInfo.id] = std::move(state);
    return true;
}

bool FileManager::saveFileChunk(const FileInfo& fileInfo, const QByteArray& data, qint64 offset)
{
    QMutexLocker locker(&m_mutex);
    
    // Get the default download directory as base path
    QString baseDir = getDefaultDownloadDirectory();
    QString destPath = baseDir + "/" + fileInfo.name;
    
    // SECURITY FIX: Validate path to prevent directory traversal attacks
    if (!isValidPath(destPath, baseDir)) {
        emit transferError(fileInfo.id, tr("Invalid file path detected"));
        return false;
    }
    
    // Use QSaveFile for atomic writes to prevent corruption
    std::unique_ptr<QSaveFile> file;
    
    // For first chunk (offset == 0), use atomic write
    if (offset == 0) {
        file = std::make_unique<QSaveFile>(destPath);
    } else {
        // For subsequent chunks, we need regular QFile with append
        auto regularFile = std::make_unique<QFile>(destPath);
        if (!regularFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
            emit transferError(fileInfo.id, tr("Failed to open file for writing: %1").arg(destPath));
            return false;
        }
        regularFile->seek(offset);
        if (regularFile->write(data) != data.size()) {
            emit transferError(fileInfo.id, tr("Failed to write data"));
            return false;
        }
        
        // Update progress
        auto it = m_activeTransfers.find(fileInfo.id);
        if (it != m_activeTransfers.end()) {
            it->second->transferredBytes += data.size();
            double progress = static_cast<double>(it->second->transferredBytes) / it->second->totalSize;
            emit transferProgress(fileInfo.id, progress, it->second->transferredBytes);
            
            if (it->second->transferredBytes >= it->second->totalSize) {
                it->second->isActive = false;
                // Verify file integrity after completion
                QString finalHash = calculateHash(destPath);
                if (!fileInfo.hash.isEmpty() && finalHash != fileInfo.hash) {
                    emit transferError(fileInfo.id, tr("File integrity check failed"));
                    return false;
                }
                emit transferCompleted(fileInfo.id, destPath);
            }
        }
        return true;
    }
    
    if (!file->open(QIODevice::WriteOnly)) {
        emit transferError(fileInfo.id, tr("Failed to open file for writing: %1").arg(destPath));
        return false;
    }
    
    file->seek(offset);
    if (file->write(data) != data.size()) {
        emit transferError(fileInfo.id, tr("Failed to write data"));
        return false;
    }
    
    // Update progress
    auto it = m_activeTransfers.find(fileInfo.id);
    if (it != m_activeTransfers.end()) {
        it->second->transferredBytes += data.size();
        double progress = static_cast<double>(it->second->transferredBytes) / it->second->totalSize;
        emit transferProgress(fileInfo.id, progress, it->second->transferredBytes);
        
        if (it->second->transferredBytes >= it->second->totalSize) {
            it->second->isActive = false;
            // Commit atomic write
            if (!file->commit()) {
                emit transferError(fileInfo.id, tr("Failed to commit file write"));
                return false;
            }
            // SECURITY FIX: Verify file integrity after completion
            QString finalHash = calculateHash(destPath);
            if (!fileInfo.hash.isEmpty() && finalHash != fileInfo.hash) {
                emit transferError(fileInfo.id, tr("File integrity check failed"));
                return false;
            }
            emit transferCompleted(fileInfo.id, destPath);
        }
    }
    
    return true;
}

double FileManager::getTransferProgress(const QString& fileId) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_activeTransfers.find(fileId);
    if (it == m_activeTransfers.end() || it->second->totalSize == 0) {
        return 0.0;
    }
    
    return static_cast<double>(it->second->transferredBytes) / it->second->totalSize;
}

void FileManager::cancelTransfer(const QString& fileId)
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_activeTransfers.find(fileId);
    if (it != m_activeTransfers.end()) {
        it->second->file.close();
        m_activeTransfers.erase(it);
    }
}

QString FileManager::getDefaultDownloadDirectory()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    path += "/P2PTransfer";
    
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    return path;
}

QString FileManager::generateFileId(const QString& filePath)
{
    return QCryptographicHash::hash(filePath.toUtf8(), QCryptographicHash::Md5)
        .toHex();
}

} // namespace p2p
