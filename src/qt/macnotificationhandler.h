<<<<<<< HEAD
// Copyright (c) 2011-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_MACNOTIFICATIONHANDLER_H
#define BITCOIN_QT_MACNOTIFICATIONHANDLER_H

#include <QObject>

/** Macintosh-specific notification handler (supports UserNotificationCenter).
=======
#ifndef MACNOTIFICATIONHANDLER_H
#define MACNOTIFICATIONHANDLER_H
#include <QObject>

/** Macintosh-specific notification handler (supports UserNotificationCenter and Growl).
>>>>>>> 0.8
 */
class MacNotificationHandler : public QObject
{
    Q_OBJECT

public:
<<<<<<< HEAD
    /** shows a macOS 10.8+ UserNotification in the UserNotificationCenter
     */
    void showNotification(const QString &title, const QString &text);

    /** check if OS can handle UserNotifications */
    bool hasUserNotificationCenterSupport();
=======
    /** shows a 10.8+ UserNotification in the UserNotificationCenter
     */
    void showNotification(const QString &title, const QString &text);

    /** executes AppleScript */
    void sendAppleScript(const QString &script);

    /** check if OS can handle UserNotifications */
    bool hasUserNotificationCenterSupport(void);
>>>>>>> 0.8
    static MacNotificationHandler *instance();
};


<<<<<<< HEAD
#endif // BITCOIN_QT_MACNOTIFICATIONHANDLER_H
=======
#endif // MACNOTIFICATIONHANDLER_H
>>>>>>> 0.8
