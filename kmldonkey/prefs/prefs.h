/* -*- c++ -*-
 *
 * prefs.h
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
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

#ifndef __kmldonkey_prefs_h__
#define __kmldonkey_prefs_h__

#include <qwidget.h>
#include <QTabWidget>
#include "portedpagedialog.h"
#include <kcolorbutton.h>

#include "fontselector.h"
#include "listeditor.h"

#include "ui_generalpage.h"
#include "ui_listspage.h"
#include "ui_searchpage.h"

class QCheckBox;
class QPushButton;
class QSpinBox;
class KDoubleNumInput;
class KComboBox;

class ColorSelector : public QWidget
{
public:
    ColorSelector(QWidget* parent, const char* name, const QString caption, const QString tooltip);
    const QColor color();
    void setColor(const QColor &color);
private:
    KColorButton* button;
};

class KMLDonkeyPrefRenamerPage : public QFrame
{
    Q_OBJECT
public:
    KMLDonkeyPrefRenamerPage(QWidget *parent);

    QCheckBox *enableAutorenameCheckbox;
    AutoRenamerListEditor *renlisteditor;

public slots:
    void enableAutorename(bool);
};

class KMLDonkeyPrefColorPage : public QFrame
{
    Q_OBJECT
public:
    KMLDonkeyPrefColorPage(QWidget *parent = 0);

    QCheckBox *coloredListsCheckbox,
	      *availabilityShadingCheckbox;

    ColorSelector *colorServerNotConnected,
                  *colorServerBlacklisted,
                  *colorServerConnecting,
                  *colorServerConnected;

    ColorSelector *colorSearchFewSources,
                  *colorSearchManySources,
                  *colorSearchAlreadyDone;
    QSpinBox *searchSourcesThresholdSlider,
	         *availabilityShadingSlider;

    //QCheckBox *colorDownloadCheckbox;
    ColorSelector *downloadingColorSelect,
                  *pausedColorSelect,
                  *lookingColorSelect,
                  *notavailableColorSelect,
                  *queuedColorSelect;

    ColorSelector *noSourcesColorSelect,
                  *fewSourcesColorSelect,
                  *manySourcesColorSelect,
                  *completeColorSelect;
    QSpinBox* availabilityThresholdSlider;

    ColorSelector *colorSourceNotConnected,
                  *colorSourceBlacklisted,
                  *colorSourceConnecting,
                  *colorSourceQueued,
                  *colorSourceDownloading;

private:
    QGroupBox *grpServer, *grpSearch, *grpDown, *grpAvail, *grpSources;

public slots:
    void coloredLists(bool);
};

class KMLDonkeyPrefFontPage : public QFrame
{
public:
    KMLDonkeyPrefFontPage(QWidget *parent = 0);

    FontSelector *consoleFontSelect,
                 *listFontSelect;
};

class KMLDonkeyPreferences : public PortedPageDialog
{
    Q_OBJECT
public:
    KMLDonkeyPreferences(QWidget* parent = 0);

    Ui_KMLDonkeyPrefGeneralPage* generalPage;
    Ui_KMLDonkeyPrefListsPage* listsPage;
    Ui_KMLDonkeyPrefSearchPage* searchPage;
    KMLDonkeyPrefRenamerPage* renamerPage;
    KMLDonkeyPrefColorPage* colorPage;
    KMLDonkeyPrefFontPage* fontPage;

protected slots:
    void saveSize();
};

#endif // __kmldonkey_prefs_h__
