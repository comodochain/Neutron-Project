// Copyright (c) 2009-2010 Satoshi Nakamoto
<<<<<<< HEAD
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
=======
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
>>>>>>> 0.8
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <net.h>

#include <banman.h>
#include <clientversion.h>
#include <consensus/consensus.h>
#include <crypto/sha256.h>
#include <net_permissions.h>
#include <netbase.h>
#include <node/ui_interface.h>
#include <protocol.h>
#include <random.h>
#include <scheduler.h>
#include <util/strencodings.h>
#include <util/translation.h>

#ifdef WIN32
#include <string.h>
#else
#include <fcntl.h>
#endif

#if HAVE_DECL_GETIFADDRS && HAVE_DECL_FREEIFADDRS
#include <ifaddrs.h>
#endif

#ifdef USE_POLL
#include <poll.h>
#endif

#ifdef USE_UPNP
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>
// The minimum supported miniUPnPc API version is set to 10. This keeps compatibility
// with Ubuntu 16.04 LTS and Debian 8 libminiupnpc-dev packages.
static_assert(MINIUPNPC_API_VERSION >= 10, "miniUPnPc API version >= 10 assumed");
#endif

<<<<<<< HEAD
#include <algorithm>
#include <cstdint>
#include <unordered_map>
=======
// Dump addresses to peers.dat every 15 minutes (900s)
#define DUMP_ADDRESSES_INTERVAL 900

using namespace std;
using namespace boost;
>>>>>>> 0.8

#include <math.h>

/** Maximum number of block-relay-only anchor connections */
static constexpr size_t MAX_BLOCK_RELAY_ONLY_ANCHORS = 2;
static_assert (MAX_BLOCK_RELAY_ONLY_ANCHORS <= static_cast<size_t>(MAX_BLOCK_RELAY_ONLY_CONNECTIONS), "MAX_BLOCK_RELAY_ONLY_ANCHORS must not exceed MAX_BLOCK_RELAY_ONLY_CONNECTIONS.");
/** Anchor IP address database file name */
const char* const ANCHORS_DATABASE_FILENAME = "anchors.dat";

// How often to dump addresses to peers.dat
static constexpr std::chrono::minutes DUMP_PEERS_INTERVAL{15};

/** Number of DNS seeds to query when the number of connections is low. */
static constexpr int DNSSEEDS_TO_QUERY_AT_ONCE = 3;

/** How long to delay before querying DNS seeds
 *
 * If we have more than THRESHOLD entries in addrman, then it's likely
 * that we got those addresses from having previously connected to the P2P
 * network, and that we'll be able to successfully reconnect to the P2P
 * network via contacting one of them. So if that's the case, spend a
 * little longer trying to connect to known peers before querying the
 * DNS seeds.
 */
static constexpr std::chrono::seconds DNSSEEDS_DELAY_FEW_PEERS{11};
static constexpr std::chrono::minutes DNSSEEDS_DELAY_MANY_PEERS{5};
static constexpr int DNSSEEDS_DELAY_PEER_THRESHOLD = 1000; // "many" vs "few" peers

// We add a random period time (0 to 1 seconds) to feeler connections to prevent synchronization.
#define FEELER_SLEEP_WINDOW 1

// MSG_NOSIGNAL is not available on some platforms, if it doesn't exist define it as 0
#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif

// MSG_DONTWAIT is not available on some platforms, if it doesn't exist define it as 0
#if !defined(MSG_DONTWAIT)
#define MSG_DONTWAIT 0
#endif

/** Used to pass flags to the Bind() function */
enum BindFlags {
    BF_NONE         = 0,
    BF_EXPLICIT     = (1U << 0),
    BF_REPORT_ERROR = (1U << 1),
    /**
     * Do not call AddLocal() for our special addresses, e.g., for incoming
     * Tor connections, to prevent gossiping them over the network.
     */
    BF_DONT_ADVERTISE = (1U << 2),
};

// The set of sockets cannot be modified while waiting
// The sleep time needs to be small to avoid new sockets stalling
static const uint64_t SELECT_TIMEOUT_MILLISECONDS = 50;

const std::string NET_MESSAGE_COMMAND_OTHER = "*other*";

static const uint64_t RANDOMIZER_ID_NETGROUP = 0x6c0edd8036ef4036ULL; // SHA256("netgroup")[0:8]
static const uint64_t RANDOMIZER_ID_LOCALHOSTNONCE = 0xd93e69e2bbfa5735ULL; // SHA256("localhostnonce")[0:8]
static const uint64_t RANDOMIZER_ID_ADDRCACHE = 0x1cf2e4ddd306dda9ULL; // SHA256("addrcache")[0:8]
//
// Global state variables
//
bool fDiscover = true;
bool fListen = true;
bool g_relay_txes = !DEFAULT_BLOCKSONLY;
RecursiveMutex cs_mapLocalHost;
std::map<CNetAddr, LocalServiceInfo> mapLocalHost GUARDED_BY(cs_mapLocalHost);
static bool vfLimited[NET_MAX] GUARDED_BY(cs_mapLocalHost) = {};
std::string strSubVersion;

void CConnman::AddAddrFetch(const std::string& strDest)
{
    LOCK(m_addr_fetches_mutex);
    m_addr_fetches.push_back(strDest);
}

uint16_t GetListenPort()
{
    return (uint16_t)(gArgs.GetArg("-port", Params().GetDefaultPort()));
}

// find 'best' local address for a particular peer
bool GetLocal(CService& addr, const CNetAddr *paddrPeer)
{
    if (!fListen)
        return false;

    int nBestScore = -1;
    int nBestReachability = -1;
    {
        LOCK(cs_mapLocalHost);
        for (const auto& entry : mapLocalHost)
        {
            int nScore = entry.second.nScore;
            int nReachability = entry.first.GetReachabilityFrom(paddrPeer);
            if (nReachability > nBestReachability || (nReachability == nBestReachability && nScore > nBestScore))
            {
                addr = CService(entry.first, entry.second.nPort);
                nBestReachability = nReachability;
                nBestScore = nScore;
            }
        }
    }
    return nBestScore >= 0;
}

//! Convert the serialized seeds into usable address objects.
static std::vector<CAddress> ConvertSeeds(const std::vector<uint8_t>& vSeedsIn)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7 * 24 * 60 * 60;
    std::vector<CAddress> vSeedsOut;
    FastRandomContext rng;
    CDataStream s(vSeedsIn, SER_NETWORK, PROTOCOL_VERSION | ADDRV2_FORMAT);
    while (!s.eof()) {
        CService endpoint;
        s >> endpoint;
        CAddress addr{endpoint, GetDesirableServiceFlags(NODE_NONE)};
        addr.nTime = GetTime() - rng.randrange(nOneWeek) - nOneWeek;
        LogPrint(BCLog::NET, "Added hardcoded seed: %s\n", addr.ToString());
        vSeedsOut.push_back(addr);
    }
    return vSeedsOut;
}

// get best local address for a particular peer as a CAddress
// Otherwise, return the unroutable 0.0.0.0 but filled in with
// the normal parameters, since the IP may be changed to a useful
// one by discovery.
CAddress GetLocalAddress(const CNetAddr *paddrPeer, ServiceFlags nLocalServices)
{
    CAddress ret(CService(CNetAddr(),GetListenPort()), nLocalServices);
    CService addr;
    if (GetLocal(addr, paddrPeer))
    {
        ret = CAddress(addr, nLocalServices);
    }
    ret.nTime = GetAdjustedTime();
    return ret;
}

static int GetnScore(const CService& addr)
{
    LOCK(cs_mapLocalHost);
    if (mapLocalHost.count(addr) == 0) return 0;
    return mapLocalHost[addr].nScore;
}

// Is our peer's addrLocal potentially useful as an external IP source?
bool IsPeerAddrLocalGood(CNode *pnode)
{
    CService addrLocal = pnode->GetAddrLocal();
    return fDiscover && pnode->addr.IsRoutable() && addrLocal.IsRoutable() &&
           IsReachable(addrLocal.GetNetwork());
}

// pushes our own address to a peer
void AdvertiseLocal(CNode *pnode)
{
    if (fListen && pnode->fSuccessfullyConnected)
    {
        CAddress addrLocal = GetLocalAddress(&pnode->addr, pnode->GetLocalServices());
        if (gArgs.GetBoolArg("-addrmantest", false)) {
            // use IPv4 loopback during addrmantest
            addrLocal = CAddress(CService(LookupNumeric("127.0.0.1", GetListenPort())), pnode->GetLocalServices());
        }
        // If discovery is enabled, sometimes give our peer the address it
        // tells us that it sees us as in case it has a better idea of our
        // address than we do.
        FastRandomContext rng;
        if (IsPeerAddrLocalGood(pnode) && (!addrLocal.IsRoutable() ||
             rng.randbits((GetnScore(addrLocal) > LOCAL_MANUAL) ? 3 : 1) == 0))
        {
            addrLocal.SetIP(pnode->GetAddrLocal());
        }
        if (addrLocal.IsRoutable() || gArgs.GetBoolArg("-addrmantest", false))
        {
            LogPrint(BCLog::NET, "AdvertiseLocal: advertising address %s\n", addrLocal.ToString());
            pnode->PushAddress(addrLocal, rng);
        }
    }
}

// learn a new local address
bool AddLocal(const CService& addr, int nScore)
{
    if (!addr.IsRoutable())
        return false;

    if (!fDiscover && nScore < LOCAL_MANUAL)
        return false;

    if (!IsReachable(addr))
        return false;

    LogPrintf("AddLocal(%s,%i)\n", addr.ToString(), nScore);

    {
        LOCK(cs_mapLocalHost);
        bool fAlready = mapLocalHost.count(addr) > 0;
        LocalServiceInfo &info = mapLocalHost[addr];
        if (!fAlready || nScore >= info.nScore) {
            info.nScore = nScore + (fAlready ? 1 : 0);
            info.nPort = addr.GetPort();
        }
    }

    return true;
}

bool AddLocal(const CNetAddr &addr, int nScore)
{
    return AddLocal(CService(addr, GetListenPort()), nScore);
}

void RemoveLocal(const CService& addr)
{
    LOCK(cs_mapLocalHost);
    LogPrintf("RemoveLocal(%s)\n", addr.ToString());
    mapLocalHost.erase(addr);
}

void SetReachable(enum Network net, bool reachable)
{
    if (net == NET_UNROUTABLE || net == NET_INTERNAL)
        return;
    LOCK(cs_mapLocalHost);
    vfLimited[net] = !reachable;
}

bool IsReachable(enum Network net)
{
    LOCK(cs_mapLocalHost);
    return !vfLimited[net];
}

bool IsReachable(const CNetAddr &addr)
{
    return IsReachable(addr.GetNetwork());
}

/** vote for a local address */
bool SeenLocal(const CService& addr)
{
    {
        LOCK(cs_mapLocalHost);
        if (mapLocalHost.count(addr) == 0)
            return false;
        mapLocalHost[addr].nScore++;
    }
    return true;
}


/** check whether a given address is potentially local */
bool IsLocal(const CService& addr)
{
    LOCK(cs_mapLocalHost);
    return mapLocalHost.count(addr) > 0;
}

<<<<<<< HEAD
CNode* CConnman::FindNode(const CNetAddr& ip)
=======
/** check whether a given address is in a network we can probably connect to */
bool IsReachable(const CNetAddr& addr)
{
    LOCK(cs_mapLocalHost);
    enum Network net = addr.GetNetwork();
    return vfReachable[net] && !vfLimited[net];
}

bool GetMyExternalIP2(const CService& addrConnect, const char* pszGet, const char* pszKeyword, CNetAddr& ipRet)
{
    SOCKET hSocket;
    if (!ConnectSocket(addrConnect, hSocket))
        return error("GetMyExternalIP() : connection to %s failed", addrConnect.ToString().c_str());

    send(hSocket, pszGet, strlen(pszGet), MSG_NOSIGNAL);

    string strLine;
    while (RecvLine(hSocket, strLine))
    {
        if (strLine.empty()) // HTTP response is separated from headers by blank line
        {
            loop
            {
                if (!RecvLine(hSocket, strLine))
                {
                    closesocket(hSocket);
                    return false;
                }
                if (pszKeyword == NULL)
                    break;
                if (strLine.find(pszKeyword) != string::npos)
                {
                    strLine = strLine.substr(strLine.find(pszKeyword) + strlen(pszKeyword));
                    break;
                }
            }
            closesocket(hSocket);
            if (strLine.find("<") != string::npos)
                strLine = strLine.substr(0, strLine.find("<"));
            strLine = strLine.substr(strspn(strLine.c_str(), " \t\n\r"));
            while (strLine.size() > 0 && isspace(strLine[strLine.size()-1]))
                strLine.resize(strLine.size()-1);
            CService addr(strLine,0,true);
            printf("GetMyExternalIP() received [%s] %s\n", strLine.c_str(), addr.ToString().c_str());
            if (!addr.IsValid() || !addr.IsRoutable())
                return false;
            ipRet.SetIP(addr);
            return true;
        }
    }
    closesocket(hSocket);
    return error("GetMyExternalIP() : connection closed");
}

bool GetMyExternalIP(CNetAddr& ipRet)
{
    CService addrConnect;
    const char* pszGet;
    const char* pszKeyword;

    for (int nLookup = 0; nLookup <= 1; nLookup++)
    for (int nHost = 1; nHost <= 1; nHost++)
    {
        // We should be phasing out our use of sites like these. If we need
        // replacements, we should ask for volunteers to put this simple
        // php file on their web server that prints the client IP:
        //  <?php echo $_SERVER["REMOTE_ADDR"]; ?>
        if (nHost == 1)
        {
            addrConnect = CService("91.198.22.70", 80); // checkip.dyndns.org

            if (nLookup == 1)
            {
                CService addrIP("checkip.dyndns.org", 80, true);
                if (addrIP.IsValid())
                    addrConnect = addrIP;
            }

            pszGet = "GET / HTTP/1.1\r\n"
                     "Host: checkip.dyndns.org\r\n"
                     "User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)\r\n"
                     "Connection: close\r\n"
                     "\r\n";

            pszKeyword = "Address:";
        }

        if (GetMyExternalIP2(addrConnect, pszGet, pszKeyword, ipRet))
            return true;
    }

    return false;
}

void ThreadGetMyExternalIP(void* parg)
{
    // Make this thread recognisable as the external IP detection thread
    RenameThread("bitcoin-ext-ip");

    CNetAddr addrLocalHost;
    if (GetMyExternalIP(addrLocalHost))
    {
        printf("GetMyExternalIP() returned %s\n", addrLocalHost.ToStringIP().c_str());
        AddLocal(addrLocalHost, LOCAL_HTTP);
    }
}





void AddressCurrentlyConnected(const CService& addr)
{
    addrman.Connected(addr);
}







CNode* FindNode(const CNetAddr& ip)
>>>>>>> 0.8
{
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes) {
      if (static_cast<CNetAddr>(pnode->addr) == ip) {
            return pnode;
        }
    }
    return nullptr;
}

CNode* CConnman::FindNode(const CSubNet& subNet)
{
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes) {
        if (subNet.Match(static_cast<CNetAddr>(pnode->addr))) {
            return pnode;
        }
    }
    return nullptr;
}

CNode* CConnman::FindNode(const std::string& addrName)
{
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes) {
        if (pnode->GetAddrName() == addrName) {
            return pnode;
        }
    }
    return nullptr;
}

CNode* CConnman::FindNode(const CService& addr)
{
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes) {
        if (static_cast<CService>(pnode->addr) == addr) {
            return pnode;
        }
    }
    return nullptr;
}

bool CConnman::AlreadyConnectedToAddress(const CAddress& addr)
{
    return FindNode(static_cast<CNetAddr>(addr)) || FindNode(addr.ToStringIPPort());
}

bool CConnman::CheckIncomingNonce(uint64_t nonce)
{
    LOCK(cs_vNodes);
    for (const CNode* pnode : vNodes) {
        if (!pnode->fSuccessfullyConnected && !pnode->IsInboundConn() && pnode->GetLocalNonce() == nonce)
            return false;
    }
    return true;
}

/** Get the bind address for a socket as CAddress */
static CAddress GetBindAddress(SOCKET sock)
{
    CAddress addr_bind;
    struct sockaddr_storage sockaddr_bind;
    socklen_t sockaddr_bind_len = sizeof(sockaddr_bind);
    if (sock != INVALID_SOCKET) {
        if (!getsockname(sock, (struct sockaddr*)&sockaddr_bind, &sockaddr_bind_len)) {
            addr_bind.SetSockAddr((const struct sockaddr*)&sockaddr_bind);
        } else {
            LogPrint(BCLog::NET, "Warning: getsockname failed\n");
        }
    }
    return addr_bind;
}

CNode* CConnman::ConnectNode(CAddress addrConnect, const char *pszDest, bool fCountFailure, ConnectionType conn_type)
{
    assert(conn_type != ConnectionType::INBOUND);

    if (pszDest == nullptr) {
        if (IsLocal(addrConnect))
            return nullptr;

        // Look for an existing connection
        CNode* pnode = FindNode(static_cast<CService>(addrConnect));
        if (pnode)
        {
            LogPrintf("Failed to open new connection, already connected\n");
            return nullptr;
        }
    }

    /// debug print
    LogPrint(BCLog::NET, "trying connection %s lastseen=%.1fhrs\n",
        pszDest ? pszDest : addrConnect.ToString(),
        pszDest ? 0.0 : (double)(GetAdjustedTime() - addrConnect.nTime)/3600.0);

    // Resolve
    const int default_port = Params().GetDefaultPort();
    if (pszDest) {
        std::vector<CService> resolved;
        if (Lookup(pszDest, resolved,  default_port, fNameLookup && !HaveNameProxy(), 256) && !resolved.empty()) {
            addrConnect = CAddress(resolved[GetRand(resolved.size())], NODE_NONE);
            if (!addrConnect.IsValid()) {
                LogPrint(BCLog::NET, "Resolver returned invalid address %s for %s\n", addrConnect.ToString(), pszDest);
                return nullptr;
            }
            // It is possible that we already have a connection to the IP/port pszDest resolved to.
            // In that case, drop the connection that was just created, and return the existing CNode instead.
            // Also store the name we used to connect in that CNode, so that future FindNode() calls to that
            // name catch this early.
            LOCK(cs_vNodes);
            CNode* pnode = FindNode(static_cast<CService>(addrConnect));
            if (pnode)
            {
                pnode->MaybeSetAddrName(std::string(pszDest));
                LogPrintf("Failed to open new connection, already connected\n");
                return nullptr;
            }
        }
    }

    // Connect
    bool connected = false;
    SOCKET hSocket = INVALID_SOCKET;
    proxyType proxy;
    if (addrConnect.IsValid()) {
        bool proxyConnectionFailed = false;

        if (GetProxy(addrConnect.GetNetwork(), proxy)) {
            hSocket = CreateSocket(proxy.proxy);
            if (hSocket == INVALID_SOCKET) {
                return nullptr;
            }
            connected = ConnectThroughProxy(proxy, addrConnect.ToStringIP(), addrConnect.GetPort(), hSocket, nConnectTimeout, proxyConnectionFailed);
        } else {
            // no proxy needed (none set for target network)
            hSocket = CreateSocket(addrConnect);
            if (hSocket == INVALID_SOCKET) {
                return nullptr;
            }
            connected = ConnectSocketDirectly(addrConnect, hSocket, nConnectTimeout, conn_type == ConnectionType::MANUAL);
        }
        if (!proxyConnectionFailed) {
            // If a connection to the node was attempted, and failure (if any) is not caused by a problem connecting to
            // the proxy, mark this as an attempt.
            addrman.Attempt(addrConnect, fCountFailure);
        }
    } else if (pszDest && GetNameProxy(proxy)) {
        hSocket = CreateSocket(proxy.proxy);
        if (hSocket == INVALID_SOCKET) {
            return nullptr;
        }
        std::string host;
        int port = default_port;
        SplitHostPort(std::string(pszDest), port, host);
        bool proxyConnectionFailed;
        connected = ConnectThroughProxy(proxy, host, port, hSocket, nConnectTimeout, proxyConnectionFailed);
    }
    if (!connected) {
        CloseSocket(hSocket);
        return nullptr;
    }

    // Add node
    NodeId id = GetNewNodeId();
    uint64_t nonce = GetDeterministicRandomizer(RANDOMIZER_ID_LOCALHOSTNONCE).Write(id).Finalize();
    CAddress addr_bind = GetBindAddress(hSocket);
    CNode* pnode = new CNode(id, nLocalServices, GetBestHeight(), hSocket, addrConnect, CalculateKeyedNetGroup(addrConnect), nonce, addr_bind, pszDest ? pszDest : "", conn_type);
    pnode->AddRef();

    // We're making a new connection, harvest entropy from the time (and our peer count)
    RandAddEvent((uint32_t)id);

    return pnode;
}

void CNode::CloseSocketDisconnect()
{
    fDisconnect = true;
    LOCK(cs_hSocket);
    if (hSocket != INVALID_SOCKET)
    {
        LogPrint(BCLog::NET, "disconnecting peer=%d\n", id);
        CloseSocket(hSocket);
    }
}

void CConnman::AddWhitelistPermissionFlags(NetPermissionFlags& flags, const CNetAddr &addr) const {
    for (const auto& subnet : vWhitelistedRange) {
        if (subnet.m_subnet.Match(addr)) NetPermissions::AddFlag(flags, subnet.m_flags);
    }
}

std::string CNode::ConnectionTypeAsString() const
{
    switch (m_conn_type) {
    case ConnectionType::INBOUND:
        return "inbound";
    case ConnectionType::MANUAL:
        return "manual";
    case ConnectionType::FEELER:
        return "feeler";
    case ConnectionType::OUTBOUND_FULL_RELAY:
        return "outbound-full-relay";
    case ConnectionType::BLOCK_RELAY:
        return "block-relay-only";
    case ConnectionType::ADDR_FETCH:
        return "addr-fetch";
    } // no default case, so the compiler can warn about missing cases

    assert(false);
}

std::string CNode::GetAddrName() const {
    LOCK(cs_addrName);
    return addrName;
}

void CNode::MaybeSetAddrName(const std::string& addrNameIn) {
    LOCK(cs_addrName);
    if (addrName.empty()) {
        addrName = addrNameIn;
    }
}

CService CNode::GetAddrLocal() const {
    LOCK(cs_addrLocal);
    return addrLocal;
}

void CNode::SetAddrLocal(const CService& addrLocalIn) {
    LOCK(cs_addrLocal);
    if (addrLocal.IsValid()) {
        error("Addr local already set for node: %i. Refusing to change from %s to %s", id, addrLocal.ToString(), addrLocalIn.ToString());
    } else {
        addrLocal = addrLocalIn;
    }
}

Network CNode::ConnectedThroughNetwork() const
{
    return IsInboundConn() && m_inbound_onion ? NET_ONION : addr.GetNetClass();
}

#undef X
#define X(name) stats.name = name
void CNode::copyStats(CNodeStats &stats, const std::vector<bool> &m_asmap)
{
    stats.nodeid = this->GetId();
    X(nServices);
    X(addr);
    X(addrBind);
    stats.m_network = GetNetworkName(ConnectedThroughNetwork());
    stats.m_mapped_as = addr.GetMappedAS(m_asmap);
    if (m_tx_relay != nullptr) {
        LOCK(m_tx_relay->cs_filter);
        stats.fRelayTxes = m_tx_relay->fRelayTxes;
    } else {
        stats.fRelayTxes = false;
    }
    X(nLastSend);
    X(nLastRecv);
    X(nLastTXTime);
    X(nLastBlockTime);
    X(nTimeConnected);
    X(nTimeOffset);
    stats.addrName = GetAddrName();
    X(nVersion);
<<<<<<< HEAD
    {
        LOCK(cs_SubVer);
        X(cleanSubVer);
    }
    stats.fInbound = IsInboundConn();
    stats.m_manual_connection = IsManualConn();
    X(nStartingHeight);
    {
        LOCK(cs_vSend);
        X(mapSendBytesPerMsgCmd);
        X(nSendBytes);
    }
    {
        LOCK(cs_vRecv);
        X(mapRecvBytesPerMsgCmd);
        X(nRecvBytes);
    }
    X(m_legacyWhitelisted);
    X(m_permissionFlags);
    if (m_tx_relay != nullptr) {
        LOCK(m_tx_relay->cs_feeFilter);
        stats.minFeeFilter = m_tx_relay->minFeeFilter;
    } else {
        stats.minFeeFilter = 0;
    }

    // It is common for nodes with good ping times to suddenly become lagged,
    // due to a new block arriving or other large transfer.
    // Merely reporting pingtime might fool the caller into thinking the node was still responsive,
    // since pingtime does not update until the ping is complete, which might take a while.
    // So, if a ping is taking an unusually long time in flight,
    // the caller can immediately detect that this is happening.
    std::chrono::microseconds ping_wait{0};
    if ((0 != nPingNonceSent) && (0 != m_ping_start.load().count())) {
        ping_wait = GetTime<std::chrono::microseconds>() - m_ping_start.load();
    }

    // Raw ping time is in microseconds, but show it to user as whole seconds (Bitcoin users should be well used to small numbers with many decimal places by now :)
    stats.m_ping_usec = nPingUsecTime;
    stats.m_min_ping_usec  = nMinPingUsecTime;
    stats.m_ping_wait_usec = count_microseconds(ping_wait);

    // Leave string empty if addrLocal invalid (not filled in yet)
    CService addrLocalUnlocked = GetAddrLocal();
    stats.addrLocal = addrLocalUnlocked.IsValid() ? addrLocalUnlocked.ToString() : "";

    stats.m_conn_type_string = ConnectionTypeAsString();
=======
    X(cleanSubVer);
    X(fInbound);
    X(nStartingHeight);
    X(nMisbehavior);
    X(nSendBytes);
    X(nRecvBytes);
    X(nBlocksRequested);
    stats.fSyncNode = (this == pnodeSync);
>>>>>>> 0.8
}
#undef X

/**
 * Receive bytes from the buffer and deserialize them into messages.
 *
 * @param[in]   pch         A pointer to the raw data
 * @param[in]   nBytes      Size of the data
 * @param[out]  complete    Set True if at least one message has been
 *                          deserialized and is ready to be processed
 * @return  True if the peer should stay connected,
 *          False if the peer should be disconnected from.
 */
bool CNode::ReceiveMsgBytes(const char *pch, unsigned int nBytes, bool& complete)
{
    complete = false;
    const auto time = GetTime<std::chrono::microseconds>();
    LOCK(cs_vRecv);
    nLastRecv = std::chrono::duration_cast<std::chrono::seconds>(time).count();
    nRecvBytes += nBytes;
    while (nBytes > 0) {
        // absorb network data
        int handled = m_deserializer->Read(pch, nBytes);
        if (handled < 0) {
            // Serious header problem, disconnect from the peer.
            return false;
        }

        pch += handled;
        nBytes -= handled;

        if (m_deserializer->Complete()) {
            // decompose a transport agnostic CNetMessage from the deserializer
            uint32_t out_err_raw_size{0};
            Optional<CNetMessage> result{m_deserializer->GetMessage(time, out_err_raw_size)};
            if (!result) {
                // Message deserialization failed.  Drop the message but don't disconnect the peer.
                // store the size of the corrupt message
                mapRecvBytesPerMsgCmd.find(NET_MESSAGE_COMMAND_OTHER)->second += out_err_raw_size;
                continue;
            }

            //store received bytes per message command
            //to prevent a memory DOS, only allow valid commands
            mapMsgCmdSize::iterator i = mapRecvBytesPerMsgCmd.find(result->m_command);
            if (i == mapRecvBytesPerMsgCmd.end())
                i = mapRecvBytesPerMsgCmd.find(NET_MESSAGE_COMMAND_OTHER);
            assert(i != mapRecvBytesPerMsgCmd.end());
            i->second += result->m_raw_message_size;

            // push the message to the process queue,
            vRecvMsg.push_back(std::move(*result));

            complete = true;
        }
    }

    return true;
}

int V1TransportDeserializer::readHeader(const char *pch, unsigned int nBytes)
{
    // copy data to temporary parsing buffer
    unsigned int nRemaining = CMessageHeader::HEADER_SIZE - nHdrPos;
    unsigned int nCopy = std::min(nRemaining, nBytes);

    memcpy(&hdrbuf[nHdrPos], pch, nCopy);
    nHdrPos += nCopy;

    // if header incomplete, exit
    if (nHdrPos < CMessageHeader::HEADER_SIZE)
        return nCopy;

    // deserialize to CMessageHeader
    try {
        hdrbuf >> hdr;
    }
    catch (const std::exception&) {
        LogPrint(BCLog::NET, "HEADER ERROR - UNABLE TO DESERIALIZE, peer=%d\n", m_node_id);
        return -1;
    }

    // Check start string, network magic
    if (memcmp(hdr.pchMessageStart, m_chain_params.MessageStart(), CMessageHeader::MESSAGE_START_SIZE) != 0) {
        LogPrint(BCLog::NET, "HEADER ERROR - MESSAGESTART (%s, %u bytes), received %s, peer=%d\n", hdr.GetCommand(), hdr.nMessageSize, HexStr(hdr.pchMessageStart), m_node_id);
        return -1;
    }

    // reject messages larger than MAX_SIZE or MAX_PROTOCOL_MESSAGE_LENGTH
    if (hdr.nMessageSize > MAX_SIZE || hdr.nMessageSize > MAX_PROTOCOL_MESSAGE_LENGTH) {
        LogPrint(BCLog::NET, "HEADER ERROR - SIZE (%s, %u bytes), peer=%d\n", hdr.GetCommand(), hdr.nMessageSize, m_node_id);
        return -1;
    }

    // switch state to reading message data
    in_data = true;

    return nCopy;
}

int V1TransportDeserializer::readData(const char *pch, unsigned int nBytes)
{
    unsigned int nRemaining = hdr.nMessageSize - nDataPos;
    unsigned int nCopy = std::min(nRemaining, nBytes);

    if (vRecv.size() < nDataPos + nCopy) {
        // Allocate up to 256 KiB ahead, but never more than the total message size.
        vRecv.resize(std::min(hdr.nMessageSize, nDataPos + nCopy + 256 * 1024));
    }

    hasher.Write({(const unsigned char*)pch, nCopy});
    memcpy(&vRecv[nDataPos], pch, nCopy);
    nDataPos += nCopy;

    return nCopy;
}

const uint256& V1TransportDeserializer::GetMessageHash() const
{
    assert(Complete());
    if (data_hash.IsNull())
        hasher.Finalize(data_hash);
    return data_hash;
}

Optional<CNetMessage> V1TransportDeserializer::GetMessage(const std::chrono::microseconds time, uint32_t& out_err_raw_size)
{
    // decompose a single CNetMessage from the TransportDeserializer
    Optional<CNetMessage> msg(std::move(vRecv));

    // store command string, time, and sizes
    msg->m_command = hdr.GetCommand();
    msg->m_time = time;
    msg->m_message_size = hdr.nMessageSize;
    msg->m_raw_message_size = hdr.nMessageSize + CMessageHeader::HEADER_SIZE;

    uint256 hash = GetMessageHash();

    // We just received a message off the wire, harvest entropy from the time (and the message checksum)
    RandAddEvent(ReadLE32(hash.begin()));

    // Check checksum and header command string
    if (memcmp(hash.begin(), hdr.pchChecksum, CMessageHeader::CHECKSUM_SIZE) != 0) {
        LogPrint(BCLog::NET, "CHECKSUM ERROR (%s, %u bytes), expected %s was %s, peer=%d\n",
                 SanitizeString(msg->m_command), msg->m_message_size,
                 HexStr(Span<uint8_t>(hash.begin(), hash.begin() + CMessageHeader::CHECKSUM_SIZE)),
                 HexStr(hdr.pchChecksum),
                 m_node_id);
        out_err_raw_size = msg->m_raw_message_size;
        msg = nullopt;
    } else if (!hdr.IsCommandValid()) {
        LogPrint(BCLog::NET, "HEADER ERROR - COMMAND (%s, %u bytes), peer=%d\n",
                 hdr.GetCommand(), msg->m_message_size, m_node_id);
        out_err_raw_size = msg->m_raw_message_size;
        msg = nullopt;
    }

    // Always reset the network deserializer (prepare for the next message)
    Reset();
    return msg;
}

void V1TransportSerializer::prepareForTransport(CSerializedNetMsg& msg, std::vector<unsigned char>& header) {
    // create dbl-sha256 checksum
    uint256 hash = Hash(msg.data);

    // create header
    CMessageHeader hdr(Params().MessageStart(), msg.m_type.c_str(), msg.data.size());
    memcpy(hdr.pchChecksum, hash.begin(), CMessageHeader::CHECKSUM_SIZE);

    // serialize header
    header.reserve(CMessageHeader::HEADER_SIZE);
    CVectorWriter{SER_NETWORK, INIT_PROTO_VERSION, header, 0, hdr};
}

size_t CConnman::SocketSendData(CNode *pnode) const EXCLUSIVE_LOCKS_REQUIRED(pnode->cs_vSend)
{
    auto it = pnode->vSendMsg.begin();
    size_t nSentSize = 0;

    while (it != pnode->vSendMsg.end()) {
        const auto &data = *it;
        assert(data.size() > pnode->nSendOffset);
        int nBytes = 0;
        {
            LOCK(pnode->cs_hSocket);
            if (pnode->hSocket == INVALID_SOCKET)
                break;
            nBytes = send(pnode->hSocket, reinterpret_cast<const char*>(data.data()) + pnode->nSendOffset, data.size() - pnode->nSendOffset, MSG_NOSIGNAL | MSG_DONTWAIT);
        }
        if (nBytes > 0) {
            pnode->nLastSend = GetSystemTimeInSeconds();
            pnode->nSendBytes += nBytes;
            pnode->nSendOffset += nBytes;
            nSentSize += nBytes;
            if (pnode->nSendOffset == data.size()) {
                pnode->nSendOffset = 0;
                pnode->nSendSize -= data.size();
                pnode->fPauseSend = pnode->nSendSize > nSendBufferMaxSize;
                it++;
            } else {
                // could not send full message; stop sending more
                break;
            }
        } else {
            if (nBytes < 0) {
                // error
                int nErr = WSAGetLastError();
                if (nErr != WSAEWOULDBLOCK && nErr != WSAEMSGSIZE && nErr != WSAEINTR && nErr != WSAEINPROGRESS)
                {
                    LogPrintf("socket send error %s\n", NetworkErrorString(nErr));
                    pnode->CloseSocketDisconnect();
                }
            }
            // couldn't send anything at all
            break;
        }
    }

    if (it == pnode->vSendMsg.end()) {
        assert(pnode->nSendOffset == 0);
        assert(pnode->nSendSize == 0);
    }
    pnode->vSendMsg.erase(pnode->vSendMsg.begin(), it);
    return nSentSize;
}

struct NodeEvictionCandidate
{
    NodeId id;
    int64_t nTimeConnected;
    int64_t nMinPingUsecTime;
    int64_t nLastBlockTime;
    int64_t nLastTXTime;
    bool fRelevantServices;
    bool fRelayTxes;
    bool fBloomFilter;
    uint64_t nKeyedNetGroup;
    bool prefer_evict;
    bool m_is_local;
};

static bool ReverseCompareNodeMinPingTime(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b)
{
    return a.nMinPingUsecTime > b.nMinPingUsecTime;
}

static bool ReverseCompareNodeTimeConnected(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b)
{
    return a.nTimeConnected > b.nTimeConnected;
}

static bool CompareLocalHostTimeConnected(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b)
{
    if (a.m_is_local != b.m_is_local) return b.m_is_local;
    return a.nTimeConnected > b.nTimeConnected;
}

static bool CompareNetGroupKeyed(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b) {
    return a.nKeyedNetGroup < b.nKeyedNetGroup;
}

static bool CompareNodeBlockTime(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b)
{
    // There is a fall-through here because it is common for a node to have many peers which have not yet relayed a block.
    if (a.nLastBlockTime != b.nLastBlockTime) return a.nLastBlockTime < b.nLastBlockTime;
    if (a.fRelevantServices != b.fRelevantServices) return b.fRelevantServices;
    return a.nTimeConnected > b.nTimeConnected;
}

static bool CompareNodeTXTime(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b)
{
    // There is a fall-through here because it is common for a node to have more than a few peers that have not yet relayed txn.
    if (a.nLastTXTime != b.nLastTXTime) return a.nLastTXTime < b.nLastTXTime;
    if (a.fRelayTxes != b.fRelayTxes) return b.fRelayTxes;
    if (a.fBloomFilter != b.fBloomFilter) return a.fBloomFilter;
    return a.nTimeConnected > b.nTimeConnected;
}

// Pick out the potential block-relay only peers, and sort them by last block time.
static bool CompareNodeBlockRelayOnlyTime(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b)
{
    if (a.fRelayTxes != b.fRelayTxes) return a.fRelayTxes;
    if (a.nLastBlockTime != b.nLastBlockTime) return a.nLastBlockTime < b.nLastBlockTime;
    if (a.fRelevantServices != b.fRelevantServices) return b.fRelevantServices;
    return a.nTimeConnected > b.nTimeConnected;
}

//! Sort an array by the specified comparator, then erase the last K elements.
template<typename T, typename Comparator>
static void EraseLastKElements(std::vector<T> &elements, Comparator comparator, size_t k)
{
    std::sort(elements.begin(), elements.end(), comparator);
    size_t eraseSize = std::min(k, elements.size());
    elements.erase(elements.end() - eraseSize, elements.end());
}

/** Try to find a connection to evict when the node is full.
 *  Extreme care must be taken to avoid opening the node to attacker
 *   triggered network partitioning.
 *  The strategy used here is to protect a small number of peers
 *   for each of several distinct characteristics which are difficult
 *   to forge.  In order to partition a node the attacker must be
 *   simultaneously better at all of them than honest peers.
 */
bool CConnman::AttemptToEvictConnection()
{
    std::vector<NodeEvictionCandidate> vEvictionCandidates;
    {
        LOCK(cs_vNodes);

        for (const CNode* node : vNodes) {
            if (node->HasPermission(PF_NOBAN))
                continue;
            if (!node->IsInboundConn())
                continue;
            if (node->fDisconnect)
                continue;
            bool peer_relay_txes = false;
            bool peer_filter_not_null = false;
            if (node->m_tx_relay != nullptr) {
                LOCK(node->m_tx_relay->cs_filter);
                peer_relay_txes = node->m_tx_relay->fRelayTxes;
                peer_filter_not_null = node->m_tx_relay->pfilter != nullptr;
            }
            NodeEvictionCandidate candidate = {node->GetId(), node->nTimeConnected, node->nMinPingUsecTime,
                                               node->nLastBlockTime, node->nLastTXTime,
                                               HasAllDesirableServiceFlags(node->nServices),
                                               peer_relay_txes, peer_filter_not_null, node->nKeyedNetGroup,
                                               node->m_prefer_evict, node->addr.IsLocal()};
            vEvictionCandidates.push_back(candidate);
        }
    }

    // Protect connections with certain characteristics

    // Deterministically select 4 peers to protect by netgroup.
    // An attacker cannot predict which netgroups will be protected
    EraseLastKElements(vEvictionCandidates, CompareNetGroupKeyed, 4);
    // Protect the 8 nodes with the lowest minimum ping time.
    // An attacker cannot manipulate this metric without physically moving nodes closer to the target.
    EraseLastKElements(vEvictionCandidates, ReverseCompareNodeMinPingTime, 8);
    // Protect 4 nodes that most recently sent us novel transactions accepted into our mempool.
    // An attacker cannot manipulate this metric without performing useful work.
    EraseLastKElements(vEvictionCandidates, CompareNodeTXTime, 4);
    // Protect up to 8 non-tx-relay peers that have sent us novel blocks.
    std::sort(vEvictionCandidates.begin(), vEvictionCandidates.end(), CompareNodeBlockRelayOnlyTime);
    size_t erase_size = std::min(size_t(8), vEvictionCandidates.size());
    vEvictionCandidates.erase(std::remove_if(vEvictionCandidates.end() - erase_size, vEvictionCandidates.end(), [](NodeEvictionCandidate const &n) { return !n.fRelayTxes && n.fRelevantServices; }), vEvictionCandidates.end());

    // Protect 4 nodes that most recently sent us novel blocks.
    // An attacker cannot manipulate this metric without performing useful work.
    EraseLastKElements(vEvictionCandidates, CompareNodeBlockTime, 4);

    // Protect the half of the remaining nodes which have been connected the longest.
    // This replicates the non-eviction implicit behavior, and precludes attacks that start later.
    // Reserve half of these protected spots for localhost peers, even if
    // they're not longest-uptime overall. This helps protect tor peers, which
    // tend to be otherwise disadvantaged under our eviction criteria.
    size_t initial_size = vEvictionCandidates.size();
    size_t total_protect_size = initial_size / 2;

    // Pick out up to 1/4 peers that are localhost, sorted by longest uptime.
    std::sort(vEvictionCandidates.begin(), vEvictionCandidates.end(), CompareLocalHostTimeConnected);
    size_t local_erase_size = total_protect_size / 2;
    vEvictionCandidates.erase(std::remove_if(vEvictionCandidates.end() - local_erase_size, vEvictionCandidates.end(), [](NodeEvictionCandidate const &n) { return n.m_is_local; }), vEvictionCandidates.end());
    // Calculate how many we removed, and update our total number of peers that
    // we want to protect based on uptime accordingly.
    total_protect_size -= initial_size - vEvictionCandidates.size();
    EraseLastKElements(vEvictionCandidates, ReverseCompareNodeTimeConnected, total_protect_size);

    if (vEvictionCandidates.empty()) return false;

    // If any remaining peers are preferred for eviction consider only them.
    // This happens after the other preferences since if a peer is really the best by other criteria (esp relaying blocks)
    //  then we probably don't want to evict it no matter what.
    if (std::any_of(vEvictionCandidates.begin(),vEvictionCandidates.end(),[](NodeEvictionCandidate const &n){return n.prefer_evict;})) {
        vEvictionCandidates.erase(std::remove_if(vEvictionCandidates.begin(),vEvictionCandidates.end(),
                                  [](NodeEvictionCandidate const &n){return !n.prefer_evict;}),vEvictionCandidates.end());
    }

    // Identify the network group with the most connections and youngest member.
    // (vEvictionCandidates is already sorted by reverse connect time)
    uint64_t naMostConnections;
    unsigned int nMostConnections = 0;
    int64_t nMostConnectionsTime = 0;
    std::map<uint64_t, std::vector<NodeEvictionCandidate> > mapNetGroupNodes;
    for (const NodeEvictionCandidate &node : vEvictionCandidates) {
        std::vector<NodeEvictionCandidate> &group = mapNetGroupNodes[node.nKeyedNetGroup];
        group.push_back(node);
        int64_t grouptime = group[0].nTimeConnected;

        if (group.size() > nMostConnections || (group.size() == nMostConnections && grouptime > nMostConnectionsTime)) {
            nMostConnections = group.size();
            nMostConnectionsTime = grouptime;
            naMostConnections = node.nKeyedNetGroup;
        }
    }

    // Reduce to the network group with the most connections
    vEvictionCandidates = std::move(mapNetGroupNodes[naMostConnections]);

    // Disconnect from the network group with the most connections
    NodeId evicted = vEvictionCandidates.front().id;
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes) {
        if (pnode->GetId() == evicted) {
            pnode->fDisconnect = true;
            return true;
        }
    }
    return false;
}

void CConnman::AcceptConnection(const ListenSocket& hListenSocket) {
    struct sockaddr_storage sockaddr;
    socklen_t len = sizeof(sockaddr);
    SOCKET hSocket = accept(hListenSocket.socket, (struct sockaddr*)&sockaddr, &len);
    CAddress addr;
    int nInbound = 0;
    int nMaxInbound = nMaxConnections - m_max_outbound;

    if (hSocket != INVALID_SOCKET) {
        if (!addr.SetSockAddr((const struct sockaddr*)&sockaddr)) {
            LogPrintf("Warning: Unknown socket family\n");
        }
    }

    NetPermissionFlags permissionFlags = NetPermissionFlags::PF_NONE;
    hListenSocket.AddSocketPermissionFlags(permissionFlags);
    AddWhitelistPermissionFlags(permissionFlags, addr);
    bool legacyWhitelisted = false;
    if (NetPermissions::HasFlag(permissionFlags, NetPermissionFlags::PF_ISIMPLICIT)) {
        NetPermissions::ClearFlag(permissionFlags, PF_ISIMPLICIT);
        if (gArgs.GetBoolArg("-whitelistforcerelay", DEFAULT_WHITELISTFORCERELAY)) NetPermissions::AddFlag(permissionFlags, PF_FORCERELAY);
        if (gArgs.GetBoolArg("-whitelistrelay", DEFAULT_WHITELISTRELAY)) NetPermissions::AddFlag(permissionFlags, PF_RELAY);
        NetPermissions::AddFlag(permissionFlags, PF_MEMPOOL);
        NetPermissions::AddFlag(permissionFlags, PF_NOBAN);
        legacyWhitelisted = true;
    }

    {
        LOCK(cs_vNodes);
        for (const CNode* pnode : vNodes) {
            if (pnode->IsInboundConn()) nInbound++;
        }
    }

    if (hSocket == INVALID_SOCKET)
    {
        int nErr = WSAGetLastError();
        if (nErr != WSAEWOULDBLOCK)
            LogPrintf("socket error accept failed: %s\n", NetworkErrorString(nErr));
        return;
    }

    if (!fNetworkActive) {
        LogPrintf("connection from %s dropped: not accepting new connections\n", addr.ToString());
        CloseSocket(hSocket);
        return;
    }

    if (!IsSelectableSocket(hSocket))
    {
        LogPrintf("connection from %s dropped: non-selectable socket\n", addr.ToString());
        CloseSocket(hSocket);
        return;
    }

    // According to the internet TCP_NODELAY is not carried into accepted sockets
    // on all platforms.  Set it again here just to be sure.
    SetSocketNoDelay(hSocket);

    // Don't accept connections from banned peers.
    bool banned = m_banman && m_banman->IsBanned(addr);
    if (!NetPermissions::HasFlag(permissionFlags, NetPermissionFlags::PF_NOBAN) && banned)
    {
        LogPrint(BCLog::NET, "connection from %s dropped (banned)\n", addr.ToString());
        CloseSocket(hSocket);
        return;
    }

    // Only accept connections from discouraged peers if our inbound slots aren't (almost) full.
    bool discouraged = m_banman && m_banman->IsDiscouraged(addr);
    if (!NetPermissions::HasFlag(permissionFlags, NetPermissionFlags::PF_NOBAN) && nInbound + 1 >= nMaxInbound && discouraged)
    {
        LogPrint(BCLog::NET, "connection from %s dropped (discouraged)\n", addr.ToString());
        CloseSocket(hSocket);
        return;
    }

    if (nInbound >= nMaxInbound)
    {
        if (!AttemptToEvictConnection()) {
            // No connection to evict, disconnect the new connection
            LogPrint(BCLog::NET, "failed to find an eviction candidate - connection dropped (full)\n");
            CloseSocket(hSocket);
            return;
        }
    }

    NodeId id = GetNewNodeId();
    uint64_t nonce = GetDeterministicRandomizer(RANDOMIZER_ID_LOCALHOSTNONCE).Write(id).Finalize();
    CAddress addr_bind = GetBindAddress(hSocket);

    ServiceFlags nodeServices = nLocalServices;
    if (NetPermissions::HasFlag(permissionFlags, PF_BLOOMFILTER)) {
        nodeServices = static_cast<ServiceFlags>(nodeServices | NODE_BLOOM);
    }

    const bool inbound_onion = std::find(m_onion_binds.begin(), m_onion_binds.end(), addr_bind) != m_onion_binds.end();
    CNode* pnode = new CNode(id, nodeServices, GetBestHeight(), hSocket, addr, CalculateKeyedNetGroup(addr), nonce, addr_bind, "", ConnectionType::INBOUND, inbound_onion);
    pnode->AddRef();
    pnode->m_permissionFlags = permissionFlags;
    // If this flag is present, the user probably expect that RPC and QT report it as whitelisted (backward compatibility)
    pnode->m_legacyWhitelisted = legacyWhitelisted;
    pnode->m_prefer_evict = discouraged;
    m_msgproc->InitializeNode(pnode);

    LogPrint(BCLog::NET, "connection from %s accepted\n", addr.ToString());

    {
        LOCK(cs_vNodes);
        vNodes.push_back(pnode);
    }

    // We received a new connection, harvest entropy from the time (and our peer count)
    RandAddEvent((uint32_t)id);
}

void CConnman::DisconnectNodes()
{
    {
        LOCK(cs_vNodes);

        if (!fNetworkActive) {
            // Disconnect any connected nodes
            for (CNode* pnode : vNodes) {
                if (!pnode->fDisconnect) {
                    LogPrint(BCLog::NET, "Network not active, dropping peer=%d\n", pnode->GetId());
                    pnode->fDisconnect = true;
                }
            }
        }

        // Disconnect unused nodes
        std::vector<CNode*> vNodesCopy = vNodes;
        for (CNode* pnode : vNodesCopy)
        {
            if (pnode->fDisconnect)
            {
                // remove from vNodes
                vNodes.erase(remove(vNodes.begin(), vNodes.end(), pnode), vNodes.end());

                // release outbound grant (if any)
                pnode->grantOutbound.Release();

                // close socket and cleanup
                pnode->CloseSocketDisconnect();

                // hold in disconnected pool until all refs are released
                pnode->Release();
                vNodesDisconnected.push_back(pnode);
            }
        }
    }
    {
        // Delete disconnected nodes
        std::list<CNode*> vNodesDisconnectedCopy = vNodesDisconnected;
        for (CNode* pnode : vNodesDisconnectedCopy)
        {
            // wait until threads are done using it
            if (pnode->GetRefCount() <= 0) {
                bool fDelete = false;
                {
                    TRY_LOCK(pnode->cs_vSend, lockSend);
                    if (lockSend) {
                        fDelete = true;
                    }
                }
                if (fDelete) {
                    vNodesDisconnected.remove(pnode);
                    DeleteNode(pnode);
                }
            }
        }
    }
}

void CConnman::NotifyNumConnectionsChanged()
{
    size_t vNodesSize;
    {
        LOCK(cs_vNodes);
        vNodesSize = vNodes.size();
    }
    if(vNodesSize != nPrevNodeCount) {
        nPrevNodeCount = vNodesSize;
        if(clientInterface)
            clientInterface->NotifyNumConnectionsChanged(vNodesSize);
    }
}

void CConnman::InactivityCheck(CNode *pnode)
{
    int64_t nTime = GetSystemTimeInSeconds();
    if (nTime - pnode->nTimeConnected > m_peer_connect_timeout)
    {
        if (pnode->nLastRecv == 0 || pnode->nLastSend == 0)
        {
            LogPrint(BCLog::NET, "socket no message in first %i seconds, %d %d from %d\n", m_peer_connect_timeout, pnode->nLastRecv != 0, pnode->nLastSend != 0, pnode->GetId());
            pnode->fDisconnect = true;
        }
        else if (nTime - pnode->nLastSend > TIMEOUT_INTERVAL)
        {
            LogPrintf("socket sending timeout: %is\n", nTime - pnode->nLastSend);
            pnode->fDisconnect = true;
        }
        else if (nTime - pnode->nLastRecv > (pnode->GetCommonVersion() > BIP0031_VERSION ? TIMEOUT_INTERVAL : 90*60))
        {
            LogPrintf("socket receive timeout: %is\n", nTime - pnode->nLastRecv);
            pnode->fDisconnect = true;
        }
        else if (pnode->nPingNonceSent && pnode->m_ping_start.load() + std::chrono::seconds{TIMEOUT_INTERVAL} < GetTime<std::chrono::microseconds>())
        {
            LogPrintf("ping timeout: %fs\n", 0.000001 * count_microseconds(GetTime<std::chrono::microseconds>() - pnode->m_ping_start.load()));
            pnode->fDisconnect = true;
        }
        else if (!pnode->fSuccessfullyConnected)
        {
            LogPrint(BCLog::NET, "version handshake timeout from %d\n", pnode->GetId());
            pnode->fDisconnect = true;
        }
    }
}

bool CConnman::GenerateSelectSet(std::set<SOCKET> &recv_set, std::set<SOCKET> &send_set, std::set<SOCKET> &error_set)
{
    for (const ListenSocket& hListenSocket : vhListenSocket) {
        recv_set.insert(hListenSocket.socket);
    }

    {
        LOCK(cs_vNodes);
        for (CNode* pnode : vNodes)
        {
            // Implement the following logic:
            // * If there is data to send, select() for sending data. As this only
            //   happens when optimistic write failed, we choose to first drain the
            //   write buffer in this case before receiving more. This avoids
            //   needlessly queueing received data, if the remote peer is not themselves
            //   receiving data. This means properly utilizing TCP flow control signalling.
            // * Otherwise, if there is space left in the receive buffer, select() for
            //   receiving data.
            // * Hand off all complete messages to the processor, to be handled without
            //   blocking here.

            bool select_recv = !pnode->fPauseRecv;
            bool select_send;
            {
                LOCK(pnode->cs_vSend);
                select_send = !pnode->vSendMsg.empty();
            }

            LOCK(pnode->cs_hSocket);
            if (pnode->hSocket == INVALID_SOCKET)
                continue;

            error_set.insert(pnode->hSocket);
            if (select_send) {
                send_set.insert(pnode->hSocket);
                continue;
            }
            if (select_recv) {
                recv_set.insert(pnode->hSocket);
            }
        }
    }

    return !recv_set.empty() || !send_set.empty() || !error_set.empty();
}

#ifdef USE_POLL
void CConnman::SocketEvents(std::set<SOCKET> &recv_set, std::set<SOCKET> &send_set, std::set<SOCKET> &error_set)
{
    std::set<SOCKET> recv_select_set, send_select_set, error_select_set;
    if (!GenerateSelectSet(recv_select_set, send_select_set, error_select_set)) {
        interruptNet.sleep_for(std::chrono::milliseconds(SELECT_TIMEOUT_MILLISECONDS));
        return;
    }

    std::unordered_map<SOCKET, struct pollfd> pollfds;
    for (SOCKET socket_id : recv_select_set) {
        pollfds[socket_id].fd = socket_id;
        pollfds[socket_id].events |= POLLIN;
    }

    for (SOCKET socket_id : send_select_set) {
        pollfds[socket_id].fd = socket_id;
        pollfds[socket_id].events |= POLLOUT;
    }

    for (SOCKET socket_id : error_select_set) {
        pollfds[socket_id].fd = socket_id;
        // These flags are ignored, but we set them for clarity
        pollfds[socket_id].events |= POLLERR|POLLHUP;
    }

    std::vector<struct pollfd> vpollfds;
    vpollfds.reserve(pollfds.size());
    for (auto it : pollfds) {
        vpollfds.push_back(std::move(it.second));
    }

    if (poll(vpollfds.data(), vpollfds.size(), SELECT_TIMEOUT_MILLISECONDS) < 0) return;

    if (interruptNet) return;

    for (struct pollfd pollfd_entry : vpollfds) {
        if (pollfd_entry.revents & POLLIN)            recv_set.insert(pollfd_entry.fd);
        if (pollfd_entry.revents & POLLOUT)           send_set.insert(pollfd_entry.fd);
        if (pollfd_entry.revents & (POLLERR|POLLHUP)) error_set.insert(pollfd_entry.fd);
    }
}
#else
void CConnman::SocketEvents(std::set<SOCKET> &recv_set, std::set<SOCKET> &send_set, std::set<SOCKET> &error_set)
{
    std::set<SOCKET> recv_select_set, send_select_set, error_select_set;
    if (!GenerateSelectSet(recv_select_set, send_select_set, error_select_set)) {
        interruptNet.sleep_for(std::chrono::milliseconds(SELECT_TIMEOUT_MILLISECONDS));
        return;
    }

    //
    // Find which sockets have data to receive
    //
    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = SELECT_TIMEOUT_MILLISECONDS * 1000; // frequency to poll pnode->vSend

    fd_set fdsetRecv;
    fd_set fdsetSend;
    fd_set fdsetError;
    FD_ZERO(&fdsetRecv);
    FD_ZERO(&fdsetSend);
    FD_ZERO(&fdsetError);
    SOCKET hSocketMax = 0;

    for (SOCKET hSocket : recv_select_set) {
        FD_SET(hSocket, &fdsetRecv);
        hSocketMax = std::max(hSocketMax, hSocket);
    }

    for (SOCKET hSocket : send_select_set) {
        FD_SET(hSocket, &fdsetSend);
        hSocketMax = std::max(hSocketMax, hSocket);
    }

    for (SOCKET hSocket : error_select_set) {
        FD_SET(hSocket, &fdsetError);
        hSocketMax = std::max(hSocketMax, hSocket);
    }

    int nSelect = select(hSocketMax + 1, &fdsetRecv, &fdsetSend, &fdsetError, &timeout);

    if (interruptNet)
        return;

    if (nSelect == SOCKET_ERROR)
    {
        int nErr = WSAGetLastError();
        LogPrintf("socket select error %s\n", NetworkErrorString(nErr));
        for (unsigned int i = 0; i <= hSocketMax; i++)
            FD_SET(i, &fdsetRecv);
        FD_ZERO(&fdsetSend);
        FD_ZERO(&fdsetError);
        if (!interruptNet.sleep_for(std::chrono::milliseconds(SELECT_TIMEOUT_MILLISECONDS)))
            return;
    }

    for (SOCKET hSocket : recv_select_set) {
        if (FD_ISSET(hSocket, &fdsetRecv)) {
            recv_set.insert(hSocket);
        }
    }

    for (SOCKET hSocket : send_select_set) {
        if (FD_ISSET(hSocket, &fdsetSend)) {
            send_set.insert(hSocket);
        }
    }

    for (SOCKET hSocket : error_select_set) {
        if (FD_ISSET(hSocket, &fdsetError)) {
            error_set.insert(hSocket);
        }
    }
}
#endif

void CConnman::SocketHandler()
{
    std::set<SOCKET> recv_set, send_set, error_set;
    SocketEvents(recv_set, send_set, error_set);

    if (interruptNet) return;

    //
    // Accept new connections
    //
    for (const ListenSocket& hListenSocket : vhListenSocket)
    {
        if (hListenSocket.socket != INVALID_SOCKET && recv_set.count(hListenSocket.socket) > 0)
        {
            AcceptConnection(hListenSocket);
        }
    }

    //
    // Service each socket
    //
    std::vector<CNode*> vNodesCopy;
    {
        LOCK(cs_vNodes);
        vNodesCopy = vNodes;
        for (CNode* pnode : vNodesCopy)
            pnode->AddRef();
    }
    for (CNode* pnode : vNodesCopy)
    {
        if (interruptNet)
            return;

        //
        // Receive
        //
        bool recvSet = false;
        bool sendSet = false;
        bool errorSet = false;
        {
            LOCK(pnode->cs_hSocket);
            if (pnode->hSocket == INVALID_SOCKET)
                continue;
            recvSet = recv_set.count(pnode->hSocket) > 0;
            sendSet = send_set.count(pnode->hSocket) > 0;
            errorSet = error_set.count(pnode->hSocket) > 0;
        }
        if (recvSet || errorSet)
        {
            // typical socket buffer is 8K-64K
            char pchBuf[0x10000];
            int nBytes = 0;
            {
                LOCK(pnode->cs_hSocket);
                if (pnode->hSocket == INVALID_SOCKET)
                    continue;
                nBytes = recv(pnode->hSocket, pchBuf, sizeof(pchBuf), MSG_DONTWAIT);
            }
            if (nBytes > 0)
            {
                bool notify = false;
                if (!pnode->ReceiveMsgBytes(pchBuf, nBytes, notify))
                    pnode->CloseSocketDisconnect();
                RecordBytesRecv(nBytes);
                if (notify) {
                    size_t nSizeAdded = 0;
                    auto it(pnode->vRecvMsg.begin());
                    for (; it != pnode->vRecvMsg.end(); ++it) {
                        // vRecvMsg contains only completed CNetMessage
                        // the single possible partially deserialized message are held by TransportDeserializer
                        nSizeAdded += it->m_raw_message_size;
                    }
                    {
                        LOCK(pnode->cs_vProcessMsg);
                        pnode->vProcessMsg.splice(pnode->vProcessMsg.end(), pnode->vRecvMsg, pnode->vRecvMsg.begin(), it);
                        pnode->nProcessQueueSize += nSizeAdded;
                        pnode->fPauseRecv = pnode->nProcessQueueSize > nReceiveFloodSize;
                    }
                    WakeMessageHandler();
                }
            }
            else if (nBytes == 0)
            {
                // socket closed gracefully
                if (!pnode->fDisconnect) {
                    LogPrint(BCLog::NET, "socket closed for peer=%d\n", pnode->GetId());
                }
                pnode->CloseSocketDisconnect();
            }
            else if (nBytes < 0)
            {
                // error
                int nErr = WSAGetLastError();
                if (nErr != WSAEWOULDBLOCK && nErr != WSAEMSGSIZE && nErr != WSAEINTR && nErr != WSAEINPROGRESS)
                {
                    if (!pnode->fDisconnect) {
                        LogPrint(BCLog::NET, "socket recv error for peer=%d: %s\n", pnode->GetId(), NetworkErrorString(nErr));
                    }
                    pnode->CloseSocketDisconnect();
                }
            }
        }

        //
        // Send
        //
        if (sendSet)
        {
            LOCK(pnode->cs_vSend);
            size_t nBytes = SocketSendData(pnode);
            if (nBytes) {
                RecordBytesSent(nBytes);
            }
        }

        InactivityCheck(pnode);
    }
    {
        LOCK(cs_vNodes);
        for (CNode* pnode : vNodesCopy)
            pnode->Release();
    }
}

void CConnman::ThreadSocketHandler()
{
    while (!interruptNet)
    {
        DisconnectNodes();
        NotifyNumConnectionsChanged();
        SocketHandler();
    }
}

void CConnman::WakeMessageHandler()
{
    {
        LOCK(mutexMsgProc);
        fMsgProcWake = true;
    }
    condMsgProc.notify_one();
}






#ifdef USE_UPNP
static CThreadInterrupt g_upnp_interrupt;
static std::thread g_upnp_thread;
static void ThreadMapPort()
{
    std::string port = strprintf("%u", GetListenPort());
    const char * multicastif = nullptr;
    const char * minissdpdpath = nullptr;
    struct UPNPDev * devlist = nullptr;
    char lanaddr[64];

    int error = 0;
#if MINIUPNPC_API_VERSION < 14
    devlist = upnpDiscover(2000, multicastif, minissdpdpath, 0, 0, &error);
#else
    devlist = upnpDiscover(2000, multicastif, minissdpdpath, 0, 0, 2, &error);
#endif

    struct UPNPUrls urls;
    struct IGDdatas data;
    int r;

    r = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr));
    if (r == 1)
    {
        if (fDiscover) {
            char externalIPAddress[40];
            r = UPNP_GetExternalIPAddress(urls.controlURL, data.first.servicetype, externalIPAddress);
            if (r != UPNPCOMMAND_SUCCESS) {
                LogPrintf("UPnP: GetExternalIPAddress() returned %d\n", r);
            } else {
                if (externalIPAddress[0]) {
                    CNetAddr resolved;
                    if (LookupHost(externalIPAddress, resolved, false)) {
                        LogPrintf("UPnP: ExternalIPAddress = %s\n", resolved.ToString());
                        AddLocal(resolved, LOCAL_UPNP);
                    }
                } else {
                    LogPrintf("UPnP: GetExternalIPAddress failed.\n");
                }
            }
        }

<<<<<<< HEAD
        std::string strDesc = PACKAGE_NAME " " + FormatFullVersion();
=======
        string strDesc = "Neutroncoin " + FormatFullVersion();
>>>>>>> 0.8

        do {
            r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port.c_str(), port.c_str(), lanaddr, strDesc.c_str(), "TCP", 0, "0");

            if (r != UPNPCOMMAND_SUCCESS) {
                LogPrintf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n", port, port, lanaddr, r, strupnperror(r));
            } else {
                LogPrintf("UPnP Port Mapping successful.\n");
            }
        } while (g_upnp_interrupt.sleep_for(std::chrono::minutes(20)));

        r = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, port.c_str(), "TCP", 0);
        LogPrintf("UPNP_DeletePortMapping() returned: %d\n", r);
        freeUPNPDevlist(devlist); devlist = nullptr;
        FreeUPNPUrls(&urls);
    } else {
        LogPrintf("No valid UPnP IGDs found\n");
        freeUPNPDevlist(devlist); devlist = nullptr;
        if (r != 0)
            FreeUPNPUrls(&urls);
    }
}

void StartMapPort()
{
    if (!g_upnp_thread.joinable()) {
        assert(!g_upnp_interrupt);
        g_upnp_thread = std::thread((std::bind(&TraceThread<void (*)()>, "upnp", &ThreadMapPort)));
    }
}

void InterruptMapPort()
{
    if(g_upnp_thread.joinable()) {
        g_upnp_interrupt();
    }
}

void StopMapPort()
{
    if(g_upnp_thread.joinable()) {
        g_upnp_thread.join();
        g_upnp_interrupt.reset();
    }
}

#else
void StartMapPort()
{
    // Intentionally left blank.
}
void InterruptMapPort()
{
    // Intentionally left blank.
}
void StopMapPort()
{
    // Intentionally left blank.
}
#endif






<<<<<<< HEAD
void CConnman::ThreadDNSAddressSeed()
=======



// DNS seeds
// Each pair gives a source name and a seed name.
// The first name is used as information source for addrman.
// The second name should resolve to a list of seed addresses.
static const char *strMainNetDNSSeed[][2] = {
    {"neutroncointools.com", "dnsseed.neutroncointools.com"},
    {"neutroncoinpool.org", "dnsseed.neutroncoinpool.org"},
    {"xurious.com", "dnsseed.ltc.xurious.com"},
    {"koin-project.com", "dnsseed.koin-project.com"},
    {"weminemnc.com", "dnsseed.weminemnc.com"},
    {NULL, NULL}
};

static const char *strTestNetDNSSeed[][2] = {
    {"neutroncointools.com", "testnet-seed.neutroncointools.com"},
    {"xurious.com", "testnet-seed.ltc.xurious.com"},
    {"wemine-testnet.com", "dnsseed.wemine-testnet.com"},
    {NULL, NULL}
};

void ThreadDNSAddressSeed()
>>>>>>> 0.8
{
    FastRandomContext rng;
    std::vector<std::string> seeds = Params().DNSSeeds();
    Shuffle(seeds.begin(), seeds.end(), rng);
    int seeds_right_now = 0; // Number of seeds left before testing if we have enough connections
    int found = 0;

    if (gArgs.GetBoolArg("-forcednsseed", DEFAULT_FORCEDNSSEED)) {
        // When -forcednsseed is provided, query all.
        seeds_right_now = seeds.size();
    } else if (addrman.size() == 0) {
        // If we have no known peers, query all.
        // This will occur on the first run, or if peers.dat has been
        // deleted.
        seeds_right_now = seeds.size();
    }

    // goal: only query DNS seed if address need is acute
    // * If we have a reasonable number of peers in addrman, spend
    //   some time trying them first. This improves user privacy by
    //   creating fewer identifying DNS requests, reduces trust by
    //   giving seeds less influence on the network topology, and
    //   reduces traffic to the seeds.
    // * When querying DNS seeds query a few at once, this ensures
    //   that we don't give DNS seeds the ability to eclipse nodes
    //   that query them.
    // * If we continue having problems, eventually query all the
    //   DNS seeds, and if that fails too, also try the fixed seeds.
    //   (done in ThreadOpenConnections)
    const std::chrono::seconds seeds_wait_time = (addrman.size() >= DNSSEEDS_DELAY_PEER_THRESHOLD ? DNSSEEDS_DELAY_MANY_PEERS : DNSSEEDS_DELAY_FEW_PEERS);

    for (const std::string& seed : seeds) {
        if (seeds_right_now == 0) {
            seeds_right_now += DNSSEEDS_TO_QUERY_AT_ONCE;

            if (addrman.size() > 0) {
                LogPrintf("Waiting %d seconds before querying DNS seeds.\n", seeds_wait_time.count());
                std::chrono::seconds to_wait = seeds_wait_time;
                while (to_wait.count() > 0) {
                    // if sleeping for the MANY_PEERS interval, wake up
                    // early to see if we have enough peers and can stop
                    // this thread entirely freeing up its resources
                    std::chrono::seconds w = std::min(DNSSEEDS_DELAY_FEW_PEERS, to_wait);
                    if (!interruptNet.sleep_for(w)) return;
                    to_wait -= w;

                    int nRelevant = 0;
                    {
                        LOCK(cs_vNodes);
                        for (const CNode* pnode : vNodes) {
                            if (pnode->fSuccessfullyConnected && pnode->IsOutboundOrBlockRelayConn()) ++nRelevant;
                        }
                    }
                    if (nRelevant >= 2) {
                        if (found > 0) {
                            LogPrintf("%d addresses found from DNS seeds\n", found);
                            LogPrintf("P2P peers available. Finished DNS seeding.\n");
                        } else {
                            LogPrintf("P2P peers available. Skipped DNS seeding.\n");
                        }
                        return;
                    }
                }
            }
        }

        if (interruptNet) return;

        // hold off on querying seeds if P2P network deactivated
        if (!fNetworkActive) {
            LogPrintf("Waiting for network to be reactivated before querying DNS seeds.\n");
            do {
                if (!interruptNet.sleep_for(std::chrono::seconds{1})) return;
            } while (!fNetworkActive);
        }

        LogPrintf("Loading addresses from DNS seed %s\n", seed);
        if (HaveNameProxy()) {
            AddAddrFetch(seed);
        } else {
            std::vector<CNetAddr> vIPs;
            std::vector<CAddress> vAdd;
            ServiceFlags requiredServiceBits = GetDesirableServiceFlags(NODE_NONE);
            std::string host = strprintf("x%x.%s", requiredServiceBits, seed);
            CNetAddr resolveSource;
            if (!resolveSource.SetInternal(host)) {
                continue;
            }
            unsigned int nMaxIPs = 256; // Limits number of IPs learned from a DNS seed
            if (LookupHost(host, vIPs, nMaxIPs, true)) {
                for (const CNetAddr& ip : vIPs) {
                    int nOneDay = 24*3600;
                    CAddress addr = CAddress(CService(ip, Params().GetDefaultPort()), requiredServiceBits);
                    addr.nTime = GetTime() - 3*nOneDay - rng.randrange(4*nOneDay); // use a random age between 3 and 7 days old
                    vAdd.push_back(addr);
                    found++;
                }
                addrman.Add(vAdd, resolveSource);
            } else {
                // We now avoid directly using results from DNS Seeds which do not support service bit filtering,
                // instead using them as a addrfetch to get nodes with our desired service bits.
                AddAddrFetch(seed);
            }
        }
        --seeds_right_now;
    }
    LogPrintf("%d addresses found from DNS seeds\n", found);
}

void CConnman::DumpAddresses()
{
<<<<<<< HEAD
    int64_t nStart = GetTimeMillis();
=======
    0x38a9b992, 0x73d4f3a2, 0x43eda52e, 0xa1c4a2b2, 0x73c41955, 0x6992f3a2, 0x729cb992, 0x8b53b205,
    0xb651ec36, 0x8b422e4e, 0x0fe421b2, 0x83c1a2b2, 0xbd432705, 0x2e11b018, 0x281544c1, 0x8b72f3a2,
    0xb934555f, 0x2ba02e4e, 0x6ab7c936, 0x8728555f, 0x03bfd143, 0x0a73df5b, 0xcd2b5a50, 0x746df3a2,
    0x7481bb25, 0x6f4d4550, 0x78582f4e, 0xa03a0f46, 0xe8b0e2bc, 0xa2d17042, 0x718a09b0, 0xdaffd4a2,
    0xbb1a175e, 0xb21f09b0, 0xb5549bc0, 0xe404c755, 0x95d882c3, 0xfff3692e, 0x3777d9c7, 0x425b2746,
    0x497990c6, 0xb2782dcc, 0xf9352225, 0xa75cd443, 0x4c05fb94, 0x44c91c2e, 0x47c6a5bc, 0xd606fb94,
    0xc1b9e2bc, 0x32acd23e, 0x89560da2, 0x5bebdad8, 0x3a210e08, 0xbdc5795b, 0xcc86bb25, 0xbe9f28bc,
    0xef3ff3a2, 0xca29df59, 0xe4fd175e, 0x1f3eaa6b, 0xacdbaa6b, 0xb05f042e, 0x81ed6cd8, 0x9a3c0cc3,
    0x4200175e, 0x5a017ebc, 0x42ef4c90, 0x8abfd143, 0x24fbf3a2, 0x140846a6, 0x4f7d9553, 0xeea5d151,
    0xe67c0905, 0x52d8048e, 0xcabd2e4e, 0xe276692e, 0x07dea445, 0xdde3f3a2, 0x6c47bb25, 0xae0efb94,
    0xf5e15a51, 0xaebdd25b, 0xf341175e, 0x46532705, 0xc47728bc, 0xe4e14c90, 0x9dc8f752, 0x050c042e,
    0x1c84bb25, 0x4f163b25, 0x1a017ebc, 0xa5282e4e, 0x8c667e60, 0xc7113b25, 0xf0b44832, 0xf1a134d0,
    0x973212d4, 0xd35cbb25, 0xd5123b25, 0x68220254, 0x7ad43e32, 0x9268e32e, 0xdf143b25, 0xaf04c436,
    0xaded0051, 0xfa86d454, 0x09db048e, 0x26003b25, 0x58764c90, 0x9a2f555f, 0x0c24ec97, 0x92123b25,
    0x0526d35f, 0x17db048e, 0xd2e42f4e, 0x38cca5bc, 0xc6320ab9, 0xe28ac836, 0xc560aa6b, 0xa5c16041,
    0x70a6f1c0, 0x011ec8c1, 0xd6e9c332, 0x131263c0, 0xa15a4450, 0xef218abc, 0x2729f948, 0x02835443,
    0x5614336c, 0xb12aacb2, 0xe368aa6b, 0x3cc6ffad, 0x36206494, 0x2c90e9c1, 0x32bb53d4, 0xca03de5c,
    0x775c1955, 0x19ef1ba3, 0x0b00dc1f, 0x244d0f40, 0x54d9e2bc, 0x25ced152, 0x967b03ad, 0x951c555f,
    0x4c3f3b25, 0x13f6f3a2, 0x17fca5bc, 0x0e2d306c, 0xacd8764b, 0xca230bcc, 0x8569d3c6, 0x3264d8a2,
    0xe8630905, 0x25e02a64, 0x3aba1fb0, 0x6bbdd25b, 0xee9a4c90, 0xcda25982, 0x8b3e804e, 0xf043fb94,
    0x4b05fb94, 0x0c44c052, 0xf403f45c, 0x4333aa6b, 0xc193484d, 0x3fbf5d4c, 0x0bd7f74d, 0x150e3fb2,
    0x8e2eddb0, 0x09daf150, 0x8a67c7c6, 0x22a9e52e, 0x05cff476, 0xc99b2f4e, 0x0f183b25, 0xd0358953,
    0x20f232c6, 0x0ce9e217, 0x09f55d18, 0x0555795b, 0x5ed2fa59, 0x2ec85917, 0x2bf61fb0, 0x024ef54d,
    0x3c53b859, 0x441cbb25, 0x50c8aa6b, 0x1b79175e, 0x3125795b, 0x27fc1fb0, 0xbcd53e32, 0xfc781718,
    0x7a8ec345, 0x1da6985d, 0x34bd1f32, 0xcb00edcf, 0xf9a5fdce, 0x21ccdbac, 0xb7730118, 0x6a43f6cc,
    0x6e65e262, 0x21ca1f3d, 0x10143b25, 0xc8dea132, 0xaf076693, 0x7e431bc6, 0xaa3df5c6, 0x44f0c536,
    0xeea80925, 0x262371d4, 0xc85c895b, 0xa6611bc6, 0x1844e47a, 0x49084c90, 0xf3d95157, 0x63a4a844,
    0x00477c70, 0x2934d35f, 0xe8d24465, 0x13df88b7, 0x8fcb7557, 0xa591bd5d, 0xc39453d4, 0xd5c49452,
    0xc8de1a56, 0x3cdd0608, 0x3c147a55, 0x49e6cf4a, 0xb38c8705, 0x0bef3479, 0x01540f48, 0xd9c3ec57,
    0xed6d4c90, 0xa529fb94, 0xe1c81955, 0xfde617c6, 0x72d18932, 0x9d61bb6a, 0x6d5cb258, 0x27c7d655,
    0xc5644c90, 0x31fae3bc, 0x3afaf25e, 0x98efe2bc, 0x91020905, 0xb566795b, 0xaf91bd5d, 0xb164d655,
    0x72eb47d4, 0xae62f3a2, 0xb4193b25, 0x0613fb94, 0xa6db048e, 0xf002464b, 0xc15ebb6a, 0x8a51f3a2,
    0x485e2ed5, 0x119675a3, 0x1f3f555f, 0x39dbc082, 0x09dea445, 0x74382446, 0x3e836c4e, 0x6e43f6cc,
    0x134dd9a2, 0x5876f945, 0x3516f725, 0x670c81d4, 0xaf7f170c, 0xb0e31155, 0xe271894e, 0x615e175e,
    0xb3446fd0, 0x13d58ac1, 0x07cff476, 0xe601e405, 0x8321277d, 0x0997548d, 0xdb55336c, 0xa1271d73,
    0x582463c0, 0xc2543025, 0xf6851c73, 0xe75d32ae, 0xf916b4dd, 0xf558fb94, 0x52111d73, 0x2bc8615d,
    0xd4dcaf42, 0x65e30979, 0x2e1b2360, 0x0da01d73, 0x3f1263c0, 0xd15c735d, 0x9cf2134c, 0x20d0048e,
    0x48cf0125, 0xf585bf5d, 0x12d7645e, 0xd5ace2bc, 0x0c6220b2, 0xbe13bb25, 0x88d0a52e, 0x559425b0,
    0x24079e3d, 0xfaa37557, 0xf219b96a, 0x07e61e4c, 0x3ea1d295, 0x24e0d852, 0xdde212df, 0x44c37758,
    0x55c243a4, 0xe77dd35f, 0x10c19a5f, 0x14d1048e, 0x1d50fbc6, 0x1570456c, 0x567c692e, 0x641d6d5b,
    0xab0c0cc6, 0xab6803c0, 0x136f21b2, 0x6a72545d, 0x21d031b2, 0xff8b5fad, 0xfd0096b2, 0x5f469ac3,
    0x3f6ffa62, 0x7501f863, 0x48471f60, 0xcccff3a2, 0x7f772e44, 0xc1de7757, 0x0c94c3cb, 0x620ac658,
    0x520f1d25, 0x37366c5f, 0x7594b159, 0x3804f23c, 0xb81ff054, 0x96dd32c6, 0x928228bc, 0xf4006e41,
    0x0241c244, 0x8dbdf243, 0x26d1b247, 0xd5381c73, 0xf3136e41, 0x4afa9bc0, 0xa3abf8ce, 0x464ce718,
    0xbd9d017b, 0xf4f26132, 0x141b32d4, 0x2ec50c18, 0x4df119d9, 0x93f81772, 0xd9607c70, 0x3522b648,
    0xf2006e41, 0x761fe550, 0x40104836, 0x55dffe96, 0xc45db158, 0xe75e4836, 0x8dfab7ad, 0xe3eff3a2,
    0x6a6eaa6b, 0x2177d9c7, 0x724ce953, 0xafe918b9, 0xf9368a55, 0xdc0a555f, 0xa4b2d35f, 0x4d87b0b8,
    0x93496a54, 0x5a5c967b, 0xd47028bc, 0x3c44e47a, 0x11c363c0, 0x28107c70, 0xb756a558, 0xb82bbb6a,
    0x285d49ad, 0x3b0ce85c, 0xe53eb45f, 0xa836e23c, 0x409f63c0, 0xc80fbd44, 0x3447f677, 0xe4ca983b,
    0x20673774, 0x96471ed9, 0x4c1d09b0, 0x91d5685d, 0x55beec4d, 0x1008be48, 0x660455d0, 0xf8170fda,
    0x3c21dd3a, 0x8239bb36, 0x9fe7e2bc, 0x900c47ae, 0x6a355643, 0x03dcb318, 0xefca123d, 0x6af8c4d9,
    0x5195e1a5, 0x32e89925, 0x0adc51c0, 0x45d7164c, 0x02495177, 0x8131175e, 0x681b5177, 0x41b6aa6b,
    0x55a9603a, 0x1a0c1c1f, 0xdb4da043, 0x3b9b1632, 0x37e08368, 0x8b54e260, 0xcd14d459, 0x82a663c0,
    0x05adc7dd, 0xe683f3a2, 0x4cddb150, 0x67a1a62e, 0x8c0acd25, 0x07f01f3e, 0x3111296c, 0x2d0fda2e,
    0xa4f163c0, 0xca6db982, 0x78ed2359, 0x7eaa21c1, 0x62e4f3a2, 0x50b81d73, 0xcd074a3a, 0xcb2d904b,
    0x9b3735ce, 0xab67f25c, 0xa0eb5036, 0x62ae5344, 0xe2569bb2, 0xc4422e4e, 0xab5ec8d5, 0xaa81e8dd,
    0xa39264c6, 0xf391563d, 0xb79bbb25, 0x174a7857, 0x0fd4aa43, 0x3e158c32, 0x3ae8b982, 0xea342225,
    0x48d1a842, 0xa52bf0da, 0x4bcb4a4c, 0xa6d3c15b, 0x49a0d35f, 0x97131b4c, 0xf197e252, 0xfe3ebd18,
    0x156dacb8, 0xf63328bc, 0x8e95b84c, 0x560455d0, 0xee918c4e, 0x1d3e435f, 0xe1292f50, 0x0f1ec018,
    0x7d70c60e, 0x6a29d5be, 0xf5fecb18, 0xd6da1f63, 0xccce1c2e, 0x7a289f5e, 0x2775ae47, 0x696df560,
    0x4dbe00ae, 0x474e6c5c, 0x604141d5, 0xaed0c718, 0x8acfd23e, 0x7ff4b84c, 0x4b44fc60, 0xdf58aa4f,
    0x9b7440c0, 0xb811c854, 0xd90ec24e, 0xcff75c46, 0xa5a9cc57, 0xb3d21e4c, 0x794779d9, 0xe5613d46,
    0x9478be43, 0xc5d11152, 0xe85fbb6a, 0x3e1ed052, 0xf747e418, 0x3b9c61c2, 0xb2532949, 0x43077432,
    0xa3db0b68, 0xb3b6e35a, 0x70361b6c, 0x3a8bad3e, 0x23079e3d, 0x09314c32, 0x92f90053, 0x4fc31955,
    0xa59b0905, 0x924128bc, 0x4e63c444, 0x344dc236, 0x43055fcb, 0xdc1a1c73, 0x38aaaa6b, 0xa61cf554,
    0x6d8f63c0, 0x24800a4c, 0x2406f953, 0x9558bb6a, 0x1d281660, 0x054c4954, 0x2de4d418, 0x5fdaf043,
    0xb681356c, 0xf8c3febc, 0x8854f950, 0x55b45d32, 0xde07bcd1, 0x156e4bda, 0x924cf718, 0xc34a0d47,
    0xdd5e1c45, 0x4a0d63c0, 0xaf4b88d5, 0x7852b958, 0x6f205fc0, 0x838af043, 0x45ac795b, 0x43bbaa6b,
    0x998d1c73, 0x11c1d558, 0x749ec444, 0x9a38c232, 0xad2f8c32, 0x3446c658, 0x8fe7a52e, 0x4e846b61,
    0x064b2d05, 0x0fd09740, 0x7a81a743, 0xf1f08e3f, 0x35ca1967, 0x24bdb17d, 0x144c2d05, 0x5505bb46,
    0x13fd1bb9, 0x25de2618, 0xc80a8b4b, 0xcec0fd6c, 0xdc30ad4c, 0x4009f3a2, 0x472f3fb2, 0x5e69c936,
    0x0380796d, 0xa463f8a2, 0xa46fbdc7, 0x3b0cc547, 0xb6644f46, 0x4b90fc47, 0x39e3f563, 0x72d81e56,
    0xe177d9c7, 0x95bff743, 0xea985542, 0xc210ec55, 0xeef70b67, 0xc9eb175e, 0x844d38ad, 0x65afa247,
    0x72da6d26, 0xed165dbc, 0xe8c83ad0, 0x9a8f37d8, 0x925adf50, 0x6b6ac162, 0x4b969e32, 0x735e1c45,
    0x4423ff60, 0xfa57ec6d, 0xcde2fb65, 0x11093257, 0x4748cd5b, 0x720c03dd, 0x8c7b0905, 0xba8b2e48
};

void DumpAddresses()
{
    int64 nStart = GetTimeMillis();
>>>>>>> 0.8

    CAddrDB adb;
    adb.Write(addrman);

    LogPrint(BCLog::NET, "Flushed %d addresses to peers.dat  %dms\n",
           addrman.size(), GetTimeMillis() - nStart);
}

void CConnman::ProcessAddrFetch()
{
    std::string strDest;
    {
        LOCK(m_addr_fetches_mutex);
        if (m_addr_fetches.empty())
            return;
        strDest = m_addr_fetches.front();
        m_addr_fetches.pop_front();
    }
    CAddress addr;
    CSemaphoreGrant grant(*semOutbound, true);
    if (grant) {
        OpenNetworkConnection(addr, false, &grant, strDest.c_str(), ConnectionType::ADDR_FETCH);
    }
}

bool CConnman::GetTryNewOutboundPeer()
{
    return m_try_another_outbound_peer;
}

void CConnman::SetTryNewOutboundPeer(bool flag)
{
    m_try_another_outbound_peer = flag;
    LogPrint(BCLog::NET, "net: setting try another outbound peer=%s\n", flag ? "true" : "false");
}

// Return the number of peers we have over our outbound connection limit
// Exclude peers that are marked for disconnect, or are going to be
// disconnected soon (eg ADDR_FETCH and FEELER)
// Also exclude peers that haven't finished initial connection handshake yet
// (so that we don't decide we're over our desired connection limit, and then
// evict some peer that has finished the handshake)
int CConnman::GetExtraOutboundCount()
{
    int nOutbound = 0;
    {
        LOCK(cs_vNodes);
        for (const CNode* pnode : vNodes) {
            if (pnode->fSuccessfullyConnected && !pnode->fDisconnect && pnode->IsOutboundOrBlockRelayConn()) {
                ++nOutbound;
            }
        }
    }
    return std::max(nOutbound - m_max_outbound_full_relay - m_max_outbound_block_relay, 0);
}

void CConnman::ThreadOpenConnections(const std::vector<std::string> connect)
{
    // Connect to specific addresses
    if (!connect.empty())
    {
        for (int64_t nLoop = 0;; nLoop++)
        {
            ProcessAddrFetch();
            for (const std::string& strAddr : connect)
            {
                CAddress addr(CService(), NODE_NONE);
                OpenNetworkConnection(addr, false, nullptr, strAddr.c_str(), ConnectionType::MANUAL);
                for (int i = 0; i < 10 && i < nLoop; i++)
                {
                    if (!interruptNet.sleep_for(std::chrono::milliseconds(500)))
                        return;
                }
            }
            if (!interruptNet.sleep_for(std::chrono::milliseconds(500)))
                return;
        }
    }

    // Initiate network connections
    int64_t nStart = GetTime();

    // Minimum time before next feeler connection (in microseconds).
    int64_t nNextFeeler = PoissonNextSend(nStart*1000*1000, FEELER_INTERVAL);
    while (!interruptNet)
    {
        ProcessAddrFetch();

        if (!interruptNet.sleep_for(std::chrono::milliseconds(500)))
            return;

        CSemaphoreGrant grant(*semOutbound);
        if (interruptNet)
            return;

        // Add seed nodes if DNS seeds are all down (an infrastructure attack?).
        // Note that we only do this if we started with an empty peers.dat,
        // (in which case we will query DNS seeds immediately) *and* the DNS
        // seeds have not returned any results.
        if (addrman.size() == 0 && (GetTime() - nStart > 60)) {
            static bool done = false;
            if (!done) {
                LogPrintf("Adding fixed seed nodes as DNS doesn't seem to be available.\n");
                CNetAddr local;
                local.SetInternal("fixedseeds");
                addrman.Add(ConvertSeeds(Params().FixedSeeds()), local);
                done = true;
            }
        }

        //
        // Choose an address to connect to based on most recently seen
        //
        CAddress addrConnect;

        // Only connect out to one peer per network group (/16 for IPv4).
        int nOutboundFullRelay = 0;
        int nOutboundBlockRelay = 0;
        std::set<std::vector<unsigned char> > setConnected;

        {
            LOCK(cs_vNodes);
            for (const CNode* pnode : vNodes) {
                if (pnode->IsFullOutboundConn()) nOutboundFullRelay++;
                if (pnode->IsBlockOnlyConn()) nOutboundBlockRelay++;

                // Netgroups for inbound and manual peers are not excluded because our goal here
                // is to not use multiple of our limited outbound slots on a single netgroup
                // but inbound and manual peers do not use our outbound slots. Inbound peers
                // also have the added issue that they could be attacker controlled and used
                // to prevent us from connecting to particular hosts if we used them here.
                switch (pnode->m_conn_type) {
                    case ConnectionType::INBOUND:
                    case ConnectionType::MANUAL:
                        break;
                    case ConnectionType::OUTBOUND_FULL_RELAY:
                    case ConnectionType::BLOCK_RELAY:
                    case ConnectionType::ADDR_FETCH:
                    case ConnectionType::FEELER:
                        setConnected.insert(pnode->addr.GetGroup(addrman.m_asmap));
                } // no default case, so the compiler can warn about missing cases
            }
        }

        ConnectionType conn_type = ConnectionType::OUTBOUND_FULL_RELAY;
        int64_t nTime = GetTimeMicros();
        bool anchor = false;
        bool fFeeler = false;

        // Determine what type of connection to open. Opening
        // BLOCK_RELAY connections to addresses from anchors.dat gets the highest
        // priority. Then we open OUTBOUND_FULL_RELAY priority until we
        // meet our full-relay capacity. Then we open BLOCK_RELAY connection
        // until we hit our block-relay-only peer limit.
        // GetTryNewOutboundPeer() gets set when a stale tip is detected, so we
        // try opening an additional OUTBOUND_FULL_RELAY connection. If none of
        // these conditions are met, check the nNextFeeler timer to decide if
        // we should open a FEELER.

        if (!m_anchors.empty() && (nOutboundBlockRelay < m_max_outbound_block_relay)) {
            conn_type = ConnectionType::BLOCK_RELAY;
            anchor = true;
        } else if (nOutboundFullRelay < m_max_outbound_full_relay) {
            // OUTBOUND_FULL_RELAY
        } else if (nOutboundBlockRelay < m_max_outbound_block_relay) {
            conn_type = ConnectionType::BLOCK_RELAY;
        } else if (GetTryNewOutboundPeer()) {
            // OUTBOUND_FULL_RELAY
        } else if (nTime > nNextFeeler) {
            nNextFeeler = PoissonNextSend(nTime, FEELER_INTERVAL);
            conn_type = ConnectionType::FEELER;
            fFeeler = true;
        } else {
            // skip to next iteration of while loop
            continue;
        }

        addrman.ResolveCollisions();

        int64_t nANow = GetAdjustedTime();
        int nTries = 0;
        while (!interruptNet)
        {
            if (anchor && !m_anchors.empty()) {
                const CAddress addr = m_anchors.back();
                m_anchors.pop_back();
                if (!addr.IsValid() || IsLocal(addr) || !IsReachable(addr) ||
                    !HasAllDesirableServiceFlags(addr.nServices) ||
                    setConnected.count(addr.GetGroup(addrman.m_asmap))) continue;
                addrConnect = addr;
                LogPrint(BCLog::NET, "Trying to make an anchor connection to %s\n", addrConnect.ToString());
                break;
            }

            // If we didn't find an appropriate destination after trying 100 addresses fetched from addrman,
            // stop this loop, and let the outer loop run again (which sleeps, adds seed nodes, recalculates
            // already-connected network ranges, ...) before trying new addrman addresses.
            nTries++;
            if (nTries > 100)
                break;

            CAddrInfo addr;

            if (fFeeler) {
                // First, try to get a tried table collision address. This returns
                // an empty (invalid) address if there are no collisions to try.
                addr = addrman.SelectTriedCollision();

                if (!addr.IsValid()) {
                    // No tried table collisions. Select a new table address
                    // for our feeler.
                    addr = addrman.Select(true);
                } else if (AlreadyConnectedToAddress(addr)) {
                    // If test-before-evict logic would have us connect to a
                    // peer that we're already connected to, just mark that
                    // address as Good(). We won't be able to initiate the
                    // connection anyway, so this avoids inadvertently evicting
                    // a currently-connected peer.
                    addrman.Good(addr);
                    // Select a new table address for our feeler instead.
                    addr = addrman.Select(true);
                }
            } else {
                // Not a feeler
                addr = addrman.Select();
            }

            // Require outbound connections, other than feelers, to be to distinct network groups
            if (!fFeeler && setConnected.count(addr.GetGroup(addrman.m_asmap))) {
                break;
            }

            // if we selected an invalid or local address, restart
            if (!addr.IsValid() || IsLocal(addr)) {
                break;
            }

            if (!IsReachable(addr))
                continue;

            // only consider very recently tried nodes after 30 failed attempts
            if (nANow - addr.nLastTry < 600 && nTries < 30)
                continue;

            // for non-feelers, require all the services we'll want,
            // for feelers, only require they be a full node (only because most
            // SPV clients don't have a good address DB available)
            if (!fFeeler && !HasAllDesirableServiceFlags(addr.nServices)) {
                continue;
            } else if (fFeeler && !MayHaveUsefulAddressDB(addr.nServices)) {
                continue;
            }

            // Do not allow non-default ports, unless after 50 invalid
            // addresses selected already. This is to prevent malicious peers
            // from advertising themselves as a service on another host and
            // port, causing a DoS attack as nodes around the network attempt
            // to connect to it fruitlessly.
            if (addr.GetPort() != Params().GetDefaultPort() && nTries < 50)
                continue;

            addrConnect = addr;
            break;
        }

        if (addrConnect.IsValid()) {

            if (fFeeler) {
                // Add small amount of random noise before connection to avoid synchronization.
                int randsleep = GetRandInt(FEELER_SLEEP_WINDOW * 1000);
                if (!interruptNet.sleep_for(std::chrono::milliseconds(randsleep)))
                    return;
                LogPrint(BCLog::NET, "Making feeler connection to %s\n", addrConnect.ToString());
            }

            OpenNetworkConnection(addrConnect, (int)setConnected.size() >= std::min(nMaxConnections - 1, 2), &grant, nullptr, conn_type);
        }
    }
}

std::vector<CAddress> CConnman::GetCurrentBlockRelayOnlyConns() const
{
    std::vector<CAddress> ret;
    LOCK(cs_vNodes);
    for (const CNode* pnode : vNodes) {
        if (pnode->IsBlockOnlyConn()) {
            ret.push_back(pnode->addr);
        }
    }

    return ret;
}

std::vector<AddedNodeInfo> CConnman::GetAddedNodeInfo()
{
    std::vector<AddedNodeInfo> ret;

    std::list<std::string> lAddresses(0);
    {
        LOCK(cs_vAddedNodes);
        ret.reserve(vAddedNodes.size());
        std::copy(vAddedNodes.cbegin(), vAddedNodes.cend(), std::back_inserter(lAddresses));
    }


    // Build a map of all already connected addresses (by IP:port and by name) to inbound/outbound and resolved CService
    std::map<CService, bool> mapConnected;
    std::map<std::string, std::pair<bool, CService>> mapConnectedByName;
    {
        LOCK(cs_vNodes);
        for (const CNode* pnode : vNodes) {
            if (pnode->addr.IsValid()) {
                mapConnected[pnode->addr] = pnode->IsInboundConn();
            }
            std::string addrName = pnode->GetAddrName();
            if (!addrName.empty()) {
                mapConnectedByName[std::move(addrName)] = std::make_pair(pnode->IsInboundConn(), static_cast<const CService&>(pnode->addr));
            }
        }
    }

    for (const std::string& strAddNode : lAddresses) {
        CService service(LookupNumeric(strAddNode, Params().GetDefaultPort()));
        AddedNodeInfo addedNode{strAddNode, CService(), false, false};
        if (service.IsValid()) {
            // strAddNode is an IP:port
            auto it = mapConnected.find(service);
            if (it != mapConnected.end()) {
                addedNode.resolvedAddress = service;
                addedNode.fConnected = true;
                addedNode.fInbound = it->second;
            }
        } else {
            // strAddNode is a name
            auto it = mapConnectedByName.find(strAddNode);
            if (it != mapConnectedByName.end()) {
                addedNode.resolvedAddress = it->second.second;
                addedNode.fConnected = true;
                addedNode.fInbound = it->second.first;
            }
        }
        ret.emplace_back(std::move(addedNode));
    }

    return ret;
}

void CConnman::ThreadOpenAddedConnections()
{
    while (true)
    {
        CSemaphoreGrant grant(*semAddnode);
        std::vector<AddedNodeInfo> vInfo = GetAddedNodeInfo();
        bool tried = false;
        for (const AddedNodeInfo& info : vInfo) {
            if (!info.fConnected) {
                if (!grant.TryAcquire()) {
                    // If we've used up our semaphore and need a new one, let's not wait here since while we are waiting
                    // the addednodeinfo state might change.
                    break;
                }
                tried = true;
                CAddress addr(CService(), NODE_NONE);
                OpenNetworkConnection(addr, false, &grant, info.strAddedNode.c_str(), ConnectionType::MANUAL);
                if (!interruptNet.sleep_for(std::chrono::milliseconds(500)))
                    return;
            }
        }
        // Retry every 60 seconds if a connection was attempted, otherwise two seconds
        if (!interruptNet.sleep_for(std::chrono::seconds(tried ? 60 : 2)))
            return;
    }
}

// if successful, this moves the passed grant to the constructed node
void CConnman::OpenNetworkConnection(const CAddress& addrConnect, bool fCountFailure, CSemaphoreGrant *grantOutbound, const char *pszDest, ConnectionType conn_type)
{
    assert(conn_type != ConnectionType::INBOUND);

    //
    // Initiate outbound network connection
    //
    if (interruptNet) {
        return;
    }
    if (!fNetworkActive) {
        return;
    }
    if (!pszDest) {
        bool banned_or_discouraged = m_banman && (m_banman->IsDiscouraged(addrConnect) || m_banman->IsBanned(addrConnect));
        if (IsLocal(addrConnect) || banned_or_discouraged || AlreadyConnectedToAddress(addrConnect)) {
            return;
        }
    } else if (FindNode(std::string(pszDest)))
        return;

    CNode* pnode = ConnectNode(addrConnect, pszDest, fCountFailure, conn_type);

    if (!pnode)
        return;
    if (grantOutbound)
        grantOutbound->MoveTo(pnode->grantOutbound);

    m_msgproc->InitializeNode(pnode);
    {
        LOCK(cs_vNodes);
        vNodes.push_back(pnode);
    }
}

void CConnman::ThreadMessageHandler()
{
    while (!flagInterruptMsgProc)
    {
        std::vector<CNode*> vNodesCopy;
        {
            LOCK(cs_vNodes);
            vNodesCopy = vNodes;
            for (CNode* pnode : vNodesCopy) {
                pnode->AddRef();
            }
        }

        bool fMoreWork = false;

<<<<<<< HEAD
        for (CNode* pnode : vNodesCopy)
=======
        // Poll the connected nodes for messages
        CNode* pnodeTrickle = NULL;
        if (!vNodesCopy.empty())
            pnodeTrickle = vNodesCopy[GetRand(vNodesCopy.size())];

        bool fSleep = true;

        BOOST_FOREACH(CNode* pnode, vNodesCopy)
>>>>>>> 0.8
        {
            if (pnode->fDisconnect)
                continue;

            // Receive messages
<<<<<<< HEAD
            bool fMoreNodeWork = m_msgproc->ProcessMessages(pnode, flagInterruptMsgProc);
            fMoreWork |= (fMoreNodeWork && !pnode->fPauseSend);
            if (flagInterruptMsgProc)
                return;
=======
            {
                TRY_LOCK(pnode->cs_vRecvMsg, lockRecv);
                if (lockRecv)
                {
                    if (!ProcessMessages(pnode))
                        pnode->CloseSocketDisconnect();

                    if (pnode->nSendSize < SendBufferSize())
                    {
                        if (!pnode->vRecvGetData.empty() || (!pnode->vRecvMsg.empty() && pnode->vRecvMsg[0].complete()))
                        {
                            fSleep = false;
                        }
                    }
                }
            }
            boost::this_thread::interruption_point();

>>>>>>> 0.8
            // Send messages
            {
                LOCK(pnode->cs_sendProcessing);
                m_msgproc->SendMessages(pnode);
            }

            if (flagInterruptMsgProc)
                return;
        }

        {
            LOCK(cs_vNodes);
            for (CNode* pnode : vNodesCopy)
                pnode->Release();
        }

<<<<<<< HEAD
        WAIT_LOCK(mutexMsgProc, lock);
        if (!fMoreWork) {
            condMsgProc.wait_until(lock, std::chrono::steady_clock::now() + std::chrono::milliseconds(100), [this]() EXCLUSIVE_LOCKS_REQUIRED(mutexMsgProc) { return fMsgProcWake; });
        }
        fMsgProcWake = false;
=======
        if (fSleep)
            MilliSleep(100);
>>>>>>> 0.8
    }
}

bool CConnman::BindListenPort(const CService& addrBind, bilingual_str& strError, NetPermissionFlags permissions)
{
    int nOne = 1;

    // Create socket for listening for incoming connections
    struct sockaddr_storage sockaddr;
    socklen_t len = sizeof(sockaddr);
    if (!addrBind.GetSockAddr((struct sockaddr*)&sockaddr, &len))
    {
        strError = strprintf(Untranslated("Error: Bind address family for %s not supported"), addrBind.ToString());
        LogPrintf("%s\n", strError.original);
        return false;
    }

    SOCKET hListenSocket = CreateSocket(addrBind);
    if (hListenSocket == INVALID_SOCKET)
    {
        strError = strprintf(Untranslated("Error: Couldn't open socket for incoming connections (socket returned error %s)"), NetworkErrorString(WSAGetLastError()));
        LogPrintf("%s\n", strError.original);
        return false;
    }

    // Allow binding if the port is still in TIME_WAIT state after
    // the program was closed and restarted.
    setsockopt(hListenSocket, SOL_SOCKET, SO_REUSEADDR, (sockopt_arg_type)&nOne, sizeof(int));

    // some systems don't have IPV6_V6ONLY but are always v6only; others do have the option
    // and enable it by default or not. Try to enable it, if possible.
    if (addrBind.IsIPv6()) {
#ifdef IPV6_V6ONLY
<<<<<<< HEAD
        setsockopt(hListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (sockopt_arg_type)&nOne, sizeof(int));
=======
#ifdef WIN32
        setsockopt(hListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&nOne, sizeof(int));
#else
        setsockopt(hListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&nOne, sizeof(int));
>>>>>>> 0.8
#endif
#endif
#ifdef WIN32
        int nProtLevel = PROTECTION_LEVEL_UNRESTRICTED;
        setsockopt(hListenSocket, IPPROTO_IPV6, IPV6_PROTECTION_LEVEL, (const char*)&nProtLevel, sizeof(int));
#endif
    }

    if (::bind(hListenSocket, (struct sockaddr*)&sockaddr, len) == SOCKET_ERROR)
    {
        int nErr = WSAGetLastError();
        if (nErr == WSAEADDRINUSE)
<<<<<<< HEAD
            strError = strprintf(_("Unable to bind to %s on this computer. %s is probably already running."), addrBind.ToString(), PACKAGE_NAME);
=======
            strError = strprintf(_("Unable to bind to %s on this computer. Neutroncoin is probably already running."), addrBind.ToString().c_str());
>>>>>>> 0.8
        else
            strError = strprintf(_("Unable to bind to %s on this computer (bind returned error %s)"), addrBind.ToString(), NetworkErrorString(nErr));
        LogPrintf("%s\n", strError.original);
        CloseSocket(hListenSocket);
        return false;
    }
    LogPrintf("Bound to %s\n", addrBind.ToString());

    // Listen for incoming connections
    if (listen(hListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        strError = strprintf(_("Error: Listening for incoming connections failed (listen returned error %s)"), NetworkErrorString(WSAGetLastError()));
        LogPrintf("%s\n", strError.original);
        CloseSocket(hListenSocket);
        return false;
    }

    vhListenSocket.push_back(ListenSocket(hListenSocket, permissions));
    return true;
}

void Discover()
{
    if (!fDiscover)
        return;

#ifdef WIN32
    // Get local host IP
    char pszHostName[256] = "";
    if (gethostname(pszHostName, sizeof(pszHostName)) != SOCKET_ERROR)
    {
        std::vector<CNetAddr> vaddr;
        if (LookupHost(pszHostName, vaddr, 0, true))
        {
            for (const CNetAddr &addr : vaddr)
            {
                if (AddLocal(addr, LOCAL_IF))
                    LogPrintf("%s: %s - %s\n", __func__, pszHostName, addr.ToString());
            }
        }
    }
#elif (HAVE_DECL_GETIFADDRS && HAVE_DECL_FREEIFADDRS)
    // Get local host ip
    struct ifaddrs* myaddrs;
    if (getifaddrs(&myaddrs) == 0)
    {
        for (struct ifaddrs* ifa = myaddrs; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == nullptr) continue;
            if ((ifa->ifa_flags & IFF_UP) == 0) continue;
            if (strcmp(ifa->ifa_name, "lo") == 0) continue;
            if (strcmp(ifa->ifa_name, "lo0") == 0) continue;
            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                struct sockaddr_in* s4 = (struct sockaddr_in*)(ifa->ifa_addr);
                CNetAddr addr(s4->sin_addr);
                if (AddLocal(addr, LOCAL_IF))
                    LogPrintf("%s: IPv4 %s: %s\n", __func__, ifa->ifa_name, addr.ToString());
            }
            else if (ifa->ifa_addr->sa_family == AF_INET6)
            {
                struct sockaddr_in6* s6 = (struct sockaddr_in6*)(ifa->ifa_addr);
                CNetAddr addr(s6->sin6_addr);
                if (AddLocal(addr, LOCAL_IF))
                    LogPrintf("%s: IPv6 %s: %s\n", __func__, ifa->ifa_name, addr.ToString());
            }
        }
        freeifaddrs(myaddrs);
    }
#endif
}

void CConnman::SetNetworkActive(bool active)
{
    LogPrintf("%s: %s\n", __func__, active);

    if (fNetworkActive == active) {
        return;
    }

    fNetworkActive = active;

    uiInterface.NotifyNetworkActiveChanged(fNetworkActive);
}

CConnman::CConnman(uint64_t nSeed0In, uint64_t nSeed1In, bool network_active)
    : nSeed0(nSeed0In), nSeed1(nSeed1In)
{
    SetTryNewOutboundPeer(false);

    Options connOptions;
    Init(connOptions);
    SetNetworkActive(network_active);
}

NodeId CConnman::GetNewNodeId()
{
    return nLastNodeId.fetch_add(1, std::memory_order_relaxed);
}


bool CConnman::Bind(const CService &addr, unsigned int flags, NetPermissionFlags permissions) {
    if (!(flags & BF_EXPLICIT) && !IsReachable(addr))
        return false;
    bilingual_str strError;
    if (!BindListenPort(addr, strError, permissions)) {
        if ((flags & BF_REPORT_ERROR) && clientInterface) {
            clientInterface->ThreadSafeMessageBox(strError, "", CClientUIInterface::MSG_ERROR);
        }
        return false;
    }

    if (addr.IsRoutable() && fDiscover && !(flags & BF_DONT_ADVERTISE) && !(permissions & PF_NOBAN)) {
        AddLocal(addr, LOCAL_BIND);
    }

    return true;
}

bool CConnman::InitBinds(
    const std::vector<CService>& binds,
    const std::vector<NetWhitebindPermissions>& whiteBinds,
    const std::vector<CService>& onion_binds)
{
    bool fBound = false;
    for (const auto& addrBind : binds) {
        fBound |= Bind(addrBind, (BF_EXPLICIT | BF_REPORT_ERROR), NetPermissionFlags::PF_NONE);
    }
    for (const auto& addrBind : whiteBinds) {
        fBound |= Bind(addrBind.m_service, (BF_EXPLICIT | BF_REPORT_ERROR), addrBind.m_flags);
    }
    if (binds.empty() && whiteBinds.empty()) {
        struct in_addr inaddr_any;
        inaddr_any.s_addr = htonl(INADDR_ANY);
        struct in6_addr inaddr6_any = IN6ADDR_ANY_INIT;
        fBound |= Bind(CService(inaddr6_any, GetListenPort()), BF_NONE, NetPermissionFlags::PF_NONE);
        fBound |= Bind(CService(inaddr_any, GetListenPort()), !fBound ? BF_REPORT_ERROR : BF_NONE, NetPermissionFlags::PF_NONE);
    }

    for (const auto& addr_bind : onion_binds) {
        fBound |= Bind(addr_bind, BF_EXPLICIT | BF_DONT_ADVERTISE, NetPermissionFlags::PF_NONE);
    }

    return fBound;
}

bool CConnman::Start(CScheduler& scheduler, const Options& connOptions)
{
    Init(connOptions);

    {
        LOCK(cs_totalBytesRecv);
        nTotalBytesRecv = 0;
    }
    {
        LOCK(cs_totalBytesSent);
        nTotalBytesSent = 0;
        nMaxOutboundTotalBytesSentInCycle = 0;
        nMaxOutboundCycleStartTime = 0;
    }

    if (fListen && !InitBinds(connOptions.vBinds, connOptions.vWhiteBinds, connOptions.onion_binds)) {
        if (clientInterface) {
            clientInterface->ThreadSafeMessageBox(
                _("Failed to listen on any port. Use -listen=0 if you want this."),
                "", CClientUIInterface::MSG_ERROR);
        }
        return false;
    }

    for (const auto& strDest : connOptions.vSeedNodes) {
        AddAddrFetch(strDest);
    }

    if (clientInterface) {
        clientInterface->InitMessage(_("Loading P2P addresses...").translated);
    }
    // Load addresses from peers.dat
    int64_t nStart = GetTimeMillis();
    {
        CAddrDB adb;
        if (adb.Read(addrman))
            LogPrintf("Loaded %i addresses from peers.dat  %dms\n", addrman.size(), GetTimeMillis() - nStart);
        else {
            addrman.Clear(); // Addrman can be in an inconsistent state after failure, reset it
            LogPrintf("Invalid or missing peers.dat; recreating\n");
            DumpAddresses();
        }
    }

    if (m_use_addrman_outgoing) {
        // Load addresses from anchors.dat
        m_anchors = ReadAnchors(GetDataDir() / ANCHORS_DATABASE_FILENAME);
        if (m_anchors.size() > MAX_BLOCK_RELAY_ONLY_ANCHORS) {
            m_anchors.resize(MAX_BLOCK_RELAY_ONLY_ANCHORS);
        }
        LogPrintf("%i block-relay-only anchors will be tried for connections.\n", m_anchors.size());
    }

    uiInterface.InitMessage(_("Starting network threads...").translated);

    fAddressesInitialized = true;

    if (semOutbound == nullptr) {
        // initialize semaphore
        semOutbound = MakeUnique<CSemaphore>(std::min(m_max_outbound, nMaxConnections));
    }
    if (semAddnode == nullptr) {
        // initialize semaphore
        semAddnode = MakeUnique<CSemaphore>(nMaxAddnode);
    }

    //
    // Start threads
    //
    assert(m_msgproc);
    InterruptSocks5(false);
    interruptNet.reset();
    flagInterruptMsgProc = false;

    {
        LOCK(mutexMsgProc);
        fMsgProcWake = false;
    }

    // Send and receive from sockets, accept connections
    threadSocketHandler = std::thread(&TraceThread<std::function<void()> >, "net", std::function<void()>(std::bind(&CConnman::ThreadSocketHandler, this)));

    if (!gArgs.GetBoolArg("-dnsseed", true))
        LogPrintf("DNS seeding disabled\n");
    else
        threadDNSAddressSeed = std::thread(&TraceThread<std::function<void()> >, "dnsseed", std::function<void()>(std::bind(&CConnman::ThreadDNSAddressSeed, this)));

    // Initiate manual connections
    threadOpenAddedConnections = std::thread(&TraceThread<std::function<void()> >, "addcon", std::function<void()>(std::bind(&CConnman::ThreadOpenAddedConnections, this)));

    if (connOptions.m_use_addrman_outgoing && !connOptions.m_specified_outgoing.empty()) {
        if (clientInterface) {
            clientInterface->ThreadSafeMessageBox(
                _("Cannot provide specific connections and have addrman find outgoing connections at the same."),
                "", CClientUIInterface::MSG_ERROR);
        }
        return false;
    }
    if (connOptions.m_use_addrman_outgoing || !connOptions.m_specified_outgoing.empty())
        threadOpenConnections = std::thread(&TraceThread<std::function<void()> >, "opencon", std::function<void()>(std::bind(&CConnman::ThreadOpenConnections, this, connOptions.m_specified_outgoing)));

    // Process messages
    threadMessageHandler = std::thread(&TraceThread<std::function<void()> >, "msghand", std::function<void()>(std::bind(&CConnman::ThreadMessageHandler, this)));

    // Dump network addresses
<<<<<<< HEAD
    scheduler.scheduleEvery([this] { DumpAddresses(); }, DUMP_PEERS_INTERVAL);
=======
    threadGroup.create_thread(boost::bind(&LoopForever<void (*)()>, "dumpaddr", &DumpAddresses, DUMP_ADDRESSES_INTERVAL * 1000));
}

bool StopNode()
{
    printf("StopNode()\n");
    GenerateBitcoins(false, NULL);
    MapPort(false);
    nTransactionsUpdated++;
    if (semOutbound)
        for (int i=0; i<MAX_OUTBOUND_CONNECTIONS; i++)
            semOutbound->post();
    MilliSleep(50);
    DumpAddresses();
>>>>>>> 0.8

    return true;
}

class CNetCleanup
{
public:
    CNetCleanup() {}

    ~CNetCleanup()
    {
#ifdef WIN32
        // Shutdown Windows Sockets
        WSACleanup();
#endif
    }
};
static CNetCleanup instance_of_cnetcleanup;

void CConnman::Interrupt()
{
    {
        LOCK(mutexMsgProc);
        flagInterruptMsgProc = true;
    }
    condMsgProc.notify_all();

    interruptNet();
    InterruptSocks5(true);

    if (semOutbound) {
        for (int i=0; i<m_max_outbound; i++) {
            semOutbound->post();
        }
    }

    if (semAddnode) {
        for (int i=0; i<nMaxAddnode; i++) {
            semAddnode->post();
        }
    }
}

void CConnman::StopThreads()
{
    if (threadMessageHandler.joinable())
        threadMessageHandler.join();
    if (threadOpenConnections.joinable())
        threadOpenConnections.join();
    if (threadOpenAddedConnections.joinable())
        threadOpenAddedConnections.join();
    if (threadDNSAddressSeed.joinable())
        threadDNSAddressSeed.join();
    if (threadSocketHandler.joinable())
        threadSocketHandler.join();
}

void CConnman::StopNodes()
{
    if (fAddressesInitialized) {
        DumpAddresses();
        fAddressesInitialized = false;

        if (m_use_addrman_outgoing) {
            // Anchor connections are only dumped during clean shutdown.
            std::vector<CAddress> anchors_to_dump = GetCurrentBlockRelayOnlyConns();
            if (anchors_to_dump.size() > MAX_BLOCK_RELAY_ONLY_ANCHORS) {
                anchors_to_dump.resize(MAX_BLOCK_RELAY_ONLY_ANCHORS);
            }
            DumpAnchors(GetDataDir() / ANCHORS_DATABASE_FILENAME, anchors_to_dump);
        }
    }

    // Close sockets
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes)
        pnode->CloseSocketDisconnect();
    for (ListenSocket& hListenSocket : vhListenSocket)
        if (hListenSocket.socket != INVALID_SOCKET)
            if (!CloseSocket(hListenSocket.socket))
                LogPrintf("CloseSocket(hListenSocket) failed with error %s\n", NetworkErrorString(WSAGetLastError()));

    // clean up some globals (to help leak detection)
    for (CNode* pnode : vNodes) {
        DeleteNode(pnode);
    }
    for (CNode* pnode : vNodesDisconnected) {
        DeleteNode(pnode);
    }
    vNodes.clear();
    vNodesDisconnected.clear();
    vhListenSocket.clear();
    semOutbound.reset();
    semAddnode.reset();
}

void CConnman::DeleteNode(CNode* pnode)
{
    assert(pnode);
    bool fUpdateConnectionTime = false;
    m_msgproc->FinalizeNode(*pnode, fUpdateConnectionTime);
    if (fUpdateConnectionTime) {
        addrman.Connected(pnode->addr);
    }
    delete pnode;
}

CConnman::~CConnman()
{
    Interrupt();
    Stop();
}

void CConnman::SetServices(const CService &addr, ServiceFlags nServices)
{
    addrman.SetServices(addr, nServices);
}

void CConnman::MarkAddressGood(const CAddress& addr)
{
    addrman.Good(addr);
}

bool CConnman::AddNewAddresses(const std::vector<CAddress>& vAddr, const CAddress& addrFrom, int64_t nTimePenalty)
{
    return addrman.Add(vAddr, addrFrom, nTimePenalty);
}

std::vector<CAddress> CConnman::GetAddresses(size_t max_addresses, size_t max_pct)
{
    std::vector<CAddress> addresses = addrman.GetAddr(max_addresses, max_pct);
    if (m_banman) {
        addresses.erase(std::remove_if(addresses.begin(), addresses.end(),
                        [this](const CAddress& addr){return m_banman->IsDiscouraged(addr) || m_banman->IsBanned(addr);}),
                        addresses.end());
    }
    return addresses;
}

std::vector<CAddress> CConnman::GetAddresses(CNode& requestor, size_t max_addresses, size_t max_pct)
{
    SOCKET socket;
    WITH_LOCK(requestor.cs_hSocket, socket = requestor.hSocket);
    auto local_socket_bytes = GetBindAddress(socket).GetAddrBytes();
    uint64_t cache_id = GetDeterministicRandomizer(RANDOMIZER_ID_ADDRCACHE)
        .Write(requestor.addr.GetNetwork())
        .Write(local_socket_bytes.data(), local_socket_bytes.size())
        .Finalize();
    const auto current_time = GetTime<std::chrono::microseconds>();
    auto r = m_addr_response_caches.emplace(cache_id, CachedAddrResponse{});
    CachedAddrResponse& cache_entry = r.first->second;
    if (cache_entry.m_cache_entry_expiration < current_time) { // If emplace() added new one it has expiration 0.
        cache_entry.m_addrs_response_cache = GetAddresses(max_addresses, max_pct);
        // Choosing a proper cache lifetime is a trade-off between the privacy leak minimization
        // and the usefulness of ADDR responses to honest users.
        //
        // Longer cache lifetime makes it more difficult for an attacker to scrape
        // enough AddrMan data to maliciously infer something useful.
        // By the time an attacker scraped enough AddrMan records, most of
        // the records should be old enough to not leak topology info by
        // e.g. analyzing real-time changes in timestamps.
        //
        // It takes only several hundred requests to scrape everything from an AddrMan containing 100,000 nodes,
        // so ~24 hours of cache lifetime indeed makes the data less inferable by the time
        // most of it could be scraped (considering that timestamps are updated via
        // ADDR self-announcements and when nodes communicate).
        // We also should be robust to those attacks which may not require scraping *full* victim's AddrMan
        // (because even several timestamps of the same handful of nodes may leak privacy).
        //
        // On the other hand, longer cache lifetime makes ADDR responses
        // outdated and less useful for an honest requestor, e.g. if most nodes
        // in the ADDR response are no longer active.
        //
        // However, the churn in the network is known to be rather low. Since we consider
        // nodes to be "terrible" (see IsTerrible()) if the timestamps are older than 30 days,
        // max. 24 hours of "penalty" due to cache shouldn't make any meaningful difference
        // in terms of the freshness of the response.
        cache_entry.m_cache_entry_expiration = current_time + std::chrono::hours(21) + GetRandMillis(std::chrono::hours(6));
    }
    return cache_entry.m_addrs_response_cache;
}

bool CConnman::AddNode(const std::string& strNode)
{
    LOCK(cs_vAddedNodes);
    for (const std::string& it : vAddedNodes) {
        if (strNode == it) return false;
    }

    vAddedNodes.push_back(strNode);
    return true;
}

bool CConnman::RemoveAddedNode(const std::string& strNode)
{
    LOCK(cs_vAddedNodes);
    for(std::vector<std::string>::iterator it = vAddedNodes.begin(); it != vAddedNodes.end(); ++it) {
        if (strNode == *it) {
            vAddedNodes.erase(it);
            return true;
        }
    }
    return false;
}

size_t CConnman::GetNodeCount(NumConnections flags)
{
    LOCK(cs_vNodes);
    if (flags == CConnman::CONNECTIONS_ALL) // Shortcut if we want total
        return vNodes.size();

    int nNum = 0;
    for (const auto& pnode : vNodes) {
        if (flags & (pnode->IsInboundConn() ? CONNECTIONS_IN : CONNECTIONS_OUT)) {
            nNum++;
        }
    }

    return nNum;
}

void CConnman::GetNodeStats(std::vector<CNodeStats>& vstats)
{
    vstats.clear();
    LOCK(cs_vNodes);
    vstats.reserve(vNodes.size());
    for (CNode* pnode : vNodes) {
        vstats.emplace_back();
        pnode->copyStats(vstats.back(), addrman.m_asmap);
    }
}

bool CConnman::DisconnectNode(const std::string& strNode)
{
    LOCK(cs_vNodes);
    if (CNode* pnode = FindNode(strNode)) {
        pnode->fDisconnect = true;
        return true;
    }
    return false;
}

bool CConnman::DisconnectNode(const CSubNet& subnet)
{
    bool disconnected = false;
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes) {
        if (subnet.Match(pnode->addr)) {
            pnode->fDisconnect = true;
            disconnected = true;
        }
    }
    return disconnected;
}

bool CConnman::DisconnectNode(const CNetAddr& addr)
{
    return DisconnectNode(CSubNet(addr));
}

bool CConnman::DisconnectNode(NodeId id)
{
    LOCK(cs_vNodes);
    for(CNode* pnode : vNodes) {
        if (id == pnode->GetId()) {
            pnode->fDisconnect = true;
            return true;
        }
    }
    return false;
}

void CConnman::RecordBytesRecv(uint64_t bytes)
{
    LOCK(cs_totalBytesRecv);
    nTotalBytesRecv += bytes;
}

void CConnman::RecordBytesSent(uint64_t bytes)
{
    LOCK(cs_totalBytesSent);
    nTotalBytesSent += bytes;

    uint64_t now = GetTime();
    if (nMaxOutboundCycleStartTime + nMaxOutboundTimeframe < now)
    {
        // timeframe expired, reset cycle
        nMaxOutboundCycleStartTime = now;
        nMaxOutboundTotalBytesSentInCycle = 0;
    }

    // TODO, exclude peers with download permission
    nMaxOutboundTotalBytesSentInCycle += bytes;
}

void CConnman::SetMaxOutboundTarget(uint64_t limit)
{
    LOCK(cs_totalBytesSent);
    nMaxOutboundLimit = limit;
}

uint64_t CConnman::GetMaxOutboundTarget()
{
    LOCK(cs_totalBytesSent);
    return nMaxOutboundLimit;
}

uint64_t CConnman::GetMaxOutboundTimeframe()
{
    LOCK(cs_totalBytesSent);
    return nMaxOutboundTimeframe;
}

uint64_t CConnman::GetMaxOutboundTimeLeftInCycle()
{
    LOCK(cs_totalBytesSent);
    if (nMaxOutboundLimit == 0)
        return 0;

    if (nMaxOutboundCycleStartTime == 0)
        return nMaxOutboundTimeframe;

    uint64_t cycleEndTime = nMaxOutboundCycleStartTime + nMaxOutboundTimeframe;
    uint64_t now = GetTime();
    return (cycleEndTime < now) ? 0 : cycleEndTime - GetTime();
}

void CConnman::SetMaxOutboundTimeframe(uint64_t timeframe)
{
    LOCK(cs_totalBytesSent);
    if (nMaxOutboundTimeframe != timeframe)
    {
        // reset measure-cycle in case of changing
        // the timeframe
        nMaxOutboundCycleStartTime = GetTime();
    }
    nMaxOutboundTimeframe = timeframe;
}

bool CConnman::OutboundTargetReached(bool historicalBlockServingLimit)
{
    LOCK(cs_totalBytesSent);
    if (nMaxOutboundLimit == 0)
        return false;

    if (historicalBlockServingLimit)
    {
        // keep a large enough buffer to at least relay each block once
        uint64_t timeLeftInCycle = GetMaxOutboundTimeLeftInCycle();
        uint64_t buffer = timeLeftInCycle / 600 * MAX_BLOCK_SERIALIZED_SIZE_WITH_MWEB;
        if (buffer >= nMaxOutboundLimit || nMaxOutboundTotalBytesSentInCycle >= nMaxOutboundLimit - buffer)
            return true;
    }
    else if (nMaxOutboundTotalBytesSentInCycle >= nMaxOutboundLimit)
        return true;

    return false;
}

uint64_t CConnman::GetOutboundTargetBytesLeft()
{
    LOCK(cs_totalBytesSent);
    if (nMaxOutboundLimit == 0)
        return 0;

    return (nMaxOutboundTotalBytesSentInCycle >= nMaxOutboundLimit) ? 0 : nMaxOutboundLimit - nMaxOutboundTotalBytesSentInCycle;
}

uint64_t CConnman::GetTotalBytesRecv()
{
    LOCK(cs_totalBytesRecv);
    return nTotalBytesRecv;
}

uint64_t CConnman::GetTotalBytesSent()
{
    LOCK(cs_totalBytesSent);
    return nTotalBytesSent;
}

ServiceFlags CConnman::GetLocalServices() const
{
    return nLocalServices;
}

void CConnman::SetBestHeight(int height)
{
    nBestHeight.store(height, std::memory_order_release);
}

int CConnman::GetBestHeight() const
{
    return nBestHeight.load(std::memory_order_acquire);
}

unsigned int CConnman::GetReceiveFloodSize() const { return nReceiveFloodSize; }

CNode::CNode(NodeId idIn, ServiceFlags nLocalServicesIn, int nMyStartingHeightIn, SOCKET hSocketIn, const CAddress& addrIn, uint64_t nKeyedNetGroupIn, uint64_t nLocalHostNonceIn, const CAddress& addrBindIn, const std::string& addrNameIn, ConnectionType conn_type_in, bool inbound_onion)
    : nTimeConnected(GetSystemTimeInSeconds()),
    addr(addrIn),
    addrBind(addrBindIn),
    nKeyedNetGroup(nKeyedNetGroupIn),
    // Don't relay addr messages to peers that we connect to as block-relay-only
    // peers (to prevent adversaries from inferring these links from addr
    // traffic).
    id(idIn),
    nLocalHostNonce(nLocalHostNonceIn),
    m_conn_type(conn_type_in),
    nLocalServices(nLocalServicesIn),
    nMyStartingHeight(nMyStartingHeightIn),
    m_inbound_onion(inbound_onion)
{
    hSocket = hSocketIn;
    addrName = addrNameIn == "" ? addr.ToStringIPPort() : addrNameIn;
    hashContinue = uint256();
    if (conn_type_in != ConnectionType::BLOCK_RELAY) {
        m_tx_relay = MakeUnique<TxRelay>();
    }

    if (RelayAddrsWithConn()) {
        m_addr_known = MakeUnique<CRollingBloomFilter>(5000, 0.001);
    }

    for (const std::string &msg : getAllNetMessageTypes())
        mapRecvBytesPerMsgCmd[msg] = 0;
    mapRecvBytesPerMsgCmd[NET_MESSAGE_COMMAND_OTHER] = 0;

    if (fLogIPs) {
        LogPrint(BCLog::NET, "Added connection to %s peer=%d\n", addrName, id);
    } else {
        LogPrint(BCLog::NET, "Added connection peer=%d\n", id);
    }

    m_deserializer = MakeUnique<V1TransportDeserializer>(V1TransportDeserializer(Params(), GetId(), SER_NETWORK, INIT_PROTO_VERSION));
    m_serializer = MakeUnique<V1TransportSerializer>(V1TransportSerializer());
}

CNode::~CNode()
{
    CloseSocket(hSocket);
}

bool CConnman::NodeFullyConnected(const CNode* pnode)
{
    return pnode && pnode->fSuccessfullyConnected && !pnode->fDisconnect;
}

void CConnman::PushMessage(CNode* pnode, CSerializedNetMsg&& msg)
{
    size_t nMessageSize = msg.data.size();
    LogPrint(BCLog::NET, "sending %s (%d bytes) peer=%d\n",  SanitizeString(msg.m_type), nMessageSize, pnode->GetId());

    // make sure we use the appropriate network transport format
    std::vector<unsigned char> serializedHeader;
    pnode->m_serializer->prepareForTransport(msg, serializedHeader);
    size_t nTotalSize = nMessageSize + serializedHeader.size();

    size_t nBytesSent = 0;
    {
        LOCK(pnode->cs_vSend);
        bool optimisticSend(pnode->vSendMsg.empty());

        //log total amount of bytes per message type
        pnode->mapSendBytesPerMsgCmd[msg.m_type] += nTotalSize;
        pnode->nSendSize += nTotalSize;

        if (pnode->nSendSize > nSendBufferMaxSize)
            pnode->fPauseSend = true;
        pnode->vSendMsg.push_back(std::move(serializedHeader));
        if (nMessageSize)
            pnode->vSendMsg.push_back(std::move(msg.data));

        // If write queue empty, attempt "optimistic write"
        if (optimisticSend == true)
            nBytesSent = SocketSendData(pnode);
    }
    if (nBytesSent)
        RecordBytesSent(nBytesSent);
}

bool CConnman::ForNode(NodeId id, std::function<bool(CNode* pnode)> func)
{
    CNode* found = nullptr;
    LOCK(cs_vNodes);
    for (auto&& pnode : vNodes) {
        if(pnode->GetId() == id) {
            found = pnode;
            break;
        }
    }
    return found != nullptr && NodeFullyConnected(found) && func(found);
}

int64_t CConnman::PoissonNextSendInbound(int64_t now, int average_interval_seconds)
{
    if (m_next_send_inv_to_incoming < now) {
        // If this function were called from multiple threads simultaneously
        // it would possible that both update the next send variable, and return a different result to their caller.
        // This is not possible in practice as only the net processing thread invokes this function.
        m_next_send_inv_to_incoming = PoissonNextSend(now, average_interval_seconds);
    }
    return m_next_send_inv_to_incoming;
}

int64_t PoissonNextSend(int64_t now, int average_interval_seconds)
{
    return now + (int64_t)(log1p(GetRand(1ULL << 48) * -0.0000000000000035527136788 /* -1/2^48 */) * average_interval_seconds * -1000000.0 + 0.5);
}

CSipHasher CConnman::GetDeterministicRandomizer(uint64_t id) const
{
    return CSipHasher(nSeed0, nSeed1).Write(id);
}

uint64_t CConnman::CalculateKeyedNetGroup(const CAddress& ad) const
{
    std::vector<unsigned char> vchNetGroup(ad.GetGroup(addrman.m_asmap));

    return GetDeterministicRandomizer(RANDOMIZER_ID_NETGROUP).Write(vchNetGroup.data(), vchNetGroup.size()).Finalize();
}
