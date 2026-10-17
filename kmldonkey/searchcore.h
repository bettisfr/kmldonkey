/* -*- c++ -*-
 *
 * searchcore.h
 *
 * Copyright (C) 2003-2004 Petter Stokke <ummo@hellokitty.com>
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

#ifndef __kmldonkey_searchcore_h__
#define __kmldonkey_searchcore_h__

#include <QMap>
#include "autobox.h"

#include <searchquery.h>

class KComboBox;
class KLineEdit;
class KConfig;
class QPushButton;
class QScrollArea;
class QSpinBox;

class SearchPage;
class SearchCoreWidget;

class KSharedConfig;
#include <KSharedConfig>

long int filesizeStr2Int(QString filesize);

class SearchCore : public VBoxWidget
{
    Q_OBJECT
public:
    SearchCore(QWidget *parent, SearchPage *page);

    QString activateDef;
    int searchNum;
    int maxHits;
    int searchType;
    QString searchNetwork;

    void restoreState(KSharedConfigPtr);
    void saveState(KSharedConfigPtr);
    void clear();
    void activateActions();
    void deactivateActions();

private:
    SearchPage *m_page;

    QScrollArea *m_formscroll;
    VBoxWidget *m_form;
    SearchCoreWidget* m_formwidget;
    QMap<QString, SearchQuery*> m_formqueries;
    KComboBox *m_formcombo;
    QSpinBox *maxhitsEdit;
    KComboBox *searchtypeCombo;
    KComboBox *searchnetCombo;
    QPushButton *startButton;

    void clearFormWidget();
    void clearFormQueries();

protected slots:
    void setNetworks(int);
    void startSearch();
    void updatePages();
    void activateFormCombo(int);
};

#endif // __kmldonkey_searchcore_h__
