#include "fileNameTemplateValidator.hpp"

#include <QFileInfo>

FileNameTemplateValidator::FileNameTemplateValidator(QObject *parent) : QValidator(parent) {
}

QValidator::State FileNameTemplateValidator::validate(QString &input, int &) const {
    if (input.isEmpty())
        return Acceptable; // an empty string is valid (takes default file name)

    if (input.length() > 255)
        return Invalid;

    // 1. Invalid characters
    static QRegularExpression invalid_chars(R"([<>:"/\\|?*])"); // Windows forbidden + null
    if (input.contains(invalid_chars)) {
        return Invalid;
    }

    // 2. Check placeholders
    static const QStringList allowed_templates = {"%Y", "%M", "%D", "%h", "%m", "%s", "%i"};
    static QRegularExpression placeholder_pattern("%.");
    auto it = placeholder_pattern.globalMatch(input);
    while (it.hasNext()) {
        if (const auto match = it.next(); !allowed_templates.contains(match.captured())) {
            return Invalid;
        }
    }

    // 3. Windows reserved names
#ifdef Q_OS_WIN
    const QString base_name = QFileInfo(input).completeBaseName().toUpper();
    static const QStringList reserved = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };
    if (reserved.contains(base_name)) {
        return Invalid;
    }
#endif

    return Acceptable;
}