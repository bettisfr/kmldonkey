#include <KConfig>
#include <KConfigGroup>
/* -*- c++ -*-
 *
 * hostdialog.cpp
 *
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
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

#include "hostdialog.h"
#include "hostmanager.h"
#include "hostselectaction.h"
#include "donkeyprotocol.h"
#include "donkeyhost.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QListWidget>
#include <QDebug>

#include <KLocalizedString>
#include <QIcon>
#include "porteddialog.h"

const int HostInterfaceRole = Qt::UserRole + 1;

/*
QListWidgetItem
        void setDefault(bool isdefault) { setIcon( isdefault ? QIcon::fromTheme("kmldonkey") : QIcon() ); }
*/

HostDialog::HostDialog(QWidget* parent, HostManager* hostmanager, DonkeyProtocol* donkey)
    : QWidget( parent )
    , hostmanager(hostmanager)
    , donkey(donkey)
    , m_defaultitem(0)
{
    setObjectName( "HostDialog" );
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    {
        QWidget* box = new QWidget(this);
        layout->addWidget(box, 2);
        QHBoxLayout* boxLayout = new QHBoxLayout(box);
        boxLayout->setContentsMargins(0, 0, 0, 0);
        box->setLayout(boxLayout);

        hostList = new QListWidget(box);
        boxLayout->addWidget(hostList, 1);
        //hostList->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 1, 0, hostList->sizePolicy().hasHeightForWidth() ) );

        QWidget* btnbox = new QWidget(this);
        boxLayout->addWidget(btnbox);
        QVBoxLayout* btnLayout = new QVBoxLayout(btnbox);
        btnLayout->setSpacing(6);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        btnbox->setLayout(btnLayout);

        connectButton = new QPushButton(i18n("Connect"), box);
        btnLayout->addWidget( connectButton );
        connectButton->setIcon(QIcon::fromTheme("network-connect"));
        connect( connectButton, SIGNAL( clicked() ), this, SLOT( connectButtonClicked() ) );

        disconnectButton = new QPushButton(i18n("Disconnect"), box);
        btnLayout->addWidget( disconnectButton );
        disconnectButton->setIcon(QIcon::fromTheme("network-disconnect"));
        connect( disconnectButton, SIGNAL( clicked() ), this, SLOT( disconnectButtonClicked() ) );

        QFrame* f = new QFrame(box);
        f->setFrameShape(QFrame::HLine);
        f->setFrameShadow(QFrame::Sunken);
        btnLayout->addWidget(f);

        defaultButton = new QPushButton(i18n("Set as Default"), box);
        btnLayout->addWidget( defaultButton );
        defaultButton->setIcon(QIcon::fromTheme("kmldonkey"));
        connect( defaultButton, SIGNAL( clicked() ), this, SLOT( defaultButtonClicked() ) );

        newButton = new QPushButton(i18n("Add"), box);
        btnLayout->addWidget( newButton );
        newButton->setIcon(QIcon::fromTheme("list-add"));
        connect( newButton, SIGNAL( clicked() ), this, SLOT( newButtonClicked() ) );

        deleteButton = new QPushButton(i18n("Remove"), box);
        btnLayout->addWidget( deleteButton );
        deleteButton->setIcon(QIcon::fromTheme("list-remove"));
        connect( deleteButton, SIGNAL( clicked() ), this, SLOT( deleteButtonClicked() ) );

        btnLayout->addStretch(1);
    }

    QWidget* hostFrame = new QWidget( this );
    layout->addWidget(hostFrame, 1);
    hostFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGridLayout* hostFrameLayout = new QGridLayout( hostFrame );
    hostFrameLayout->setContentsMargins(0, 0, 0, 0);
    hostFrame->setLayout(hostFrameLayout);
    {
        QLabel* label = new QLabel(i18n("Name:"), hostFrame);
        hostFrameLayout->addWidget( label, 0, 0 );
        nameEntry = new QLineEdit( hostFrame );
        hostFrameLayout->addWidget( nameEntry, 0, 1 );
        label->setBuddy( nameEntry );
        connect( nameEntry, SIGNAL(textEdited(QString)), this, SLOT(editChanged()) );
    }
    {
        QLabel* label = new QLabel(i18n("Address:"), hostFrame);
        hostFrameLayout->addWidget( label, 1, 0 );
        addressEntry = new QLineEdit( hostFrame );
        addressEntry->setObjectName( "addressEntry" );
        addressEntry->setMinimumWidth(160);
        hostFrameLayout->addWidget( addressEntry, 1, 1 );
        label->setBuddy( addressEntry );
        connect( addressEntry, SIGNAL(textEdited(QString)), this, SLOT(editChanged()) );
    }
    {
        QLabel* label = new QLabel(i18n("GUI Port:"), hostFrame);
        hostFrameLayout->addWidget( label, 2, 0 );
        portEntry = new QSpinBox( hostFrame );
        portEntry->setRange(0, 65535);
        portEntry->setValue(4001);
        hostFrameLayout->addWidget( portEntry, 2, 1 );
        label->setBuddy( portEntry );
        connect( portEntry, SIGNAL(valueChanged(int)), this, SLOT(editChanged()) );
    }
    {
        QLabel* label = new QLabel(i18n("Username:"), hostFrame);
        hostFrameLayout->addWidget( label, 4, 0 );
        usernameEntry = new QLineEdit( hostFrame );
        hostFrameLayout->addWidget( usernameEntry, 4, 1 );
        label->setBuddy( usernameEntry );
        connect( usernameEntry, SIGNAL(textEdited(QString)), this, SLOT(editChanged()) );
    }
    {
        QLabel* label = new QLabel(i18n("Password:"), hostFrame);
        hostFrameLayout->addWidget( label, 5, 0 );
        passwordEntry = new QLineEdit( hostFrame );
        passwordEntry->setEchoMode( QLineEdit::Password );
        hostFrameLayout->addWidget( passwordEntry, 5, 1 );
        label->setBuddy( passwordEntry );
        connect( passwordEntry, SIGNAL(textEdited(QString)), this, SLOT(editChanged()) );
    }

    hostFrameLayout->setRowStretch(hostFrameLayout->count(), 1);
    resize(QSize(560, 360).expandedTo( minimumSizeHint() ));

    load();

    connect(hostList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(entrySelected()));
    connect(hostList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(currentChanged()));
    connect(donkey, SIGNAL(signalConnected()), this, SLOT(currentChanged()));
    connect(donkey, SIGNAL(signalDisconnected(int)), this, SLOT(currentChanged()));
    currentChanged();
}

HostDialog::~HostDialog()
{
}

void HostDialog::entrySelected()
{
    QListWidgetItem *item = hostList->currentItem();
    DonkeyHost* host = item ? item->data(HostInterfaceRole).value<DonkeyHost*>() : 0;
    nameEntry->setText(host ? host->name() : "");
    addressEntry->setText(host ? host->address() : "");
    portEntry->blockSignals(true);
    portEntry->setValue(host ? host->port() : 0);
    portEntry->blockSignals(false);
    usernameEntry->setText(host ? host->username() : "");
    passwordEntry->setText(host ? host->password() : "");
}

void HostDialog::currentChanged()
{
    QListWidgetItem *item = hostList->currentItem();
    DonkeyHost* host = item ? item->data(HostInterfaceRole).value<DonkeyHost*>() : 0;
    connectButton->setEnabled( host && (host != donkey->getHost() || ! donkey->isConnected()) );
    disconnectButton->setEnabled( host && host == donkey->getHost() && donkey->isConnected() );
    defaultButton->setEnabled( host && item != m_defaultitem );
    //newButton->setEnabled( true );
    deleteButton->setEnabled( item );
}

void HostDialog::connectButtonClicked()
{
    QListWidgetItem *item = hostList->currentItem();
    DonkeyHost* host = item ? item->data(HostInterfaceRole).value<DonkeyHost*>() : 0;
    if( host ) {
        donkey->setHost(host);
        donkey->connectToCore();
    }
}

void HostDialog::disconnectButtonClicked()
{
    QListWidgetItem *item = hostList->currentItem();
    DonkeyHost* host = item ? item->data(HostInterfaceRole).value<DonkeyHost*>() : 0;
    if( host && host == donkey->getHost() )
        donkey->disconnectFromCore();
}

void HostDialog::defaultButtonClicked()
{
    QListWidgetItem *item = hostList->currentItem();
    if (! item || item == m_defaultitem)
        return;
    if (m_defaultitem)
        m_defaultitem->setIcon(QIcon());
    m_defaultitem = item;
    m_defaultitem->setIcon(QIcon::fromTheme("kmldonkey"));
    /*
    setDefaultEntry(entry);
    //hostList->triggerUpdate(false);
    configChanged();
    */
}

void HostDialog::newButtonClicked()
{
    DonkeyHost* host = new DonkeyHost(i18n("New host"), "localhost", 4001, 4080, "admin", "");
    QVariant v;
    v.setValue(host);
    QListWidgetItem* item = new QListWidgetItem(hostList);
    item->setText(host->name());
    item->setData(HostInterfaceRole, v);
    hostList->setCurrentItem(item);
    editChanged();
    /*
    DonkeyEntry* entry = new DonkeyEntry(hostList, i18n( "New host" ), QString(), 4001, 4080, QString(), QString());
    if (!m_default) setDefaultEntry(entry);
    hostList->setSelected(entry, true);
    configChanged();
    */
}

void HostDialog::deleteButtonClicked()
{
    QListWidgetItem *item = hostList->currentItem();
    if( ! item )
        return;
    if( item == m_defaultitem )
        m_defaultitem = 0;
    delete item;
    //if (hostList->count() > 0) hostList->setCurrentItem( hostList->item(0) );
    //configChanged();
}

void HostDialog::editChanged()
{
    QListWidgetItem *item = hostList->currentItem();
    DonkeyHost* host = item ? item->data(HostInterfaceRole).value<DonkeyHost*>() : 0;
    if (! host ) return;
    host->setName( nameEntry->text() );
    item->setText( host->name() );
    host->setAddress( addressEntry->text() );
    host->setPort( portEntry->value() );
    host->setUsername( usernameEntry->text() );
    host->setPassword( passwordEntry->text() );
}

void HostDialog::save()
{
    qDebug()<<"HostDialog::save";
    KConfig *config = new KConfig("mldonkeyrc");

    foreach(const QString &s, config->groupList())
        config->group(s).deleteGroup();

    for(int i = 0; i < hostList->count(); ++i) {
        QListWidgetItem *item = hostList->item(i);
        Q_ASSERT(item);
        DonkeyHost* host = item->data(HostInterfaceRole).value<DonkeyHost*>();
        Q_ASSERT(host);
        KConfigGroup group = config->group( host->name() );
        group.writeEntry("DonkeyHost", host->address());
        group.writeEntry("DonkeyGuiPort", host->port());
        //group.writeEntry("DonkeyHTTPPort", host->httpport());
        group.writeEntry("DonkeyUsername", host->username());
        group.writeEntry("DonkeyPassword", host->password());
        //group.writeEntry("HostMode", entry->getRealHostMode());
        //group.writeEntry("BinaryPath", entry->getBinary());
        //group.writeEntry("RootPath", entry->getRoot());
        //group.writeEntry("StartupMode", entry->getStartupMode());
        group.writeEntry("Default", m_defaultitem == item);
        //if (group.hasKey("LocalHost")) group.deleteEntry("LocalHost");
    }

    config->sync();
    delete config;

    //if( m_defaultitem ) hostmanager->setDefaultHostName( m_defaultitem->name );
    //hostmanager->refreshHostList();
    //emit changed(true);
}

void HostDialog::load()
{
    qDebug()<<"HostDialog::load";
    m_defaultitem = 0;
    hostList->clear();
    foreach(const QString &n, hostmanager->hostList()) {
        DonkeyHost* host = dynamic_cast<DonkeyHost*>( hostmanager->hostProperties(n) );
        Q_ASSERT(host);
        QVariant v;
        v.setValue(host);
        QListWidgetItem* item = new QListWidgetItem(hostList);
        item->setText(host->name());
        item->setData(HostInterfaceRole, v);
        if(host->name() == hostmanager->defaultHostName())
            m_defaultitem = item;
    }
    if (m_defaultitem) {
        hostList->setCurrentItem(m_defaultitem);
        m_defaultitem->setIcon(QIcon::fromTheme("kmldonkey"));
    }
    entrySelected();
}

void HostDialog::showDialog(QWidget* parent, HostManager* hostmanager, DonkeyProtocol* donkey)
{
    PortedDialog dialog(parent);
    dialog.setCaption(i18n("Connections"));
    HostDialog* hostdialog = new HostDialog( dialog.mainWidget(), hostmanager, donkey );
    dialog.setMainWidget(hostdialog);
    connect(&dialog, SIGNAL(accepted()), hostdialog, SLOT(save()));
    dialog.exec();
    hostmanager->refreshHostList();
}

