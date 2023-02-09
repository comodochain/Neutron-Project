// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/test/uritests.h>

#include <qt/guiutil.h>
#include <qt/walletmodel.h>

#include <QUrl>

void URITests::uriTests()
{
    SendCoinsRecipient rv;
    QUrl uri;
<<<<<<< HEAD
    uri.setUrl(QString("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?req-dontexist="));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?dontexist="));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?label=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2"));
    QVERIFY(rv.label == QString("Wikipedia Example Address"));
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?amount=0.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100000);

    uri.setUrl(QString("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?amount=1.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100100000);

    uri.setUrl(QString("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?amount=100&label=Wikipedia Example"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2"));
    QVERIFY(rv.amount == 10000000000LL);
    QVERIFY(rv.label == QString("Wikipedia Example"));

    uri.setUrl(QString("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?message=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2"));
    QVERIFY(rv.label == QString());

    QVERIFY(GUIUtil::parseBitcoinURI("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?message=Wikipedia Example Address", &rv));
    QVERIFY(rv.address == QString("LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2"));
    QVERIFY(rv.label == QString());

    uri.setUrl(QString("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?req-message=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?amount=1,000&label=Wikipedia Example"));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("litecoin:LEr4HnaeFWYhBmGxCfP2po1NPRueIk8kM2?amount=1,000.0&label=Wikipedia Example"));
=======
    uri.setUrl(QString("neutroncoin:LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?req-dontexist="));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("neutroncoin:LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?dontexist="));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("neutroncoin:LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?label=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9"));
    QVERIFY(rv.label == QString("Wikipedia Example Address"));
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("neutroncoin:LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?amount=0.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100000);

    uri.setUrl(QString("neutroncoin:LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?amount=1.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100100000);

    uri.setUrl(QString("neutroncoin:LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?amount=100&label=Wikipedia Example"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9"));
    QVERIFY(rv.amount == 10000000000LL);
    QVERIFY(rv.label == QString("Wikipedia Example"));

    uri.setUrl(QString("neutroncoin:LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?message=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9"));
    QVERIFY(rv.label == QString());

    QVERIFY(GUIUtil::parseBitcoinURI("neutroncoin://LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?message=Wikipedia Example Address", &rv));
    QVERIFY(rv.address == QString("LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9"));
    QVERIFY(rv.label == QString());

    // We currently don't implement the message parameter (ok, yea, we break spec...)
    uri.setUrl(QString("neutroncoin:LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?req-message=Wikipedia Example Address"));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("neutroncoin:LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?amount=1,000&label=Wikipedia Example"));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("neutroncoin:LQDPC5rbjDB72fGFVHu4enYhxGAZuRiFh9?amount=1,000.0&label=Wikipedia Example"));
>>>>>>> 0.8
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));
}
