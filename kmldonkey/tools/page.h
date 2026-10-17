/* -*- c++ -*-
 *
 * page.h
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

#ifndef __kmldonkey_page_h__
#define __kmldonkey_page_h__

#include <QStringList>
#include <kconfig.h>
#include <ksharedconfig.h>

class KActionCollection;
class QFont;
class KMLDonkeyPreferences;


/**
 * The interface definition for a KMLDonkey page, containing only virtual
 * functions that any object providing this interface should redefine.
 * They are not pure virtual functions, but instead provide default
 * implementations that do nothing, as it is expected that not all of these
 * methods will be needed by all pages.
 */
class KMLDonkeyPage
{
public:
    virtual ~KMLDonkeyPage() {}

    /**
     * This method is passed KMLDonkey's KActionCollection, to which the page
     * should add its actions.
     */
    virtual void setupActions(KActionCollection*) {}

    /**
     * This method is called when the prefs dialog is about to be opened.
     * If the page needs to load some values into it, this is where it
     * should do it.
     */
    virtual void configurePrefsDialog(KMLDonkeyPreferences*) {}

    /**
     * This method is called when the global prefs have changed, and the
     * page needs to update its state. If prefs is non-null, the prefs have
     * just changed because the prefs dialog was applied, and if there is
     * any local state that needs to be fetched from it (ie. anything that
     * was loaded into it in configurePrefsDialog() above) this should be
     * done first of all.
     */
    virtual void applyPreferences(KMLDonkeyPreferences* = 0) {}

    /**
     * Called when KMLDonkey wants the page to dump its state. It is passed
     * the KConfig object into which it should do this.
     */
    virtual void saveState(KSharedConfigPtr) {}

    /**
     * KMLDonkey passed a KConfig object to this function, from which it
     * expects the page to restore its state. It is implied that it should
     * also act upon this state - this is usually done through a simple
     * call to applyPreferences(0) above.
     */
    virtual void restoreState(KSharedConfigPtr) { applyPreferences(); }

    /**
     * This is called when the core disconnects. The page should unload all
     * its connection specific data in this method.
     */
    virtual void clear() {}

    /**
     * Return a list of actions supported by this page. This is only called when a page
     * becomes visible, so it can also be used to activate page specific actions.
     */
    virtual QStringList supportedGenericActions() { return QStringList(); }

    /**
     * Handle a generic action.
     */
    virtual void handleGenericAction(const QString&) {}

    /**
     * Connect a signal to the specified slot that should be emitted when the list of supported actions changes.
     * The signal should contain a pointer to this KMLDonkeyPage as its only argument.
     *
     * There is no need to implement this method if the list of supported actions never changes.
     */
    virtual void plugGenericActions(QObject*, const char*) {}

    /**
     * Called when a page is no longer visible. This should deactivate any actions that
     * depend on the page being visible.
     */
    virtual void deactivatePageActions() {}
};

#endif
