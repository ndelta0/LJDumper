#pragma once
#include <QComboBox>
#include <QStandardItemModel>

template<typename E>
constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

template<typename E>
class EnumIterator {
    int value;

public:
    explicit EnumIterator(int v) : value(v) {
    }

    E operator*() const { return static_cast<E>(value); }

    EnumIterator &operator++() {
        ++value;
        return *this;
    }

    bool operator!=(const EnumIterator &other) const { return value != other.value; }
};

template<typename E>
class EnumRange {
    int begin_value, end_value;

public:
    EnumRange(E begin, E end) : begin_value(to_underlying(begin)), end_value(to_underlying(end)) {
    }

    EnumIterator<E> begin() const { return EnumIterator<E>(begin_value); }
    EnumIterator<E> end() const { return EnumIterator<E>(end_value + 1); }
};

template<typename E>
EnumRange<E> enum_range(E begin, E end) {
    return EnumRange<E>(begin, end);
}

inline void ComboBoxItemSetEnabled(const QComboBox *cb, const int index, const bool enabled) {
    const auto model =
            qobject_cast<QStandardItemModel *>(cb->model());
    Q_ASSERT(model != nullptr);
    QStandardItem *item = model->item(index);
    item->setFlags(enabled
                       ? item->flags() | Qt::ItemIsEnabled
                       : item->flags() & ~Qt::ItemIsEnabled);
}

inline QString FormatTime(const double seconds) {
    struct Unit {
        const char *name;
        double scale;
    };

    const double absVal = std::abs(seconds);

    for (int i = 0; i < 4; ++i) {
        constexpr Unit units[] = {
            {"ns", 1e-9},
            {"us", 1e-6},
            {"ms", 1e-3},
            {"s", 1.0}
        };
        if (const double value = absVal / units[i].scale; value >= 1.0 || i == 3) {
            return QString::number(seconds / units[i].scale, 'f', 3)
                   + " " + units[i].name;
        }
    }

    return "-- s"; // unreachable
}

inline QString FormatFrequency(const double hz) {
    struct Unit {
        const char *name;
        double scale;
    };

    const double absVal = std::abs(hz);

    for (int i = 2; i >= 0; --i) {
        constexpr Unit units[] = {
            {"Hz", 1.0},
            {"kHz", 1e3},
            {"MHz", 1e6}
        };
        // pick the largest usable unit
        if (const double value = absVal / units[i].scale; value >= 1.0 || i == 0) {
            return QString::number(hz / units[i].scale, 'f', 3)
                   + " " + units[i].name;
        }
    }

    return "-- Hz"; // unreachable
}