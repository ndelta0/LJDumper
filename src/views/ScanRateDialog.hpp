#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE

namespace Ui {
    class ScanRateDialog;
}

QT_END_NAMESPACE

class ScanRateDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScanRateDialog(QWidget *parent = nullptr);

    ~ScanRateDialog() override;

private:
    Ui::ScanRateDialog *ui;
};