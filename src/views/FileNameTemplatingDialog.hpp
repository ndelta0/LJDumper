#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE

namespace Ui {
    class FileNameTemplatingDialog;
}

QT_END_NAMESPACE

class FileNameTemplatingDialog : public QDialog {
    Q_OBJECT

public:
    explicit FileNameTemplatingDialog(QWidget *parent = nullptr);

    ~FileNameTemplatingDialog() override;

private:
    Ui::FileNameTemplatingDialog *ui;
};