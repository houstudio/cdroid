/**
 * BluetoothAdapter — port of android.bluetooth.BluetoothAdapter
 * (android-36), module phase: process singleton over the BlueZ D-Bus
 * transport (BluezClient). See the header for the state-mapping notes.
 */
#include <algorithm>
#include <cstdio>

#include <bluetoothadapter.h>
#include <bluetoothpan.h>
#include <bluezclient.h>
#include <bluetoothsocket.h>
#include <bluetoothgatt.h>
#include <bluetoothle.h>

namespace cdroid {

BluetoothAdapter& BluetoothAdapter::getDefaultAdapter() {
    /* Connect ONCE, here in the singleton's guarded init — never on
     * subsequent accesses: BluetoothDevice getters funnel through this
     * function, and monitor-thread dispatches (which hold the bus lock)
     * reach those getters; a per-access connect() self-deadlocks them.
     * Reconnection is the monitor's own job. */
    static BluetoothAdapter instance;
    return instance;
}

BluetoothAdapter::BluetoothAdapter()
    : mClient(*makeClient(this)) {
    mClient.connect();
}

BluetoothAdapter::~BluetoothAdapter() {
    delete &mClient;
}

BluezClient* BluetoothAdapter::makeClient(BluetoothAdapter* adapter) {
    /* BluezClient event slot (EventSet + lambdas — value semantics, no
     * listener object to own). Runs in the ctor's member-init list: only
     * the adapter POINTER is captured here, never dereferenced before
     * the constructor body runs. */
    BluezClient::Events events;
    events.onAdapterBoolChanged =
            [adapter](const std::string& name, bool value) {
                adapter->onAdapterBoolChanged(name, value);
            };
    events.onAdapterStringChanged =
            [adapter](const std::string& name, const std::string& value) {
                adapter->onAdapterStringChanged(name, value);
            };
    events.onDeviceAdded = [adapter](const BluezDevice& device) {
        adapter->onDeviceAdded(device);
    };
    events.onDevicePropertyChanged =
            [adapter](const BluezDevice& device, const std::string& name) {
                adapter->onDevicePropertyChanged(device, name);
            };
    events.onDeviceRemoved = [adapter](const std::string& objectPath) {
        adapter->onDeviceRemoved(objectPath);
    };
    events.onBluezDisconnected = [adapter] { adapter->onBluezDisconnected(); };
    events.onBluezReconnected = [adapter] { adapter->onBluezReconnected(); };
    events.onGattCharacteristicChanged =
            [adapter](const BluezGattCharacteristic& ch) {
                adapter->onGattCharacteristicChanged(ch);
            };
    events.onPairingPinRequested = [adapter](const std::string& address) {
        adapter->onPairingPinRequested(address);
    };
    events.onPairingPasskeyRequested = [adapter](const std::string& address) {
        adapter->onPairingPasskeyRequested(address);
    };
    events.onPairingConfirmationRequested =
            [adapter](const std::string& address, uint32_t passkey) {
                adapter->onPairingConfirmationRequested(address, passkey);
            };
    events.onPairingConsentRequested = [adapter](const std::string& address) {
        adapter->onPairingConsentRequested(address);
    };
    events.onDisplayPasskey =
            [adapter](const std::string& address, uint32_t passkey) {
                adapter->onDisplayPasskey(address, passkey);
            };
    events.onPairingCancelled = [adapter] { adapter->onPairingCancelled(); };
    return new BluezClient(events);
}

/* --- radio power ----------------------------------------------------- */

bool BluetoothAdapter::enable() {
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mAdapterState == STATE_ON || mAdapterState == STATE_TURNING_ON)
            return true;
    }
    if (!mClient.setAdapterBool("Powered", true)) return false;
    /* BlueZ applies Power asynchronously — report the transition now,
     * PropertiesChanged(Powered=true) lands the STATE_ON. */
    setStateAndNotify(STATE_TURNING_ON);
    return true;
}

bool BluetoothAdapter::disable() {
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mAdapterState == STATE_OFF || mAdapterState == STATE_TURNING_OFF)
            return true;
    }
    if (!mClient.setAdapterBool("Powered", false)) return false;
    setStateAndNotify(STATE_TURNING_OFF);
    return true;
}

bool BluetoothAdapter::isEnabled() {
    bool powered = false;
    if (!mClient.getAdapterBool("Powered", powered)) return false;
    return powered;
}

int BluetoothAdapter::getState() {
    bool powered = false;
    const bool know = mClient.getAdapterBool("Powered", powered);
    std::lock_guard<std::mutex> lock(mStateMutex);
    if (!know) {
        /* no adapter / no bluez: settle out of any transitional state */
        if (mAdapterState != STATE_OFF) mAdapterState = STATE_OFF;
        return mAdapterState;
    }
    const int target = powered ? STATE_ON : STATE_OFF;
    if (mAdapterState != target) mAdapterState = target;
    return mAdapterState;
}

/* --- identity ---------------------------------------------------------- */

std::string BluetoothAdapter::getAddress() {
    /* Cached Adapter1.Address (the snapshot + signals maintain it). */
    std::string address;
    if (mClient.getAdapterString("Address", address)) return address;
    return std::string();
}

std::string BluetoothAdapter::getName() {
    std::string name;
    if (mClient.getAdapterString("Alias", name)) return name;
    return std::string();
}

bool BluetoothAdapter::setName(const std::string& name) {
    return mClient.setAdapterString("Alias", name);
}

/* --- discovery ---------------------------------------------------------- */

bool BluetoothAdapter::startDiscovery() {
    if (!mClient.startDiscovery()) return false;
    /* Listeners fire from the Discovering PropertiesChanged — the
     * single source of truth — EXCEPT when BlueZ session-refcounting
     * swallows the flip: discovery already active under another client
     * makes StartDiscovery succeed with no signal, and the listener
     * would never fire (review round 2). Mirror the target state into
     * the local edge detector so both paths deliver exactly once. */
    bool was = false;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        was = mDiscovering;
        mDiscovering = true;
    }
    if (!was) {
        notifyDiscoveryStarted();
    }
    return true;
}

bool BluetoothAdapter::cancelDiscovery() {
    if (!mClient.cancelDiscovery()) return false;
    /* Same session-refcount consideration as startDiscovery. */
    bool was = false;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        was = mDiscovering;
        mDiscovering = false;
    }
    if (was) {
        notifyDiscoveryFinished();
    }
    return true;
}

bool BluetoothAdapter::isDiscovering() {
    bool discovering = false;
    if (!mClient.getAdapterBool("Discovering", discovering)) {
        std::lock_guard<std::mutex> lock(mStateMutex);
        return mDiscovering;
    }
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        mDiscovering = discovering;
    }
    return discovering;
}

/* --- RFCOMM listeners ------------------------------------------------------- */

BluetoothServerSocket* BluetoothAdapter::listenUsingRfcommOn(int channel) {
    return new BluetoothServerSocket(channel, true, std::string());
}

BluetoothServerSocket* BluetoothAdapter::listenUsingInsecureRfcommOn(
        int channel) {
    return new BluetoothServerSocket(channel, false, std::string());
}

BluetoothServerSocket* BluetoothAdapter::listenUsingRfcommWithServiceRecord(
        const std::string& name, const BluetoothUuid& uuid) {
    /* SDP record registration needs the radio (deferred with the SDP
     * resolver); the listener serves the conventional SPP channel. */
    if (uuid == BluetoothUuid::SerialPort())
        return new BluetoothServerSocket(1, true, name);
    return nullptr;
}

BluetoothServerSocket* BluetoothAdapter::listenUsingInsecureRfcommWithServiceRecord(
        const std::string& name, const BluetoothUuid& uuid) {
    if (uuid == BluetoothUuid::SerialPort())
        return new BluetoothServerSocket(1, false, name);
    return nullptr;
}

BluetoothLeScanner* BluetoothAdapter::getBluetoothLeScanner() {
    if (mLeScanner == nullptr) mLeScanner = new BluetoothLeScanner(*this);
    return mLeScanner;
}

void BluetoothAdapter::registerGattSession(
        const std::shared_ptr<BluetoothGatt>& session) {
    std::lock_guard<std::mutex> lock(mStateMutex);
    mGattSessions.push_back(session);
}

void BluetoothAdapter::unregisterGattSession(BluetoothGatt* session) {
    std::lock_guard<std::mutex> lock(mStateMutex);
    mGattSessions.erase(std::remove_if(mGattSessions.begin(),
                                       mGattSessions.end(),
                                       [session](const std::weak_ptr<BluetoothGatt>& w) {
                                           return w.lock().get() == session;
                                       }),
                        mGattSessions.end());
}

void BluetoothAdapter::onGattCharacteristicChanged(
        const BluezGattCharacteristic& ch) {
    /* Lock the registry into strong refs: a session closed+deleted by
     * the app between copy and dispatch was a use-after-free (review
     * round 2) — the strong ref keeps it alive across the callback. */
    std::vector<std::shared_ptr<BluetoothGatt>> sessions;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        sessions.reserve(mGattSessions.size());
        for (auto& weak : mGattSessions) {
            if (auto strong = weak.lock()) sessions.push_back(std::move(strong));
        }
    }
    for (const auto& gatt : sessions)
        gatt->onCharacteristicChangedInternal(ch.objectPath, ch.value);
}

/* --- profile proxies ----------------------------------------------------------- */

bool BluetoothAdapter::getProfileProxy(
        const BluetoothProfile::ServiceListener& listener, int profile) {
    /* The proxy is minted fresh per call; only fire the slot when the
     * caller set one (an all-empty listener is a valid no-op). */
    if (!listener.onServiceConnected) return false;
    switch (profile) {
    case BluetoothProfile::A2DP:
        listener.onServiceConnected(profile, new BluetoothA2dp());
        return true;
    case BluetoothProfile::HEADSET:
        listener.onServiceConnected(profile, new BluetoothHeadset());
        return true;
    case BluetoothProfile::PAN:
        listener.onServiceConnected(profile,
                new BluetoothPan(*this));
        return true;
    default:
        return false;   /* profile not ported */
    }
}

void BluetoothAdapter::closeProfileProxy(int /*profile*/,
                                         BluetoothProfile* proxy) {
    delete proxy;
}

/* --- pairing agent ----------------------------------------------------------- */

bool BluetoothAdapter::registerPairingAgent(const std::string& capability) {
    return mClient.registerAgent(capability);
}

void BluetoothAdapter::addPairingListener(const BluetoothPairingListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mPairingListeners.push_back(listener);
}

void BluetoothAdapter::removePairingListener(const BluetoothPairingListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mPairingListeners.erase(std::remove(mPairingListeners.begin(),
                                        mPairingListeners.end(), listener),
                            mPairingListeners.end());
}

bool BluetoothAdapter::replyPairingPin(const std::string& pin) {
    return mClient.replyPairingPin(pin);
}

bool BluetoothAdapter::replyPairingPasskey(uint32_t passkey) {
    return mClient.replyPairingPasskey(passkey);
}

bool BluetoothAdapter::replyPairingConfirmation(bool confirm) {
    return mClient.replyPairingConfirmation(confirm);
}

void BluetoothAdapter::cancelPairingUserInput() {
    mClient.cancelPairingReply();
}

void BluetoothAdapter::onPairingPinRequested(const std::string& address) {
    notifyPairingRequest(BluetoothDevice(address),
                         BluetoothDevice::PAIRING_VARIANT_PIN, 0);
}

void BluetoothAdapter::onPairingPasskeyRequested(const std::string& address) {
    notifyPairingRequest(BluetoothDevice(address),
                         BluetoothDevice::PAIRING_VARIANT_PASSKEY, 0);
}

void BluetoothAdapter::onPairingConfirmationRequested(const std::string& address,
                                                    uint32_t passkey) {
    /* The passkey rides along (AOSP carries it as EXTRA_PAIRING_KEY on
     * ACTION_PAIRING_REQUEST) — the dialog shows the 6-digit code. */
    notifyPairingRequest(BluetoothDevice(address),
            BluetoothDevice::PAIRING_VARIANT_PASSKEY_CONFIRMATION, passkey);
}

void BluetoothAdapter::onPairingConsentRequested(const std::string& address) {
    notifyPairingRequest(BluetoothDevice(address),
                         BluetoothDevice::PAIRING_VARIANT_CONSENT, 0);
}

void BluetoothAdapter::onDisplayPasskey(const std::string& address,
                                        uint32_t passkey) {
    notifyDisplayPasskey(BluetoothDevice(address), passkey, 0);
}

void BluetoothAdapter::onPairingCancelled() {
    notifyPairingCancelled(BluetoothDevice(std::string()));
}

/* --- remote devices ------------------------------------------------------- */

BluetoothDevice BluetoothAdapter::getRemoteDevice(const std::string& address) {
    return BluetoothDevice(address);
}

std::vector<BluetoothDevice> BluetoothAdapter::getBondedDevices() {
    std::vector<BluetoothDevice> out;
    for (const BluezDevice& d : mClient.getDevices()) {
        if (d.paired) out.push_back(BluetoothDevice(d.address));
    }
    return out;
}

/* --- listener plumbing (value semantics) -------------------------------------- */

void BluetoothAdapter::addAdapterStateListener(const AdapterStateListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mStateListeners.push_back(listener);
}
void BluetoothAdapter::removeAdapterStateListener(const AdapterStateListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mStateListeners.erase(std::remove(mStateListeners.begin(),
                                      mStateListeners.end(), listener),
                          mStateListeners.end());
}
void BluetoothAdapter::addDiscoveryListener(const DiscoveryListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mDiscoveryListeners.push_back(listener);
}
void BluetoothAdapter::removeDiscoveryListener(const DiscoveryListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mDiscoveryListeners.erase(std::remove(mDiscoveryListeners.begin(),
                                          mDiscoveryListeners.end(), listener),
                              mDiscoveryListeners.end());
}
void BluetoothAdapter::addBondStateListener(const BondStateListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mBondListeners.push_back(listener);
}
void BluetoothAdapter::removeBondStateListener(const BondStateListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mBondListeners.erase(std::remove(mBondListeners.begin(),
                                     mBondListeners.end(), listener),
                         mBondListeners.end());
}

/* --- notify tails: snapshot under the lock, invoke without it ----------------- */

void BluetoothAdapter::notifyAdapterStateChanged(int newState, int prevState) {
    std::vector<AdapterStateListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mStateListeners;
    }
    /* Writable copies: CallbackBase::operator() is non-const. */
    for (AdapterStateListener l : listeners) l(newState, prevState);
}

void BluetoothAdapter::notifyDiscoveryStarted() {
    std::vector<DiscoveryListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mDiscoveryListeners;
    }
    for (const DiscoveryListener& l : listeners) l.onDiscoveryStarted();
}

void BluetoothAdapter::notifyDiscoveryFinished() {
    std::vector<DiscoveryListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mDiscoveryListeners;
    }
    for (const DiscoveryListener& l : listeners) l.onDiscoveryFinished();
}

void BluetoothAdapter::notifyDeviceFound(const BluetoothDevice& device) {
    std::vector<DiscoveryListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mDiscoveryListeners;
    }
    for (const DiscoveryListener& l : listeners) l.onDeviceFound(device);
}

void BluetoothAdapter::notifyBondStateChanged(const BluetoothDevice& device,
                                              int bondState, int prevState) {
    std::vector<BondStateListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mBondListeners;
    }
    /* Writable copies: CallbackBase::operator() is non-const. */
    for (BondStateListener l : listeners) l(device, bondState, prevState);
}

void BluetoothAdapter::notifyPairingRequest(const BluetoothDevice& device,
                                            int pairingVariant, uint32_t passkey) {
    std::vector<BluetoothPairingListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mPairingListeners;
    }
    for (const BluetoothPairingListener& l : listeners)
        l.onPairingRequest(device, pairingVariant, passkey);
}

void BluetoothAdapter::notifyDisplayPasskey(const BluetoothDevice& device,
                                            uint32_t passkey, int pairedDuration) {
    std::vector<BluetoothPairingListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mPairingListeners;
    }
    for (const BluetoothPairingListener& l : listeners)
        l.onDisplayPasskey(device, passkey, pairedDuration);
}

void BluetoothAdapter::notifyPairingCancelled(const BluetoothDevice& device) {
    std::vector<BluetoothPairingListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mPairingListeners;
    }
    for (const BluetoothPairingListener& l : listeners)
        l.onPairingCancelled(device);
}

/* --- device resolve path (BluetoothDevice getters land here) ---------------- */

bool BluetoothAdapter::resolveDeviceName(const std::string& address,
                                         std::string& out) const {
    BluezDevice d;
    if (!const_cast<BluezClient&>(mClient).findDevice(address, d)) return false;
    out = d.alias.empty() ? d.name : d.alias;
    return !out.empty();
}

bool BluetoothAdapter::resolveDeviceAlias(const std::string& address,
                                          std::string& out) const {
    BluezDevice d;
    if (!const_cast<BluezClient&>(mClient).findDevice(address, d)) return false;
    out = d.alias;
    return true;
}

bool BluetoothAdapter::setDeviceAlias(const std::string& address,
                                      const std::string& alias) {
    return mClient.setDeviceAlias(address, alias);
}

int BluetoothAdapter::resolveBondState(const std::string& address) const {
    BluezDevice d;
    if (!const_cast<BluezClient&>(mClient).findDevice(address, d))
        return BluetoothDevice::BOND_NONE;
    return d.paired ? BluetoothDevice::BOND_BONDED
                    : BluetoothDevice::BOND_NONE;
}

int BluetoothAdapter::resolveDeviceClass(const std::string& address) const {
    BluezDevice d;
    if (!const_cast<BluezClient&>(mClient).findDevice(address, d)) return 0;
    return (int)d.cod;
}

int BluetoothAdapter::resolveDeviceType(const std::string& address) const {
    BluezDevice d;
    if (!const_cast<BluezClient&>(mClient).findDevice(address, d))
        return BluetoothDevice::DEVICE_TYPE_UNKNOWN;
    /* BlueZ marks LE-only devices with the AddressType property; a CoD
     * means BR/EDR. Both = DUAL, neither = UNKNOWN (not seen yet). */
    const bool le = !d.addressType.empty();
    const bool classic = d.cod != 0;
    if (le && classic) return BluetoothDevice::DEVICE_TYPE_DUAL;
    if (le) return BluetoothDevice::DEVICE_TYPE_LE;
    if (classic) return BluetoothDevice::DEVICE_TYPE_CLASSIC;
    return BluetoothDevice::DEVICE_TYPE_UNKNOWN;
}

bool BluetoothAdapter::bondDevice(const std::string& address) {
    return mClient.pairDevice(address);
}

bool BluetoothAdapter::unbondDevice(const std::string& address) {
    return mClient.removeDevice(address);
}

/* --- BluezClient::Events (monitor thread) ------------------------------------ */

void BluetoothAdapter::onAdapterBoolChanged(const std::string& name, bool value) {
    /* Values arrive WITH the event (monitor thread, bus lock held): no
     * synchronous re-read, that would self-deadlock the monitor. */
    if (name == "Powered") {
        setStateAndNotify(value ? STATE_ON : STATE_OFF);
    } else if (name == "Discovering") {
        bool was = false;
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            was = mDiscovering;
            mDiscovering = value;
        }
        if (was && !value) {
            notifyDiscoveryFinished();
        } else if (!was && value) {
            notifyDiscoveryStarted();
        }
    }
}

void BluetoothAdapter::onAdapterStringChanged(const std::string&,
                                              const std::string&) {
    /* Alias flips have no listener surface yet (ACTION_LOCAL_NAME_CHANGED
     * joins when the broadcast system does). */
}

void BluetoothAdapter::onDeviceAdded(const BluezDevice& device) {
    dispatchFound(device);
}

void BluetoothAdapter::onDevicePropertyChanged(const BluezDevice& device,
                                               const std::string& name) {
    bool discovering;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        discovering = mDiscovering;
    }
    if (name == "Paired") {
        /* maintain the prev-state map and fire the bond listeners */
        const int bond = device.paired ? BluetoothDevice::BOND_BONDED
                                       : BluetoothDevice::BOND_NONE;
        int prev = BluetoothDevice::BOND_NONE;
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            auto it = mBondStates.find(device.objectPath);
            if (it != mBondStates.end()) prev = it->second;
            mBondStates[device.objectPath] = bond;
        }
        if (prev != bond) {
            notifyBondStateChanged(BluetoothDevice(device.address), bond, prev);
        }
        return;
    }
    /* Name/Alias/RSSI refreshes during discovery re-fire ACTION_FOUND,
     * matching the Java sticky-update behavior. */
    if (discovering) dispatchFound(device);
}

void BluetoothAdapter::onDeviceRemoved(const std::string& objectPath) {
    std::lock_guard<std::mutex> lock(mStateMutex);
    mBondStates.erase(objectPath);
}

void BluetoothAdapter::onBluezDisconnected() {
    setStateAndNotify(STATE_OFF);
}

void BluetoothAdapter::onBluezReconnected() {
    /* State resync happens through the snapshot's synthesized property
     * events (refreshManagedObjects ran before this callback). */
}

void BluetoothAdapter::setStateAndNotify(int newState) {
    int prev;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mAdapterState == newState) return;
        prev = mAdapterState;
        mAdapterState = newState;
    }
    notifyAdapterStateChanged(newState, prev);
}

void BluetoothAdapter::dispatchFound(const BluezDevice& device) {
    notifyDeviceFound(BluetoothDevice(device.address));
}

} // namespace cdroid
