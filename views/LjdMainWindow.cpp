#include "LjdMainWindow.hpp"

#include <iostream>
#include <LabJackM.h>

#include "ui_LjdMainWindow.h"

#include "lj/ljManager.hpp"
#include "lj/utils.hpp"

#include <QMessageBox>
#include <QCloseEvent>
#include <QStandardPaths>
#include <QFileDialog>
#include <QString>

#include "fileNameTemplateValidator.hpp"
#include "FileNameTemplatingDialog.hpp"
#include "ScanRateDialog.hpp"

#include <charconv>
#include <utility>

#include "LjOutputControlWindow.hpp"


LjdMainWindow::LjdMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::LjdMainWindow) {
    ui->setupUi(this);
    ui->areaConfig->setEnabled(false);

    ui->editOutputDir->setText(
        QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).filePath(
            "ljd_output" + QString(QDir::separator())));
    ui->editFileName->setValidator(new FileNameTemplateValidator);

    m_ljm = new LjManager(nullptr);

    connect(ui->btnConnection, &QPushButton::clicked, this, &LjdMainWindow::btnConnection_clicked);
    connect(ui->btnStartStop, &QPushButton::clicked, this, &LjdMainWindow::btnStartStop_clicked);
    connect(ui->btnFileNameTemplatingHelp, &QToolButton::clicked, this, [this] {
        auto *dialog = new FileNameTemplatingDialog(this);
        dialog->exec();
        delete dialog;
    });
    connect(ui->btnSampleRateHelp, &QToolButton::clicked, this, [this] {
        auto *dialog = new ScanRateDialog(this);
        dialog->exec();
        delete dialog;
    });
    connect(ui->btnSetOutputDir, &QToolButton::clicked, this, [this] {
        const auto dir = QFileDialog::getExistingDirectory(
            this,
            "Wybierz folder zapisów",
            ui->editOutputDir->text(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!dir.isEmpty()) {
            ui->editOutputDir->setText(dir);
        }
    });

    auto *l = new DFlowLayout;
    m_inputConfigLayout = l;
    ui->boxConfigInput->setLayout(l);

    // AIN 0-13
    for (auto i = 0; i <= 13; ++i) {
        auto *pin = new DPinConfig(QString("AIN%1").arg(i), i + 3);
        m_pinConfigs.push_back(pin);
        l->addWidget(pin);
    }
    // FIO 0-7
    for (auto i = 0; i <= 7; ++i) {
        auto *pin = new DPinConfig(QString("FIO%1").arg(i), i + 17);
        m_pinConfigs.push_back(pin);
        l->addWidget(pin);
        connect(pin, &DPinConfig::cb_checked, this, &LjdMainWindow::inputEnabled);
    }
    // MIO 0-2
    for (auto i = 0; i <= 2; ++i) {
        auto *pin = new DPinConfig(QString("MIO%1").arg(i), i + 25);
        m_pinConfigs.push_back(pin);
        l->addWidget(pin);
        connect(pin, &DPinConfig::cb_checked, this, &LjdMainWindow::inputEnabled);
    }
    // CIO 0-3
    for (auto i = 0; i <= 3; ++i) {
        auto *pin = new DPinConfig(QString("CIO%1").arg(i), i + 28);
        m_pinConfigs.push_back(pin);
        l->addWidget(pin);
        connect(pin, &DPinConfig::cb_checked, this, &LjdMainWindow::inputEnabled);
    }

#ifndef NDEBUG
    btnConnection_clicked(); // Simulate connecting
#endif
}

LjdMainWindow::~LjdMainWindow() {
    if (m_isConnected) {
        if (m_isScanning) {
            btnStartStop_clicked();
        }
        m_ljm->closeDeviceSync();
    }

    delete m_ljm;
    delete ui;
}

void LjdMainWindow::btnConnection_clicked() {
    ui->btnConnection->setEnabled(false);

    if (m_isConnected) {
#ifdef NDEBUG
        if (QMessageBox::warning(this, "Rozłączenie", "Czy na pewno rozłączyć od urządzenia?",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
            ui->btnConnection->setEnabled(true);
            return;
        }
#endif

        std::cerr << "Disconnecting...\n";

        ui->dispConnType->setText("");
        ui->areaConfig->setEnabled(false);
        ui->dispStatus->setText("<span style=\"color:yellow;\">Rozłączanie...</span>");

        m_outputControlWindow->close();
        delete m_outputControlWindow;
        m_outputControlWindow = nullptr;

        m_ljm->closeDevice([this](int /*err*/) {
            m_isConnected = false;
            ui->btnConnection->setEnabled(true);
            ui->btnConnection->setText("Połącz");
            ui->dispStatus->setText("<span style=\"color:red;\">Rozłączony</span>");
        });

        return;
    }

    std::cerr << "Connecting...\n";
    ui->dispStatus->setText("<span style=\"color:yellow;\">Łączenie...</span>");

    m_ljm->openDevice(LJM_dtT7, LJM_ctANY, "ANY", [this](int /*handle*/, const int err) {
        ui->btnConnection->setEnabled(true);

        if (err) {
            if (err == LJME_DEVICE_NOT_FOUND) {
                QMessageBox::warning(this, "Błąd połączenia",
                                     "Niepowodzenie połączenia:\nNie znaleziono dostępnego urządzenia LabJack T7.\nSprawdź kabel USB lub połączenie sieciowe.",
                                     QMessageBox::Ok);
            } else {
                const auto err_str = lj::ErrorToString(err);
                std::cerr << "Error: " << err_str.toStdString() << '\n';
                QMessageBox::warning(this, "Błąd połączenia", QString("Niepowodzenie połączenia:\n%1").arg(err_str),
                                     QMessageBox::Ok);
            }

            m_isConnected = false;
            ui->btnConnection->setText("Połącz");
            ui->dispStatus->setText("<span style=\"color:red;\">Rozłączony</span>");
        } else {
            m_isConnected = true;
            ui->areaConfig->setEnabled(true);
            ui->btnConnection->setText("Rozłącz");
            ui->dispStatus->setText("<span style=\"color:green;\">Połączony</span>");

            m_outputControlWindow = new LjOutputControlWindow(m_ljm, nullptr);
            m_outputControlWindow->setAttribute(Qt::WA_DeleteOnClose, false);
            m_outputControlWindow->show();

            if (const auto [name, err_a] = m_ljm->readNameStringSync("DEVICE_NAME_DEFAULT"); err_a) {
                const auto err_str = lj::ErrorToString(err_a);
                std::cerr << "Error: " << err_str.toStdString() << '\n';
            } else {
                ui->dispStatus->setText("<span style=\"color:green;\">Połączony</span> (" + name + ")");
            }

            if (const auto [info, err_a] = m_ljm->getHandleInfoSync(); err_a) {
                const auto err_str = lj::ErrorToString(err_a);
                std::cerr << "Error: " << err_str.toStdString() << '\n';
                ui->dispConnType->setText("<span style=\"color:gray;\">Nieznany</span>");
            } else {
                std::string out;
                switch (info.deviceType) {
                    case LJM_dtT4: out = "T4";
                        break;
                    case LJM_dtT7: out = "T7";
                        break;
                    case LJM_dtT8: out = "T8";
                        break;
                    case LJM_dtDIGIT: out = "DIGIT";
                        break;
                    default: out = "Unknown";
                }
                out += " (SN#" + std::to_string(info.serialNumber) + "), ";
                switch (info.connectionType) {
                    case LJM_ctUSB: out += "USB";
                        break;
                    case LJM_ctETHERNET: out += "Ethernet";
                        break;
                    case LJM_ctWIFI: out += "WiFi";
                        break;
                    default: out += "Unknown";
                        break;
                }
                ui->dispConnType->setText(QString::fromStdString(out));
            }
        }
    });
}

QString RenderFileNameTemplate(const QString &tmpl) {
    static int incremental_counter = 0;

    const auto dt = QDateTime::currentDateTime();
    QString result = tmpl;
    result.replace("%Y", dt.toString("yyyy"));
    result.replace("%M", dt.toString("MM"));
    result.replace("%D", dt.toString("dd"));
    result.replace("%h", dt.toString("hh"));
    result.replace("%m", dt.toString("mm"));
    result.replace("%s", dt.toString("ss"));
    result.replace("%i", QString::number(incremental_counter++));
    return result;
}


void LjdMainWindow::btnStartStop_clicked() {
    // 1. Disable button
    ui->btnStartStop->setEnabled(false);

    // Check what state we are in
    if (m_isScanning) {
        m_isScanning = false;
        ui->dispStatus->setText("<span style=\"color:orange;\">Zatrzymywanie...</span>");
        m_ljm->streamStop([this](int /*err*/) {
            m_callbackInfo.outputFile->flush();
            m_callbackInfo.outputFile->close();
            delete m_callbackInfo.outputFile;

            ui->btnStartStop->setEnabled(true);
            ui->btnStartStop->setText("Rozpocznij zapis");
            ui->dispStatus->setText("<span style=\"color:green;\">Połączony</span>");
            ui->btnConnection->setEnabled(true);
            ui->boxConfigInput->setEnabled(true);
            ui->boxConfigLogging->setEnabled(true);

            free(m_dataBuffer);
        });
    } else {
        // We are starting
        // 2. Check if we can create the file and write to it
        // Make sure all directories are created
        const auto output_dir = QDir(ui->editOutputDir->text());
        if (!output_dir.mkpath(".")) {
            std::cerr << "Error: Could not create output directory.\n";
            QMessageBox::critical(this, "Błąd", "Brak uprawnień do zapisu danych w podanej lokalizacji.");
            ui->btnStartStop->setEnabled(true);
            return;
        }

        // Create an empty file
        auto file_name = ui->editFileName->text();
        if (file_name.isEmpty()) {
            file_name = ui->editFileName->placeholderText();
        }
        // Fill out any template parameters
        file_name = RenderFileNameTemplate(file_name);
        // Check if it has an extension
        if (QFileInfo(file_name).suffix().isEmpty()) {
            file_name += ".csv";
        }

        // Create and open
        auto file = new QFile(output_dir.filePath(file_name));
#ifdef NDEBUG
        if (file->exists()) {
            if (QMessageBox::warning(this, "Plik istnieje", "Plik o podanej nazwie istnieje. Czy chcesz go zastąpić?",
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
                ui->btnStartStop->setEnabled(true);
                delete file;
                return;
            }
            std::cerr << "Overwriting existing file...\n";
        }
#endif

        if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
            std::cerr << "Error: Could not open output file.\n";
            QMessageBox::critical(this, "Błąd", "Brak uprawnień do zapisu danych w podanej lokalizacji.");
            ui->btnStartStop->setEnabled(true);
            delete file;
            return;
        }

        // 3. Prepare information for the callback handler
        m_callbackInfo.sampleNumber = 0;
        m_callbackInfo.ljm = m_ljm;
        m_callbackInfo.outputFile = file;
        m_callbackInfo.scansPerRead = ui->sbSampleRate->value(); // Read data 1/sec
        m_callbackInfo.scanRate = ui->sbSampleRate->value();
        m_callbackInfo.precision = ui->sbPrecision->value();
        m_callbackInfo.timeResolution = 1.0 / m_callbackInfo.scanRate;
        m_callbackInfo.timePrecision = static_cast<int>(std::ceil(std::log10(m_callbackInfo.scanRate)));

        std::vector<int> columns;
        std::vector<int> column_registers;

        file->write("time");

        uint64_t io_dir_bitmap = 0;
        auto [tmp, err] = m_ljm->readNameSync("DIO_DIRECTION");
        if (err) {
            const auto err_str = lj::ErrorToString(err);
            std::cerr << "Warning: Failed to read DIO direction: " << err_str.toStdString() << '\n';
        } else {
            io_dir_bitmap = static_cast<uint64_t>(tmp);
        }
        for (const auto &pin: m_pinConfigs) {
            const auto [enabled, hwName, dataName] = pin->info();
            if (!enabled) {
                continue;
            }

            const auto col_name = dataName.isEmpty() ? hwName : dataName;
            file->write((",\"" + col_name + "\"").toUtf8());

            int addr, type;
            if (const auto err = LJM_NameToAddress(hwName.toUtf8().constData(), &addr, &type)) {
                const auto err_str = lj::ErrorToString(err);
                std::cerr << "Error: Could not resolve pin name '" << hwName.toStdString() << "': " << err_str.
                        toStdString() << '\n';
                QMessageBox::critical(this, "Błąd",
                                      QString("Nie udało się znaleźć wejścia %1\n(Err: %2)").arg(hwName).arg(err_str));
                ui->btnStartStop->setEnabled(false);
                m_callbackInfo.outputFile->close();
                delete m_callbackInfo.outputFile;
                return;
            }
            columns.push_back(type);
            column_registers.push_back(addr);

            if (!pin->info().hwName.startsWith("AIN")) {
                // Make sure the pin is set as input
                const auto dio_number = lj::PinHwNameToDioNumber(pin->info().hwName);
                if (dio_number == -1) {
                    std::cerr << "Warning: Got DIO number " << dio_number << " for pin " << pin->info().hwName.
                            toStdString()
                            << "\n";
                    continue;
                }
                // Set bit n to 0, n=dio_number
                io_dir_bitmap &= ~(1ULL << dio_number);
                const auto dio_reg_name = QString("DIO%1_EF_ENABLE").arg(dio_number);
                if (const auto err = m_ljm->writeNameSync(dio_reg_name, 0)) {
                    const auto err_str = lj::ErrorToString(err);
                    std::cerr << "Warning: Failed to disable EF functionality for pin '" << hwName.toStdString() <<
                            "': " <<
                            err_str.
                            toStdString() << '\n';
                }
            }
        }
        m_ljm->writeNameSync("DIO_DIRECTION", io_dir_bitmap);

        file->write("\n");
        file->flush();
        m_callbackInfo.columns = columns;

        if (columns.size() == 0) {
            std::cerr << "Warning: No pins selected.\n";
            QMessageBox::critical(this, "Błąd", "Nie wybrano żadnych kanałów.");
            ui->btnStartStop->setEnabled(true);
            m_callbackInfo.outputFile->close();
            delete m_callbackInfo.outputFile;
            return;
        }

        // Calculate the required buffer size
        const size_t row_size = columns.size() * sizeof(double);
        const size_t buffer_size = row_size * m_callbackInfo.scansPerRead;
        m_dataBuffer = static_cast<double *>(malloc(buffer_size));
        if (!m_dataBuffer) {
            std::cerr << "Error: Out of memory.\n";
            QMessageBox::critical(this, "Błąd", "Nie udało się zarezerwować pamięci RAM.");
            ui->btnStartStop->setEnabled(true);
            m_callbackInfo.outputFile->close();
            delete m_callbackInfo.outputFile;
            return;
        }

        ui->btnConnection->setEnabled(false);
        ui->boxConfigInput->setEnabled(false);
        ui->boxConfigLogging->setEnabled(false);

        auto stream_start_cb = [this](const double scanRate, int err) {
            if (scanRate != m_callbackInfo.scanRate) {
                m_callbackInfo.scanRate = scanRate;
                m_callbackInfo.timeResolution = 1 / scanRate;
                m_callbackInfo.timePrecision = static_cast<int>(std::ceil(
                    std::log10(m_callbackInfo.scanRate)));
            }

            if (err) {
                const auto err_str = lj::ErrorToString(err);
                std::cerr << "Error: Failed to start stream: " << err_str.toStdString() << '\n';
                QMessageBox::critical(this, "Błąd",
                                      QString("Nie udało się rozpocząć pobierania danych\n(Err: %1)")
                                      .arg(err_str));
                m_callbackInfo.outputFile->close();
                delete m_callbackInfo.outputFile;

                ui->btnStartStop->setEnabled(true);
                ui->btnConnection->setEnabled(true);
                ui->boxConfigInput->setEnabled(true);
                ui->boxConfigLogging->setEnabled(true);
                return;
            }

            // Enable button
            ui->btnStartStop->setEnabled(true);
            ui->btnStartStop->setText("Zatrzymaj zapis");
            ui->sbSampleRate->setValue(static_cast<int>(scanRate));
            ui->dispStatus->setText("<span style=\"color:cyan;\">Akwizycja...</span>");

            // Enable callback
            m_isScanning = true;

            err = LJM_SetStreamCallback(m_ljm->ljHandle(), [](void *user_data) {
                static_cast<LjdMainWindow *>(user_data)->streamReadCallback();
            }, this);
            if (err) {
                const auto err_str = lj::ErrorToString(err);
                std::cerr << "Error: Failed to bind callback: " << err_str.toStdString() << '\n';
                QMessageBox::critical(this, "Błąd",
                                      QString("Nie udało się rozpocząć pobierania danych\n(Err: %1)")
                                      .arg(err_str));
                m_callbackInfo.outputFile->close();
                delete m_callbackInfo.outputFile;

                ui->btnStartStop->setEnabled(true);
                ui->btnConnection->setEnabled(true);
                ui->boxConfigInput->setEnabled(true);
                ui->boxConfigLogging->setEnabled(true);

                m_ljm->streamStopSync();
            }
        };

        m_ljm->streamStopSync(); // Returns err=0 both when already streaming and when not streaming, failsafe to force stop streaming when card in unknown state
        m_ljm->streamStart(m_callbackInfo.scansPerRead, column_registers,
                           m_callbackInfo.scanRate, stream_start_cb);
    }
}

void LjdMainWindow::inputEnabled(const bool enabled, const QString &hwName) const {
    if (m_isConnected && m_outputControlWindow) {
        m_outputControlWindow->SetOutputDisabled(enabled, hwName);
    }
}

void LjdMainWindow::closeEvent(QCloseEvent *event) {
#ifdef NDEBUG
    if (m_isConnected) {
        // Confirm whether the window should be closed.
        if (QMessageBox::warning(this, "Zamknięcie programu",
                                 "Program jest połączony z urządzeniem.\nCzy na pewno rozłączyć i zamknąć program?\nMoże wystąpić utrata danych!",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
            event->ignore();
            return;
        }
        event->accept();
    }
#endif

    if (m_isConnected) {
        m_outputControlWindow->close();
        delete m_outputControlWindow;
    }
}

void LjdMainWindow::streamReadCallback() {
    int devBacklog, ljmBacklog;
    if (const int err = m_ljm->streamReadSync(m_dataBuffer, &devBacklog, &ljmBacklog)) {
        const auto err_str = lj::ErrorToString(err);
        std::cerr << "Error: Failed to read data: " << err_str.constData() << '\n';
        m_callbackInfo.error = err;
        emit btnStartStop_clicked();
        return;
    }

    const int col_count = m_callbackInfo.columns.size();
    const int row_count = m_callbackInfo.scansPerRead;

    char buf[64];

    for (int row = 0; row < row_count; ++row) {
        const auto time = m_callbackInfo.sampleNumber * m_callbackInfo.timeResolution;
        auto res = (std::to_chars_result) std::to_chars(buf, buf + sizeof(buf), time, std::chars_format::fixed,
                                                        m_callbackInfo.timePrecision);
        m_callbackInfo.outputFile->write(buf, res.ptr - buf);

        for (int col = 0; col < col_count; ++col) {
            const auto val = m_dataBuffer[row * col_count + col];

            switch (m_callbackInfo.columns[col]) {
                case LJM_UINT16:
                case LJM_UINT32:
                    res = std::to_chars(buf, buf + sizeof(buf), static_cast<uint32_t>(val));
                    m_callbackInfo.outputFile->write((',' + std::string(buf, res.ptr - buf)).c_str());
                    break;
                case LJM_INT32:
                    res = std::to_chars(buf, buf + sizeof(buf), static_cast<int32_t>(val));
                    m_callbackInfo.outputFile->write((',' + std::string(buf, res.ptr - buf)).c_str());
                    break;
                case LJM_FLOAT32:
                    res = (std::to_chars_result) std::to_chars(buf, buf + sizeof(buf), val,
                                                               std::chars_format::fixed, m_callbackInfo.precision);
                    m_callbackInfo.outputFile->write((',' + std::string(buf, res.ptr - buf)).c_str());
                    break;
            }
        }

        m_callbackInfo.outputFile->write("\n");
        m_callbackInfo.sampleNumber++;

#ifndef NDEBUG
        m_callbackInfo.outputFile->flush();
#endif
    }

#ifndef NDEBUG
    std::cerr << "Wrote " << row_count << " rows.\n";
    std::cerr << "Backlog: dev=" << devBacklog << ", ljm=" << ljmBacklog << '\n';
#endif
}
