// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_NETBASE_H
#define BITCOIN_NETBASE_H

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <compat.h>
#include <netaddress.h>
#include <serialize.h>

#include <stdint.h>
#include <string>
#include <vector>

extern int nConnectTimeout;
extern bool fNameLookup;

//! -timeout default
static const int DEFAULT_CONNECT_TIMEOUT = 5000;
//! -dns default
static const int DEFAULT_NAME_LOOKUP = true;

class proxyType
{
public:
    proxyType(): randomize_credentials(false) {}
    explicit proxyType(const CService &_proxy, bool _randomize_credentials=false): proxy(_proxy), randomize_credentials(_randomize_credentials) {}

<<<<<<< HEAD
    bool IsValid() const { return proxy.IsValid(); }
=======
    public:
        CNetAddr();
        CNetAddr(const struct in_addr& ipv4Addr);
        explicit CNetAddr(const char *pszIp, bool fAllowLookup = false);
        explicit CNetAddr(const std::string &strIp, bool fAllowLookup = false);
        void Init();
        void SetIP(const CNetAddr& ip);
        bool SetSpecial(const std::string &strName); // for Tor addresses
        bool IsIPv4() const;    // IPv4 mapped address (::FFFF:0:0/96, 0.0.0.0/0)
        bool IsIPv6() const;    // IPv6 address (not mapped IPv4, not Tor)
        bool IsRFC1918() const; // IPv4 private networks (10.0.0.0/8, 192.168.0.0/16, 172.16.0.0/12)
        bool IsRFC3849() const; // IPv6 documentation address (2001:0DB8::/32)
        bool IsRFC3927() const; // IPv4 autoconfig (169.254.0.0/16)
        bool IsRFC3964() const; // IPv6 6to4 tunnelling (2002::/16)
        bool IsRFC4193() const; // IPv6 unique local (FC00::/7)
        bool IsRFC4380() const; // IPv6 Teredo tunnelling (2001::/32)
        bool IsRFC4843() const; // IPv6 ORCHID (2001:10::/28)
        bool IsRFC4862() const; // IPv6 autoconfig (FE80::/64)
        bool IsRFC6052() const; // IPv6 well-known prefix (64:FF9B::/96)
        bool IsRFC6145() const; // IPv6 IPv4-translated address (::FFFF:0:0:0/96)
        bool IsTor() const;
        bool IsLocal() const;
        bool IsRoutable() const;
        bool IsValid() const;
        bool IsMulticast() const;
        enum Network GetNetwork() const;
        std::string ToString() const;
        std::string ToStringIP() const;
        unsigned int GetByte(int n) const;
        uint64 GetHash() const;
        bool GetInAddr(struct in_addr* pipv4Addr) const;
        std::vector<unsigned char> GetGroup() const;
        int GetReachabilityFrom(const CNetAddr *paddrPartner = NULL) const;
        void print() const;
>>>>>>> 0.8

    CService proxy;
    bool randomize_credentials;
};

enum Network ParseNetwork(const std::string& net);
std::string GetNetworkName(enum Network net);
bool SetProxy(enum Network net, const proxyType &addrProxy);
bool GetProxy(enum Network net, proxyType &proxyInfoOut);
bool IsProxy(const CNetAddr &addr);
bool SetNameProxy(const proxyType &addrProxy);
bool HaveNameProxy();
bool GetNameProxy(proxyType &nameProxyOut);
bool LookupHost(const std::string& name, std::vector<CNetAddr>& vIP, unsigned int nMaxSolutions, bool fAllowLookup);
bool LookupHost(const std::string& name, CNetAddr& addr, bool fAllowLookup);
bool Lookup(const std::string& name, CService& addr, int portDefault, bool fAllowLookup);
bool Lookup(const std::string& name, std::vector<CService>& vAddr, int portDefault, bool fAllowLookup, unsigned int nMaxSolutions);
CService LookupNumeric(const std::string& name, int portDefault = 0);
bool LookupSubNet(const std::string& strSubnet, CSubNet& subnet);
SOCKET CreateSocket(const CService &addrConnect);
bool ConnectSocketDirectly(const CService &addrConnect, const SOCKET& hSocketRet, int nTimeout, bool manual_connection);
bool ConnectThroughProxy(const proxyType &proxy, const std::string& strDest, int port, const SOCKET& hSocketRet, int nTimeout, bool& outProxyConnectionFailed);
/** Return readable error string for a network error code */
std::string NetworkErrorString(int err);
/** Close socket and set hSocket to INVALID_SOCKET */
bool CloseSocket(SOCKET& hSocket);
/** Disable or enable blocking-mode for a socket */
bool SetSocketNonBlocking(const SOCKET& hSocket, bool fNonBlocking);
/** Set the TCP_NODELAY flag on a socket */
bool SetSocketNoDelay(const SOCKET& hSocket);
/**
 * Convert milliseconds to a struct timeval for e.g. select.
 */
struct timeval MillisToTimeval(int64_t nTimeout);
void InterruptSocks5(bool interrupt);

#endif // BITCOIN_NETBASE_H
