// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "porteddialog.h"
#include <KLineEdit>
#include <QLabel>

// Small text prompts share the same KDE heading and buttons as other dialogs.
namespace KdeInputDialog {
inline QString getText(QWidget* parent, const QString& title, const QString& prompt,
                       QLineEdit::EchoMode mode, const QString& value, bool* accepted)
{
    PortedDialog dialog(parent);
    dialog.setCaption(title);
    auto* layout = new QVBoxLayout(dialog.mainWidget());
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    auto* label = new QLabel(prompt, dialog.mainWidget());
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    auto* edit = new KLineEdit(dialog.mainWidget());
    edit->setEchoMode(mode);
    edit->setText(value);
    edit->setClearButtonEnabled(true);
    label->setBuddy(edit);
    layout->addWidget(label);
    layout->addWidget(edit);
    edit->selectAll();
    edit->setFocus();
    dialog.resize(QSize(480, dialog.sizeHint().height()).expandedTo(dialog.minimumSizeHint()));
    const bool ok = dialog.exec() == QDialog::Accepted;
    if (accepted) *accepted = ok;
    return ok ? edit->text() : QString();
}
}
