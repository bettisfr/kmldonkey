// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <QBoxLayout>
#include <QChildEvent>
#include <QWidget>
class BoxWidget : public QWidget {
public:
    BoxWidget(QBoxLayout::Direction direction, QWidget* parent = nullptr)
        : QWidget(parent)
        , box(new QBoxLayout(direction, this))
    {
        box->setContentsMargins(0, 0, 0, 0);
    }
    void setSpacing(int n) { box->setSpacing(n); }
    void setMargin(int n) { box->setContentsMargins(n, n, n, n); }
    void setStretchFactor(QWidget* w, int n)
    {
        insertInOrder(w);
        box->setStretchFactor(w, n);
    }

protected:
    void childEvent(QChildEvent* e) override
    {
        QWidget::childEvent(e);
        if (e->polished())
            if (auto* w = qobject_cast<QWidget*>(e->child()))
                if (!w->isWindow() && box->indexOf(w) < 0)
                    insertInOrder(w);
    }

private:
    void insertInOrder(QWidget* widget)
    {
        if (box->indexOf(widget) >= 0)
            return;
        int index = 0;
        for (auto* child : children()) {
            if (child == widget)
                break;
            if (auto* sibling = qobject_cast<QWidget*>(child))
                if (box->indexOf(sibling) >= 0)
                    ++index;
        }
        box->insertWidget(index, widget);
    }
    QBoxLayout* box;
};
class VBoxWidget : public BoxWidget {
public:
    explicit VBoxWidget(QWidget* p = nullptr)
        : BoxWidget(QBoxLayout::TopToBottom, p)
    {
    }
};
class HBoxWidget : public BoxWidget {
public:
    explicit HBoxWidget(QWidget* p = nullptr)
        : BoxWidget(QBoxLayout::LeftToRight, p)
    {
    }
};
