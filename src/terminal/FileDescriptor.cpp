#include "FileDescriptor.h"
#include <unistd.h>

FileDescriptor::~FileDescriptor() {
    if (m_fd >= 0) ::close(m_fd);
}

FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept
    : m_fd(std::exchange(other.m_fd, -1)) {}

FileDescriptor &FileDescriptor::operator=(FileDescriptor &&other) noexcept {
    if (this != &other) {
        reset();
        m_fd = std::exchange(other.m_fd, -1);
    }
    return *this;
}

int FileDescriptor::release() {
    return std::exchange(m_fd, -1);
}

void FileDescriptor::reset(int fd) {
    if (m_fd >= 0) ::close(m_fd);
    m_fd = fd;
}
