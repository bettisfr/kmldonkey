/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KMLDONKEY_EXPORT_H
#define KMLDONKEY_EXPORT_H

#ifndef LIBKMLDONKEY_NO_KDE

/* needed for Q_DECL_EXPORT and Q_DECL_IMPORT macros */
#include <QtGlobal>

#ifndef KMLDONKEY_EXPORT
# if defined(MAKE_LIBKMLDONKEY_LIB)
   /* We are building this library */ 
#  define KMLDONKEY_EXPORT Q_DECL_EXPORT
# else
   /* We are using this library */ 
#  define KMLDONKEY_EXPORT Q_DECL_IMPORT
# endif
#endif

# ifndef KMLDONKEY_EXPORT_DEPRECATED
#  define KMLDONKEY_EXPORT_DEPRECATED Q_DECL_DEPRECATED KMLDONKEY_EXPORT
# endif

#else // LIBKMLDONKEY_NO_KDE

#define KMLDONKEY_EXPORT
#define KMLDONKEY_EXPORT_DEPRECATED

#endif // LIBKMLDONKEY_NO_KDE

#endif // KMLDONKEY_EXPORT_H
