// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "porteddialog.h"
#include <KConfigGroup>
#include <KPageDialog>
#include <QPushButton>
class PortedPageDialog : public KPageDialog {
    Q_OBJECT
public:
    using ButtonCode = PortedDialog::ButtonCode;
    static constexpr auto Ok = PortedDialog::Ok, Apply = PortedDialog::Apply;
    explicit PortedPageDialog(QWidget* p = nullptr)
        : KPageDialog(p)
    {
        connect(buttonBox(), &QDialogButtonBox::clicked, this, [this](QAbstractButton* b) {
            if (buttonBox()->buttonRole(b) == QDialogButtonBox::AcceptRole)
                emit okClicked();
            if (buttonBox()->buttonRole(b) == QDialogButtonBox::ApplyRole)
                emit applyClicked();
        });
    }
    void setCaption(const QString& s) { setWindowTitle(s); }
    QWidget* mainWidget() { return pageWidget(); }
    void setButtons(int flags)
    {
        QDialogButtonBox::StandardButtons buttons;
        if (flags & PortedDialog::Ok)
            buttons |= QDialogButtonBox::Ok;
        if (flags & PortedDialog::Cancel)
            buttons |= QDialogButtonBox::Cancel;
        if (flags & PortedDialog::Apply)
            buttons |= QDialogButtonBox::Apply;
        if (flags & PortedDialog::Close)
            buttons |= QDialogButtonBox::Close;
        if (flags & PortedDialog::Help)
            buttons |= QDialogButtonBox::Help;
        setStandardButtons(buttons);
        for (auto type : {QDialogButtonBox::Ok, QDialogButtonBox::Cancel, QDialogButtonBox::Apply,
                          QDialogButtonBox::Close, QDialogButtonBox::Help}) {
            if (auto* b = buttonBox()->button(type)) {
                switch (type) {
                case QDialogButtonBox::Ok: KGuiItem::assign(b, KStandardGuiItem::ok()); break;
                case QDialogButtonBox::Cancel: KGuiItem::assign(b, KStandardGuiItem::cancel()); break;
                case QDialogButtonBox::Apply: KGuiItem::assign(b, KStandardGuiItem::apply()); break;
                case QDialogButtonBox::Close: KGuiItem::assign(b, KStandardGuiItem::close()); break;
                case QDialogButtonBox::Help: KGuiItem::assign(b, KStandardGuiItem::help()); break;
                default: break;
                }
            }
        }
    }
    void enableButton(ButtonCode c, bool e)
    {
        if (auto* b = buttonBox()->button(c == Apply ? QDialogButtonBox::Apply : QDialogButtonBox::Ok))
            b->setEnabled(e);
    }
    void enableButtonOk(bool e) { enableButton(Ok, e); }
    void restoreDialogSize(const KConfigGroup& g) { resize(g.readEntry("Size", QSize(640, 480))); }
    void saveDialogSize(KConfigGroup& g) { g.writeEntry("Size", size()); }
signals:
    void okClicked();
    void applyClicked();
};
