/* -*- c++ -*-
 *
 * search.h
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

#ifndef __kmldonkey_search_h__
#define __kmldonkey_search_h__

#include "page.h"
#include "clipboard.h"
#include "searchcore.h"
#include "searchtabresult.h"

#include <QHash>
#include <QStringList>

#include "autobox.h"
#include <KCoreAddons>

class HBoxWidget;
class QSplitter;
class QTreeWidget;
class KComboBox;
class KLineEdit;
class QTabWidget;

class SearchPage : public VBoxWidget, public KMLDonkeyPage, public ClipboardHelper
{
    Q_OBJECT
public:
    SearchPage(QWidget *parent = 0);
    virtual ~SearchPage();

    virtual void setupActions(KActionCollection*);
    virtual QStringList supportedGenericActions();
    virtual void handleGenericAction(const QString&);
    virtual void plugGenericActions(QObject*, const char*);
    virtual void deactivatePageActions();

    virtual void configurePrefsDialog(KMLDonkeyPreferences*);
    virtual void applyPreferences(KMLDonkeyPreferences*);
    virtual void saveState(KSharedConfigPtr);
    virtual void restoreState(KSharedConfigPtr);
    virtual void clear();

    // Add a SearchResult.
    SearchResult* addSearchResult(int searchNum, SearchQuery* query, const QString& tablabel);

    // Don't call this function! It's only used to clean up
    // previous searches at KMLDonkey::close()!
    void stopAllSearches();
    // Stop a search and remove the searchresulttab.
    void stopSearch(int);

private:
    QTabWidget *resultTabs;
    QSplitter *splitter;
    int restoredSidebarWidth = 340;
    bool sidebarRestorePending = true;

    SearchCore *searchcore;

    HBoxWidget* ed2kBox;
    KLineEdit* ed2kEntry;

    QHash<int, SearchResult *> Results;

    bool activateNewTabs;
    bool closeTabsOnDisconnect;
    bool showNumbersOnTabs;
    bool showSubmitURL;

    QString getTabLabel(SearchResult *tab);

signals:
    void genericActionsChanged(KMLDonkeyPage*);

protected:
    void showEvent(QShowEvent *event) override;
    void copySearchToClipboard(ClipFormat format);

protected slots:
    void closeSearch(QWidget*);
    void currentTabChanged(QWidget*);
    void searchUpdated(int, QList<ResultInfo*> resultInfos);
    void closeAllSearches();
    void pleaseUpdateActions();
    void submitED2K();
    void tabContextMenu( const QPoint& );

public slots:
    void actionDownload();
    void actionDownloadAs();
    void actionForceDownload();

    void actionActivatePage();

    void closeCurrentSearch();
};

#endif // __kmldonkey_search_h__
