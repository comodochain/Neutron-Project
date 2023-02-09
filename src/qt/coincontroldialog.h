<<<<<<< HEAD
// Copyright (c) 2011-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_COINCONTROLDIALOG_H
#define BITCOIN_QT_COINCONTROLDIALOG_H

#include <amount.h>
#include <wallet/coinselection.h>
=======
#ifndef COINCONTROLDIALOG_H
#define COINCONTROLDIALOG_H
>>>>>>> 0.8

#include <QAbstractButton>
#include <QAction>
#include <QDialog>
#include <QList>
#include <QMenu>
#include <QPoint>
#include <QString>
#include <QTreeWidgetItem>

<<<<<<< HEAD
class PlatformStyle;
class WalletModel;

class CCoinControl;

namespace Ui {
    class CoinControlDialog;
}

#define ASYMP_UTF8 "\xE2\x89\x88"

class CCoinControlWidgetItem : public QTreeWidgetItem
{
public:
    explicit CCoinControlWidgetItem(QTreeWidget *parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    explicit CCoinControlWidgetItem(QTreeWidgetItem *parent, int type = Type) : QTreeWidgetItem(parent, type) {}

    bool operator<(const QTreeWidgetItem &other) const override;
};

=======
namespace Ui {
    class CoinControlDialog;
}
class WalletModel;
class CCoinControl;
>>>>>>> 0.8

class CoinControlDialog : public QDialog
{
    Q_OBJECT

public:
<<<<<<< HEAD
    explicit CoinControlDialog(CCoinControl& coin_control, WalletModel* model, const PlatformStyle *platformStyle, QWidget *parent = nullptr);
    ~CoinControlDialog();

    // static because also called from sendcoinsdialog
    static void updateLabels(CCoinControl& m_coin_control, WalletModel*, QDialog*);

    static QList<CAmount> payAmounts;
    static bool fSubtractFeeFromAmount;

private:
    Ui::CoinControlDialog *ui;
    CCoinControl& m_coin_control;
    WalletModel *model;
    int sortColumn;
    Qt::SortOrder sortOrder;

=======
    explicit CoinControlDialog(QWidget *parent = 0);
    ~CoinControlDialog();

    void setModel(WalletModel *model);
    
    // static because also called from sendcoinsdialog
    static void updateLabels(WalletModel*, QDialog*);
    static QString getPriorityLabel(double);
    
    static QList<qint64> payAmounts;
    static CCoinControl *coinControl;

private:
    Ui::CoinControlDialog *ui;
    WalletModel *model;
    int sortColumn;
    Qt::SortOrder sortOrder;
    
>>>>>>> 0.8
    QMenu *contextMenu;
    QTreeWidgetItem *contextMenuItem;
    QAction *copyTransactionHashAction;
    QAction *lockAction;
    QAction *unlockAction;
<<<<<<< HEAD

    const PlatformStyle *platformStyle;

    CInputCoin BuildInputCoin(QTreeWidgetItem* item);
    OutputIndex BuildOutputIndex(QTreeWidgetItem* item);

    bool IsMWEB(QTreeWidgetItem* item);
    bool IsCanonical(QTreeWidgetItem* item);

    void sortView(int, Qt::SortOrder);
    void updateView();

    enum
    {
        COLUMN_CHECKBOX = 0,
=======
    
    QString strPad(QString, int, QString);
    void sortView(int, Qt::SortOrder);
    void updateView();
    
    enum
    {
        COLUMN_CHECKBOX,
>>>>>>> 0.8
        COLUMN_AMOUNT,
        COLUMN_LABEL,
        COLUMN_ADDRESS,
        COLUMN_DATE,
        COLUMN_CONFIRMATIONS,
<<<<<<< HEAD
    };

    enum
    {
        TxHashRole = Qt::UserRole,
        VOutRole,
        PubKeyRole,
        MWEBOutRole
    };

    friend class CCoinControlWidgetItem;

private Q_SLOTS:
=======
        COLUMN_PRIORITY,
        COLUMN_TXHASH,
        COLUMN_VOUT_INDEX,
        COLUMN_AMOUNT_INT64,
        COLUMN_PRIORITY_INT64
    };
    
private slots:
>>>>>>> 0.8
    void showMenu(const QPoint &);
    void copyAmount();
    void copyLabel();
    void copyAddress();
    void copyTransactionHash();
    void lockCoin();
    void unlockCoin();
    void clipboardQuantity();
    void clipboardAmount();
    void clipboardFee();
    void clipboardAfterFee();
    void clipboardBytes();
<<<<<<< HEAD
=======
    void clipboardPriority();
>>>>>>> 0.8
    void clipboardLowOutput();
    void clipboardChange();
    void radioTreeMode(bool);
    void radioListMode(bool);
    void viewItemChanged(QTreeWidgetItem*, int);
    void headerSectionClicked(int);
    void buttonBoxClicked(QAbstractButton*);
    void buttonSelectAllClicked();
    void updateLabelLocked();
};

<<<<<<< HEAD
#endif // BITCOIN_QT_COINCONTROLDIALOG_H
=======
#endif // COINCONTROLDIALOG_H
>>>>>>> 0.8
