/* -*- c++ -*-
 *
 * fontselector.cpp
 *
 * Copyright (C) 2003-2004 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003-2004 Sebastian Sauer <mail@dipe.org>
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

#include "fontselector.h"

#include <QHBoxLayout>
#include <KSharedConfig>
#include <KLocalizedString>
#include <QDebug>
#include <QFontDialog>
#include <ksqueezedtextlabel.h>
#include <QPushButton>

FontSelector::FontSelector(QWidget *parent, const char *name, bool fixed)
    : QFrame(parent)
{
    setObjectName(name ? name : "FontSelector");
    QHBoxLayout* layout = new QHBoxLayout(this);
    setLayout(layout);

    wantFixed = fixed;

    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    preview = new KSqueezedTextLabel(this);
    layout->addWidget(preview, 2);
    preview->setFrameShape(QFrame::StyledPanel);
    preview->setFrameShadow(QFrame::Sunken);
	
	QSizePolicy sizePol(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sizePol.setHorizontalStretch( 0 );
	sizePol.setVerticalStretch( 0 );
	sizePol.setHeightForWidth( preview->sizePolicy().hasHeightForWidth() );
    preview->setSizePolicy( sizePol );

    chooseButton = new QPushButton(i18n("Choose..."), this);
    layout->addWidget(chooseButton);
    chooseButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    //setSizePolicy(QSizePolicy::Expanding,  QSizePolicy::Fixed);
    connect(chooseButton, SIGNAL(clicked()), this, SLOT(selectFont()));
}

void FontSelector::selectFont()
{
    QFont font = ft;
    bool accepted=false;
    font=QFontDialog::getFont(&accepted,ft,this,QString(),wantFixed?QFontDialog::MonospacedFonts:QFontDialog::FontDialogOptions());
    if(accepted) {
        setFont(font);
        emit fontSelected(font);
    }
}

QFont& FontSelector::font()
{
    return ft;
}

void FontSelector::setFont(QFont& font)
{
    ft = font;
    preview->setFont(font);
    preview->setText(font.family() + " " + QString::number(font.pointSize()));
    setMinimumSize(sizeHint());
}

