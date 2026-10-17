// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <KGuiItem>
#include <KStandardGuiItem>
#include <KTitleWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

// Shared button layout for dialogs migrated from KDE4's KDialog.
class PortedDialog : public QDialog {
    Q_OBJECT
public:
    enum ButtonCode { None = 0, Ok = 1, Cancel = 2, Apply = 4, Close = 8, Help = 16, Default = 32, Try = 64 };
    explicit PortedDialog(QWidget* parent = nullptr)
        : QDialog(parent)
        , m_layout(new QVBoxLayout(this))
        , m_main(new QWidget(this))
        , m_buttons(new QDialogButtonBox(this))
    {
        m_layout->setContentsMargins(marginHint(), marginHint(), marginHint(), marginHint());
        m_layout->setSpacing(spacingHint());
        m_title = new KTitleWidget(this);
        m_layout->addWidget(m_title);
        m_layout->addWidget(m_main);
        m_layout->addWidget(m_buttons);
        connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(m_buttons, &QDialogButtonBox::clicked, this, [this](QAbstractButton* b) {
            auto role = m_buttons->buttonRole(b);
            if (role == QDialogButtonBox::AcceptRole)
                emit okClicked();
            if (role == QDialogButtonBox::ApplyRole)
                emit applyClicked();
            if (role == QDialogButtonBox::ResetRole)
                emit defaultClicked();
            if (role == QDialogButtonBox::ActionRole)
                emit tryClicked();
            if (role == QDialogButtonBox::HelpRole)
                emit helpClicked();
        });
        setButtons(Ok | Cancel);
    }
    void setCaption(const QString& s) { setWindowTitle(s); m_title->setText(s); }
    QWidget* mainWidget() const { return m_main; }
    void setMainWidget(QWidget* w)
    {
        if (w == m_main)
            return;
        m_layout->removeWidget(m_main);
        m_main->deleteLater();
        m_main = w;
        m_layout->insertWidget(0, w);
    }
    void setButtons(int flags)
    {
        QDialogButtonBox::StandardButtons buttons;
        for (auto code : { Ok, Cancel, Apply, Close, Help, Default })
            if (flags & code)
                buttons |= standard(code);
        if (m_try) {
            m_buttons->removeButton(m_try);
            delete m_try;
            m_try = nullptr;
        }
        m_buttons->setStandardButtons(buttons);
        for (auto code : {Ok, Cancel, Apply, Close, Help, Default}) {
            if (auto* b = button(code)) {
                switch (code) {
                case Ok: KGuiItem::assign(b, KStandardGuiItem::ok()); break;
                case Cancel: KGuiItem::assign(b, KStandardGuiItem::cancel()); break;
                case Apply: KGuiItem::assign(b, KStandardGuiItem::apply()); break;
                case Close: KGuiItem::assign(b, KStandardGuiItem::close()); break;
                case Help: KGuiItem::assign(b, KStandardGuiItem::help()); break;
                case Default: KGuiItem::assign(b, KStandardGuiItem::defaults()); break;
                default: break;
                }
            }
        }
        if (flags & Try)
            m_try = m_buttons->addButton(tr("Preview"), QDialogButtonBox::ActionRole);
    }
    void enableButton(ButtonCode code, bool enabled)
    {
        if (auto* b = button(code))
            b->setEnabled(enabled);
    }
    void enableButtonOk(bool enabled) { enableButton(Ok, enabled); }
    QPushButton* button(ButtonCode code) const { return code == Try ? m_try : m_buttons->button(standard(code)); }
    void setButtonText(ButtonCode code, const QString& text)
    {
        if (auto* b = button(code))
            b->setText(text);
    }
    void setButtonGuiItem(ButtonCode code, const KGuiItem& item)
    {
        if (auto* b = button(code)) {
            b->setText(item.text());
            b->setIcon(item.icon());
            b->setToolTip(item.toolTip());
        }
    }
    void setDefaultButton(ButtonCode code)
    {
        if (auto* b = button(code))
            b->setDefault(true);
    }
    static int spacingHint() { return 12; }
    static int marginHint() { return 12; }
signals:
    void okClicked();
    void applyClicked();
    void defaultClicked();
    void tryClicked();
    void helpClicked();

private:
    static QDialogButtonBox::StandardButton standard(ButtonCode c)
    {
        switch (c) {
        case Ok:
            return QDialogButtonBox::Ok;
        case Cancel:
            return QDialogButtonBox::Cancel;
        case Apply:
            return QDialogButtonBox::Apply;
        case Close:
            return QDialogButtonBox::Close;
        case Help:
            return QDialogButtonBox::Help;
        case Default:
            return QDialogButtonBox::RestoreDefaults;
        case Try:
            return QDialogButtonBox::Retry;
        default:
            return QDialogButtonBox::NoButton;
        }
    }
    QVBoxLayout* m_layout;
    KTitleWidget* m_title;
    QWidget* m_main;
    QDialogButtonBox* m_buttons;
    QPushButton* m_try = nullptr;
};
