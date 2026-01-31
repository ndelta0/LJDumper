#include "LjOutputControlWindow.hpp"

#include <QCloseEvent>

#include "ui_LjOutputControlWindow.h"

#include "lj/utils.hpp"

#include "widgets/WClockConfig.hpp"
#include "widgets/WOutputPinRow.hpp"


constexpr WOutputPinRow::ESupportedFunctions operator|(const WOutputPinRow::ESupportedFunctions lhs,
                                                       const WOutputPinRow::ESupportedFunctions rhs) {
    return static_cast<WOutputPinRow::ESupportedFunctions>(to_underlying(lhs) + to_underlying(rhs));
}

LjOutputControlWindow::LjOutputControlWindow(LjManager *ljm, QWidget *parent) : QWidget(parent),
    ui(new Ui::LjOutputControlWindow), ljm(ljm) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    // clockConfig0 = new WClockConfig(0, 4294967296, ljm);
    // ui->verticalLayout->addWidget(clockConfig0);
    ljm->writeNameSync("DIO_EF_CLOCK0_ENABLE", 0);
    clockConfig1 = new WClockConfig(1, 65536, ljm);
    ui->verticalLayout->addWidget(clockConfig1);
    clockConfig2 = new WClockConfig(2, 65536, ljm);
    ui->verticalLayout->addWidget(clockConfig2);

    // Supporting all funcs (FIO0, FIO2-5)
    for (auto i = 0; i <= 5; ++i) {
        const auto f = i == 1
                           ? WOutputPinRow::ESupportedFunctions::None
                           : WOutputPinRow::ESupportedFunctions::Pwm | WOutputPinRow::ESupportedFunctions::Pulse;
        auto *pin = new WOutputPinRow(QString("FIO%1").arg(i), i + 17, f, ljm);
        pins.push_back(pin);
        ui->verticalLayout->addWidget(pin);
    }

    // All other pins
    for (auto i = 6; i <= 7; ++i) {
        auto *pin = new WOutputPinRow(QString("FIO%1").arg(i), i + 17, WOutputPinRow::ESupportedFunctions::None, ljm);
        pins.push_back(pin);
        ui->verticalLayout->addWidget(pin);
    }
    for (auto i = 0; i <= 2; ++i) {
        auto *pin = new WOutputPinRow(QString("MIO%1").arg(i), i + 25, WOutputPinRow::ESupportedFunctions::None, ljm);
        pins.push_back(pin);
        ui->verticalLayout->addWidget(pin);
    }
    for (auto i = 0; i <= 3; ++i) {
        auto *pin = new WOutputPinRow(QString("CIO%1").arg(i), i + 28, WOutputPinRow::ESupportedFunctions::None, ljm);
        pins.push_back(pin);
        ui->verticalLayout->addWidget(pin);
    }
    for (auto i = 0; i <= 7; ++i) {
        auto *pin = new WOutputPinRow(QString("EIO%1").arg(i), i + 32, WOutputPinRow::ESupportedFunctions::None, ljm);
        pins.push_back(pin);
        ui->verticalLayout->addWidget(pin);
    }
}

LjOutputControlWindow::~LjOutputControlWindow() {
    delete ui;
}

void LjOutputControlWindow::SetOutputDisabled(bool disable, const QString &hwName) {
    for (const auto pin: pins) {
        if (pin->getName() == hwName) {
            pin->setDisabled(disable);
            lj::SetPinDirection(ljm, lj::PinHwNameToDioNumber(hwName),
                                disable ? PinDirection::Input : PinDirection::Output);
            break;
        }
    }
}

void LjOutputControlWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
}
