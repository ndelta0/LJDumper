#include "ljManager.hpp"

#include <LabJackM.h>
#include <QtConcurrent/QtConcurrent>

LjManager::LjManager(QObject *parent) : QObject(parent) {
    moveToThread(&m_workerThread);
    m_workerThread.start();
}

LjManager::~LjManager() {
    m_workerThread.quit();
    m_workerThread.wait();
}


std::pair<int, int>
LjManager::openDeviceSync(const int deviceType, const int connectionType, const QString &identifier) {
    int handle = -1;
    const int error = LJM_Open(deviceType, connectionType, identifier.toUtf8().constData(), &handle);
    m_ljHandle = handle;
    return {handle, error};
}

int LjManager::ljHandle() const {
    return m_ljHandle;
}

void LjManager::openDevice(int deviceType, int connectionType,
                           const QString &identifier,
                           const std::function<void(int, int)> &callback) {
    execute<std::pair<int, int> >(
        [this, deviceType, connectionType, identifier]() -> auto {
            return openDeviceSync(deviceType, connectionType, identifier);
        },
        [this, callback](const std::pair<int, int> &result) -> void {
            m_ljHandle = result.first;
            if (callback) {
                callback(result.first, result.second);
            }
        }
    );
}

int LjManager::closeDeviceSync() {
    if (m_ljHandle < 0) {
        return LJME_NOERROR;
    }
    const int error = LJM_Close(m_ljHandle);
    m_ljHandle = -1;
    return error;
}

void LjManager::closeDevice(const std::function<void(int)> &callback) {
    execute<int>(
        [this]() -> int {
            return closeDeviceSync();
        },
        [callback](const int result) -> void {
            if (callback) {
                callback(result);
            }
        }
    );
}

// ReSharper disable once CppMemberFunctionMayBeConst
std::pair<LjHandleInfo, int> LjManager::getHandleInfoSync() {
    if (m_ljHandle < 0) {
        return {{}, LJME_DEVICE_DISCONNECTED};
    }
    LjHandleInfo info{};
    const int err = LJM_GetHandleInfo(m_ljHandle, &info.deviceType, &info.connectionType, &info.serialNumber,
                                      &info.ipAddress, &info.port, &info.maxBytesPerMB);
    return {info, err};
}

void LjManager::getHandleInfo(const std::function < void(LjHandleInfo, int) > &callback) {
    execute<std::pair<LjHandleInfo, int> >(
        [this]() -> auto {
            return getHandleInfoSync();
        },
        [callback](const std::pair<LjHandleInfo, int> &result) -> void {
            if (callback) {
                callback(result.first, result.second);
            }
        }
    );
}

// ReSharper disable once CppMemberFunctionMayBeConst
std::pair<double, int> LjManager::readNameSync(const QString &name) {
    if (m_ljHandle < 0) {
        return {0.0, LJME_DEVICE_DISCONNECTED};
    }
    double value = 0;
    const int error = LJM_eReadName(m_ljHandle, name.toUtf8().constData(), &value);
    return {value, error};
}

void LjManager::readName(const QString &name,
                         const std::function<void(double, int)> &callback) {
    execute<std::pair<double, int> >(
        [this, name]() -> auto {
            return readNameSync(name);
        },
        [callback](const std::pair<double, int> &result) -> void {
            if (callback) {
                callback(result.first, result.second);
            }
        }
    );
}

// ReSharper disable once CppMemberFunctionMayBeConst
std::pair<QString, int> LjManager::readNameStringSync(const QString &name) {
    if (m_ljHandle < 0) {
        return {"", LJME_DEVICE_DISCONNECTED};
    }
    std::array<char, LJM_MAX_NAME_SIZE> value{};
    const int error = LJM_eReadNameString(m_ljHandle, name.toUtf8().constData(), value.data());
    return {QString::fromUtf8(value.data()), error};
}

void LjManager::readNameString(const QString &name,
                               const std::function<void(QString, int)> &callback) {
    execute<std::pair<QString, int> >(
        [this, name]() -> auto {
            return readNameStringSync(name);
        },
        [callback](const std::pair<QString, int> &result) -> void {
            if (callback) {
                callback(result.first, result.second);
            }
        }
    );
}

// ReSharper disable once CppMemberFunctionMayBeConst
int LjManager::writeNameSync(const QString &name, const double value) {
    if (m_ljHandle < 0) {
        return LJME_DEVICE_DISCONNECTED;
    }
    return LJM_eWriteName(m_ljHandle, name.toUtf8().constData(), value);
}

void LjManager::writeName(const QString &name, double value,
                          const std::function<void(int)> &callback) {
    execute<int>(
        [this, name, value]() -> int {
            return writeNameSync(name, value);
        },
        [callback](const int result) -> void {
            if (callback) {
                callback(result);
            }
        }
    );
}

// ReSharper disable once CppMemberFunctionMayBeConst
int LjManager::writeNameStringSync(const QString &name, const QString &value) {
    if (m_ljHandle < 0) {
        return LJME_DEVICE_DISCONNECTED;
    }
    return LJM_eWriteNameString(m_ljHandle, name.toUtf8().constData(), value.toUtf8().constData());
}

void LjManager::writeNameString(const QString &name, const QString &value,
                                const std::function<void(int)> &callback) {
    execute<int>(
        [this, name, value]() -> int {
            return writeNameStringSync(name, value);
        },
        [callback](const int result) -> void {
            if (callback) {
                callback(result);
            }
        }
    );
}

std::pair<double, int> LjManager::streamStartSync(const int scansPerRead, const std::vector<int> &addresses,
                                                  const double scanRate) {
    if (m_ljHandle < 0) {
        return {0.0, LJME_DEVICE_DISCONNECTED};
    }
    double sr = scanRate;
    const int error = LJM_eStreamStart(m_ljHandle, scansPerRead, addresses.size(), addresses.data(), &sr);
    return {sr, error};
}

void LjManager::streamStart(int scansPerRead, const std::vector<int> &addresses, double scanRate,
                            const std::function<void(double, int)> &callback) {
    execute<std::pair<double, int> >(
        [this, scansPerRead, addresses, scanRate]() -> auto {
            return streamStartSync(scansPerRead, addresses, scanRate);
        },
        [callback](const std::pair<double, int> &result) -> void {
            if (callback) {
                callback(result.first, result.second);
            }
        }
    );
}

auto LjManager::streamStopSync() -> int {
    return LJM_eStreamStop(m_ljHandle);
}

auto LjManager::streamStop(const std::function<void(int)> &callback) -> void {
    execute<int>(
        [this]() -> int {
            return streamStopSync();
        },
        [callback](const int result) -> void {
            if (callback) {
                callback(result);
            }
        }
    );
}

auto LjManager::streamReadSync(double *data, int *devBacklog, int *ljmBacklog) -> int {
    return LJM_eStreamRead(m_ljHandle, data, devBacklog, ljmBacklog);
}

auto LjManager::streamRead(double *data, int *devBacklog, int *ljmBacklog,
                           const std::function<void(int)> &callback) -> void {
    execute<int>(
        [this, data, devBacklog, ljmBacklog]() -> int {
            return streamReadSync(data, devBacklog, ljmBacklog);
        },
        [callback](const int result) -> void {
            if (callback) {
                callback(result);
            }
        }
    );
}