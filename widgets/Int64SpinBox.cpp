#include "Int64SpinBox.hpp"

#include <QLineEdit>
#include <climits>
#include <cstdlib>
#include <limits>

// Some functions and pieces of code are too different between int64 and uint64 for the simple
// search and replace Python script that generates the unsigned version. But the script *can*
// toggle an ifdef. Any implementation must be done in the *signed* .hpp/.cpp.
#define SPINBOX_IS_SIGNED_IMPL


const int Int64SpinBox::valueTypeMetaId = qRegisterMetaType<Int64SpinBox::ValueType>("ValueType");


Int64SpinBox::Int64SpinBox(QWidget *parent)
    : QAbstractSpinBox(parent)
      , m_minimum(std::numeric_limits<ValueType>::min())
      , m_maximum(std::numeric_limits<ValueType>::max()) {
    Q_ASSERT(lineEdit());
    connect(lineEdit(), &QLineEdit::textEdited, this, [&] { setValueText(lineEdit()->text()); });
    connect(this, &QAbstractSpinBox::editingFinished, this, &Int64SpinBox::onEditingFinished);
}


QString Int64SpinBox::cleanText() const {
    return textFromValue(m_value);
}


QString Int64SpinBox::displayedText() const {
    return QStringLiteral("%1%2%3").arg(m_prefix, textFromValue(m_value), m_suffix);
}


void Int64SpinBox::setDisplayIntegerBase(qint32 base) {
    Q_ASSERT(2 <= base && base <= 36); // Qt usually works with those same limits.
    m_base = base;
    setValue(m_value);
}


void Int64SpinBox::setRange(ValueType min, ValueType max) {
    Q_ASSERT(min <= max);

    m_minimum = min;
    m_maximum = max;

    if (m_value < min) {
        setValue(min);
    } else if (m_value > max) {
        setValue(max);
    }
}


void Int64SpinBox::setPrefix(const QString &prefix) {
    m_prefix = prefix;
    setValue(m_value);
}


void Int64SpinBox::setSuffix(const QString &suffix) {
    m_suffix = suffix;
    setValue(m_value);
}


void Int64SpinBox::setSingleStep(qint32 step_size) {
    Q_ASSERT(step_size > 0);
    m_step_size = step_size;
}


void Int64SpinBox::stepBy(qint32 steps) {
    // With qint32 multiplication can never reach the edges of qint64’s range.
    // No extra LLONG_MIN checks are needed.
    qint64 delta = static_cast<qint64>(steps) * m_step_size;
    quint64 abs_delta = static_cast<quint64>(std::llabs(delta));
    quint64 headroom = calcHeadroom(delta > 0);

    if (abs_delta <= headroom) {
        setValue(m_value + delta);
        selectValueText();
    } else if (wrapping()) {
        abs_delta -= headroom;
        setValue(delta > 0 ? m_minimum + abs_delta - 1 : m_maximum - abs_delta + 1);
        selectValueText();
    } else {
        setValue(delta > 0 ? m_maximum : m_minimum);
        selectValueText();
    }
}


void Int64SpinBox::setValue(ValueType value) {
    Q_ASSERT((value >= m_minimum) && (value <= m_maximum));

    // Cannot bail out immediately when previous and new value are the same. This could be
    // a reset to the previous value after the user entered something illegal. In that case
    // the displayed text must be restored.
    const bool is_same_value = value == m_value;

    m_value = value;
    auto displayed_text = displayedText();

    if (displayed_text != lineEdit()->text()) {
        lineEdit()->setText(displayed_text);
    }

    if (is_same_value) {
        return;
    }

    // Ensures that up/down buttons are repainted. They might have become en/disabled
    update();

    Q_EMIT valueChanged(value);
    Q_EMIT valueChanged(displayed_text);
}


void Int64SpinBox::setValueText(QString value_text) {
    if (value_text.isEmpty()) {
        lineEdit()->setText(m_prefix + m_suffix);
        lineEdit()->setCursorPosition(m_prefix.size());
        return;
    }

    int pos = 0;
    auto validity = validate(value_text, pos);

    if (validity == QValidator::Invalid) {
        fixup(value_text);
        validity = validate(value_text, pos);
    }

    switch (validity) {
        case QValidator::Acceptable:
            setValue(m_cache.value);
            break;
        case QValidator::Invalid:
            setValue(m_cache.value);
            break;
        case QValidator::Intermediate:
            // Cannot do anything right now. Must wait for more input from the user.
            break;
    }
}


QAbstractSpinBox::StepEnabled Int64SpinBox::stepEnabled() const {
    if (isReadOnly()) {
        return StepNone;
    }

    if (wrapping()) {
        return StepUpEnabled | StepDownEnabled;
    }

    StepEnabled available_dirs = StepNone;

    if (m_value > m_minimum) {
        available_dirs |= StepDownEnabled;
    }

    if (m_value < m_maximum) {
        available_dirs |= StepUpEnabled;
    }

    return available_dirs;
}


void Int64SpinBox::fixup(QString &input) const {
    if (!isGroupSeparatorShown()) {
        input.remove(locale().groupSeparator());
    }

    switch (m_base) {
        case 2:
            if (input.startsWith(QLatin1String("0b")) || input.startsWith(QLatin1String("0B"))) {
                input = input.mid(2);
            }
            break;
        case 8:
            if (input.startsWith(QLatin1String("0o")) || input.startsWith(QLatin1String("0O"))) {
                input = input.mid(2);
            }
            break;
        case 16:
            if (input.startsWith(QLatin1String("0x")) || input.startsWith(QLatin1String("0X"))) {
                input = input.mid(2);
            }
            break;
        default:
            break;
    }
}


QValidator::State Int64SpinBox::validate(QString &text, int &pos) const {
    Q_UNUSED(pos);

    if (m_cache.matches(text)) {
        return m_cache.state;
    }

    QString value_text(text);

    if (!m_prefix.isEmpty() && text.startsWith(m_prefix)) {
        value_text = value_text.mid(m_prefix.size());
    }
    if (!m_suffix.isEmpty() && text.endsWith(m_suffix)) {
        value_text = value_text.left(value_text.size() - m_suffix.size());
    }

    value_text = value_text.trimmed();

    if (!specialValueText().isEmpty() && value_text == specialValueText()) {
        m_cache = {text, QValidator::Acceptable, m_minimum};
        return m_cache.state;
    }

    if (value_text.isEmpty()) {
        m_cache = {text, QValidator::Intermediate, m_minimum};
        return m_cache.state;
    }
#ifdef SPINBOX_IS_SIGNED_IMPL
    if ((m_minimum < 0 && value_text == "-") || (m_maximum >= 0 && value_text == "+")) {
#else
        if (value_text == "+") { 
#endif
        m_cache = {text, QValidator::Intermediate, m_value};
        return m_cache.state;
    }

    bool ok;
    ValueType value;

    if (m_base == 10) {
        value = locale().toLongLong(value_text, &ok);
    } else {
        value = value_text.toLongLong(&ok, m_base);
    }

    if (!ok) {
        m_cache = {text, QValidator::Invalid, m_value};
        return m_cache.state;
    }

    if (m_minimum <= value && value <= m_maximum) {
        m_cache = {text, QValidator::Acceptable, value};
        return m_cache.state;
    }

#ifdef SPINBOX_IS_SIGNED_IMPL
    if ((value >= 0 && value > m_maximum) || (value < 0 && value < m_minimum)) {
#else
        if (value > m_maximum) { 
#endif
        const bool revert = correctionMode() == QAbstractSpinBox::CorrectToPreviousValue;
        m_cache = {text, QValidator::Invalid, (revert ? m_value : clamp(value))};
        return m_cache.state;
    }

    m_cache = {text, QValidator::Intermediate, value};
    return m_cache.state;
}


Int64SpinBox::ValueType Int64SpinBox::clamp(ValueType value) const {
    if (value < m_minimum) {
        return m_minimum;
    }

    if (value > m_maximum) {
        return m_maximum;
    }

    return value;
}


void Int64SpinBox::onEditingFinished() {
    if (m_cache.text.isEmpty() || m_cache.state != QValidator::Intermediate) {
        return;
    }

    if (correctionMode() == QAbstractSpinBox::CorrectToPreviousValue) {
        setValue(m_value);
    } else {
        setValue(clamp(m_cache.value));
    }
}


void Int64SpinBox::selectValueText() {
    lineEdit()->setSelection(
        m_prefix.size(), lineEdit()->text().size() - m_prefix.size() - m_suffix.size());
}


#ifdef SPINBOX_IS_SIGNED_IMPL
quint64 Int64SpinBox::calcHeadroom(bool stepping_up) const {
    const ValueType bound = stepping_up ? m_maximum : m_minimum;
    quint64 abs_bound =
            bound == LLONG_MIN ? static_cast<quint64>(LLONG_MAX) + 1ull : std::llabs(bound);
    quint64 abs_value =
            m_value == LLONG_MIN ? static_cast<quint64>(LLONG_MAX) + 1ull : std::llabs(m_value);

    if (abs_value > abs_bound) {
        std::swap(abs_value, abs_bound);
    }

    return abs_bound - abs_value;
}


QString Int64SpinBox::textFromValue(ValueType value) const {
    if (m_base == 10) {
        if (isGroupSeparatorShown()) {
            return locale().toString(value);
        }

        return QString::number(value);
    }

    if (Q_UNLIKELY(value == LLONG_MIN)) {
        const auto abs_value = static_cast<unsigned long long>(LLONG_MAX) + 1ULL;
        return '-' + QString::number(abs_value, m_base);
    } else if (value >= 0) {
        return QString::number(value, m_base);
    } else {
        return '-' + QString::number(std::abs(value), m_base);
    }
}

#else

quint64 Int64SpinBox::calcHeadroom(bool stepping_up) const {
    if (stepping_up) {
        return m_maximum - m_value;
    } else {
        return m_value - m_minimum;
    }
}


QString Int64SpinBox::textFromValue(ValueType value) const {
    if (m_base == 10 && isGroupSeparatorShown()) {
        return locale().toString(value);
    }

    return QString::number(value, m_base);
}

#endif  // SPINBOX_IS_SIGNED_IMPL