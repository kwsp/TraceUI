#pragma once

#include <utility>

/// RAII wrapper for POSIX file descriptors.
class FileDescriptor {
public:
    FileDescriptor() = default;
    explicit FileDescriptor(int fd) : m_fd(fd) {}
    ~FileDescriptor();

    FileDescriptor(const FileDescriptor &) = delete;
    FileDescriptor &operator=(const FileDescriptor &) = delete;
    FileDescriptor(FileDescriptor &&other) noexcept;
    FileDescriptor &operator=(FileDescriptor &&other) noexcept;

    [[nodiscard]] int get() const { return m_fd; }
    [[nodiscard]] bool isValid() const { return m_fd >= 0; }
    int release();
    void reset(int fd = -1);

private:
    int m_fd = -1;
};
