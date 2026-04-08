#pragma once

#include <QAbstractSpinBox>

// Taken from https://codeberg.org/besc/int64spinbox

class Int64SpinBox : public QAbstractSpinBox {
    Q_OBJECT

public:
    using ValueType = qint64;
    static const int valueTypeMetaId;

private:
    Q_PROPERTY(QString cleanText READ cleanText)

    Q_PROPERTY(QString displayedText READ displayedText)

    Q_PROPERTY(qint32 displayIntegerBase READ displayIntegerBase WRITE setDisplayIntegerBase)

    Q_PROPERTY(ValueType maximum READ maximum)

    Q_PROPERTY(ValueType minimum READ minimum)

    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)

    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)

    Q_PROPERTY(qint32 singleStep READ singleStep WRITE setSingleStep)

    //    Q_PROPERTY(ValueType value READ value WRITE setValue NOTIFY valueChanged USERtrue)

public:
    explicit Int64SpinBox(QWidget *parent = nullptr);

    Int64SpinBox(const Int64SpinBox &) = delete;

    Int64SpinBox(Int64SpinBox &&) = delete;

    Int64SpinBox &operator=(const Int64SpinBox &) = delete;

    Int64SpinBox &&operator=(Int64SpinBox &&) = delete;

    /** @name Read access to properties */
    ///@{
    /** Returns the value as a string, excluding prefix and suffix. */
    QString cleanText() const;

    /** Returns the complete displayed text, i.e. the value including prefix and suffix. */
    QString displayedText() const;

    qint32 displayIntegerBase() const { return m_base; }
    ValueType maximum() const { return m_maximum; }
    ValueType minimum() const { return m_minimum; }
    QString prefix() const { return m_prefix; }
    QString suffix() const { return m_suffix; }
    qint32 singleStep() const { return m_step_size; }
    ValueType value() const { return m_value; }
    ///@}


    /**
    Sets the integer format used for displaying the value. Must be a number between ``2`` and
    ``36``. Default is ``10`` (decimal).

    Note: ``displayIntegerBase`` only sets the number format itself. If you want to display a
    prefix for the format – like *0x* for hexadecimal – see the ``prefix`` property.
    */
    void setDisplayIntegerBase(qint32 base);


    /**
    Sets the allowed minimum and maximum value. ``min`` must be less than or equal to ``max``.
    By default the full 64bit integer range is allowed. The current value is automatically clamped
    to the new range.
    */
    void setRange(ValueType min, ValueType max);


    /** Sets the text that gets prepended to the displayed value. Empty by default. */
    void setPrefix(const QString &prefix);


    /** Sets the text that gets appended to the displayed value.  Empty by default. */
    void setSuffix(const QString &suffix);


    /**
    Sets the amount the value is incremented or decremented in a single step, e.g. when the user
    clicks the up or down button. Must be greater than ``0``. Defaults to ``1``.
    */
    void setSingleStep(qint32 step_size);


    /**
    Reimplemented from ``QAbstractSpinBox``. Changes the value by the given number of steps, taking
    ``singleStep`` into account. If ``steps`` is ``0`` the function has no effect.
    */
    void stepBy(qint32 steps) override;

public
    slots:
    /**
    Sets a new value. ``value`` must be valid for the current range. The ``valueChanged()`` signals
    are emitted only if the value actually changes.
    */
    Q_SLOT

    void setValue(ValueType value);


    /**
    Sets a new value from the given text. This is equivalent to pasting text into the widget.

    Depending on ``displayIntegerBase()`` the text may include the well-known prefix for that
    number format:

    * *0b* or *0B* for base ``2`` (binary)
    * *0o* or *0O* for base ``8`` (octal)
    * *0x* and *0X* for base ``16`` (hexadecimal)

    The ``valueChanged()`` signals are emitted only if the  value actually changes. If validation
    fails, the function has no effect.
    */
    Q_SLOT void setValueText(QString value_text);


    signals:
    /**
    Both signals are emitted when the value changes (numeric version first). Like ``QSpinBox`` the
    first signal contains the numeric value and the second signal the full text as returned by
    ``displayedText()``.
    */ ///@{


    

    void valueChanged(ValueType new_value);

    void valueChanged(QString new_displayed_text);

    /// @}


protected:
    /** Works the same as described for ``QAbstractSpinBox::stepEnabled()``. */
    StepEnabled stepEnabled() const override;


    /**
    Same as ``QSpinBox::textFromValue()``. Takes an integer value and returns the display string
    corresponding to that value – without prefix and/or suffix. For details about reimplementing
    this function in derived classes see the class description.
    */
    virtual QString textFromValue(ValueType value) const;


    /**
    Same as ``QSpinBox::fixup()``. Is called after validation of ``input`` failed to change it to
    a valid input string, if possible. For details about reimplementing this function in derived
    classes see the class description.
    */
    void fixup(QString &input) const override;


    /**
    Similar to ``QAbstractSpinBox::validate()``. Parses ``text`` to get a valid value and returns
    the resulting state of the parsing. Must be able to handle input text with and without
    prefix and/or suffix. Also must set ``m_cache``. For details about reimplementing this function
    in derived classes see the class description.
    */
    QValidator::State validate(QString &text, int &pos) const override;

    mutable struct {
        bool matches(const QString &t) const { return !text.isEmpty() && t == text; }
        QString text;
        QValidator::State state;
        ValueType value;
    } m_cache{};

private:
    ValueType clamp(ValueType value) const;

    quint64 calcHeadroom(bool stepping_up) const;

    Q_SLOT void onEditingFinished();

    void selectValueText();

    qint32 m_base = 10;
    qint32 m_step_size = 1;
    QString m_prefix;
    QString m_suffix;
    ValueType m_minimum;
    ValueType m_maximum;
    ValueType m_value = 0;
};

Q_DECLARE_METATYPE(Int64SpinBox::ValueType);