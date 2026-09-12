#ifndef __CDROID_BLUETOOTH_SOCKET_H__
#define __CDROID_BLUETOOTH_SOCKET_H__

#include <cstdint>
#include <string>

#include <bluetoothdevice.h>
#include <bluetoothuuid.h>

namespace cdroid {

class BluetoothServerSocket;

/**
 * Port of android.bluetooth.BluetoothSocket (android-36): a connected
 * RFCOMM socket to a remote service. The Java class exposes the byte
 * streams java.io gives it; CDROID has no java.io, so the minimal
 * InputStream/OutputStream abstract pair lives here and the socket owns
 * the concrete fd-backed implementations (returned by value pointers —
 * getInputStream()/getOutputStream() mirror the AOSP names, lifetime =
 * the socket's).
 *
 * connect()/close()/accept() report failures as negative errno instead
 * of Java's IOException (the C++ port idiom).
 */
class BluetoothSocket {
public:
    /* java.io minimals over the socket fd. */
    class InputStream {
    public:
        virtual ~InputStream() = default;
        /* Reads up to len bytes; returns the count, 0 on EOF, -errno on
         * error. */
        virtual int read(char* buffer, int len) = 0;
        virtual void close() = 0;
    };
    class OutputStream {
    public:
        virtual ~OutputStream() = default;
        /* Writes len bytes; returns the count or -errno. */
        virtual int write(const char* buffer, int len) = 0;
        virtual void flush() = 0;
        virtual void close() = 0;
    };

    /* Client-side construction goes through BluetoothDevice
     * createRfcommSocket()/createRfcommSocketToServiceRecord(); accept()
     * on the server socket mints the connected ones. */
    BluetoothSocket(const BluetoothDevice& device, int channel, bool secure);
    /* Service-record flavor: channel -1, resolved via SDP in connect(). */
    BluetoothSocket(const BluetoothDevice& device, int channel, bool secure,
                    const BluetoothUuid& uuid);
    ~BluetoothSocket();

    BluetoothSocket(const BluetoothSocket&) = delete;
    BluetoothSocket& operator=(const BluetoothSocket&) = delete;

    /* Attempt to connect to the remote over RFCOMM (blocking). */
    int connect();
    /* Immediately close the socket (idempotent). */
    int close();

    bool isConnected() const { return mFd >= 0; }
    BluetoothDevice getRemoteDevice() const { return mDevice; }
    int getChannel() const { return mChannel; }

    InputStream* getInputStream();
    OutputStream* getOutputStream();

private:
    friend class BluetoothServerSocket;
    /* accepted-socket path: fd already connected */
    BluetoothSocket(const BluetoothDevice& device, int channel, int fd);

    BluetoothDevice mDevice;
    int mChannel;                 /* -1 = resolve via SDP in connect() */
    BluetoothUuid mServiceUuid;   /* service-record sockets */
    bool mResolveViaSdp = false;
    int mFd;
    InputStream* mIn = nullptr;
    OutputStream* mOut = nullptr;
};

/**
 * Port of android.bluetooth.BluetoothServerSocket (android-36): an
 * RFCOMM service listener. listenUsingRfcommWithServiceRecord() on the
 * adapter creates it (SDP registration lands with the SDP resolver —
 * until then use listenUsingRfcommOn(channel) with an agreed channel).
 */
class BluetoothServerSocket {
public:
    BluetoothServerSocket(int channel, bool secure, const std::string& name);
    ~BluetoothServerSocket();

    BluetoothServerSocket(const BluetoothServerSocket&) = delete;
    BluetoothServerSocket& operator=(const BluetoothServerSocket&) = delete;

    /* Block until a connection arrives; timeout<0 = forever. Returns
     * the connected socket (caller owns) or nullptr on close/error. */
    BluetoothSocket* accept(int timeoutMs = -1);
    int close();

    int getChannel() const { return mChannel; }

private:
    friend class BluetoothAdapter;   /* nullptr-on-bind-failure factory */
    bool isBoundInternal() const { return mListenFd >= 0; }
    int mChannel;
    bool mSecure;
    std::string mName;
    bool mBindFailed = false;   /* factory turns this into a nullptr return */
    int mListenFd;
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_SOCKET_H__ */
