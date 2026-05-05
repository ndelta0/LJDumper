#include "utils.hpp"

#include <iostream>
#include <LabJackM.h>
#include "ljManager.hpp"

QString lj::ErrorToString(const int error) {
    std::array<char, LJM_MAX_NAME_SIZE> err_str{};
    LJM_ErrorToString(error, err_str.data());
    return QString::fromUtf8(err_str.data());
}

int lj::PinHwNameToDioNumber(QString name) {
    if (name.isEmpty()) {
        return -1;
    }

    const auto id_num = name.split("IO").at(1).toInt();;

    if (name[0] == 'F') {
        return id_num;
    }
    if (name[0] == 'E') {
        return id_num + 8;
    }
    if (name[0] == 'C') {
        return id_num + 16;
    }
    if (name[0] == 'M') {
        return id_num + 20;
    }

    return -1;
}

void lj::SetPinDirection(LjManager *ljm, const int dio_n, const PinDirection dir) {
    uint64_t io_dir_bitmap = 0;
    auto [tmp, err] = ljm->readNameSync("DIO_DIRECTION");
    if (err) {
        const auto err_str = ErrorToString(err);
        std::cerr << "Error: Failed to read DIO direction: " << err_str.toStdString() << '\n';
        return;
    }
    io_dir_bitmap = static_cast<uint64_t>(tmp);
    if (dir == PinDirection::Output) {
        io_dir_bitmap |= (1ULL << dio_n);
    } else {
        io_dir_bitmap &= ~(1ULL << dio_n);
    }
    err = ljm->writeNameSync("DIO_DIRECTION", io_dir_bitmap);
    if (err) {
        const auto err_str = ErrorToString(err);
        std::cerr << "Error: Failed to set DIO direction: " << err_str.toStdString() << '\n';
    }
}

void lj::SetPinState(LjManager *ljm, const int dio_n, const bool state) {
    uint64_t io_state_bitmap = 0;
    auto [tmp, err] = ljm->readNameSync("DIO_STATE");
    if (err) {
        const auto err_str = ErrorToString(err);
        std::cerr << "Error: Failed to read DIO state: " << err_str.toStdString() << '\n';
        return;
    }
    io_state_bitmap = static_cast<uint64_t>(tmp);
    if (state) {
        io_state_bitmap |= (1ULL << dio_n);
    } else {
        io_state_bitmap &= ~(1ULL << dio_n);
    }
    err = ljm->writeNameSync("DIO_STATE", io_state_bitmap);
    if (err) {
        const auto err_str = ErrorToString(err);
        std::cerr << "Error: Failed to set DIO state: " << err_str.toStdString() << '\n';
    }
}
