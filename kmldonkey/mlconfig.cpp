/* -*- c++ -*-
 *
 * mlconfig.cpp
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

#include <QCheckBox>
#include <QSignalBlocker>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QDir>
#include <QHostAddress>
#include <QPointer>
#include <QTimer>
#include <QRegularExpression>
#include <QDialogButtonBox>
#include <QPushButton>
#include <KMessageBox>
#include <KLineEdit>
#include <memory>
#include "hostiface.h"
#include <QVBoxLayout>
#include <QList>

#include <QDebug>
#include <KLocalizedString>

#include "donkeyprotocol.h"
#include "network.h"
#include "mlconfig.h"
#include "kmldonkey.h"

namespace {
using CommandReply = std::function<void(bool, const QString&)>;
class PathsReply : public ConsoleCallbackInterface {
public:
    explicit PathsReply(std::function<void(const QString&)> reply) : m_reply(std::move(reply)) {}
    void callback(const QString&, const QString& result) override { m_reply(result); }
private:
    std::function<void(const QString&)> m_reply;
};

// A dialog can be closed before a core reply arrives. The protocol owns the
// callback; guard the UI independently and bound the wait for each command.
void pathCommand(DonkeyProtocol* core, QObject* owner, const QString& command, CommandReply reply)
{
    QPointer<QObject> guard(owner);
    auto settled = std::make_shared<bool>(false);
    auto* timer = new QTimer(owner);
    timer->setSingleShot(true);
    QPointer<QTimer> timerGuard(timer);
    QObject::connect(timer, &QTimer::timeout, owner, [guard, settled, reply, timerGuard] {
        if (!guard || *settled) return;
        *settled = true;
        if (timerGuard) timerGuard->deleteLater();
        reply(false, i18n("The MLDonkey server did not respond. Reconnect and check the paths before retrying."));
    });
    timer->start(10000);
    core->sendConsoleMessage(command, new PathsReply([guard, settled, reply, timerGuard](const QString& result) {
        if (!guard || *settled) return;
        *settled = true;
        if (timerGuard) { timerGuard->stop(); timerGuard->deleteLater(); }
        reply(true, result.trimmed());
    }));
}

QString quoteCoreString(QString value)
{
    value.replace('\\', "\\\\");
    value.replace('"', "\\\"");
    value.replace('\t', "\\t");
    return '"' + value + '"';
}
}

QList<DownloadPathsWidget::Share> DownloadPathsWidget::parseShares(const QString& reply)
{
    QList<Share> entries;
    const QRegularExpression row(QStringLiteral("^\\s*(-?\\d+)\\s+(.+)\\s+(\\S+)\\s*$"));
    for (const QString& line : reply.split('\n')) {
        const auto match = row.match(line);
        if (match.hasMatch())
            entries.append({match.captured(1).toInt(), match.captured(2), match.captured(3)});
    }
    return entries;
}

DownloadPathsWidget::DownloadPathsWidget(DonkeyProtocol* core, QWidget* parent)
    : QWidget(parent), m_core(core)
{
    auto* layout = new QVBoxLayout(this);
    auto* group = new QGroupBox(i18n("Download folders"), this);
    auto* form = new QFormLayout(group);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(12);
    form->setRowWrapPolicy(QFormLayout::WrapLongRows);
    QString host = core->getHost() ? core->getHost()->address() : QStringLiteral("localhost");
    const bool local = host.compare(QStringLiteral("localhost"), Qt::CaseInsensitive) == 0
        || QHostAddress(host).isLoopback();
    auto addFolder = [this, local](QFormLayout* target, const QString& label) {
        auto* row = new QWidget(this);
        auto* box = new QHBoxLayout(row);
        box->setContentsMargins(0, 0, 0, 0);
        auto* edit = new KLineEdit(row);
        edit->setClearButtonEnabled(true);
        auto* browse = new QPushButton(QIcon::fromTheme("document-open-folder"), i18n("Browse…"), row);
        box->addWidget(edit, 1);
        box->addWidget(browse);
        browse->setEnabled(local);
        if (!local) browse->setToolTip(i18n("Enter a folder on the remote MLDonkey server."));
        connect(browse, &QPushButton::clicked, this, [this, edit] {
            const QString path = QFileDialog::getExistingDirectory(this, i18n("Choose download folder"), edit->text());
            if (!path.isEmpty()) edit->setText(path);
        });
        connect(edit, &QLineEdit::textChanged, this, &DownloadPathsWidget::pathsChanged);
        edit->setEnabled(false);
        target->addRow(label, row);
        return edit;
    };
    m_completed = addFolder(form, i18n("Completed downloads:"));
    m_completed->setObjectName("completedDownloadsPath");
    m_completed->setPlaceholderText(i18n("Separate folders configured — choose one folder for all downloads"));
    auto* help = new QLabel(i18n("Files and folders you finish downloading go here. "
        "The folder must be writable by MLDonkey. Existing files are not moved."), group);
    help->setWordWrap(true);
    form->addRow(help);

    auto* advanced = new QCheckBox(i18n("Advanced folder settings"), group);
    form->addRow(advanced);
    m_advanced = new QWidget(group);
    auto* advancedLayout = new QVBoxLayout(m_advanced);
    advancedLayout->setContentsMargins(0, 0, 0, 0);
    auto* advancedForm = new QFormLayout;
    advancedLayout->addLayout(advancedForm);
    m_files = addFolder(advancedForm, i18n("Individual files:"));
    m_files->setObjectName("completedFilesPath");
    m_directories = addFolder(advancedForm, i18n("Multi-file downloads:"));
    m_directories->setObjectName("completedDirectoriesPath");
    m_temp = addFolder(advancedForm, i18n("Temporary files:"));
    m_temp->setObjectName("temporaryFilesPath");
    auto* advancedHelp = new QLabel(i18n("These paths refer to folders on the server. Use absolute paths for new folders. "
        "Keep the temporary folder unchanged while downloads are active."), m_advanced);
    advancedHelp->setWordWrap(true);
    advancedLayout->addWidget(advancedHelp);
    form->addRow(m_advanced);
    m_advanced->hide();
    connect(advanced, &QCheckBox::toggled, m_advanced, &QWidget::setVisible);

    // A common destination updates both core strategies only when edited.
    // Preserve existing separate destinations when opening or closing the dialog.
    connect(m_completed, &QLineEdit::textChanged, this, [this](const QString& path) {
        const QSignalBlocker files(m_files), directories(m_directories);
        m_files->setText(path);
        m_directories->setText(path);
    });
    auto summarize = [this] {
        const QSignalBlocker summary(m_completed);
        m_completed->setText(m_files->text() == m_directories->text() ? m_files->text() : QString());
    };
    connect(m_files, &QLineEdit::textChanged, this, summarize);
    connect(m_directories, &QLineEdit::textChanged, this, summarize);
    m_status = new QLabel(i18n("Loading folders from MLDonkey…"), group);
    m_status->setWordWrap(true);
    m_status->setTextFormat(Qt::PlainText);
    form->addRow(m_status);
    layout->addWidget(group);
    QTimer::singleShot(0, this, &DownloadPathsWidget::load);
}

void DownloadPathsWidget::addAdvancedOptions(QWidget* options)
{
    m_advanced->layout()->addWidget(options);
}

void DownloadPathsWidget::load()
{
    pathCommand(m_core, this, QStringLiteral("shares"), [this](bool ok, const QString& reply) {
        m_shares = ok ? parseShares(reply) : QList<Share>();
        for (const Share& entry : m_shares) {
            if (entry.strategy == "incoming_files" && m_originalFiles.isEmpty()) m_originalFiles = entry.path;
            if (entry.strategy == "incoming_directories" && m_originalDirectories.isEmpty()) m_originalDirectories = entry.path;
        }
        if (!ok || m_originalFiles.isEmpty() || m_originalDirectories.isEmpty()) {
            m_status->setText(ok ? i18n("Unable to read download folders. Administrator access to the core is required.") : reply);
            return;
        }
        m_originalTemp = m_core->getOption("temp_directory");
        if (m_originalTemp.isEmpty()) {
            m_status->setText(i18n("The temporary folder has not been received. Reconnect and reopen this dialog."));
            return;
        }
        const QSignalBlocker blockCommon(m_completed), blockFiles(m_files), blockDirectories(m_directories), blockTemp(m_temp);
        m_completed->setText(m_originalFiles == m_originalDirectories ? m_originalFiles : QString());
        m_files->setText(m_originalFiles);
        m_directories->setText(m_originalDirectories);
        m_temp->setText(m_originalTemp);
        for (auto* edit : {m_completed, m_files, m_directories, m_temp}) edit->setEnabled(true);
        m_ready = true;
        m_status->clear();
    });
}

QString DownloadPathsWidget::validationError() const
{
    if (!m_ready) return QString(); // Other settings remain editable if paths cannot be loaded.
    const QList<QPair<QString, QString>> paths = {{m_files->text(), m_originalFiles},
        {m_directories->text(), m_originalDirectories}, {m_temp->text(), m_originalTemp}};
    for (const auto& entry : paths) {
        const QString& path = entry.first;
        if (path.isEmpty() || path.contains('\n') || path.contains('\r') || path.contains(QChar::Null))
            return i18n("Folder paths must not be empty or contain line breaks.");
        if (path != entry.second && !QDir::isAbsolutePath(path))
            return i18n("Use an absolute server path for a new folder: %1", path);
    }
    return QString();
}

void DownloadPathsWidget::checkFolders(QStringList paths, std::function<void(bool, const QString&)> finished)
{
    if (paths.isEmpty()) { finished(true, {}); return; }
    const QString path = paths.takeFirst();
    pathCommand(m_core, this, QStringLiteral("check_download_directory ") + quoteCoreString(path),
        [this, paths, path, finished](bool ok, const QString& result) {
            if (ok && result == QStringLiteral("directory writable")) {
                checkFolders(paths, finished);
            } else {
                const QString error = result.startsWith(QStringLiteral("No such command"))
                    ? i18n("Update the MLDonkey server to 3.2.1+ocaml5.5 or newer to configure download folders safely.")
                    : i18n("MLDonkey cannot write to %1: %2", path, result);
                finished(false, error);
            }
        });
}

void DownloadPathsWidget::apply(std::function<void(bool)> finished)
{
    const bool changed = m_ready && (m_files->text() != m_originalFiles
        || m_directories->text() != m_originalDirectories || m_temp->text() != m_originalTemp);
    auto persist = [this, finished](bool ok, const QString& error) {
        if (!ok) { m_status->setText(error); finished(false); return; }
        pathCommand(m_core, this, QStringLiteral("save options"), [this, finished](bool saved, const QString& reply) {
            const bool success = saved && reply.contains(QStringLiteral("saved"), Qt::CaseInsensitive)
                && !reply.contains(QStringLiteral("disabled"), Qt::CaseInsensitive);
            m_status->setText(success ? i18n("Settings saved by MLDonkey.") : reply);
            if (success && m_ready) {
                m_originalFiles = m_files->text();
                m_originalDirectories = m_directories->text();
                m_originalTemp = m_temp->text();
            }
            finished(success);
        });
    };
    if (!changed) { persist(true, {}); return; }
    QList<Share> desired = m_shares;
    bool files = false, directories = false;
    for (Share& entry : desired) {
        if (entry.strategy == "incoming_files" && !files) { entry.path = m_files->text(); files = true; }
        if (entry.strategy == "incoming_directories" && !directories) { entry.path = m_directories->text(); directories = true; }
    }
    m_status->setText(i18n("Saving folders…"));
    QStringList checks;
    if (m_files->text() != m_originalFiles) checks.append(m_files->text());
    if (m_directories->text() != m_originalDirectories) checks.append(m_directories->text());
    if (m_temp->text() != m_originalTemp) checks.append(m_temp->text());
    checks.removeDuplicates();
    checkFolders(checks, [this, desired, persist](bool writable, const QString& error) {
        if (!writable) { persist(false, error); return; }
        pathCommand(m_core, this, QStringLiteral("set_download_folders ")
            + quoteCoreString(m_files->text()) + QChar(32) + quoteCoreString(m_directories->text()),
            [this, desired, persist](bool ok, const QString& reply) {
                if (!ok || !reply.contains("download folders changed")) { persist(false, reply); return; }
                pathCommand(m_core, this, QStringLiteral("shares"), [this, desired, persist](bool read, const QString& result) {
                    const auto actual = parseShares(result);
                    bool matches = read && actual.size() == desired.size();
                    for (int i = 0; matches && i < actual.size(); ++i)
                        matches = actual[i].path == desired[i].path && actual[i].strategy == desired[i].strategy
                            && actual[i].priority == desired[i].priority;
                    if (!matches) { persist(false, i18n("The server did not confirm the requested folders. Reopen this dialog to check its settings.")); return; }
                    m_shares = actual;
                    if (m_temp->text() == m_originalTemp) { persist(true, {}); return; }
                    pathCommand(m_core, this, QStringLiteral("set temp_directory ") + quoteCoreString(m_temp->text()),
                        [persist](bool accepted, const QString& response) {
                            persist(accepted && response.contains("option temp_directory value changed"), response);
                        });
                });
            });
    });
}

NetworkPage::NetworkPage(QWidget* parent, const char* name)
    : QWidget(parent)
{
    setObjectName(name);
    QVBoxLayout* layout = new QVBoxLayout(this);

    foreach (Network *net, KMLDonkey::App->donkey->availableNetworks()) {
        QCheckBox* c = new QCheckBox(net->networkName(), this);
        c->setChecked(net->networkEnabled());
        layout->addWidget(c);
        connect(c, SIGNAL(toggled(bool)), SLOT(becomeDirty(bool)));
        nwBox.insert(net->networkNo(), c);
    }
    layout->addStretch(1);
}

void NetworkPage::becomeDirty(bool)
{
    emit pageDirty();
}

void NetworkPage::applyOptions(DonkeyProtocol* target)
{
    QHash<int, QCheckBox *>::const_iterator it;
    for ( it = nwBox.begin(); it != nwBox.end(); ++it) {
        Network* net = target->findNetworkNo( it.key() );
        if (!net || net->networkEnabled() == it.value()->isChecked())
            continue;
        target->enableNetwork(it.key(), it.value()->isChecked());
    }
}

MLDonkeyConfigDialog::MLDonkeyConfigDialog(QWidget* parent, const char* name)
    : PortedPageDialog(parent)
{
    setCaption(i18n("MLDonkey Configuration"));
    setObjectName(name ? name : "MLDonkeyConfigDialog");
    setFaceType(PortedPageDialog::Tree);
    setButtons(PortedDialog::Ok|PortedDialog::Apply|PortedDialog::Cancel);
    enableButton(Apply, false);

    m_paths = new DownloadPathsWidget(KMLDonkey::App->donkey, this);
    connect(m_paths, &DownloadPathsWidget::pathsChanged, this, &MLDonkeyConfigDialog::setDirtyDialog);
    // Identify the section by its option, independent of the core's language.
    QString pathsSection = QStringLiteral("Paths");
    for (const DonkeyOption& option : KMLDonkey::App->donkey->sectionOptions())
        if (option.optionName() == QStringLiteral("temp_directory")) pathsSection = option.optionSection();
    auto wrapOptions = [this](OptionEditorWidget* table) {
        auto* wrapper = new QWidget(this);
        auto* layout = new QVBoxLayout(wrapper);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->setSpacing(12);
        auto* filter = new KLineEdit(wrapper);
        filter->setPlaceholderText(i18n("Search options in this category…"));
        filter->setClearButtonEnabled(true);
        filter->setAccessibleName(i18n("Search options"));
        filter->addAction(QIcon::fromTheme(QStringLiteral("edit-find")), QLineEdit::LeadingPosition);
        auto* description = new QLabel(i18n("Select an option to read its description. Double-click a value to edit it."), wrapper);
        description->setTextFormat(Qt::PlainText);
        description->setWordWrap(true);
        description->setMinimumHeight(description->fontMetrics().height() * 3);
        layout->addWidget(filter);
        layout->addWidget(table, 1);
        layout->addWidget(description);
        connect(filter, &QLineEdit::textChanged, table, [table](const QString& text) {
            for (int row = 0; row < table->rowCount(); ++row) {
                const auto* name = table->item(row, 0);
                const auto* value = table->item(row, 1);
                const QString searchable = name->text() + QChar(32) + name->data(Qt::UserRole).toString()
                    + QChar(32) + name->toolTip() + QChar(32) + value->text();
                table->setRowHidden(row, !searchable.contains(text, Qt::CaseInsensitive));
            }
        });
        connect(table, &QTableWidget::currentCellChanged, description,
            [table, description](int row, int, int, int) {
                if (row >= 0 && table->item(row, 0))
                    description->setText(table->item(row, 0)->toolTip());
            });
        return wrapper;
    };
    auto sectionIcon = [](const QString& section) {
        const QString name = section.toLower();
        QString icon = QStringLiteral("preferences-other");
        if (name.contains("bandwidth")) icon = QStringLiteral("network-workgroup");
        else if (name.contains("security")) icon = QStringLiteral("security-high");
        else if (name.contains("path") || name.contains("download")) icon = QStringLiteral("folder-download");
        else if (name.contains("interface") || name.contains("network")) icon = QStringLiteral("network-wired");
        else if (name.contains("startup")) icon = QStringLiteral("system-run");
        else if (name.contains("mail")) icon = QStringLiteral("mail-message");
        return QIcon::fromTheme(icon);
    };
    // Add regular option sections
    OptionEditorWidget* page;
    foreach (const DonkeyOption &opt, KMLDonkey::App->donkey->sectionOptions()) {
        if (!KMLDonkey::App->advancedConfig && opt.optionAdvanced())
            continue;
        const QString section = opt.optionSection();
        page = pages.value(section);
        if (!page) {
            page = new OptionEditorWidget(this);
            if (section == pathsSection) {
                m_paths->addAdvancedOptions(wrapOptions(page));
                m_pathsItem = addPage(m_paths, section);
            } else {
                auto* item = addPage(wrapOptions(page), section);
                item->setIcon(sectionIcon(section));
                item->setHeader(section);
            }
            connect(page, SIGNAL(listIsDirty()), SLOT(setDirtyDialog()));
            pages.insert(section, page);
        }
        if (opt.optionName() != QStringLiteral("temp_directory")) page->addOption(opt);
    }
    if (!m_pathsItem) m_pathsItem = addPage(m_paths, i18n("Paths"));
    m_pathsItem->setIcon(QIcon::fromTheme(QStringLiteral("folder-download")));
    // Add the root network page
    QString networksName = i18nc("mldonkey config: network options page", "Networks");

    QWidget* w = new QWidget(this);
    QVBoxLayout* l = new QVBoxLayout(w);
    l->setSpacing(12);
    l->setContentsMargins(0, 0, 0, 0);

    KPageWidgetItem* networkItem = addPage(w, networksName);
    networkItem->setIcon(QIcon::fromTheme(QStringLiteral("network-wired")));
    m_networkPage = new NetworkPage(w);
    l->addWidget(m_networkPage);
    connect(m_networkPage, SIGNAL(pageDirty()), SLOT(setDirtyDialog()));

    // Add the network sub-pages
    foreach (const DonkeyOption &opt, KMLDonkey::App->donkey->pluginOptions()) {
        if (!KMLDonkey::App->advancedConfig && opt.optionAdvanced())
            continue;
        const QString section = opt.optionSection();
        page = pages.value(section);
        if (!page) {
            page = new OptionEditorWidget(this);
            auto* item = addSubPage(networkItem, wrapOptions(page), section);
            item->setIcon(QIcon::fromTheme(QStringLiteral("network-server")));
            item->setHeader(section);
            connect(page, SIGNAL(listIsDirty()), SLOT(setDirtyDialog()));
            pages.insert(section, page);
        }
        page->addOption(opt);
    }

    resize(QSize(980, 700).expandedTo( minimumSizeHint() ));
    //setInitialSize(configDialogSize("MLConfigDialog"));
    foreach (OptionEditorWidget *w, pages) {
        w->sortByColumn(-1, Qt::AscendingOrder);
        w->setSortingEnabled(true);
    }

    connect(this, SIGNAL(applyClicked()), this, SLOT(applyOptions()));

    connect(this, SIGNAL(finished(int)), this, SLOT(closeDialog()));
}

void MLDonkeyConfigDialog::setDirtyDialog()
{
    m_dirty = true;
    enableButton(Apply, !m_saving);
}

void MLDonkeyConfigDialog::accept()
{
    saveOptions(true);
}

void MLDonkeyConfigDialog::applyOptions()
{
    saveOptions(false);
}

void MLDonkeyConfigDialog::saveOptions(bool closeWhenSaved)
{
    if (m_saving) { m_closeAfterSave |= closeWhenSaved; return; }
    if (!m_dirty) { if (closeWhenSaved) QDialog::accept(); return; }
    m_closeAfterSave = closeWhenSaved;
    const QString error = m_paths->validationError();
    if (!error.isEmpty()) {
        KMessageBox::error(this, error, i18n("Download folders"), KMessageBox::PlainText);
        return;
    }
    if (!KMLDonkey::App->donkey->isConnected()) {
        KMessageBox::error(this, i18n("Connect to MLDonkey before saving settings."), i18n("MLDonkey Configuration"));
        return;
    }
    m_saving = true;
    pageWidget()->setEnabled(false);
    enableButton(Apply, false);
    foreach (OptionEditorWidget *w, pages) w->applyChangedOptions(KMLDonkey::App->donkey);
    m_networkPage->applyOptions(KMLDonkey::App->donkey);
    QPointer<MLDonkeyConfigDialog> guard(this);
    m_paths->apply([guard](bool success) {
        if (!guard) return;
        guard->m_saving = false;
        guard->pageWidget()->setEnabled(true);
        guard->enableButton(Ok, true);
        guard->m_dirty = !success;
        guard->enableButton(Apply, !success);
        if (!success) guard->setCurrentPage(guard->m_pathsItem);
        if (success && guard->m_closeAfterSave) guard->QDialog::accept();
    });
}

void MLDonkeyConfigDialog::closeDialog()
{
    //saveDialogSize("MLConfigDialog");
    // This dialog is owned by its caller and may live on the stack.
}

