#include <KConfigGroup>
/* -*- c++ -*-
 *
 * addserver.cpp
 *
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

#include "addserver.h"

#include "kmldonkey.h"
#include "donkeyprotocol.h"
#include "network.h"

#include <QLayout>
#include <QLabel>
#include <QGridLayout>

#include <KSharedConfig>
#include <kconfig.h>
#include <KLocalizedString>
#include <QDebug>
#include <kcombobox.h>
#include <klineedit.h>
#include <QSpinBox>

AddServerDialog::AddServerDialog(QWidget* parent)
    : PortedDialog(parent)
{
    setObjectName("AddServerDialog");
    setButtons(PortedDialog::Ok|PortedDialog::Cancel);
    setCaption(i18n("Add Server"));
    setModal(true);

    KSharedConfigPtr conf = KSharedConfig::openConfig();
    KConfigGroup group = conf->group( "AddServerDialog" );
    QString defnetwork = group.readEntry("Network", "Donkey");
    QString defaddress = group.readEntry("Address", "");
    int defport = group.readEntry("Port", 4662);

    QGridLayout* boxl = new QGridLayout(mainWidget());
    boxl->setAlignment(Qt::AlignTop);
    boxl->setHorizontalSpacing(12);
    boxl->setVerticalSpacing(12);

    QLabel* label = new QLabel(i18n("&Address:"), this);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    boxl->addWidget(label, 0, 0);

    m_addressEntry = new KLineEdit(this);
    m_addressEntry->setPlaceholderText(i18n("Server address or hostname"));
    m_addressEntry->setClearButtonEnabled(true);
    label->setBuddy(m_addressEntry);
    boxl->addWidget(m_addressEntry, 0, 1, 1, 3);

    label = new QLabel(i18n("&Port:"), this);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    boxl->addWidget(label, 1, 0);

    m_portEntry = new QSpinBox(this);
    m_portEntry->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_portEntry->setRange(1, 65535);

    label->setBuddy(m_portEntry);
    boxl->addWidget(m_portEntry, 1, 1);

    label = new QLabel(i18n("&Network:"), this);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    boxl->addWidget(label, 1, 2);

    m_networkEntry = new KComboBox(false, this);
    m_networkEntry->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    label->setBuddy(m_networkEntry);
    boxl->addWidget(m_networkEntry, 1, 3);

    foreach (Network *nw, KMLDonkey::App->donkey->availableNetworks()) {
        if (nw->networkEnabled()
            && (nw->networkFlags() & Network::NetworkHasServers)) {
            m_networkEntry->addItem(nw->networkName());
            if (nw->networkName() == defnetwork) {
                m_networkEntry->setCurrentIndex( m_networkEntry->count() - 1 );
            }
        }
    }

    setTabOrder(m_addressEntry, m_portEntry);
    setTabOrder(m_portEntry, m_networkEntry);

    m_addressEntry->setText(defaddress);
    m_addressEntry->selectAll();
    m_addressEntry->setFocus();
    connect(m_addressEntry, SIGNAL(textChanged(const QString &)), this, SLOT(addressEditChanged()));
    addressEditChanged();

    m_portEntry->setValue(defport);

    resize(QSize(486, 124).expandedTo(minimumSizeHint()));

    connect(this, SIGNAL(finished(int)), this, SLOT(closeDialog()));
    connect(this, SIGNAL(okClicked()), this, SLOT(addServer()));
}

void AddServerDialog::addressEditChanged()
{
    enableButton(PortedDialog::Ok, !m_addressEntry->text().isEmpty());
}

void AddServerDialog::addServer()
{
    int network = 0;
    if (m_networkEntry) {
        Network *nw = KMLDonkey::App->donkey->findNetworkName(m_networkEntry->currentText());
        if (nw) network = nw->networkNo();
    }
    QString address = m_addressEntry->text().trimmed();
    int16 port = (int16)m_portEntry->value();

    KMLDonkey::App->donkey->addServer(network, address, port);
}

void AddServerDialog::closeDialog()
{
    KSharedConfigPtr conf = KSharedConfig::openConfig();
    KConfigGroup group = conf->group( "AddServerDialog" );
    if (m_networkEntry) group.writeEntry("Network", m_networkEntry->currentText());
    group.writeEntry("Address", m_addressEntry->text());
    group.writeEntry("Port", m_portEntry->value());

    deleteLater();
}
