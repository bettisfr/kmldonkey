#include <QLocale>
/* -*- c++ -*-
 *
 * availability.cpp
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003,2004,2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2009 Aleksey Markelov <markelovai@gmail.com>
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

#include "availability.h"
#include "kmldonkey.h"
#include <clientinfo.h>
#include "network.h"

#include <KSharedConfig>
#include <kconfig.h>
#include <KLocalizedString>
#include <QDebug>
#include <QPainter>
#include <QPixmapCache>
#include <QImage>
#include <QPaintEvent>
#include <QApplication>


inline QColor blend_availability(int avail, int threshold)
{
    if (!avail) return KMLDonkey::App->colorNoSources;
    --avail;
    if (avail >= threshold) return KMLDonkey::App->colorManySources;

    QColor dark = KMLDonkey::App->colorFewSources;
    QColor light = KMLDonkey::App->colorManySources;

    int rdiff = light.red() - dark.red(),
        gdiff = light.green() - dark.green(),
        bdiff = light.blue() - dark.blue();
    return QColor(dark.red() + (rdiff * avail) / threshold,
            dark.green() + (gdiff * avail) / threshold,
            dark.blue() + (bdiff * avail) / threshold);
}

QPixmap AvailabilityRenderer::paint(const FileInfo *fi, const ClientInfo *ci, QString *cacheKey)
{
    static const int pmHeight = 40;
    int chunks = fi->fileChunks().size();
    QPixmap pm(chunks ? chunks : pmHeight, pmHeight);
    if (!chunks) {
        pm.fill(Qt::transparent);
        if (cacheKey) {
            *cacheKey = QString();
        }
        return pm;
    }

    QByteArray availability;
    if (KMLDonkey::App->donkey->findNetworkNo(fi->fileNetwork())->networkName() == "Donkey") {
        availability = fi->fileAvailability().value(fi->fileNetwork());
    } else {//other networks report '0' availability, fill with 0x01
        availability = QByteArray(chunks, '\1');
    }

    QByteArray download;
    if (ci) {
        const QHash<int, QByteArray> &cc = fi->fileSources();
        download = cc.value(ci->clientNo());
        if (download.isNull()) download = QByteArray(chunks, '0');
    } else {
        download = fi->fileChunks();
    }
    if (download.size() < availability.size()) {
        download += QByteArray(availability.size() - download.size(), '0');
    }
    Q_ASSERT(download.size() == availability.size());

    int threshold = KMLDonkey::App->availabilityThreshold;
    bool shading = KMLDonkey::App->availabilityShading;
    int shadingDepth = KMLDonkey::App->availabilityShadingDepth;

    QString key = QString("avail:%1:%2:%3:%4")
        .arg(QString(availability.toHex()))
        .arg(QString(download))
        .arg(threshold)
        .arg(shading ? shadingDepth : 0);
    if (cacheKey) {
        *cacheKey = key;
    }
    if (QPixmapCache::find(key, &pm)) return pm;

    QPainter p(&pm);
    QColor top, bottom;
    //TODO:here we can combine chunks that should be painted with same colors
    for (int i = 0; i < chunks; ++i) {
        top = blend_availability(availability.at(i), threshold);
        char dv = download.at(i);
        if (QChar(dv).digitValue() > 0 && dv != '0') {
            bottom = KMLDonkey::App->colorComplete;
            if (dv != '1' || ci) {
                top = bottom;
            }
        } else {
            bottom = top;
        }
        p.setPen(top);
        if (top == bottom) {
            p.drawLine(i, 0, i, pmHeight);
        } else {
            p.drawLine(i, 0, i, pmHeight/2);
            p.setPen(bottom);
            p.drawLine(i, pmHeight/2, i, pmHeight);
        }
    }

    if (shading) {
        QColor c(0,0,0,0);
        //XXX: use kconfig_update to reduce shadingDepth for lower versions?
        QColor d(0,0,0,shadingDepth);
        QLinearGradient gradient(0, 0, 0, pm.height());
        gradient.setColorAt(0.0, d);
        gradient.setColorAt(0.5, c);
        gradient.setColorAt(1.0, d);
        p.setPen(Qt::NoPen);
        p.setBrush(gradient);
        p.drawRect(pm.rect());
    }
    QPixmapCache::insert(key, pm);
    return pm;
}

AvailabilityRenderer::AvailabilityRenderer(int file)
{
    rfileno = file;
    updateFileInfo();
    isClientRenderer = false;
}

AvailabilityRenderer::AvailabilityRenderer(int file, int source)
{
    rfileno = file;
    clno = source;
    updateFileInfo();
    isClientRenderer = true;
}

void AvailabilityRenderer::updateFileInfo()
{
    FileInfo* fi = fileInfo();
    chunkNo = 0;
    if (fi) chunkNo = fi->fileChunks().size();
}

FileInfo* AvailabilityRenderer::fileInfo()
{
    return KMLDonkey::App->donkey->findDownloadFileNo(rfileno);
}

int AvailabilityRenderer::chunks() const
{
    return chunkNo;
}

void AvailabilityRenderer::paintAvailability(QPainter& p, const QRect& g)
{
    if (m_cache.isNull() || m_cache.size() != g.size()) {
        ClientInfo *ci = isClientRenderer ? KMLDonkey::App->donkey->findClientNo(clno) : 0;
        FileInfo *fi = fileInfo();
        m_cache = AvailabilityRenderer::paint(fi,ci).scaled(g.size());
    }
    p.drawPixmap(g, m_cache);
}

void AvailabilityRenderer::loseCache()
{
    m_cache = QPixmap();
}



AvailabilityWidget::AvailabilityWidget(int file, QWidget* parent, const char* name, bool frame)
    : QFrame(parent)
    , AvailabilityRenderer(file)
{
    setObjectName(name);
    if (frame)
        setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setMinimumSize(minimumSizeHint());
}

void AvailabilityWidget::paintEvent(QPaintEvent* e)
{
    QFrame::paintEvent(e);
    QPainter p(this);
    QRect g = contentsRect();
    p.setClipRect(g);
    paintAvailability(p, g);
    FileInfo* fi = fileInfo();
    QString t;
    if (fi)
        t = i18nc("percentage", "%1%", QLocale().toString(double(((float)fi->fileDownloaded() * 100.0) / (float)fi->fileSize()), 'f', 2) );
    else
        t = i18n("Unknown file.");
    QFont f = font();
    f.setBold(true);
    p.setFont(f);
    p.setPen(Qt::black);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            QRect foo = rect();
            foo.translate(x, y);
            p.drawText(foo, Qt::AlignCenter, t);
        }
    }
    p.setPen(Qt::white);
    p.drawText(rect(), Qt::AlignCenter, t);
}

QSizePolicy AvailabilityWidget::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

QSize AvailabilityWidget::minimumSizeHint() const
{
    return QSize((chunkNo > 300) ? 300 : chunkNo, QFontMetrics(font()).height() + 4 + (frameWidth() * 2));
}

void AvailabilityDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, 
        const QModelIndex &index) const
{
    QPixmap pm = index.data(Qt::UserRole + 2).value<QPixmap>();
    QString key = index.data(Qt::UserRole + 3).toString();
    Q_ASSERT(!pm.isNull());

    const QStyleOptionViewItem *optV3 = qstyleoption_cast<const QStyleOptionViewItem*>(&option);
    QStyle *style = optV3 && optV3->widget ? optV3->widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    bool selected = option.state & QStyle::State_Selected;
    bool hover = option.state & QStyle::State_MouseOver;

    QString cacheKey = QString("%1:%2:%3").arg(key).arg(option.rect.width()).arg(option.rect.height());
    if (!QPixmapCache::find(cacheKey, &pm)) {
        pm = pm.scaled(option.rect.size());
        QPixmapCache::insert(cacheKey, pm);
    }
    if (!selected && !hover) {
        painter->drawPixmap(option.rect, pm);
        return;
    }

    QPainter p;
    p.begin(&pm);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    int opacity = selected ? (hover ? 130 : 155) : 180;
    p.fillRect(pm.rect(), QColor(0, 0, 0, opacity));
    p.end();

    painter->drawPixmap(option.rect.adjusted(0,2,0,-2), pm, pm.rect().adjusted(0,2,0,-2));
}

