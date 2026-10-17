/* -*- c++ -*-
 *
 * optioneditor.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
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

#include <QHeaderView>

#include <QDebug>
#include <KLocalizedString>

#include <donkeyprotocol.h>

#include "optioneditor.h"
#include "kmldonkey.h"

MyDonkeyOption::MyDonkeyOption()
    : m_option()
    , m_value()
    , m_dirty(false)
{
}

MyDonkeyOption::MyDonkeyOption(const DonkeyOption& opt, const QString& value)
    : m_option(opt)
    , m_value(value)
    , m_dirty(false)
{
}

MyDonkeyOption::MyDonkeyOption(const MyDonkeyOption& opt) {
    m_option = opt.m_option;
    m_value = opt.m_value;
    m_dirty = opt.m_dirty;
}

void MyDonkeyOption::setValue(const QString& value)
{
    m_value = value;
    m_dirty = true;
}

bool MyDonkeyOption::isDirty() const
{
    return m_dirty;
}

void MyDonkeyOption::setDirty(bool dirty)
{
    m_dirty = dirty;
}

void MyDonkeyOption::applyOption(DonkeyProtocol* target)
{
    if (!m_dirty) return;
    target->setOption(m_option.optionName(), m_value);
    m_dirty = false;
    qDebug() << "Option \"" << m_option.optionName() << "\" set to \"" << m_value << "\"";
}

const DonkeyOption& MyDonkeyOption::option() const
{
    return m_option;
}



OptionEditorWidget::OptionEditorWidget(QWidget* parent)
    : QTableWidget(0, 2, parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
    setShowGrid(false);
    setFrameShape(QFrame::NoFrame);
    setWordWrap(false);
    verticalHeader()->setMinimumSectionSize(fontMetrics().height() + 14);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    verticalHeader()->hide();
    horizontalHeader()->setStretchLastSection(true);
    setHorizontalHeaderLabels( QStringList() << i18n("Option") << i18n("Value") );
    setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    connect(this, SIGNAL(itemChanged(QTableWidgetItem*)), SLOT(optionChanged(QTableWidgetItem*)));
}



void OptionEditorWidget::addOption(const DonkeyOption& option)
{
    int row = rowCount();
    insertRow(row);

    QString label = option.optionLabel();
    if (label == option.optionName()) {
        label.replace(QChar('_'), QChar(' '));
        if (!label.isEmpty()) label[0] = label[0].toUpper();
    }
    QTableWidgetItem *nameItem = new QTableWidgetItem(label);
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);

    QString value = KMLDonkey::App->donkey->getOption(option.optionName());
    QVariant variantValue = value;
    QTableWidgetItem *valueItem = new QTableWidgetItem();
    switch (option.type()) {
    case DonkeyOption::Boolean:
        variantValue.convert(QMetaType::fromType<bool>());
        break;
    case DonkeyOption::Integer:
        variantValue.convert(QMetaType::fromType<int>());
        break;
    case DonkeyOption::Float:
        variantValue.convert(QMetaType::fromType<double>());
        break;
    //TODO: create delegate to set validator for Ip/IpList/Address/Md4/Sha1 types
    default: break;
    }
    valueItem->setData(Qt::DisplayRole, variantValue);
    valueItem->setData(OptionIndexRole, row);
    valueItem->setData(OptionTypeRole, int(option.type()));
    nameItem->setData(Qt::UserRole, option.optionName());
    if (option.type() == DonkeyOption::Boolean) {
        valueItem->setData(Qt::DisplayRole, QVariant());
        valueItem->setFlags((valueItem->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
        valueItem->setCheckState(variantValue.toBool() ? Qt::Checked : Qt::Unchecked);
    }

    nameItem->setToolTip(option.optionName() + QStringLiteral("\n\n") + option.optionHelp());
    valueItem->setToolTip(option.optionHelp());

    setItem(row, 0, nameItem);
    setItem(row, 1, valueItem);

    resizeColumnToContents(0);
    resizeRowToContents(row);

    m_options.append(MyDonkeyOption(option, value));
}


void OptionEditorWidget::optionChanged(QTableWidgetItem *item)
{
    if (item->column() != 1) return; // ignore name change
    const int index = item->data(OptionIndexRole).toInt();
    if (m_options.size() <= index) {
        return;//ignore signals emitted in addOption
    }
    m_options[index].setValue(item->data(OptionTypeRole).toInt() == DonkeyOption::Boolean
        ? (item->checkState() == Qt::Checked ? QStringLiteral("true") : QStringLiteral("false"))
        : item->text());
    emit listIsDirty();
}

void OptionEditorWidget::applyChangedOptions(DonkeyProtocol* target)
{
    OptionList::iterator it = m_options.begin(), itend = m_options.end();
    for (; it != itend; ++it) {
        if (it->isDirty()) {
            it->applyOption(target);
        }
    }
}


