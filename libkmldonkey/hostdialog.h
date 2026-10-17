/* -*- c++ -*-
 *
 * hostdialog.h
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

#ifndef HOSTDIALOG_H
#define HOSTDIALOG_H

#include <QWidget>
#include <kmldonkey_export.h>

class QLineEdit;
class QSpinBox;
class QListWidget;
class QPushButton;
class QListWidgetItem;

class HostManager;
class HostDialogItem;
class DonkeyProtocol;

class KMLDONKEY_EXPORT HostDialog : public QWidget
{
        Q_OBJECT
    public:
        explicit HostDialog(QWidget* parent, HostManager* hostmanager, DonkeyProtocol* donkey);
        virtual ~HostDialog();

        static void showDialog(QWidget* parent, HostManager* hostmanager, DonkeyProtocol* donkey);

        HostManager* hostmanager;
        DonkeyProtocol* donkey;
        QListWidget* hostList;
        QLineEdit* nameEntry;
        QLineEdit* addressEntry;
        QLineEdit* usernameEntry;
        QLineEdit* passwordEntry;
        QSpinBox* portEntry;

    public slots:
        virtual void save();
        virtual void load();

    protected slots:
        //virtual void configChanged();
        void entrySelected();
        void currentChanged();
        void connectButtonClicked();
        void disconnectButtonClicked();
        void defaultButtonClicked();
        void newButtonClicked();
        void deleteButtonClicked();
        void editChanged();

    private:
        QListWidgetItem* m_defaultitem;
        QPushButton *connectButton, *disconnectButton, *defaultButton, *newButton, *deleteButton;
};

#endif // DONKEYCONFIG_H
