<<<<<<< HEAD
// Copyright (c) 2011-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <qt/coincontroldialog.h>
#include <qt/forms/ui_coincontroldialog.h>

#include <qt/addresstablemodel.h>
#include <qt/bitcoinunits.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/walletmodel.h>

#include <wallet/coincontrol.h>
#include <wallet/coinselection.h>
#include <interfaces/node.h>
#include <key_io.h>
#include <policy/policy.h>
#include <wallet/wallet.h>

#include <QApplication>
#include <QCheckBox>
#include <QCursor>
#include <QDialogButtonBox>
#include <QFlags>
#include <QIcon>
#include <QSettings>
#include <QTreeWidget>

QList<CAmount> CoinControlDialog::payAmounts;
bool CoinControlDialog::fSubtractFeeFromAmount = false;

bool CCoinControlWidgetItem::operator<(const QTreeWidgetItem &other) const {
    int column = treeWidget()->sortColumn();
    if (column == CoinControlDialog::COLUMN_AMOUNT || column == CoinControlDialog::COLUMN_DATE || column == CoinControlDialog::COLUMN_CONFIRMATIONS)
        return data(column, Qt::UserRole).toLongLong() < other.data(column, Qt::UserRole).toLongLong();
    return QTreeWidgetItem::operator<(other);
}

CoinControlDialog::CoinControlDialog(CCoinControl& coin_control, WalletModel* _model, const PlatformStyle *_platformStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CoinControlDialog),
    m_coin_control(coin_control),
    model(_model),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);

=======
#include "coincontroldialog.h"
#include "ui_coincontroldialog.h"

#include "init.h"
#include "bitcoinunits.h"
#include "walletmodel.h"
#include "addresstablemodel.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "coincontrol.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QColor>
#include <QCursor>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFlags>
#include <QIcon>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>

using namespace std;
QList<qint64> CoinControlDialog::payAmounts;
CCoinControl* CoinControlDialog::coinControl = new CCoinControl();

CoinControlDialog::CoinControlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CoinControlDialog),
    model(0)
{
    ui->setupUi(this);
    
>>>>>>> 0.8
    // context menu actions
    QAction *copyAddressAction = new QAction(tr("Copy address"), this);
    QAction *copyLabelAction = new QAction(tr("Copy label"), this);
    QAction *copyAmountAction = new QAction(tr("Copy amount"), this);
             copyTransactionHashAction = new QAction(tr("Copy transaction ID"), this);  // we need to enable/disable this
             lockAction = new QAction(tr("Lock unspent"), this);                        // we need to enable/disable this
             unlockAction = new QAction(tr("Unlock unspent"), this);                    // we need to enable/disable this

    // context menu
<<<<<<< HEAD
    contextMenu = new QMenu(this);
=======
    contextMenu = new QMenu();
>>>>>>> 0.8
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyLabelAction);
    contextMenu->addAction(copyAmountAction);
    contextMenu->addAction(copyTransactionHashAction);
    contextMenu->addSeparator();
    contextMenu->addAction(lockAction);
    contextMenu->addAction(unlockAction);
<<<<<<< HEAD

    // context menu signals
    connect(ui->treeWidget, &QWidget::customContextMenuRequested, this, &CoinControlDialog::showMenu);
    connect(copyAddressAction, &QAction::triggered, this, &CoinControlDialog::copyAddress);
    connect(copyLabelAction, &QAction::triggered, this, &CoinControlDialog::copyLabel);
    connect(copyAmountAction, &QAction::triggered, this, &CoinControlDialog::copyAmount);
    connect(copyTransactionHashAction, &QAction::triggered, this, &CoinControlDialog::copyTransactionHash);
    connect(lockAction, &QAction::triggered, this, &CoinControlDialog::lockCoin);
    connect(unlockAction, &QAction::triggered, this, &CoinControlDialog::unlockCoin);

=======
    
    // context menu signals
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showMenu(QPoint)));
    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(copyAddress()));
    connect(copyLabelAction, SIGNAL(triggered()), this, SLOT(copyLabel()));
    connect(copyAmountAction, SIGNAL(triggered()), this, SLOT(copyAmount()));
    connect(copyTransactionHashAction, SIGNAL(triggered()), this, SLOT(copyTransactionHash()));
    connect(lockAction, SIGNAL(triggered()), this, SLOT(lockCoin()));
    connect(unlockAction, SIGNAL(triggered()), this, SLOT(unlockCoin()));
    
>>>>>>> 0.8
    // clipboard actions
    QAction *clipboardQuantityAction = new QAction(tr("Copy quantity"), this);
    QAction *clipboardAmountAction = new QAction(tr("Copy amount"), this);
    QAction *clipboardFeeAction = new QAction(tr("Copy fee"), this);
    QAction *clipboardAfterFeeAction = new QAction(tr("Copy after fee"), this);
    QAction *clipboardBytesAction = new QAction(tr("Copy bytes"), this);
<<<<<<< HEAD
    QAction *clipboardLowOutputAction = new QAction(tr("Copy dust"), this);
    QAction *clipboardChangeAction = new QAction(tr("Copy change"), this);

    connect(clipboardQuantityAction, &QAction::triggered, this, &CoinControlDialog::clipboardQuantity);
    connect(clipboardAmountAction, &QAction::triggered, this, &CoinControlDialog::clipboardAmount);
    connect(clipboardFeeAction, &QAction::triggered, this, &CoinControlDialog::clipboardFee);
    connect(clipboardAfterFeeAction, &QAction::triggered, this, &CoinControlDialog::clipboardAfterFee);
    connect(clipboardBytesAction, &QAction::triggered, this, &CoinControlDialog::clipboardBytes);
    connect(clipboardLowOutputAction, &QAction::triggered, this, &CoinControlDialog::clipboardLowOutput);
    connect(clipboardChangeAction, &QAction::triggered, this, &CoinControlDialog::clipboardChange);

=======
    QAction *clipboardPriorityAction = new QAction(tr("Copy priority"), this);
    QAction *clipboardLowOutputAction = new QAction(tr("Copy low output"), this);
    QAction *clipboardChangeAction = new QAction(tr("Copy change"), this);
    
    connect(clipboardQuantityAction, SIGNAL(triggered()), this, SLOT(clipboardQuantity()));
    connect(clipboardAmountAction, SIGNAL(triggered()), this, SLOT(clipboardAmount()));
    connect(clipboardFeeAction, SIGNAL(triggered()), this, SLOT(clipboardFee()));
    connect(clipboardAfterFeeAction, SIGNAL(triggered()), this, SLOT(clipboardAfterFee()));
    connect(clipboardBytesAction, SIGNAL(triggered()), this, SLOT(clipboardBytes()));
    connect(clipboardPriorityAction, SIGNAL(triggered()), this, SLOT(clipboardPriority()));
    connect(clipboardLowOutputAction, SIGNAL(triggered()), this, SLOT(clipboardLowOutput()));
    connect(clipboardChangeAction, SIGNAL(triggered()), this, SLOT(clipboardChange()));
    
>>>>>>> 0.8
    ui->labelCoinControlQuantity->addAction(clipboardQuantityAction);
    ui->labelCoinControlAmount->addAction(clipboardAmountAction);
    ui->labelCoinControlFee->addAction(clipboardFeeAction);
    ui->labelCoinControlAfterFee->addAction(clipboardAfterFeeAction);
    ui->labelCoinControlBytes->addAction(clipboardBytesAction);
<<<<<<< HEAD
    ui->labelCoinControlLowOutput->addAction(clipboardLowOutputAction);
    ui->labelCoinControlChange->addAction(clipboardChangeAction);

    // toggle tree/list mode
    connect(ui->radioTreeMode, &QRadioButton::toggled, this, &CoinControlDialog::radioTreeMode);
    connect(ui->radioListMode, &QRadioButton::toggled, this, &CoinControlDialog::radioListMode);

    // click on checkbox
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &CoinControlDialog::viewItemChanged);

    // click on header
    ui->treeWidget->header()->setSectionsClickable(true);
    connect(ui->treeWidget->header(), &QHeaderView::sectionClicked, this, &CoinControlDialog::headerSectionClicked);

    // ok button
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &CoinControlDialog::buttonBoxClicked);

    // (un)select all
    connect(ui->pushButtonSelectAll, &QPushButton::clicked, this, &CoinControlDialog::buttonSelectAllClicked);

    ui->treeWidget->setColumnWidth(COLUMN_CHECKBOX, 84);
    ui->treeWidget->setColumnWidth(COLUMN_AMOUNT, 110);
    ui->treeWidget->setColumnWidth(COLUMN_LABEL, 190);
    ui->treeWidget->setColumnWidth(COLUMN_ADDRESS, 320);
    ui->treeWidget->setColumnWidth(COLUMN_DATE, 130);
    ui->treeWidget->setColumnWidth(COLUMN_CONFIRMATIONS, 110);

    // default view is sorted by amount desc
    sortView(COLUMN_AMOUNT, Qt::DescendingOrder);

    // restore list mode and sortorder as a convenience feature
    QSettings settings;
    if (settings.contains("nCoinControlMode") && !settings.value("nCoinControlMode").toBool())
        ui->radioTreeMode->click();
    if (settings.contains("nCoinControlSortColumn") && settings.contains("nCoinControlSortOrder"))
        sortView(settings.value("nCoinControlSortColumn").toInt(), (static_cast<Qt::SortOrder>(settings.value("nCoinControlSortOrder").toInt())));

    GUIUtil::handleCloseWindowShortcut(this);

    if(_model->getOptionsModel() && _model->getAddressTableModel())
    {
        updateView();
        updateLabelLocked();
        CoinControlDialog::updateLabels(m_coin_control, _model, this);
    }
}

CoinControlDialog::~CoinControlDialog()
{
    QSettings settings;
    settings.setValue("nCoinControlMode", ui->radioListMode->isChecked());
    settings.setValue("nCoinControlSortColumn", sortColumn);
    settings.setValue("nCoinControlSortOrder", (int)sortOrder);

    delete ui;
}

=======
    ui->labelCoinControlPriority->addAction(clipboardPriorityAction);
    ui->labelCoinControlLowOutput->addAction(clipboardLowOutputAction);
    ui->labelCoinControlChange->addAction(clipboardChangeAction);
    
    // toggle tree/list mode
    connect(ui->radioTreeMode, SIGNAL(toggled(bool)), this, SLOT(radioTreeMode(bool)));
    connect(ui->radioListMode, SIGNAL(toggled(bool)), this, SLOT(radioListMode(bool)));
    
    // click on checkbox
    connect(ui->treeWidget, SIGNAL(itemChanged( QTreeWidgetItem*, int)), this, SLOT(viewItemChanged( QTreeWidgetItem*, int)));
    
    // click on header
    #if QT_VERSION < 0x050000
        ui->treeWidget->header()->setClickable(true);
    #else
        ui->treeWidget->header()->setSectionsClickable(true);
    #endif
    connect(ui->treeWidget->header(), SIGNAL(sectionClicked(int)), this, SLOT(headerSectionClicked(int)));
    
    // ok button
    connect(ui->buttonBox, SIGNAL(clicked( QAbstractButton*)), this, SLOT(buttonBoxClicked(QAbstractButton*)));
    
    // (un)select all
    connect(ui->pushButtonSelectAll, SIGNAL(clicked()), this, SLOT(buttonSelectAllClicked()));
    
    ui->treeWidget->setColumnWidth(COLUMN_CHECKBOX, 84);
    ui->treeWidget->setColumnWidth(COLUMN_AMOUNT, 100);
    ui->treeWidget->setColumnWidth(COLUMN_LABEL, 170);
    ui->treeWidget->setColumnWidth(COLUMN_ADDRESS, 290);
    ui->treeWidget->setColumnWidth(COLUMN_DATE, 110);
    ui->treeWidget->setColumnWidth(COLUMN_CONFIRMATIONS, 100);
    ui->treeWidget->setColumnWidth(COLUMN_PRIORITY, 100);
    ui->treeWidget->setColumnHidden(COLUMN_TXHASH, true);         // store transacton hash in this column, but dont show it
    ui->treeWidget->setColumnHidden(COLUMN_VOUT_INDEX, true);     // store vout index in this column, but dont show it
    ui->treeWidget->setColumnHidden(COLUMN_AMOUNT_INT64, true);   // store amount int64 in this column, but dont show it
    ui->treeWidget->setColumnHidden(COLUMN_PRIORITY_INT64, true); // store priority int64 in this column, but dont show it
   
    // default view is sorted by amount desc
    sortView(COLUMN_AMOUNT_INT64, Qt::DescendingOrder);
}

CoinControlDialog::~CoinControlDialog()
{    
    delete ui;
}

void CoinControlDialog::setModel(WalletModel *model)
{
    this->model = model;
    
    if(model && model->getOptionsModel() && model->getAddressTableModel())
    {
        updateView();
        updateLabelLocked();
        CoinControlDialog::updateLabels(model, this);
    }
}

// helper function str_pad
QString CoinControlDialog::strPad(QString s, int nPadLength, QString sPadding)
{
    while (s.length() < nPadLength)
        s = sPadding + s;
        
    return s;
}

>>>>>>> 0.8
// ok button
void CoinControlDialog::buttonBoxClicked(QAbstractButton* button)
{
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
        done(QDialog::Accepted); // closes the dialog
}

// (un)select all
void CoinControlDialog::buttonSelectAllClicked()
{
    Qt::CheckState state = Qt::Checked;
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
    {
        if (ui->treeWidget->topLevelItem(i)->checkState(COLUMN_CHECKBOX) != Qt::Unchecked)
        {
            state = Qt::Unchecked;
            break;
        }
    }
    ui->treeWidget->setEnabled(false);
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
            if (ui->treeWidget->topLevelItem(i)->checkState(COLUMN_CHECKBOX) != state)
                ui->treeWidget->topLevelItem(i)->setCheckState(COLUMN_CHECKBOX, state);
    ui->treeWidget->setEnabled(true);
    if (state == Qt::Unchecked)
<<<<<<< HEAD
        m_coin_control.UnSelectAll(); // just to be sure
    CoinControlDialog::updateLabels(m_coin_control, model, this);
}

OutputIndex CoinControlDialog::BuildOutputIndex(QTreeWidgetItem* item)
{
    if (IsCanonical(item)) {
        return COutPoint(
            uint256S(item->data(COLUMN_ADDRESS, TxHashRole).toString().toStdString()),
            item->data(COLUMN_ADDRESS, VOutRole).toUInt()
        );
    } else {
        assert(IsMWEB(item));

        return mw::Hash::FromHex(item->data(COLUMN_ADDRESS, MWEBOutRole).toString().toStdString());
    }
=======
        coinControl->UnSelectAll(); // just to be sure
    CoinControlDialog::updateLabels(model, this);
>>>>>>> 0.8
}

// context menu
void CoinControlDialog::showMenu(const QPoint &point)
{
    QTreeWidgetItem *item = ui->treeWidget->itemAt(point);
    if(item)
    {
        contextMenuItem = item;
<<<<<<< HEAD

        // disable some items (like Copy Transaction ID, lock, unlock) for tree roots in context menu
        if (IsCanonical(item) || IsMWEB(item)) // this means it is a child node, so it is not a parent node in tree mode
        {
            copyTransactionHashAction->setEnabled(true);
            if (model->wallet().isLockedCoin(BuildOutputIndex(item)))
=======
        
        // disable some items (like Copy Transaction ID, lock, unlock) for tree roots in context menu
        if (item->text(COLUMN_TXHASH).length() == 64) // transaction hash is 64 characters (this means its a child node, so its not a parent node in tree mode)
        {
            copyTransactionHashAction->setEnabled(true);
            if (model->isLockedCoin(uint256(item->text(COLUMN_TXHASH).toStdString()), item->text(COLUMN_VOUT_INDEX).toUInt()))
>>>>>>> 0.8
            {
                lockAction->setEnabled(false);
                unlockAction->setEnabled(true);
            }
            else
            {
                lockAction->setEnabled(true);
                unlockAction->setEnabled(false);
            }
        }
        else // this means click on parent node in tree mode -> disable all
        {
            copyTransactionHashAction->setEnabled(false);
            lockAction->setEnabled(false);
            unlockAction->setEnabled(false);
        }

        // show context menu
        contextMenu->exec(QCursor::pos());
    }
}
<<<<<<< HEAD

// context menu action: copy amount
void CoinControlDialog::copyAmount()
{
    GUIUtil::setClipboard(BitcoinUnits::removeSpaces(contextMenuItem->text(COLUMN_AMOUNT)));
=======
    
// context menu action: copy amount
void CoinControlDialog::copyAmount()
{
    GUIUtil::setClipboard(contextMenuItem->text(COLUMN_AMOUNT));
>>>>>>> 0.8
}

// context menu action: copy label
void CoinControlDialog::copyLabel()
{
    if (ui->radioTreeMode->isChecked() && contextMenuItem->text(COLUMN_LABEL).length() == 0 && contextMenuItem->parent())
        GUIUtil::setClipboard(contextMenuItem->parent()->text(COLUMN_LABEL));
    else
        GUIUtil::setClipboard(contextMenuItem->text(COLUMN_LABEL));
}

// context menu action: copy address
void CoinControlDialog::copyAddress()
{
    if (ui->radioTreeMode->isChecked() && contextMenuItem->text(COLUMN_ADDRESS).length() == 0 && contextMenuItem->parent())
        GUIUtil::setClipboard(contextMenuItem->parent()->text(COLUMN_ADDRESS));
    else
        GUIUtil::setClipboard(contextMenuItem->text(COLUMN_ADDRESS));
}

// context menu action: copy transaction id
void CoinControlDialog::copyTransactionHash()
{
<<<<<<< HEAD
    if (IsCanonical(contextMenuItem)) {
        GUIUtil::setClipboard(contextMenuItem->data(COLUMN_ADDRESS, TxHashRole).toString());
    } else if (IsMWEB(contextMenuItem)) {
        GUIUtil::setClipboard(contextMenuItem->data(COLUMN_ADDRESS, MWEBOutRole).toString());
    }
=======
    GUIUtil::setClipboard(contextMenuItem->text(COLUMN_TXHASH));
>>>>>>> 0.8
}

// context menu action: lock coin
void CoinControlDialog::lockCoin()
{
    if (contextMenuItem->checkState(COLUMN_CHECKBOX) == Qt::Checked)
        contextMenuItem->setCheckState(COLUMN_CHECKBOX, Qt::Unchecked);
<<<<<<< HEAD

    model->wallet().lockCoin(BuildOutputIndex(contextMenuItem));
    contextMenuItem->setDisabled(true);
    contextMenuItem->setIcon(COLUMN_CHECKBOX, platformStyle->SingleColorIcon(":/icons/lock_closed"));
=======
        
    COutPoint outpt(uint256(contextMenuItem->text(COLUMN_TXHASH).toStdString()), contextMenuItem->text(COLUMN_VOUT_INDEX).toUInt());
    model->lockCoin(outpt);
    contextMenuItem->setDisabled(true);
    contextMenuItem->setIcon(COLUMN_CHECKBOX, QIcon(":/icons/lock_closed"));
>>>>>>> 0.8
    updateLabelLocked();
}

// context menu action: unlock coin
void CoinControlDialog::unlockCoin()
{
<<<<<<< HEAD
    model->wallet().unlockCoin(BuildOutputIndex(contextMenuItem));
=======
    COutPoint outpt(uint256(contextMenuItem->text(COLUMN_TXHASH).toStdString()), contextMenuItem->text(COLUMN_VOUT_INDEX).toUInt());
    model->unlockCoin(outpt);
>>>>>>> 0.8
    contextMenuItem->setDisabled(false);
    contextMenuItem->setIcon(COLUMN_CHECKBOX, QIcon());
    updateLabelLocked();
}

// copy label "Quantity" to clipboard
void CoinControlDialog::clipboardQuantity()
{
    GUIUtil::setClipboard(ui->labelCoinControlQuantity->text());
}

// copy label "Amount" to clipboard
void CoinControlDialog::clipboardAmount()
{
    GUIUtil::setClipboard(ui->labelCoinControlAmount->text().left(ui->labelCoinControlAmount->text().indexOf(" ")));
}

// copy label "Fee" to clipboard
void CoinControlDialog::clipboardFee()
{
<<<<<<< HEAD
    GUIUtil::setClipboard(ui->labelCoinControlFee->text().left(ui->labelCoinControlFee->text().indexOf(" ")).replace(ASYMP_UTF8, ""));
=======
    GUIUtil::setClipboard(ui->labelCoinControlFee->text().left(ui->labelCoinControlFee->text().indexOf(" ")));
>>>>>>> 0.8
}

// copy label "After fee" to clipboard
void CoinControlDialog::clipboardAfterFee()
{
<<<<<<< HEAD
    GUIUtil::setClipboard(ui->labelCoinControlAfterFee->text().left(ui->labelCoinControlAfterFee->text().indexOf(" ")).replace(ASYMP_UTF8, ""));
=======
    GUIUtil::setClipboard(ui->labelCoinControlAfterFee->text().left(ui->labelCoinControlAfterFee->text().indexOf(" ")));
>>>>>>> 0.8
}

// copy label "Bytes" to clipboard
void CoinControlDialog::clipboardBytes()
{
<<<<<<< HEAD
    GUIUtil::setClipboard(ui->labelCoinControlBytes->text().replace(ASYMP_UTF8, ""));
}

// copy label "Dust" to clipboard
=======
    GUIUtil::setClipboard(ui->labelCoinControlBytes->text());
}

// copy label "Priority" to clipboard
void CoinControlDialog::clipboardPriority()
{
    GUIUtil::setClipboard(ui->labelCoinControlPriority->text());
}

// copy label "Low output" to clipboard
>>>>>>> 0.8
void CoinControlDialog::clipboardLowOutput()
{
    GUIUtil::setClipboard(ui->labelCoinControlLowOutput->text());
}

// copy label "Change" to clipboard
void CoinControlDialog::clipboardChange()
{
<<<<<<< HEAD
    GUIUtil::setClipboard(ui->labelCoinControlChange->text().left(ui->labelCoinControlChange->text().indexOf(" ")).replace(ASYMP_UTF8, ""));
=======
    GUIUtil::setClipboard(ui->labelCoinControlChange->text().left(ui->labelCoinControlChange->text().indexOf(" ")));
>>>>>>> 0.8
}

// treeview: sort
void CoinControlDialog::sortView(int column, Qt::SortOrder order)
{
    sortColumn = column;
    sortOrder = order;
    ui->treeWidget->sortItems(column, order);
<<<<<<< HEAD
    ui->treeWidget->header()->setSortIndicator(sortColumn, sortOrder);
=======
    ui->treeWidget->header()->setSortIndicator((sortColumn == COLUMN_AMOUNT_INT64 ? COLUMN_AMOUNT : (sortColumn == COLUMN_PRIORITY_INT64 ? COLUMN_PRIORITY : sortColumn)), sortOrder);
>>>>>>> 0.8
}

// treeview: clicked on header
void CoinControlDialog::headerSectionClicked(int logicalIndex)
{
    if (logicalIndex == COLUMN_CHECKBOX) // click on most left column -> do nothing
<<<<<<< HEAD
    {
        ui->treeWidget->header()->setSortIndicator(sortColumn, sortOrder);
    }
    else
    {
=======
    {      
        ui->treeWidget->header()->setSortIndicator((sortColumn == COLUMN_AMOUNT_INT64 ? COLUMN_AMOUNT : (sortColumn == COLUMN_PRIORITY_INT64 ? COLUMN_PRIORITY : sortColumn)), sortOrder);
    }
    else
    {
        if (logicalIndex == COLUMN_AMOUNT) // sort by amount
            logicalIndex = COLUMN_AMOUNT_INT64;
            
        if (logicalIndex == COLUMN_PRIORITY) // sort by priority
            logicalIndex = COLUMN_PRIORITY_INT64;
            
>>>>>>> 0.8
        if (sortColumn == logicalIndex)
            sortOrder = ((sortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder);
        else
        {
            sortColumn = logicalIndex;
<<<<<<< HEAD
            sortOrder = ((sortColumn == COLUMN_LABEL || sortColumn == COLUMN_ADDRESS) ? Qt::AscendingOrder : Qt::DescendingOrder); // if label or address then default => asc, else default => desc
=======
            sortOrder = ((sortColumn == COLUMN_AMOUNT_INT64 || sortColumn == COLUMN_PRIORITY_INT64 || sortColumn == COLUMN_DATE || sortColumn == COLUMN_CONFIRMATIONS) ? Qt::DescendingOrder : Qt::AscendingOrder); // if amount,date,conf,priority then default => desc, else default => asc
>>>>>>> 0.8
        }

        sortView(sortColumn, sortOrder);
    }
}

// toggle tree mode
void CoinControlDialog::radioTreeMode(bool checked)
{
    if (checked && model)
        updateView();
}

// toggle list mode
void CoinControlDialog::radioListMode(bool checked)
{
    if (checked && model)
        updateView();
}

<<<<<<< HEAD
CInputCoin CoinControlDialog::BuildInputCoin(QTreeWidgetItem* item)
{
    if (IsMWEB(item)) {
        mw::Hash output_id = mw::Hash::FromHex(item->data(COLUMN_ADDRESS, MWEBOutRole).toString().toStdString());
        
        mw::Coin coin;
        bool found = model->wallet().findCoin(output_id, coin);
        assert(found);
        return CInputCoin(coin);
    } else {
        uint256 hash = uint256S(item->data(COLUMN_ADDRESS, TxHashRole).toString().toStdString());
        uint32_t n = item->data(COLUMN_ADDRESS, VOutRole).toUInt();
        CAmount amount = item->data(COLUMN_AMOUNT, Qt::UserRole).toUInt();

        std::vector<uint8_t> script_data = ParseHex(item->data(COLUMN_ADDRESS, PubKeyRole).toString().toStdString());
        CScript scriptPubKey(script_data.begin(), script_data.end());
        return CInputCoin(hash, n, amount, scriptPubKey);
    }
}

// checkbox clicked by user
void CoinControlDialog::viewItemChanged(QTreeWidgetItem* item, int column)
{
    if (column == COLUMN_CHECKBOX && (IsCanonical(item) || IsMWEB(item))) // this means it is a child node, so it is not a parent node in tree mode
    {
        CInputCoin input_coin = BuildInputCoin(item);

        if (item->checkState(COLUMN_CHECKBOX) == Qt::Unchecked)
            m_coin_control.UnSelect(input_coin.GetIndex());
        else if (item->isDisabled()) // locked (this happens if "check all" through parent node)
            item->setCheckState(COLUMN_CHECKBOX, Qt::Unchecked);
        else
            m_coin_control.Select(input_coin.GetIndex());

        // selection changed -> update labels
        if (ui->treeWidget->isEnabled()) // do not update on every click for (un)select all
            CoinControlDialog::updateLabels(m_coin_control, model, this);
=======
// checkbox clicked by user
void CoinControlDialog::viewItemChanged(QTreeWidgetItem* item, int column)
{
    if (column == COLUMN_CHECKBOX && item->text(COLUMN_TXHASH).length() == 64) // transaction hash is 64 characters (this means its a child node, so its not a parent node in tree mode)
    { 
        COutPoint outpt(uint256(item->text(COLUMN_TXHASH).toStdString()), item->text(COLUMN_VOUT_INDEX).toUInt());
        
        if (item->checkState(COLUMN_CHECKBOX) == Qt::Unchecked)              
            coinControl->UnSelect(outpt);
        else if (item->isDisabled()) // locked (this happens if "check all" through parent node)
            item->setCheckState(COLUMN_CHECKBOX, Qt::Unchecked);
        else
            coinControl->Select(outpt);
 
        // selection changed -> update labels
        if (ui->treeWidget->isEnabled()) // do not update on every click for (un)select all
            CoinControlDialog::updateLabels(model, this);
    }
}

// helper function, return human readable label for priority number
QString CoinControlDialog::getPriorityLabel(double dPriority)
{
    if (CTransaction::AllowFree(dPriority)) // at least medium
    {
        if      (CTransaction::AllowFree(dPriority / 10000))  return tr("highest");
        else if (CTransaction::AllowFree(dPriority / 1000))   return tr("high");
        else if (CTransaction::AllowFree(dPriority / 100))    return tr("medium-high");
        else                                    return tr("medium");
    }
    else
    {
        if      (CTransaction::AllowFree(dPriority * 100))    return tr("low-medium");
        else if (CTransaction::AllowFree(dPriority * 10000))  return tr("low");
        else                                    return tr("lowest");
>>>>>>> 0.8
    }
}

// shows count of locked unspent outputs
void CoinControlDialog::updateLabelLocked()
{
<<<<<<< HEAD
    std::vector<OutputIndex> vOutpts;
    model->wallet().listLockedCoins(vOutpts);
    if (vOutpts.size() > 0)
    {
       ui->labelLocked->setText(tr("(%1 locked)").arg(vOutpts.size()));
       ui->labelLocked->setVisible(true);
=======
    vector<COutPoint> vOutpts;
    model->listLockedCoins(vOutpts);
    if (vOutpts.size() > 0)
    {
       ui->labelLocked->setText(tr("(%1 locked)").arg(vOutpts.size()));
       ui->labelLocked->setVisible(true); 
>>>>>>> 0.8
    }
    else ui->labelLocked->setVisible(false);
}

<<<<<<< HEAD
void CoinControlDialog::updateLabels(CCoinControl& m_coin_control, WalletModel *model, QDialog* dialog)
{
    if (!model)
        return;

    // nPayAmount
    CAmount nPayAmount = 0;
    bool fDust = false;
    for (const CAmount &amount : CoinControlDialog::payAmounts)
    {
        nPayAmount += amount;

        if (amount > 0)
        {
            // Assumes a p2pkh script size
            CTxOut txout(amount, CScript() << std::vector<unsigned char>(24, 0));
            fDust |= IsDust(txout, model->node().getDustRelayFee());
        }
    }

    CAmount nAmount             = 0;
    CAmount nPayFee             = 0;
    CAmount nAfterFee           = 0;
    CAmount nChange             = 0;
    unsigned int nBytes         = 0;
    unsigned int nBytesInputs   = 0;
    unsigned int nQuantity      = 0;
    bool fWitness               = false;

    std::vector<OutputIndex> vCoinControl;
    m_coin_control.ListSelected(vCoinControl);

    size_t i = 0;
    for (const auto& out : model->wallet().getCoins(vCoinControl)) {
        if (out.depth_in_main_chain < 0) continue;

        // unselect already spent, very unlikely scenario, this could happen
        // when selected are spent elsewhere, like rpc or another computer
        const OutputIndex& outpt = vCoinControl[i++];
        if (out.is_spent)
        {
            m_coin_control.UnSelect(outpt);
=======
void CoinControlDialog::updateLabels(WalletModel *model, QDialog* dialog)
{
    if (!model) return;

    // nPayAmount
    qint64 nPayAmount = 0;
    bool fLowOutput = false;
    bool fDust = false;
    unsigned int nQuantityDust = 0;
    CTransaction txDummy;
    foreach(const qint64 &amount, CoinControlDialog::payAmounts)
    {
        nPayAmount += amount;
        
        if (amount > 0)
        {
            if (amount < CENT) {
                fLowOutput = true;
                nQuantityDust++;
            }

            CTxOut txout(amount, (CScript)vector<unsigned char>(24, 0));
            txDummy.vout.push_back(txout);
            if (txout.IsDust())
               fDust = true; 
        }
    }

    QString sPriorityLabel      = "";
    int64 nAmount               = 0;
    int64 nPayFee               = 0;
    int64 nAfterFee             = 0;
    int64 nChange               = 0;
    unsigned int nBytes         = 0;
    unsigned int nBytesInputs   = 0;
    double dPriority            = 0;
    double dPriorityInputs      = 0;
    unsigned int nQuantity      = 0;
    
    vector<COutPoint> vCoinControl;
    vector<COutput>   vOutputs;
    coinControl->ListSelected(vCoinControl);
    model->getOutputs(vCoinControl, vOutputs);

    BOOST_FOREACH(const COutput& out, vOutputs)
    {
        // unselect already spent, very unlikely scenario, this could happen when selected are spent elsewhere, like rpc or another computer
        if (out.tx->IsSpent(out.i))
        {
            uint256 txhash = out.tx->GetHash();
            COutPoint outpt(txhash, out.i);
            coinControl->UnSelect(outpt);
>>>>>>> 0.8
            continue;
        }

        // Quantity
        nQuantity++;
<<<<<<< HEAD

        // Amount
        nAmount += out.nValue;

        // Bytes
        if (!out.address.IsMWEB()) {
            const CScript& scriptPubKey = out.address.GetScript();
            CTxDestination address;
            int witnessversion = 0;
            std::vector<unsigned char> witnessprogram;
            if (scriptPubKey.IsWitnessProgram(witnessversion, witnessprogram)) {
                nBytesInputs += (32 + 4 + 1 + (107 / WITNESS_SCALE_FACTOR) + 4);
                fWitness = true;
            } else if (ExtractDestination(scriptPubKey, address)) {
                CPubKey pubkey;
                PKHash* pkhash = boost::get<PKHash>(&address);
                if (pkhash && model->wallet().getPubKey(scriptPubKey, ToKeyID(*pkhash), pubkey)) {
                    nBytesInputs += (pubkey.IsCompressed() ? 148 : 180);
                } else
                    nBytesInputs += 148; // in all error cases, simply assume 148 here
            } else
                nBytesInputs += 148;
        }
    }

    // calculation
    if (nQuantity > 0)
    {
        // MW: TODO - Implement byte & fee estimation for MWEB
        uint64_t mweb_weight = 0;

        // Bytes
        nBytes = nBytesInputs + ((CoinControlDialog::payAmounts.size() > 0 ? CoinControlDialog::payAmounts.size() + 1 : 2) * 34) + 10; // always assume +1 output for change here
        if (fWitness)
        {
            // there is some fudging in these numbers related to the actual virtual transaction size calculation that will keep this estimate from being exact.
            // usually, the result will be an overestimate within a couple of satoshis so that the confirmation dialog ends up displaying a slightly smaller fee.
            // also, the witness stack size value is a variable sized integer. usually, the number of stack items will be well under the single byte var int limit.
            nBytes += 2; // account for the serialized marker and flag bytes
            nBytes += nQuantity; // account for the witness byte that holds the number of stack items for each input.
        }

        // in the subtract fee from amount case, we can tell if zero change already and subtract the bytes, so that fee calculation afterwards is accurate
        if (CoinControlDialog::fSubtractFeeFromAmount)
            if (nAmount - nPayAmount == 0)
                nBytes -= 34;

        // Fee
        nPayFee = model->wallet().getMinimumFee(nBytes, mweb_weight, m_coin_control, nullptr /* returned_target */, nullptr /* reason */);

        if (nPayAmount > 0)
        {
            nChange = nAmount - nPayAmount;
            if (!CoinControlDialog::fSubtractFeeFromAmount)
                nChange -= nPayFee;

            // Never create dust outputs; if we would, just add the dust to the fee.
            if (nChange > 0 && nChange < MIN_CHANGE)
            {
                // Assumes a p2pkh script size
                CTxOut txout(nChange, CScript() << std::vector<unsigned char>(24, 0));
                if (IsDust(txout, model->node().getDustRelayFee()))
                {
                    nPayFee += nChange;
                    nChange = 0;
                    if (CoinControlDialog::fSubtractFeeFromAmount)
                        nBytes -= 34; // we didn't detect lack of change above
                }
            }

            if (nChange == 0 && !CoinControlDialog::fSubtractFeeFromAmount)
                nBytes -= 34;
        }

        // after fee
        nAfterFee = std::max<CAmount>(nAmount - nPayFee, 0);
    }

=======
            
        // Amount
        nAmount += out.tx->vout[out.i].nValue;
        
        // Priority
        dPriorityInputs += (double)out.tx->vout[out.i].nValue * (out.nDepth+1);
        
        // Bytes
        CTxDestination address;
        if(ExtractDestination(out.tx->vout[out.i].scriptPubKey, address))
        {
            CPubKey pubkey;
            CKeyID *keyid = boost::get<CKeyID>(&address);
            if (keyid && model->getPubKey(*keyid, pubkey))
                nBytesInputs += (pubkey.IsCompressed() ? 148 : 180);
            else
                nBytesInputs += 148; // in all error cases, simply assume 148 here
        }
        else nBytesInputs += 148;
    }
    
    // calculation
    if (nQuantity > 0)
    {
        // Bytes
        nBytes = nBytesInputs + ((CoinControlDialog::payAmounts.size() > 0 ? CoinControlDialog::payAmounts.size() + 1 : 2) * 34) + 10; // always assume +1 output for change here
        
        // Priority
        dPriority = dPriorityInputs / nBytes;
        sPriorityLabel = CoinControlDialog::getPriorityLabel(dPriority);
        
        // Fee
        int64 nFee = nTransactionFee * (1 + (int64)nBytes / 1000);
        
        // Min Fee
        int64 nMinFee = CTransaction::nMinTxFee * (1 + (int64)nBytes / 1000) + CTransaction::nMinTxFee * nQuantityDust;
        if (CTransaction::AllowFree(dPriority) && nBytes < 5000)
            nMinFee = 0;
        
        nPayFee = max(nFee, nMinFee);
        
        if (nPayAmount > 0)
        {
            nChange = nAmount - nPayFee - nPayAmount;
            
            // require CTransaction::nMinTxFee if any output is less than 0.01
            if (nPayFee < CTransaction::nMinTxFee && fLowOutput)
            {
                nChange = nChange + nPayFee - CTransaction::nMinTxFee;
                nPayFee = CTransaction::nMinTxFee * nQuantityDust;
            }
            
            // if sub-cent change is required, the fee must be raised to at least CTransaction::nMinTxFee   
            if (nPayFee < CTransaction::nMinTxFee && nChange > 0 && nChange < CENT)
            {
                if (nChange < CTransaction::nMinTxFee) // change < 0.0001 => simply move all change to fees
                {
                    nPayFee += nChange;
                    nChange = 0;
                }
                else
                {
                    nChange = nChange + nPayFee - CTransaction::nMinTxFee;
                    nPayFee = CTransaction::nMinTxFee;
                }  
            }
            
            // Never create dust outputs; if we would, just add the dust to the fee.
            if (nChange > 0 && nChange < CENT)
            {
                CTxOut txout(nChange, (CScript)vector<unsigned char>(24, 0));
                if (txout.IsDust())
                {
                    nPayFee += nChange;
                    nChange = 0;
                }
            }
            
            if (nChange == 0)
                nBytes -= 34;
        }
        
        // after fee
        nAfterFee = nAmount - nPayFee;
        if (nAfterFee < 0)
            nAfterFee = 0;
    }
    
>>>>>>> 0.8
    // actually update labels
    int nDisplayUnit = BitcoinUnits::BTC;
    if (model && model->getOptionsModel())
        nDisplayUnit = model->getOptionsModel()->getDisplayUnit();
<<<<<<< HEAD

=======
            
>>>>>>> 0.8
    QLabel *l1 = dialog->findChild<QLabel *>("labelCoinControlQuantity");
    QLabel *l2 = dialog->findChild<QLabel *>("labelCoinControlAmount");
    QLabel *l3 = dialog->findChild<QLabel *>("labelCoinControlFee");
    QLabel *l4 = dialog->findChild<QLabel *>("labelCoinControlAfterFee");
    QLabel *l5 = dialog->findChild<QLabel *>("labelCoinControlBytes");
<<<<<<< HEAD
    QLabel *l7 = dialog->findChild<QLabel *>("labelCoinControlLowOutput");
    QLabel *l8 = dialog->findChild<QLabel *>("labelCoinControlChange");

    // enable/disable "dust" and "change"
=======
    QLabel *l6 = dialog->findChild<QLabel *>("labelCoinControlPriority");
    QLabel *l7 = dialog->findChild<QLabel *>("labelCoinControlLowOutput");
    QLabel *l8 = dialog->findChild<QLabel *>("labelCoinControlChange");
    
    // enable/disable "low output" and "change"
>>>>>>> 0.8
    dialog->findChild<QLabel *>("labelCoinControlLowOutputText")->setEnabled(nPayAmount > 0);
    dialog->findChild<QLabel *>("labelCoinControlLowOutput")    ->setEnabled(nPayAmount > 0);
    dialog->findChild<QLabel *>("labelCoinControlChangeText")   ->setEnabled(nPayAmount > 0);
    dialog->findChild<QLabel *>("labelCoinControlChange")       ->setEnabled(nPayAmount > 0);
<<<<<<< HEAD

    // stats
    l1->setText(QString::number(nQuantity));                                 // Quantity
    l2->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nAmount));        // Amount
    l3->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nPayFee));        // Fee
    l4->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nAfterFee));      // After Fee
    l5->setText(((nBytes > 0) ? ASYMP_UTF8 : "") + QString::number(nBytes));        // Bytes
    l7->setText(fDust ? tr("yes") : tr("no"));                               // Dust
    l8->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nChange));        // Change
    if (nPayFee > 0)
    {
        l3->setText(ASYMP_UTF8 + l3->text());
        l4->setText(ASYMP_UTF8 + l4->text());
        if (nChange > 0 && !CoinControlDialog::fSubtractFeeFromAmount)
            l8->setText(ASYMP_UTF8 + l8->text());
    }

    // turn label red when dust
    l7->setStyleSheet((fDust) ? "color:red;" : "");

    // tool tips
    QString toolTipDust = tr("This label turns red if any recipient receives an amount smaller than the current dust threshold.");

    // how many satoshis the estimated fee can vary per byte we guess wrong
    double dFeeVary = (nBytes != 0) ? (double)nPayFee / nBytes : 0;

    QString toolTip4 = tr("Can vary +/- %1 satoshi(s) per input.").arg(dFeeVary);

    l3->setToolTip(toolTip4);
    l4->setToolTip(toolTip4);
    l7->setToolTip(toolTipDust);
    l8->setToolTip(toolTip4);
    dialog->findChild<QLabel *>("labelCoinControlFeeText")      ->setToolTip(l3->toolTip());
    dialog->findChild<QLabel *>("labelCoinControlAfterFeeText") ->setToolTip(l4->toolTip());
    dialog->findChild<QLabel *>("labelCoinControlBytesText")    ->setToolTip(l5->toolTip());
    dialog->findChild<QLabel *>("labelCoinControlLowOutputText")->setToolTip(l7->toolTip());
    dialog->findChild<QLabel *>("labelCoinControlChangeText")   ->setToolTip(l8->toolTip());

=======
    
    // stats
    l1->setText(QString::number(nQuantity));                                 // Quantity        
    l2->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nAmount));        // Amount
    l3->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nPayFee));        // Fee
    l4->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nAfterFee));      // After Fee
    l5->setText(((nBytes > 0) ? "~" : "") + QString::number(nBytes));        // Bytes
    l6->setText(sPriorityLabel);                                             // Priority
    l7->setText((fLowOutput ? (fDust ? tr("DUST") : tr("yes")) : tr("no"))); // Low Output / Dust
    l8->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, nChange));        // Change
    
    // turn labels "red"
    l5->setStyleSheet((nBytes >= 5000) ? "color:red;" : "");               // Bytes >= 5000
    l6->setStyleSheet((!CTransaction::AllowFree(dPriority)) ? "color:red;" : "");         // Priority < "medium"
    l7->setStyleSheet((fLowOutput) ? "color:red;" : "");                    // Low Output = "yes"
    l8->setStyleSheet((nChange > 0 && nChange < CENT) ? "color:red;" : ""); // Change < 0.01BTC
        
    // tool tips
    l5->setToolTip(tr("This label turns red, if the transaction size is bigger than 5000 bytes.\n\n This means a fee of at least %1 per kb is required.\n\n Can vary +/- 1 Byte per input.").arg(BitcoinUnits::formatWithUnit(nDisplayUnit, CTransaction::nMinTxFee)));
    l6->setToolTip(tr("Transactions with higher priority get more likely into a block.\n\nThis label turns red, if the priority is smaller than \"medium\".\n\n This means a fee of at least %1 per kb is required.").arg(BitcoinUnits::formatWithUnit(nDisplayUnit, CTransaction::nMinTxFee)));
    l7->setToolTip(tr("This label turns red, if any recipient receives an amount smaller than %1.\n\n This means a fee of at least %2 is required. \n\n Amounts below 0.546 times the minimum relay fee are shown as DUST.").arg(BitcoinUnits::formatWithUnit(nDisplayUnit, CENT)).arg(BitcoinUnits::formatWithUnit(nDisplayUnit, CTransaction::nMinTxFee)));
    l8->setToolTip(tr("This label turns red, if the change is smaller than %1.\n\n This means a fee of at least %2 is required.").arg(BitcoinUnits::formatWithUnit(nDisplayUnit, CENT)).arg(BitcoinUnits::formatWithUnit(nDisplayUnit, CTransaction::nMinTxFee)));
    dialog->findChild<QLabel *>("labelCoinControlBytesText")    ->setToolTip(l5->toolTip());
    dialog->findChild<QLabel *>("labelCoinControlPriorityText") ->setToolTip(l6->toolTip());
    dialog->findChild<QLabel *>("labelCoinControlLowOutputText")->setToolTip(l7->toolTip());
    dialog->findChild<QLabel *>("labelCoinControlChangeText")   ->setToolTip(l8->toolTip());
   
>>>>>>> 0.8
    // Insufficient funds
    QLabel *label = dialog->findChild<QLabel *>("labelCoinControlInsuffFunds");
    if (label)
        label->setVisible(nChange < 0);
}

void CoinControlDialog::updateView()
{
<<<<<<< HEAD
    if (!model || !model->getOptionsModel() || !model->getAddressTableModel())
        return;

=======
>>>>>>> 0.8
    bool treeMode = ui->radioTreeMode->isChecked();

    ui->treeWidget->clear();
    ui->treeWidget->setEnabled(false); // performance, otherwise updateLabels would be called for every checked checkbox
    ui->treeWidget->setAlternatingRowColors(!treeMode);
<<<<<<< HEAD
    QFlags<Qt::ItemFlag> flgCheckbox = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    QFlags<Qt::ItemFlag> flgTristate = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsTristate;

    int nDisplayUnit = model->getOptionsModel()->getDisplayUnit();

    for (const auto& coins : model->wallet().listCoins()) {
        CCoinControlWidgetItem* itemWalletAddress{nullptr};
        QString sWalletAddress = QString::fromStdString(EncodeDestination(coins.first));
        QString sWalletLabel = model->getAddressTableModel()->labelForAddress(sWalletAddress); // MWEB: We need to check if peg-in, change, etc.
        if (sWalletLabel.isEmpty())
            sWalletLabel = tr("(no label)");

        if (treeMode)
        {
            // wallet address
            itemWalletAddress = new CCoinControlWidgetItem(ui->treeWidget);

            itemWalletAddress->setFlags(flgTristate);
            itemWalletAddress->setCheckState(COLUMN_CHECKBOX, Qt::Unchecked);

=======
    QFlags<Qt::ItemFlag> flgCheckbox=Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    QFlags<Qt::ItemFlag> flgTristate=Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsTristate;    
    
    int nDisplayUnit = BitcoinUnits::BTC;
    if (model && model->getOptionsModel())
        nDisplayUnit = model->getOptionsModel()->getDisplayUnit();
        
    map<QString, vector<COutput> > mapCoins;
    model->listCoins(mapCoins);

    BOOST_FOREACH(PAIRTYPE(QString, vector<COutput>) coins, mapCoins)
    {
        QTreeWidgetItem *itemWalletAddress = new QTreeWidgetItem();
        QString sWalletAddress = coins.first;
        QString sWalletLabel = "";
        if (model->getAddressTableModel())
            sWalletLabel = model->getAddressTableModel()->labelForAddress(sWalletAddress);
        if (sWalletLabel.length() == 0)
            sWalletLabel = tr("(no label)");
        
        if (treeMode)
        {
            // wallet address
            ui->treeWidget->addTopLevelItem(itemWalletAddress);

            itemWalletAddress->setFlags(flgTristate);
            itemWalletAddress->setCheckState(COLUMN_CHECKBOX,Qt::Unchecked);
            
            for (int i = 0; i < ui->treeWidget->columnCount(); i++)
                itemWalletAddress->setBackground(i, QColor(248, 247, 246));
            
>>>>>>> 0.8
            // label
            itemWalletAddress->setText(COLUMN_LABEL, sWalletLabel);

            // address
            itemWalletAddress->setText(COLUMN_ADDRESS, sWalletAddress);
        }

<<<<<<< HEAD
        CAmount nSum = 0;
        int nChildren = 0;
        for (const auto& outpair : coins.second) {
            const OutputIndex& output = std::get<0>(outpair);
            const interfaces::WalletTxOut& out = std::get<1>(outpair);
            nSum += out.nValue;
            nChildren++;

            CCoinControlWidgetItem *itemOutput;
            if (treeMode)    itemOutput = new CCoinControlWidgetItem(itemWalletAddress);
            else             itemOutput = new CCoinControlWidgetItem(ui->treeWidget);
            itemOutput->setFlags(flgCheckbox);
            itemOutput->setCheckState(COLUMN_CHECKBOX,Qt::Unchecked);

            // address
            CTxDestination outputAddress;
            QString sAddress = "";
            if (out.address.IsMWEB()) {
                sAddress = QString::fromStdString(out.address.Encode());
            } else {
                // scriptpubkey index
                CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
                stream << out.address.GetScript();
                std::string script_hex = HexStr(std::vector<char>(stream.begin(), stream.end()));
                itemOutput->setData(COLUMN_ADDRESS, PubKeyRole, QString::fromStdString(script_hex));

                CTxDestination outputAddress;
                if (ExtractDestination(out.address.GetScript(), outputAddress)) {
                    sAddress = QString::fromStdString(EncodeDestination(outputAddress));
                }
            }

            // if listMode or change => show bitcoin address. In tree mode, address is not shown again for direct wallet address outputs
            if (!treeMode || (!(sAddress == sWalletAddress)))
                itemOutput->setText(COLUMN_ADDRESS, sAddress);

=======
        int64 nSum = 0;
        double dPrioritySum = 0;
        int nChildren = 0;
        int nInputSum = 0;
        BOOST_FOREACH(const COutput& out, coins.second)
        {
            int nInputSize = 148; // 180 if uncompressed public key
            nSum += out.tx->vout[out.i].nValue;
            nChildren++;
            
            QTreeWidgetItem *itemOutput;
            if (treeMode)    itemOutput = new QTreeWidgetItem(itemWalletAddress);
            else             itemOutput = new QTreeWidgetItem(ui->treeWidget);
            itemOutput->setFlags(flgCheckbox);
            itemOutput->setCheckState(COLUMN_CHECKBOX,Qt::Unchecked);
                
            // address
            CTxDestination outputAddress;
            QString sAddress = "";
            if(ExtractDestination(out.tx->vout[out.i].scriptPubKey, outputAddress))
            {
                sAddress = CBitcoinAddress(outputAddress).ToString().c_str();
                
                // if listMode or change => show bitcoin address. In tree mode, address is not shown again for direct wallet address outputs
                if (!treeMode || (!(sAddress == sWalletAddress)))
                    itemOutput->setText(COLUMN_ADDRESS, sAddress);
                    
                CPubKey pubkey;
                CKeyID *keyid = boost::get<CKeyID>(&outputAddress);
                if (keyid && model->getPubKey(*keyid, pubkey) && !pubkey.IsCompressed())
                    nInputSize = 180;
            }

>>>>>>> 0.8
            // label
            if (!(sAddress == sWalletAddress)) // change
            {
                // tooltip from where the change comes from
                itemOutput->setToolTip(COLUMN_LABEL, tr("change from %1 (%2)").arg(sWalletLabel).arg(sWalletAddress));
                itemOutput->setText(COLUMN_LABEL, tr("(change)"));
            }
            else if (!treeMode)
            {
<<<<<<< HEAD
                QString sLabel = model->getAddressTableModel()->labelForAddress(sAddress);
                if (sLabel.isEmpty())
                    sLabel = tr("(no label)");
                itemOutput->setText(COLUMN_LABEL, sLabel);
            }

            // amount
            itemOutput->setText(COLUMN_AMOUNT, BitcoinUnits::format(nDisplayUnit, out.nValue));
            itemOutput->setData(COLUMN_AMOUNT, Qt::UserRole, QVariant((qlonglong)out.nValue)); // padding so that sorting works correctly

            // date
            itemOutput->setText(COLUMN_DATE, GUIUtil::dateTimeStr(out.time));
            itemOutput->setData(COLUMN_DATE, Qt::UserRole, QVariant((qlonglong)out.time));

            // confirmations
            itemOutput->setText(COLUMN_CONFIRMATIONS, QString::number(out.depth_in_main_chain));
            itemOutput->setData(COLUMN_CONFIRMATIONS, Qt::UserRole, QVariant((qlonglong)out.depth_in_main_chain));

            if (out.output_index.type() == typeid(COutPoint)) {
                const COutPoint& outpoint = boost::get<COutPoint>(out.output_index);

                // transaction hash
                itemOutput->setData(COLUMN_ADDRESS, TxHashRole, QString::fromStdString(outpoint.hash.GetHex()));

                // vout index
                itemOutput->setData(COLUMN_ADDRESS, VOutRole, outpoint.n);
            } else {
                const mw::Hash& output_id = boost::get<mw::Hash>(out.output_index);
                itemOutput->setData(COLUMN_ADDRESS, MWEBOutRole, QString::fromStdString(output_id.ToHex()));
            }

             // disable locked coins
            if (model->wallet().isLockedCoin(output))
            {
                m_coin_control.UnSelect(output); // just to be sure
                itemOutput->setDisabled(true);
                itemOutput->setIcon(COLUMN_CHECKBOX, platformStyle->SingleColorIcon(":/icons/lock_closed"));
            }

            // set checkbox
            if (m_coin_control.IsSelected(output))
                itemOutput->setCheckState(COLUMN_CHECKBOX, Qt::Checked);
=======
                QString sLabel = "";
                if (model->getAddressTableModel())
                    sLabel = model->getAddressTableModel()->labelForAddress(sAddress);
                if (sLabel.length() == 0)
                    sLabel = tr("(no label)");
                itemOutput->setText(COLUMN_LABEL, sLabel); 
            }

            // amount
            itemOutput->setText(COLUMN_AMOUNT, BitcoinUnits::format(nDisplayUnit, out.tx->vout[out.i].nValue));
            itemOutput->setText(COLUMN_AMOUNT_INT64, strPad(QString::number(out.tx->vout[out.i].nValue), 15, " ")); // padding so that sorting works correctly

            // date
            itemOutput->setText(COLUMN_DATE, QDateTime::fromTime_t(out.tx->GetTxTime()).toString("yy-MM-dd hh:mm"));
            
            // confirmations
            itemOutput->setText(COLUMN_CONFIRMATIONS, strPad(QString::number(out.nDepth), 8, " "));
            
            // priority
            double dPriority = ((double)out.tx->vout[out.i].nValue  / (nInputSize + 78)) * (out.nDepth+1); // 78 = 2 * 34 + 10
            itemOutput->setText(COLUMN_PRIORITY, CoinControlDialog::getPriorityLabel(dPriority));
            itemOutput->setText(COLUMN_PRIORITY_INT64, strPad(QString::number((int64)dPriority), 20, " "));
            dPrioritySum += (double)out.tx->vout[out.i].nValue  * (out.nDepth+1);
            nInputSum    += nInputSize;
            
            // transaction hash
            uint256 txhash = out.tx->GetHash();
            itemOutput->setText(COLUMN_TXHASH, txhash.GetHex().c_str());
    
            // vout index
            itemOutput->setText(COLUMN_VOUT_INDEX, QString::number(out.i));
            
             // disable locked coins     
            if (model->isLockedCoin(txhash, out.i))
            {
                COutPoint outpt(txhash, out.i);
                coinControl->UnSelect(outpt); // just to be sure
                itemOutput->setDisabled(true);
                itemOutput->setIcon(COLUMN_CHECKBOX, QIcon(":/icons/lock_closed"));
            }
              
            // set checkbox
            if (coinControl->IsSelected(txhash, out.i))
                itemOutput->setCheckState(COLUMN_CHECKBOX,Qt::Checked);
>>>>>>> 0.8
        }

        // amount
        if (treeMode)
        {
<<<<<<< HEAD
            itemWalletAddress->setText(COLUMN_CHECKBOX, "(" + QString::number(nChildren) + ")");
            itemWalletAddress->setText(COLUMN_AMOUNT, BitcoinUnits::format(nDisplayUnit, nSum));
            itemWalletAddress->setData(COLUMN_AMOUNT, Qt::UserRole, QVariant((qlonglong)nSum));
        }
    }

=======
            dPrioritySum = dPrioritySum / (nInputSum + 78);
            itemWalletAddress->setText(COLUMN_CHECKBOX, "(" + QString::number(nChildren) + ")");
            itemWalletAddress->setText(COLUMN_AMOUNT, BitcoinUnits::format(nDisplayUnit, nSum));
            itemWalletAddress->setText(COLUMN_AMOUNT_INT64, strPad(QString::number(nSum), 15, " "));
            itemWalletAddress->setText(COLUMN_PRIORITY, CoinControlDialog::getPriorityLabel(dPrioritySum));
            itemWalletAddress->setText(COLUMN_PRIORITY_INT64, strPad(QString::number((int64)dPrioritySum), 20, " "));
        }
    }
    
>>>>>>> 0.8
    // expand all partially selected
    if (treeMode)
    {
        for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
            if (ui->treeWidget->topLevelItem(i)->checkState(COLUMN_CHECKBOX) == Qt::PartiallyChecked)
                ui->treeWidget->topLevelItem(i)->setExpanded(true);
    }
<<<<<<< HEAD

=======
    
>>>>>>> 0.8
    // sort view
    sortView(sortColumn, sortOrder);
    ui->treeWidget->setEnabled(true);
}
<<<<<<< HEAD

bool CoinControlDialog::IsMWEB(QTreeWidgetItem* item)
{
    return item->data(COLUMN_ADDRESS, MWEBOutRole).toString().length() == 64;
}

bool CoinControlDialog::IsCanonical(QTreeWidgetItem* item)
{
    return item->data(COLUMN_ADDRESS, TxHashRole).toString().length() == 64;
}
=======
>>>>>>> 0.8
