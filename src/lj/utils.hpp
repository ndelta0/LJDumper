#pragma once
#include <QString>

class LjManager;

enum class PinDirection {
    Input = 0,
    Output = 1
};

namespace lj {
    QString ErrorToString(int error);

    int PinHwNameToDioNumber(QString name);

    void SetPinDirection(LjManager *ljm, int dio_n, PinDirection dir);

    void SetPinState(LjManager *ljm, int dio_n, bool state);
}
