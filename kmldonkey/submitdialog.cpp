/* -*- c++ -*-
 *
 * submitdialog.cpp
 *
 * Copyright (C) 2003-2004,2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "submitdialog.h"

#include <QLayout>
#include <QLabel>
#include <KLocalizedString>
#include <QTextEdit>

#include "kmldonkey.h"
#include "donkeyprotocol.h"

SubmitDialog::SubmitDialog(QWidget* parent)
    : PortedDialog(parent)
{
    setCaption(i18n("Submit URLs"));
    setObjectName("SubmitDialog");
    setButtons(PortedDialog::Ok|PortedDialog::Cancel);

    m_edit = new QTextEdit(this);
    m_edit->setLineWrapMode(QTextEdit::NoWrap);
    m_edit->setAcceptRichText(false);
    m_edit->setPlaceholderText(i18n("Paste download links here, one per line…"));
    m_edit->setAccessibleName(i18n("Download links"));
    m_edit->setFocus();
    connect(m_edit, SIGNAL(textChanged()), this, SLOT(editChanged()));
    editChanged();

    setMainWidget(m_edit);
    resize(QSize(600, 400).expandedTo(minimumSizeHint()));

    connect(this, SIGNAL(finished(int)), this, SLOT(closeDialog()));
    connect(this, SIGNAL(okClicked()), this, SLOT(addUrl()));
}

SubmitDialog::~SubmitDialog()
{
}

QString SubmitDialog::text() const
{
    return m_edit->toPlainText();
}

void SubmitDialog::setText(const QString& s)
{
    m_edit->setText(s);
}

void SubmitDialog::editChanged()
{
    enableButtonOk(! m_edit->toPlainText().isEmpty());
}

void SubmitDialog::addUrl()
{
    foreach(const QString &text, m_edit->toPlainText().split("\n")) {
        const QString s = text.trimmed();
        if (! s.isEmpty())
            KMLDonkey::App->donkey->submitUrl(s);
    }
}

void SubmitDialog::closeDialog()
{
    deleteLater();
}
