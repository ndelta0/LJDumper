#include "ScanRateDialog.hpp"
#include "ui_ScanRateDialog.h"


ScanRateDialog::ScanRateDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ScanRateDialog) {
    ui->setupUi(this);
}

ScanRateDialog::~ScanRateDialog() {
    delete ui;
}