#include "DFlowLayout.hpp"

#include <QWidget>

DFlowLayout::DFlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent),
      m_hSpace(hSpacing),
      m_vSpace(vSpacing) {
    setContentsMargins(margin, margin, margin, margin);
}

DFlowLayout::~DFlowLayout() {
    QLayoutItem *item;
    while ((item = DFlowLayout::takeAt(0)))
        delete item;
}

void DFlowLayout::addItem(QLayoutItem *item) {
    m_items.append(item);
}

int DFlowLayout::horizontalSpacing() const {
    return m_hSpace >= 0 ? m_hSpace : smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int DFlowLayout::verticalSpacing() const {
    return m_vSpace >= 0 ? m_vSpace : smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int DFlowLayout::count() const {
    return m_items.size();
}

QLayoutItem *DFlowLayout::itemAt(const int index) const {
    return m_items.value(index);
}

QLayoutItem *DFlowLayout::takeAt(const int index) {
    return index >= 0 && index < m_items.size()
               ? m_items.takeAt(index)
               : nullptr;
}

Qt::Orientations DFlowLayout::expandingDirections() const {
    return {};
}

bool DFlowLayout::hasHeightForWidth() const {
    return true;
}

int DFlowLayout::heightForWidth(const int width) const {
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize DFlowLayout::minimumSize() const {
    QSize size;
    for (const QLayoutItem *item: m_items)
        size = size.expandedTo(item->minimumSize());

    const QMargins m = contentsMargins();
    size += QSize(m.left() + m.right(), m.top() + m.bottom());
    return size;
}

void DFlowLayout::setGeometry(const QRect &rect) {
    QLayout::setGeometry(rect);
    std::ignore = doLayout(rect, false);
}

QSize DFlowLayout::sizeHint() const {
    return minimumSize();
}

int DFlowLayout::doLayout(const QRect &rect, const bool testOnly) const {
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);

    const QRect area = rect.adjusted(left, top, -right, -bottom);
    if (m_items.isEmpty())
        return 0;

    const int hSpace = horizontalSpacing();
    const int vSpace = verticalSpacing();

    // ---- First pass: determine columns ----
    QVector<QVector<QLayoutItem *> > rows;
    QVector<int> rowHeights;

    QVector<QLayoutItem *> currentRow;
    int rowWidth = 0;
    int rowHeight = 0;

    for (QLayoutItem *item: m_items) {
        const QSize sz = item->sizeHint();
        const int nextWidth =
                currentRow.isEmpty()
                    ? sz.width()
                    : rowWidth + hSpace + sz.width();

        if (nextWidth > area.width() && !currentRow.isEmpty()) {
            rows.append(currentRow);
            rowHeights.append(rowHeight);
            currentRow.clear();
            rowWidth = 0;
            rowHeight = 0;
        }

        if (!currentRow.isEmpty())
            rowWidth += hSpace;

        currentRow.append(item);
        rowWidth += sz.width();
        rowHeight = qMax(rowHeight, sz.height());
    }

    if (!currentRow.isEmpty()) {
        rows.append(currentRow);
        rowHeights.append(rowHeight);
    }

    // ---- Determine max column widths ----
    int columnCount = 0;
    for (const auto &row: rows)
        columnCount = qMax(columnCount, row.size());

    QVector<int> colWidths(columnCount, 0);
    for (const auto &row: rows) {
        for (int c = 0; c < row.size(); ++c)
            colWidths[c] = qMax(colWidths[c],
                                row[c]->sizeHint().width());
    }

    // ---- Compute total width ----
    int totalWidth = 0;
    for (const int w: colWidths)
        totalWidth += w;
    totalWidth += hSpace * (columnCount - 1);

    const int startX = area.x() + qMax(0, (area.width() - totalWidth) / 2);

    // ---- Second pass: place items ----
    int y = area.y();

    for (int r = 0; r < rows.size(); ++r) {
        int x = startX;

        for (int c = 0; c < rows[r].size(); ++c) {
            QLayoutItem *item = rows[r][c];
            const QSize sz = item->sizeHint();

            if (!testOnly) {
                item->setGeometry(QRect(
                    QPoint(x, y),
                    QSize(colWidths[c], sz.height())
                ));
            }

            x += colWidths[c] + hSpace;
        }

        y += rowHeights[r] + vSpace;
    }

    return y - rect.y() + bottom;
}


int DFlowLayout::smartSpacing(const QStyle::PixelMetric pm) const {
    QObject *parentObj = parent();
    if (!parentObj)
        return -1;

    if (parentObj->isWidgetType()) {
        const auto pw = static_cast<QWidget *>(parentObj);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }

    return static_cast<QLayout *>(parentObj)->spacing();
}