<<<<<<< HEAD
// Copyright (c) 2011-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_COINCONTROLTREEWIDGET_H
#define BITCOIN_QT_COINCONTROLTREEWIDGET_H
=======
#ifndef COINCONTROLTREEWIDGET_H
#define COINCONTROLTREEWIDGET_H
>>>>>>> 0.8

#include <QKeyEvent>
#include <QTreeWidget>

<<<<<<< HEAD
class CoinControlTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit CoinControlTreeWidget(QWidget *parent = nullptr);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
};

#endif // BITCOIN_QT_COINCONTROLTREEWIDGET_H
=======
class CoinControlTreeWidget : public QTreeWidget {
Q_OBJECT

public:
    explicit CoinControlTreeWidget(QWidget *parent = 0);
    
protected:
  virtual void  keyPressEvent(QKeyEvent *event);
};

#endif // COINCONTROLTREEWIDGET_H
>>>>>>> 0.8
