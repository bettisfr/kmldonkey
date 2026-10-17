/* -*- c++ -*-
 *
 * mlconfig.h
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

#ifndef __kmldonkey_mlconfig_h__
#define __kmldonkey_mlconfig_h__

#include <QHash>
#include <functional>
#include "portedpagedialog.h"

#include "optioneditor.h"

class QCheckBox;
class QLineEdit;
class QLabel;

class DownloadPathsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DownloadPathsWidget(DonkeyProtocol* core, QWidget* parent = nullptr);
    QString validationError() const;
    void apply(std::function<void(bool)> finished);
    void addAdvancedOptions(QWidget* options);
signals:
    void pathsChanged();
private:
    struct Share { int priority; QString path; QString strategy; };
    static QList<Share> parseShares(const QString& reply);
    void load();
    void checkFolders(QStringList paths, std::function<void(bool, const QString&)> finished);
    DonkeyProtocol* m_core;
    QLineEdit *m_completed, *m_files, *m_directories, *m_temp;
    QWidget* m_advanced;
    QLabel* m_status;
    QList<Share> m_shares;
    QString m_originalFiles, m_originalDirectories, m_originalTemp;
    bool m_ready = false;
};

class NetworkPage : public QWidget
{
        Q_OBJECT
    public:
        NetworkPage(QWidget* parent = 0, const char* name = 0);
        void applyOptions(DonkeyProtocol* target);
    protected slots:
        void becomeDirty(bool);
    signals:
        void pageDirty();
    private:
        QHash<int, QCheckBox *> nwBox;
};

class MLDonkeyConfigDialog : public PortedPageDialog
{
        Q_OBJECT
    public:
        MLDonkeyConfigDialog(QWidget* parent = 0, const char* name = 0);
    public slots:
        void accept() override;
    protected slots:
        void setDirtyDialog();
        void applyOptions();
        void closeDialog();
    private:
        void saveOptions(bool closeWhenSaved);
        DownloadPathsWidget* m_paths;
        KPageWidgetItem* m_pathsItem = nullptr;
        bool m_saving = false;
        bool m_dirty = false;
        bool m_closeAfterSave = false;
        QHash<QString, OptionEditorWidget*> pages;
        NetworkPage* m_networkPage;
};

#endif // __kmldonkey_mlconfig_h__
