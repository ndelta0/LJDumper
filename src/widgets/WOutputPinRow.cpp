#include "WOutputPinRow.hpp"

#include <iostream>
#include <utility>
#include "ui_WOutputPinRow.h"

#include "lj/ljManager.hpp"
#include "lj/utils.hpp"

bool operator&(WOutputPinRow::ESupportedFunctions lhs, WOutputPinRow::ESupportedFunctions rhs) {
    return (to_underlying(lhs) & to_underlying(rhs)) != 0;
}

WOutputPinRow::WOutputPinRow(QString hwName, const int blockNum, const ESupportedFunctions funcs, LjManager *ljm,
                             QWidget *parent) : QWidget(parent), ui(new Ui::WOutputPinRow), hwName(std::move(hwName)),
                                                blockNum(blockNum), supportedFunctions(funcs), ljm(ljm) {
    ui->setupUi(this);

    ui->lblPinInfo->setText(QString("%1 (%2)").arg(this->hwName).arg(blockNum));
    ui->cbPinFunction->setCurrentIndex(0);
    ui->swFunctionControl->setCurrentIndex(0);

    int idx = 0;
    for (const auto func: enum_range(ESupportedFunctions::Pwm, ESupportedFunctions::Pulse)) {
        idx++;
        ComboBoxItemSetEnabled(ui->cbPinFunction, idx, funcs & func);
    }

    dio_num = lj::PinHwNameToDioNumber(this->hwName);
    if (this->hwName[0] == 'F' and this->hwName != "FIO1") {
        // Make sure the pin is set as output (non-EF)
        if (dio_num == -1) {
            std::cerr << "Warning: Got DIO number " << dio_num << " for pin " << this->hwName.toStdString() << "\n";
            return;
        }
        const auto dio_reg_name = QString("DIO%1_EF_ENABLE").arg(dio_num);
        if (const auto err = ljm->writeNameSync(dio_reg_name, 0)) {
            const auto err_str = lj::ErrorToString(err);
            std::cerr << "Warning: Failed to disable EF functionality for pin '" << this->hwName.toStdString() << "': "
                    << err_str.
                    toStdString() << '\n';
        }

        connect(ui->cbPinFunction, &QComboBox::activated, this, &WOutputPinRow::cbPinFunction_activated);
    }

    currentFunction = ESupportedFunctions::None;
    lj::SetPinDirection(ljm, dio_num, PinDirection::Output);
    cbDigitalOutput_checkStateChanged(Qt::Unchecked); // Force LOW at the start

    connect(ui->cbDigitalOutput, &QCheckBox::checkStateChanged, this,
            &WOutputPinRow::cbDigitalOutput_checkStateChanged);

    connect(ui->cbPwmEnable, &QCheckBox::checkStateChanged, this, &WOutputPinRow::cbPwmEnable_checkStateChanged);
    connect(ui->cbPwmClock, &QComboBox::activated, this, &WOutputPinRow::cbPwmClock_activated);
    connect(ui->sbPwmDuty, qOverload<int>(&QSpinBox::valueChanged), this, &WOutputPinRow::sbPwmDuty_valueChanged);

    connect(ui->cbPulseClock, &QComboBox::activated, this, &WOutputPinRow::cbPulseClock_activated);
    connect(ui->sbPulseCount, qOverload<int>(&QSpinBox::valueChanged), this, &WOutputPinRow::sbPulseCount_valueChanged);
    connect(ui->btnPulseStart, &QPushButton::clicked, this, &WOutputPinRow::btnPulseStart_clicked);
}

WOutputPinRow::~WOutputPinRow() {
    delete ui;
}

const QString &WOutputPinRow::getName() const {
    return hwName;
}

void WOutputPinRow::cbPinFunction_activated(const int index) {
    if (index == 0) {
        // Normal output
        ui->cbPwmEnable->setCheckState(Qt::Unchecked);
        cbPwmEnable_checkStateChanged(Qt::Unchecked);

        // Make sure the pin is set as output (non-EF)
        const auto dio_reg_name = QString("DIO%1_EF_ENABLE").arg(dio_num);
        if (const auto err = ljm->writeNameSync(dio_reg_name, 0)) {
            const auto err_str = lj::ErrorToString(err);
            std::cerr << "Warning: Failed to disable EF functionality for pin '" << hwName.toStdString() << "': " <<
                    err_str.
                    toStdString() << '\n';
        }
        lj::SetPinDirection(ljm, lj::PinHwNameToDioNumber(this->hwName), PinDirection::Output);
        return;
    }
    ui->cbDigitalOutput->setCheckState(Qt::Unchecked);
    lj::SetPinState(ljm, dio_num, false);
    ljm->writeNameSync(QString("DIO%1_EF_ENABLE").arg(dio_num), 0);
    if (index == 1) {
        // PWM out
        const auto dio_reg_name = QString("DIO%1_EF_INDEX").arg(dio_num);
        if (const auto err = ljm->writeNameSync(dio_reg_name, 0)) {
            const auto err_str = lj::ErrorToString(err);
            std::cerr << "Warning: Failed to enable EF mode for pin '" << hwName.toStdString() << "': " <<
                    err_str.
                    toStdString() << '\n';
        }
        lj::SetPinDirection(ljm, lj::PinHwNameToDioNumber(this->hwName), PinDirection::Output);
    }
    if (index == 2) {
        // Pulse out
        const auto dio_reg_name = QString("DIO%1_EF_INDEX").arg(dio_num);
        if (const auto err = ljm->writeNameSync(dio_reg_name, 2)) {
            const auto err_str = lj::ErrorToString(err);
            std::cerr << "Warning: Failed to enable EF mode for pin '" << hwName.toStdString() << "': " <<
                    err_str.
                    toStdString() << '\n';
        }
        lj::SetPinDirection(ljm, lj::PinHwNameToDioNumber(this->hwName), PinDirection::Output);
    }
}

void WOutputPinRow::cbDigitalOutput_checkStateChanged(const Qt::CheckState state) const {
    if (currentFunction != ESupportedFunctions::None) return;
    const bool enabled = state == Qt::Checked;
    lj::SetPinState(ljm, dio_num, enabled);
}

void WOutputPinRow::cbPwmEnable_checkStateChanged(Qt::CheckState state) {
    if (state == Qt::Unchecked) {
        // Disable
        ljm->writeNameSync(QString("DIO%1_EF_ENABLE").arg(dio_num), 0);
    } else {
        ljm->writeNameSync(QString("DIO%1_EF_ENABLE").arg(dio_num), 1);
    }
}

void WOutputPinRow::cbPwmClock_activated(int index) {
    pwm_settings.selected_clock = index + 1;
    updatePwmSettings();
}

void WOutputPinRow::sbPwmDuty_valueChanged(int value) {
    pwm_settings.duty_cycle = value;
    updatePwmSettings();
}

void WOutputPinRow::cbPulseClock_activated(int index) {
    pulse_settings.selected_clock = index + 1;
    updatePulseSettings();
}

void WOutputPinRow::sbPulseCount_valueChanged(int value) {
    pulse_settings.pulse_count = value;
    updatePulseSettings();
}

void WOutputPinRow::btnPulseStart_clicked() {
    ljm->readName(QString("DIO%1_EF_READ_A_AND_RESET").arg(dio_num));

    auto [en, _] = ljm->readNameSync(QString("DIO%1_EF_ENABLE").arg(dio_num));
    if (en != 1) {
        ljm->writeNameSync(QString("DIO%1_EF_ENABLE").arg(dio_num), 1);
    }
}

void WOutputPinRow::updatePwmSettings() {
    // Get clock roll value
    auto [roll, err] = ljm->readNameSync(QString("DIO_EF_CLOCK%1_ROLL_VALUE").arg(pwm_settings.selected_clock));
    if (err) {
        const auto err_str = lj::ErrorToString(err);
        std::cerr << "Warning: Failed to read clock roll value: " << err_str.toStdString() << '\n';
        return;
    }
    const auto config_a = pwm_settings.duty_cycle * roll / 100;
    err = ljm->writeNameSync(QString("DIO%1_EF_CONFIG_A").arg(dio_num), config_a);
    if (err) {
        const auto err_str = lj::ErrorToString(err);
        std::cerr << "Warning: Failed to set PWM config A: " << err_str.toStdString() << '\n';
    }
    auto r = ljm->readNameSync(QString("DIO%1_EF_CLOCK_SOURCE").arg(dio_num));
    if (r.second) {
        err = r.second;
        std::cerr << "Warning: Failed to read PWM clock source: " << lj::ErrorToString(err).toStdString() << '\n';
        return;
    }
    bool changed_clock = false;
    if (r.first != pwm_settings.selected_clock) {
        changed_clock = true;
    }
    err = ljm->writeNameSync(QString("DIO%1_EF_CLOCK_SOURCE").arg(dio_num), pwm_settings.selected_clock);
    if (err) {
        const auto err_str = lj::ErrorToString(err);
        std::cerr << "Warning: Failed to set PWM options: " << err_str.toStdString() << '\n';
    }
    if (changed_clock) {
        auto [en, _] = ljm->readNameSync(QString("DIO%1_EF_ENABLE").arg(dio_num));
        if (en != 0) {
            ljm->writeNameSync(QString("DIO%1_EF_ENABLE").arg(dio_num), 0);
            ljm->writeNameSync(QString("DIO%1_EF_ENABLE").arg(dio_num), 1);
        }
    }
}

void WOutputPinRow::updatePulseSettings() {
    // Get clock roll value
    auto [roll, err] = ljm->readNameSync(QString("DIO_EF_CLOCK%1_ROLL_VALUE").arg(pwm_settings.selected_clock));
    if (err) {
        const auto err_str = lj::ErrorToString(err);
        std::cerr << "Warning: Failed to read clock roll value: " << err_str.toStdString() << '\n';
        return;
    }
    const auto config_a = pwm_settings.duty_cycle * roll / 100;
    err = ljm->writeNameSync(QString("DIO%1_EF_CONFIG_A").arg(dio_num), config_a);
    if (err) {
        const auto err_str = lj::ErrorToString(err);
        std::cerr << "Warning: Failed to set PWM config A: " << err_str.toStdString() << '\n';
    }
    err = ljm->writeNameSync(QString("DIO%1_EF_CONFIG_B").arg(dio_num), 0);
    if (err) {
        const auto err_str = lj::ErrorToString(err);
        std::cerr << "Warning: Failed to set PWM config B: " << err_str.toStdString() << '\n';
    }
    err = ljm->writeNameSync(QString("DIO%1_EF_CONFIG_C").arg(dio_num), pulse_settings.pulse_count);
    if (err) {
        const auto err_str = lj::ErrorToString(err);
        std::cerr << "Warning: Failed to set PWM config C: " << err_str.toStdString() << '\n';
    }
    err = ljm->writeNameSync(QString("DIO%1_EF_CLOCK_SOURCE").arg(dio_num), pwm_settings.selected_clock);
    if (err) {
        const auto err_str = lj::ErrorToString(err);
        std::cerr << "Warning: Failed to set PWM options: " << err_str.toStdString() << '\n';
    }
}
