#include "FileNameTemplatingDialog.hpp"
#include "ui_FileNameTemplatingDialog.h"


FileNameTemplatingDialog::FileNameTemplatingDialog(QWidget *parent) : QDialog(parent),
                                                                      ui(new Ui::FileNameTemplatingDialog) {
    ui->setupUi(this);
}

FileNameTemplatingDialog::~FileNameTemplatingDialog() {
    delete ui;
}