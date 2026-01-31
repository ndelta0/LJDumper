#pragma once

#include <QValidator>

class FileNameTemplateValidator : public QValidator {
public:
    explicit FileNameTemplateValidator(QObject *parent = nullptr);

    State validate(QString &input, int &) const override;
};