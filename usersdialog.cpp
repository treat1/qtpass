#include "usersdialog.h"
#include "ui_usersdialog.h"
#include <QRegExp>
#include <QDebug>

UsersDialog::UsersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UsersDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(itemChange(QListWidgetItem *)));
    userList = NULL;
}

UsersDialog::~UsersDialog()
{
    delete ui;
}

Q_DECLARE_METATYPE(UserInfo *)

void UsersDialog::itemChange(QListWidgetItem *item)
{
    if (!item) return;
    UserInfo *info = item->data(Qt::UserRole).value<UserInfo *>();
    if (!info) return;
    info->enabled = item->checkState() == Qt::Checked;
}

void UsersDialog::setUsers(QList<UserInfo> *users)
{
    userList = users;
    populateList("");
}

void UsersDialog::populateList(const QString &filter)
{
    QRegExp nameFilter("*"+filter+"*");
    nameFilter.setPatternSyntax(QRegExp::Wildcard);
    nameFilter.setCaseSensitivity(Qt::CaseInsensitive);
    ui->listWidget->clear();
    if (userList) {
        for (QList<UserInfo>::iterator it = userList->begin(); it != userList->end(); ++it) {
            UserInfo &user(*it);
            if (filter.isEmpty() || nameFilter.exactMatch(user.name)) {
                if (user.validity == '-' && !ui->checkBox->isChecked()) {
                    continue;
                }
                if (user.expiry.toTime_t() > 0 && user.expiry.daysTo(QDateTime::currentDateTime()) > 0 && !ui->checkBox->isChecked()) {
                    continue;
                }
                QString userText = user.name + "\n" + user.key_id;
                if (user.created.toTime_t() > 0) {
                    userText += " " + tr("created")  + " " + user.created.toString(Qt::SystemLocaleShortDate);

                }
                if (user.expiry.toTime_t() > 0) {
                    userText += " " + tr("expires")  + " " + user.expiry.toString(Qt::SystemLocaleShortDate);
                }
                QListWidgetItem *item = new QListWidgetItem(userText, ui->listWidget);
                item->setCheckState(user.enabled ? Qt::Checked : Qt::Unchecked);
                item->setData(Qt::UserRole, QVariant::fromValue(&user));
                if (user.have_secret) {
                    item->setForeground(Qt::blue);
                } else if (user.validity == '-') {
                    item->setBackground(QColor(164, 0, 0));
                    item->setForeground(Qt::white);
                } else if (user.expiry.toTime_t() > 0 && user.expiry.daysTo(QDateTime::currentDateTime()) > 0) {
                    item->setForeground(Qt::red);
                }

                ui->listWidget->addItem(item);
            }
        }
    }
}

void UsersDialog::on_clearButton_clicked()
{
    ui->lineEdit->clear();
    populateList("");
}

void UsersDialog::on_lineEdit_textChanged(const QString &filter)
{
    populateList(filter);
}

void UsersDialog::closeEvent(QCloseEvent *event) {
    // TODO save window size or somethign
    event->accept();
}

void UsersDialog::on_checkBox_clicked() {
    populateList(ui->lineEdit->text());
}
