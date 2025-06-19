#include "utils/FileUtils.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <iostream>

namespace fs = std::filesystem;

std::string FileUtils::ReadFileToString(const std::string& filePath) {
    if (!FileExists(filePath)) {
        std::cerr << "[ERROR] File not found: " << filePath << std::endl;
        return "";
    }

    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open file: " << filePath << std::endl;
        return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

bool FileUtils::WriteStringToFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open file for writing: " << filePath << std::endl;
        return false;
    }

    file << content;
    file.close();
    return true;
}

bool FileUtils::FileExists(const std::string& filePath) {
    return fs::exists(filePath);
}

long long FileUtils::GetFileSize(const std::string& filePath) {
    if (!FileExists(filePath)) {
        return -1;
    }
    return fs::file_size(filePath);
}

bool FileUtils::CreateDirectory(const std::string& dirPath) {
    try {
        return fs::create_directories(dirPath);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to create directory: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> FileUtils::GetFilesInDirectory(const std::string& dirPath) {
    std::vector<std::string> files;

    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        std::cerr << "[ERROR] Invalid directory path: " << dirPath << std::endl;
        return files;
    }

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (fs::is_regular_file(entry.status())) {
            files.push_back(entry.path().string());
        }
    }

    return files;
}

bool FileUtils::DeleteFile(const std::string& filePath) {
    if (!FileExists(filePath)) {
        std::cerr << "[WARN] File does not exist: " << filePath << std::endl;
        return false;
    }

    try {
        return fs::remove(filePath);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to delete file: " << e.what() << std::endl;
        return false;
    }
}

bool FileUtils::ExtractArchive(const std::string& archivePath, const std::string& outputDir) {
    // TODO: 实现 ZIP / RAR 解压逻辑
    // 可以使用第三方库如 libzip、miniz 或系统调用（如 Linux 下的 unzip）
    std::cerr << "[ERROR] ExtractArchive is not implemented yet." << std::endl;
    return false;
}