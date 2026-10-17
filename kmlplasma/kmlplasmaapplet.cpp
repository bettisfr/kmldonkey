/* -*- c++ -*-
 *
 * kmlplasmaapplet.cpp
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

#include "kmlplasmaapplet.h"

KmlPlasmaApplet::KmlPlasmaApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
    , m_engine(0)
    , m_widget(0)
{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    resize(100, 80);
}

KmlPlasmaApplet::~KmlPlasmaApplet()
{
}

void KmlPlasmaApplet::init()
{
    m_pixmap = KIcon("kmldonkey").pixmap(128,128);

    const bool notInPanel = containment() ? containment()->containmentType() != Plasma::Containment::PanelContainment : true;
    KConfigGroup cg = config();
    m_runOnClick = cg.readEntry("run", true);
    m_showLabels = cg.readEntry("labels", notInPanel);
    m_connectionEnabled = cg.readEntry("connection", notInPanel);
    m_rateEnabled = cg.readEntry("rate", true);
    m_filesEnabled = cg.readEntry("files", notInPanel);
    m_transferEnabled = cg.readEntry("transfer", notInPanel);
    m_sharedEnabled = cg.readEntry("shared", notInPanel);

    Q_ASSERT( ! m_engine );
    m_engine = dataEngine("kmldonkey");
    if( ! m_engine->isValid() ) {
        kWarning() << "Invalid Plasma DataEngine for kmldonkey";
    }

    connect(m_engine, SIGNAL(sourceAdded(const QString&)),
            this, SLOT(onSourceAdded(const QString&)));
    connect(m_engine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(onSourceRemoved(const QString&)));

    m_engine->connectAllSources(this);
}

void KmlPlasmaApplet::onSourceAdded(const QString& source)
{
    kDebug() << "KmlPlasmaApplet::onSourceAdded source=" << source;
    m_engine->connectSource(source, this);
}

void KmlPlasmaApplet::onSourceRemoved(const QString& source)
{
    kDebug() << "KmlPlasmaApplet::onSourceRemoved source=" << source;
    if(source == "stats") {
        m_rate = m_files = m_transfer = m_shared = QString();
    }
    else if(source == "connection") {
        m_connection = QString();
    }
    else {
        return;
    }

    update();
}

QList<QAction*> KmlPlasmaApplet::contextualActions()
{
    QList<QAction*> list;
    if( QAction* configureconnection = m_engine->findChild<QAction*>("configure_connection") )
        list << configureconnection;
    if( QAction* selecthost = m_engine->findChild<QAction*>("HostSelectAction") )
        list << selecthost;
    return list;
}

#if 0
Qt::Orientations KmlPlasmaApplet::expandingDirections() const
{
    if (formFactor() == Plasma::Horizontal) {
        return Qt::Vertical;
    } else {
        return Qt::Horizontal;
    }
}
#endif

void KmlPlasmaApplet::dataUpdated(QString source, Plasma::DataEngine::Data data)
{
    //kDebug()<<"KmlPlasmaApplet::dataUpdated source="<<source;
    if(source == "stats") {
        m_rate = QString("%1/%2").arg(QString::number(data["rate_down"].toDouble(),'f',1)).arg(QString::number(data["rate_up"].toDouble(),'f',1));
        m_files = QString("%1/%2").arg(data["files_downloading"].toInt()).arg(data["files_finished"].toInt());
        m_transfer = QString("%1/%2").arg(data["transfer_down"].toString()).arg(data["transfer_up"].toString());
        m_shared = QString("%1/%2").arg(data["share_downloading"].toInt()).arg(data["shared_total"].toInt());
    }
    else if(source == "connection") {
        m_connection = data["name"].toString();
    }
    update();
}

void KmlPlasmaApplet::createConfigurationInterface(KConfigDialog *parent)
{
    if (! m_widget) {
        m_widget = new QWidget(parent->mainWidget());
        QVBoxLayout *l = new QVBoxLayout(m_widget);
        m_widget->setLayout(l);

        QGroupBox* optionsGrp = new QGroupBox(i18n("Options"), m_widget);
        l->addWidget(optionsGrp);
        QVBoxLayout *optionsGrpLayout = new QVBoxLayout(optionsGrp);
        optionsGrp->setLayout(optionsGrpLayout);

        m_runOnClickCheckBox = new QCheckBox(i18n("Run KMLDonkey on click"), optionsGrp);
        optionsGrpLayout->addWidget(m_runOnClickCheckBox);

        m_showLabelsCheckBox = new QCheckBox(i18n("Show Labels"), optionsGrp);
        optionsGrpLayout->addWidget(m_showLabelsCheckBox);

        QGroupBox* displayGrp = new QGroupBox(i18n("Display"), m_widget);
        l->addWidget(displayGrp);
        QVBoxLayout *displayGrpLayout = new QVBoxLayout(displayGrp);
        displayGrp->setLayout(displayGrpLayout);
        m_connectionCheckBox = new QCheckBox(i18n("Connection"), displayGrp);
        displayGrpLayout->addWidget(m_connectionCheckBox);
        m_rateCheckBox = new QCheckBox(i18n("Rate"), displayGrp);
        displayGrpLayout->addWidget(m_rateCheckBox);
        m_filesCheckBox = new QCheckBox(i18n("Files"), displayGrp);
        displayGrpLayout->addWidget(m_filesCheckBox);
        m_transferCheckBox = new QCheckBox(i18n("Transfer"), displayGrp);
        displayGrpLayout->addWidget(m_transferCheckBox);
        m_sharedCheckBox = new QCheckBox(i18n("Shared"), displayGrp);
        displayGrpLayout->addWidget(m_sharedCheckBox);
    }

    m_runOnClickCheckBox->setChecked(m_runOnClick);
    m_showLabelsCheckBox->setChecked(m_showLabels);
    m_connectionCheckBox->setChecked(m_connectionEnabled);
    m_rateCheckBox->setChecked(m_rateEnabled);
    m_filesCheckBox->setChecked(m_filesEnabled);
    m_transferCheckBox->setChecked(m_transferEnabled);
    m_sharedCheckBox->setChecked(m_sharedEnabled);

    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage(m_widget, parent->windowTitle(), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    //m_widget->show();
}

void KmlPlasmaApplet::configAccepted()
{
    m_runOnClick = m_runOnClickCheckBox->isChecked();
    m_showLabels = m_showLabelsCheckBox->isChecked();
    m_connectionEnabled = m_connectionCheckBox->isChecked();
    m_rateEnabled = m_rateCheckBox->isChecked();
    m_filesEnabled = m_filesCheckBox->isChecked();
    m_transferEnabled = m_transferCheckBox->isChecked();
    m_sharedEnabled = m_sharedCheckBox->isChecked();

    KConfigGroup cg = config();
    cg.writeEntry("run", m_runOnClick);
    cg.writeEntry("labels", m_showLabels);
    cg.writeEntry("connection", m_connectionEnabled);
    cg.writeEntry("rate", m_rateEnabled);
    cg.writeEntry("files", m_filesEnabled);
    cg.writeEntry("transfer", m_transferEnabled);
    cg.writeEntry("shared", m_sharedEnabled);

    update();
    emit configNeedsSaving();
}

void KmlPlasmaApplet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if( m_runOnClick && boundingRect().contains(event->pos()) && event->button() == Qt::LeftButton && KGlobalSettings::singleClick() ) {
        run();
    }
    Plasma::Applet::mousePressEvent(event);
}

void KmlPlasmaApplet::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if( m_runOnClick && boundingRect().contains(event->pos()) && ! KGlobalSettings::singleClick() ) {
        run();
    }
    Plasma::Applet::mouseDoubleClickEvent(event);
}

void KmlPlasmaApplet::run()
{
    KToolInvocation::startServiceByDesktopName("kmldonkey");
}

void KmlPlasmaApplet::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if( ! KUrl::List::canDecode(event->mimeData()) )
        return;

    const KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());

#ifdef QT_DEBUG
    bool ok = 
#endif
               QMetaObject::invokeMethod(m_engine,"submitUrls",Q_ARG(KUrl::List,urls));

    Q_ASSERT(ok);

    event->accept();
}

void KmlPlasmaApplet::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);

    p->setOpacity(0.18);
    p->drawPixmap(contentsRect, m_pixmap);
    p->setOpacity(1.0);

    QStringList textlist;
    if( m_connectionEnabled )
        textlist << ( m_connection );

    if( m_rateEnabled )
        textlist << ( m_showLabels && !m_rate.isNull() ? i18nc("@label Rate download/upload","Rate %1",m_rate) : m_rate );
    if( m_filesEnabled )
        textlist << ( m_showLabels && !m_files.isNull() ? i18nc("@label Files downloading/total","Files %1",m_files) : m_files );
    if( m_transferEnabled )
        textlist << ( m_showLabels && !m_transfer.isNull() ? i18nc("@label Transfer downloaded/uploaded","Transfer %1",m_transfer) : m_transfer );
    if( m_sharedEnabled )
        textlist << ( m_showLabels && !m_shared.isNull() ? i18nc("@label Shared downloading/total","Shared %1",m_shared) : m_shared );

    QString text;
    switch( formFactor() ) {
        case Plasma::Horizontal: {
            text = textlist.join(" ");
        } break;
        case Plasma::Vertical: {
            text = textlist.join("\n");
        } break;
        default: {
            const QString singleline = textlist.join(" ");
            const QString linebreaks = textlist.join("\n");
            const int w = contentsRect.width() - p->boundingRect(contentsRect, Qt::TextSingleLine, singleline).width();
            const int h = contentsRect.height() - p->boundingRect(contentsRect, 0, linebreaks).height();
            text = w > h ? singleline : linebreaks;
        } break;
    }

    QRect tmpRect;
    QFont tmpFont;
    tmpFont.setPointSizeF( contentsRect.height() );
    do {
        p->setFont(tmpFont);
        tmpFont.setPointSize(qMax(KGlobalSettings::smallestReadableFont().pointSize(), tmpFont.pointSize() - 1));
        tmpRect = p->boundingRect(contentsRect, 0, text);
    } while (
        tmpFont.pointSize() > KGlobalSettings::smallestReadableFont().pointSize() &&
            ( tmpRect.width() > contentsRect.width() || tmpRect.height() > contentsRect.height() )
    );

    p->setFont(tmpFont);

    QColor fontcolor = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme()).foreground().color();
    p->setPen(QPen(fontcolor));

    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);

    p->drawText(contentsRect, text, QTextOption(Qt::AlignHCenter));
}

#include "kmlplasmaapplet.moc"
