/* -*- c++ -*-
 *
 * prefs.cpp
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

#include <KSharedConfig>
#include <kconfig.h>
#include <KLocalizedString>
#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QToolTip>
#include <QGroupBox>
#include <qscrollarea.h>
#include <QCheckBox>
#include <kcombobox.h>
#include <QPushButton>
#include <QFontDialog>
#include <kfontrequester.h>
#include <kiconloader.h>
#include <QSpinBox>
#include <kcharsets.h>
#include <kmessagebox.h>
#include <KStandardGuiItem>

#include "prefs.h"


ColorSelector::ColorSelector(QWidget* parent, const char* name, const QString caption, const QString tooltip)
    : QWidget(parent)
{
    setObjectName(name);
    if( parent->layout() )
        parent->layout()->addWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    QLabel* label = new QLabel(caption, this);
    layout->addWidget(label);
    button = new KColorButton( this );
    layout->addWidget(button);
    button->setObjectName( "colorButton" );
    label->setBuddy(button);
    this->setToolTip(tooltip);
}

const QColor ColorSelector::color()
{
    return button->color();
}

void ColorSelector::setColor(const QColor &color)
{
    button->setColor(color);
}



KMLDonkeyPrefRenamerPage::KMLDonkeyPrefRenamerPage(QWidget *parent)
    : QFrame(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    enableAutorenameCheckbox = new QCheckBox(i18n("&Enable automatic renaming of downloading files"), this);
    enableAutorenameCheckbox->setToolTip(i18n("Should downloading files be automatically renamed according to the search and replace list?"));
    connect(enableAutorenameCheckbox, SIGNAL(toggled(bool)), this, SLOT(enableAutorename(bool)));
    layout->addWidget(enableAutorenameCheckbox);

    renlisteditor = new AutoRenamerListEditor(this);
    layout->addWidget(renlisteditor, 1);
}

void KMLDonkeyPrefRenamerPage::enableAutorename(bool on)
{
    renlisteditor->setEnabled(on);
}



KMLDonkeyPrefColorPage::KMLDonkeyPrefColorPage(QWidget *parent)
    : QFrame(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    coloredListsCheckbox = new QCheckBox(this);
    layout->addWidget(coloredListsCheckbox);
    coloredListsCheckbox->setText(i18n("&Colored list"));
    coloredListsCheckbox->setToolTip(i18n("Colorize items in the lists according to their status"));
    connect(coloredListsCheckbox, SIGNAL(toggled(bool)), this, SLOT(coloredLists(bool)));

    QScrollArea *scroll = new QScrollArea(this);
    layout->addWidget(scroll);
    QWidget *box = new QWidget(scroll);
    QVBoxLayout* boxlayout = new QVBoxLayout(box);
    box->setLayout(boxlayout);
    //scroll->setAlignment(Qt::AlignLeft|Qt::AlignTop);
    //scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    //scroll->setMinimumSize(scroll->sizeHint());

    QVBoxLayout *grplayout;

    // Server
    grpServer = new QGroupBox(i18n("Server"), box);
    boxlayout->addWidget(grpServer);
    grplayout = new QVBoxLayout(grpServer);
    grpServer->setLayout(grplayout);
    colorServerNotConnected = new ColorSelector(grpServer, "colorNotConnected", i18n("&Not connected:"), i18n("Color for disconnected servers") );
    colorServerBlacklisted = new ColorSelector(grpServer, "colorBlacklisted", i18n("&Blacklisted:"), i18n("Color for blacklisted servers") );
    colorServerConnecting = new ColorSelector(grpServer, "colorConnecting", i18n("Connectin&g:"), i18n("Color for connecting or initiating servers") );
    colorServerConnected = new ColorSelector(grpServer, "colorConnected", i18n("C&onnected:"), i18n("Color for connected servers") );

    // Search
    grpSearch = new QGroupBox(i18n("Search"), box);
    boxlayout->addWidget(grpSearch);
    grplayout = new QVBoxLayout(grpSearch);
    grpSearch->setLayout(grplayout);

    colorSearchFewSources = new ColorSelector(grpSearch, "colorFewSources", i18n("&Few sources:"), i18n("The color indicating few sources") );
    colorSearchManySources = new ColorSelector(grpSearch, "colorManySources", i18n("&Many sources:"), i18n("The color indicating many sources") );
    colorSearchAlreadyDone = new ColorSelector(grpSearch, "colorAlreadydone", i18n("&Already done:"), i18n("Color for already downloaded sources") );

    QWidget *srtBox = new QWidget(grpSearch);
    grplayout->addWidget( srtBox );
    QHBoxLayout* srtlayout = new QHBoxLayout(srtBox);
    srtBox->setLayout(srtlayout);
    srtlayout->addWidget(new QLabel(i18n("Threshold:"), srtBox));

    searchSourcesThresholdSlider = new QSpinBox(srtBox);
    srtlayout->addWidget(searchSourcesThresholdSlider);
    searchSourcesThresholdSlider->setRange(2, 100);

    searchSourcesThresholdSlider->setToolTip(i18n("The number of sources that will produce the full 'many sources' color"));

    // Downloaded
    grpDown = new QGroupBox(i18n("Downloaded"), box);
    boxlayout->addWidget(grpDown);
    grplayout = new QVBoxLayout(grpDown);
    grpDown->setLayout(grplayout);
    downloadingColorSelect = new ColorSelector(grpDown, "colorDownloading", i18n("&Downloading:"), i18n("Color for active downloads (receiving data)") );
    lookingColorSelect = new ColorSelector(grpDown, "colorLooking", i18n("&Looking:"), i18n("Color for inactive downloads (looking for sources)") );
    notavailableColorSelect = new ColorSelector(grpDown, "colorNotAvailable", i18n("Not &available:"), i18n("Color for downloads that are not available") );
    queuedColorSelect = new ColorSelector(grpDown, "colorQueued", i18n("&Queued:"), i18n("Color for queued downloads") );
    pausedColorSelect = new ColorSelector(grpDown, "colorPaused", i18n("&Paused:"), i18n("Color for paused downloads") );

    // Availability
    grpAvail = new QGroupBox(i18n("Availability"), box);
    boxlayout->addWidget(grpAvail);
    grplayout = new QVBoxLayout(grpAvail);
    grpAvail->setLayout(grplayout);

    fewSourcesColorSelect = new ColorSelector(grpAvail, "colorFewSources", i18n("&Few sources:"), i18n("The color indicating few sources") );
    manySourcesColorSelect = new ColorSelector(grpAvail, "colorManySources", i18n("&Many sources:"), i18n("The color indicating many sources") );
    completeColorSelect = new ColorSelector(grpAvail, "colorComplete", i18n("&Complete:"), i18n("The color indicating a complete chunk") );
    noSourcesColorSelect = new ColorSelector(grpAvail, "colorNoSources", i18n("&No sources:"), i18n("The color indicating no sources found"));

    QWidget *avtBox = new QWidget(grpAvail);
    grplayout->addWidget( avtBox );
    QHBoxLayout* avtlayout = new QHBoxLayout(avtBox);
    avtBox->setLayout(avtlayout);
    avtlayout->addWidget(new QLabel(i18n("Threshold:"), avtBox));
    availabilityThresholdSlider = new QSpinBox(avtBox);
    avtlayout->addWidget(availabilityThresholdSlider);
    availabilityThresholdSlider->setRange(2, 100);

    availabilityThresholdSlider->setToolTip(i18n("The number of sources that will produce the full 'many sources' color"));

    availabilityShadingCheckbox = new QCheckBox(grpAvail);
    grplayout->addWidget( availabilityShadingCheckbox );
    availabilityShadingCheckbox->setText(i18n("Shaded availability display"));
    availabilityShadingCheckbox->setToolTip(i18n("Draw a fancier looking shaded availability display bar."));

    QWidget *shdBox = new QWidget(grpAvail);
    grplayout->addWidget( shdBox );
    QHBoxLayout* shdlayout = new QHBoxLayout(shdBox);
    shdBox->setLayout(shdlayout);
    shdlayout->addWidget(new QLabel(i18n("Shading depth:"), shdBox));
    availabilityShadingSlider = new QSpinBox(shdBox);
    shdlayout->addWidget(availabilityShadingSlider);
    availabilityShadingSlider->setRange(100, 400);

    availabilityShadingSlider->setToolTip(i18n("The depth of the availability bar shading - more is darker."));
    connect(availabilityShadingCheckbox, SIGNAL(toggled(bool)), availabilityShadingSlider, SLOT(setEnabled(bool)));

    // Sources
    grpSources = new QGroupBox(i18n("Sources"), box);
    boxlayout->addWidget(grpSources);
    grplayout = new QVBoxLayout(grpSources);
    grpSources->setLayout(grplayout);

    colorSourceNotConnected = new ColorSelector(grpSources, "colorNotConnected", i18n("&Not connected:"), i18n("Color for disconnected sources") );
    colorSourceBlacklisted = new ColorSelector(grpSources, "colorBlacklisted", i18n("&Blacklisted:"), i18n("Color for blacklisted sources") );
    colorSourceConnecting = new ColorSelector(grpSources, "colorConnecting", i18n("Connectin&g:"), i18n("Color for connecting or initiating sources") );
    colorSourceQueued = new ColorSelector(grpSources, "colorQueued", i18n("&Queued:"), i18n("Color for queued sources") );
    colorSourceDownloading = new ColorSelector(grpSources, "colorDownloading", i18n("&Downloading:"), i18n("Color for sources being downloaded from") );

    scroll->setWidget(box);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setMinimumSize(sizeHint());
}

void KMLDonkeyPrefColorPage::coloredLists(bool on)
{
    grpServer->setEnabled(on);
    grpSearch->setEnabled(on);
    grpDown->setEnabled(on);
    grpSources->setEnabled(on);
}

KMLDonkeyPrefFontPage::KMLDonkeyPrefFontPage(QWidget *parent)
    : QFrame(parent)
{
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);

    layout->addWidget(new QLabel(i18n("Console font:"), this), 0, 0);
    consoleFontSelect = new FontSelector(this, "consoleFontSelect", true);
    consoleFontSelect->setToolTip(i18n("Font used for the console"));
    layout->addWidget(consoleFontSelect, 0, 1);

    layout->addWidget(new QLabel(i18n("List font:"), this), 1, 0);
    listFontSelect = new FontSelector(this, "listFontSelect");
    listFontSelect->setToolTip(i18n("Font used for the lists"));
    layout->addWidget(listFontSelect, 1, 1);

    layout->setColumnStretch(1, 2);
    setMinimumSize(sizeHint());
}

KMLDonkeyPreferences::KMLDonkeyPreferences(QWidget* parent)
    : PortedPageDialog(parent)
{
    setCaption(i18n("KMLDonkey Preferences"));
    setFaceType(PortedPageDialog::List);
    setButtons(PortedDialog::Help|PortedDialog::Ok|PortedDialog::Apply|PortedDialog::Cancel);
    //setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );
    //setMinimumSize( QSize(600,480) );

    {
        QWidget* widget = new QWidget(mainWidget());
        KPageWidgetItem* item = addPage(widget, i18n("General"));
        item->setIcon(QIcon::fromTheme("preferences-other"));
        item->setHeader(i18n("General Configuration"));
        generalPage = new Ui::KMLDonkeyPrefGeneralPage();
        generalPage->setupUi(widget);
        generalPage->encodingSelector->insertItems(0, KCharsets::charsets()->descriptiveEncodingNames());
        generalPage->pageViewModeComboBox->setItemIcon(0, QIcon::fromTheme("view-list-icons"));
        generalPage->pageViewModeComboBox->setItemIcon(1, QIcon::fromTheme("view-list-tree"));
        generalPage->pageViewModeComboBox->setItemIcon(2, QIcon::fromTheme("tab-duplicate"));
    }

    {
        QWidget* widget = new QWidget(mainWidget());
        KPageWidgetItem* item = addPage(widget, i18n("Lists"));
        item->setIcon(QIcon::fromTheme("view-list-text"));
        item->setHeader(i18n("List Configuration"));
        listsPage = new Ui::KMLDonkeyPrefListsPage();
        listsPage->setupUi(widget);
    }

    {
        QWidget* widget = new QWidget(mainWidget());
        KPageWidgetItem* item = addPage(widget, i18n("Search"));
        item->setIcon(QIcon::fromTheme("system-search"));
        item->setHeader(i18n("Search Configuration"));
        searchPage = new Ui::KMLDonkeyPrefSearchPage();
        searchPage->setupUi(widget);
    }

    {
        renamerPage = new KMLDonkeyPrefRenamerPage(mainWidget());
        KPageWidgetItem* item = addPage(renamerPage, i18n("Autorename"));
        item->setIcon(QIcon::fromTheme("document-save-as"));
        item->setHeader(i18n("Autorename Configuration"));
    }

    {
        colorPage = new KMLDonkeyPrefColorPage(mainWidget());
        KPageWidgetItem* item = addPage(colorPage, i18n("Colors"));
        item->setIcon(QIcon::fromTheme("format-fill-color"));
        item->setHeader(i18n("Color Configuration"));
    }

    {
        QWidget* widget = new QWidget(this);
        KPageWidgetItem* item = addPage(widget, i18n("Fonts"));
        item->setIcon(QIcon::fromTheme("preferences-desktop-font"));
        item->setHeader(i18n("Font Configuration"));
        fontPage = new KMLDonkeyPrefFontPage(widget);
        auto *layout = new QVBoxLayout(widget);
        layout->addWidget(fontPage);
    }

    //generalPage->initPage();
    //setInitialSize(configDialogSize("PrefsDialog"));
    connect(this, SIGNAL(finished(int)), this, SLOT(saveSize()));
}

void KMLDonkeyPreferences::saveSize()
{
    //saveDialogSize("PrefsDialog");
}

