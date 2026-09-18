/**
 * BluetoothSocket/BluetoothServerSocket — the RFCOMM data plane over the
 * kernel's AF_BLUETOOTH sockets (the same layer AOSP's own stack ends at;
 * bluetoothd is not in the data path — BlueZ exposes no fd passing over
 * D-Bus, exactly like Android's stack keeps sockets out of the service).
 */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/socket.h>

#include <bluetoothsocket.h>
#include "internal/btuapi.h"
#include "internal/sdpclient.h"

namespace cdroid {

namespace {

class FdInputStream : public BluetoothSocket::InputStream {
public:
    explicit FdInputStream(int* fd) : mFd(fd) {}
    int read(char* buffer, int len) override {
        const int fd = *mFd;
        if (fd < 0) return -EBADF;
        const ssize_t rc = ::read(fd, buffer, (size_t)len);
        return rc >= 0 ? (int)rc : -errno;
    }
    void close() override { /* the socket owns the fd */ }
private:
    int* mFd;
};

class FdOutputStream : public BluetoothSocket::OutputStream {
public:
    explicit FdOutputStream(int* fd) : mFd(fd) {}
    int write(const char* buffer, int len) override {
        const int fd = *mFd;
        if (fd < 0) return -EBADF;
        const ssize_t rc = ::write(fd, buffer, (size_t)len);
        return rc >= 0 ? (int)rc : -errno;
    }
    void flush() override { /* streams are unbuffered */ }
    void close() override { /* the socket owns the fd */ }
private:
    int* mFd;
};

int openRfcommSocket() {
    return ::socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
}

/* Apply the BT_SECURITY option like AOSP does for secure/insecure
 * flavors (level selects auth+encrypt). */
void applySecurity(int fd, bool secure) {
    struct bt_security sec;
    memset(&sec, 0, sizeof(sec));
    sec.level = secure ? BT_SECURITY_MEDIUM : BT_SECURITY_LOW;
    setsockopt(fd, SOL_BLUETOOTH, BT_SECURITY, &sec, sizeof(sec));
}

} // namespace

/* ------------------------------------------------------------------ */
/* BluetoothSocket                                                     */
/* ------------------------------------------------------------------ */

BluetoothSocket::BluetoothSocket(const BluetoothDevice& device, int channel,
                                 bool secure)
    : mDevice(device), mChannel(channel), mFd(openRfcommSocket()) {
    if (mFd >= 0) applySecurity(mFd, secure);
}

BluetoothSocket::BluetoothSocket(const BluetoothDevice& device, int channel,
                                 bool secure, const BluetoothUuid& uuid)
    : mDevice(device), mChannel(channel), mServiceUuid(uuid),
      mResolveViaSdp(true), mFd(openRfcommSocket()) {
    if (mFd >= 0) applySecurity(mFd, secure);
}

BluetoothSocket::BluetoothSocket(const BluetoothDevice& device, int channel,
                                 int fd)
    : mDevice(device), mChannel(channel), mFd(fd) {}

BluetoothSocket::~BluetoothSocket() {
    close();
    delete mIn;
    delete mOut;
}

int BluetoothSocket::connect() {
    if (mFd < 0) return -EIO;
    if (mResolveViaSdp && mChannel < 0) {
        /* SDP resolve here (the AOSP contract): bounded by the sdpclient
         * timeouts; SPP falls back to the convention channel for peers
         * with no SDP server. */
        const int resolved = sdpResolveRfcommChannel(
                mDevice.getAddress(), mServiceUuid.toBytes());
        mChannel = (resolved > 0)
                ? resolved
                : (mServiceUuid == BluetoothUuid::SerialPort() ? 1 : -1);
        if (mChannel < 0) return -ENOENT;
    }
    struct sockaddr_rc addr;
    memset(&addr, 0, sizeof(addr));
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t)mChannel;
    str2ba(mDevice.getAddress().c_str(), &addr.rc_bdaddr);
    if (::connect(mFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return -errno;
    return 0;
}

int BluetoothSocket::close() {
    if (mFd < 0) return 0;
    const int rc = ::close(mFd);
    mFd = -1;
    return rc == 0 ? 0 : -errno;
}

BluetoothSocket::InputStream* BluetoothSocket::getInputStream() {
    if (mIn == nullptr) mIn = new FdInputStream(&mFd);
    return mIn;
}

BluetoothSocket::OutputStream* BluetoothSocket::getOutputStream() {
    if (mOut == nullptr) mOut = new FdOutputStream(&mFd);
    return mOut;
}

/* ------------------------------------------------------------------ */
/* BluetoothServerSocket                                               */
/* ------------------------------------------------------------------ */

BluetoothServerSocket::BluetoothServerSocket(int channel, bool secure,
                                             const std::string& name)
    : mChannel(channel), mSecure(secure), mName(name),
      mBindFailed(false),
      mListenFd(openRfcommSocket()) {
    if (mListenFd < 0) return;
    applySecurity(mListenFd, secure);
    struct sockaddr_rc addr;
    memset(&addr, 0, sizeof(addr));
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t)channel;
    bdaddr_t any;   /* BDADDR_ANY: all zeros */
    memset(&any, 0, sizeof(any));
    addr.rc_bdaddr = any;
    if (::bind(mListenFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        mBindFailed = true;   /* factory surfaces this as nullptr (AOSP throws) */
    } else if (::listen(mListenFd, 1) < 0) {
        mBindFailed = true;
    }
    if (mBindFailed) {
        ::close(mListenFd);
        mListenFd = -1;
    }
}

BluetoothServerSocket::~BluetoothServerSocket() {
    close();
}

BluetoothSocket* BluetoothServerSocket::accept(int timeoutMs) {
    if (mListenFd < 0) return nullptr;
    if (timeoutMs >= 0) {
        struct pollfd pfd = { mListenFd, POLLIN, 0 };
        const int rc = poll(&pfd, 1, timeoutMs);
        if (rc <= 0) return nullptr;   /* timeout or error */
    }
    struct sockaddr_rc addr;
    socklen_t addrlen = sizeof(addr);
    const int fd = ::accept(mListenFd, (struct sockaddr*)&addr, &addrlen);
    if (fd < 0) return nullptr;
    char address[18] = {0};
    ba2str(&addr.rc_bdaddr, address, sizeof(address));
    return new BluetoothSocket(BluetoothDevice(address),
                               addr.rc_channel, fd);
}

int BluetoothServerSocket::close() {
    if (mListenFd < 0) return 0;
    const int rc = ::close(mListenFd);
    mListenFd = -1;
    return rc == 0 ? 0 : -errno;
}

} // namespace cdroid
