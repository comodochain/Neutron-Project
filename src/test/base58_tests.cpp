// Copyright (c) 2011-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <test/data/base58_encode_decode.json.h>

#include <base58.h>
#include <test/util/setup_common.h>
#include <util/strencodings.h>
#include <util/vector.h>

#include <univalue.h>

#include <boost/test/unit_test.hpp>


extern UniValue read_json(const std::string& jsondata);

BOOST_FIXTURE_TEST_SUITE(base58_tests, BasicTestingSetup)

// Goal: test low-level base58 encoding functionality
BOOST_AUTO_TEST_CASE(base58_EncodeBase58)
{
    UniValue tests = read_json(std::string(json_tests::base58_encode_decode, json_tests::base58_encode_decode + sizeof(json_tests::base58_encode_decode)));
    for (unsigned int idx = 0; idx < tests.size(); idx++) {
        UniValue test = tests[idx];
        std::string strTest = test.write();
        if (test.size() < 2) // Allow for extra stuff (useful for comments)
        {
            BOOST_ERROR("Bad test: " << strTest);
            continue;
        }
        std::vector<unsigned char> sourcedata = ParseHex(test[0].get_str());
        std::string base58string = test[1].get_str();
        BOOST_CHECK_MESSAGE(
                    EncodeBase58(sourcedata) == base58string,
                    strTest);
    }
}

// Goal: test low-level base58 decoding functionality
BOOST_AUTO_TEST_CASE(base58_DecodeBase58)
{
    UniValue tests = read_json(std::string(json_tests::base58_encode_decode, json_tests::base58_encode_decode + sizeof(json_tests::base58_encode_decode)));
    std::vector<unsigned char> result;

    for (unsigned int idx = 0; idx < tests.size(); idx++) {
        UniValue test = tests[idx];
        std::string strTest = test.write();
        if (test.size() < 2) // Allow for extra stuff (useful for comments)
        {
            BOOST_ERROR("Bad test: " << strTest);
            continue;
        }
        std::vector<unsigned char> expected = ParseHex(test[0].get_str());
        std::string base58string = test[1].get_str();
        BOOST_CHECK_MESSAGE(DecodeBase58(base58string, result, 256), strTest);
        BOOST_CHECK_MESSAGE(result.size() == expected.size() && std::equal(result.begin(), result.end(), expected.begin()), strTest);
    }

    BOOST_CHECK(!DecodeBase58("invalid", result, 100));
    BOOST_CHECK(!DecodeBase58(std::string("invalid"), result, 100));
    BOOST_CHECK(!DecodeBase58(std::string("\0invalid", 8), result, 100));

    BOOST_CHECK(DecodeBase58(std::string("good", 4), result, 100));
    BOOST_CHECK(!DecodeBase58(std::string("bad0IOl", 7), result, 100));
    BOOST_CHECK(!DecodeBase58(std::string("goodbad0IOl", 11), result, 100));
    BOOST_CHECK(!DecodeBase58(std::string("good\0bad0IOl", 12), result, 100));

    // check that DecodeBase58 skips whitespace, but still fails with unexpected non-whitespace at the end.
    BOOST_CHECK(!DecodeBase58(" \t\n\v\f\r skip \r\f\v\n\t a", result, 3));
    BOOST_CHECK( DecodeBase58(" \t\n\v\f\r skip \r\f\v\n\t ", result, 3));
    std::vector<unsigned char> expected = ParseHex("971a55");
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());

    BOOST_CHECK(DecodeBase58Check(std::string("3vQB7B6MrGQZaxCuFg4oh", 21), result, 100));
    BOOST_CHECK(!DecodeBase58Check(std::string("3vQB7B6MrGQZaxCuFg4oi", 21), result, 100));
    BOOST_CHECK(!DecodeBase58Check(std::string("3vQB7B6MrGQZaxCuFg4oh0IOl", 25), result, 100));
    BOOST_CHECK(!DecodeBase58Check(std::string("3vQB7B6MrGQZaxCuFg4oh\00IOl", 26), result, 100));
}

BOOST_AUTO_TEST_CASE(base58_random_encode_decode)
{
<<<<<<< HEAD
    for (int n = 0; n < 1000; ++n) {
        unsigned int len = 1 + InsecureRandBits(8);
        unsigned int zeroes = InsecureRandBool() ? InsecureRandRange(len + 1) : 0;
        auto data = Cat(std::vector<unsigned char>(zeroes, '\000'), g_insecure_rand_ctx.randbytes(len - zeroes));
        auto encoded = EncodeBase58Check(data);
        std::vector<unsigned char> decoded;
        auto ok_too_small = DecodeBase58Check(encoded, decoded, InsecureRandRange(len));
        BOOST_CHECK(!ok_too_small);
        auto ok = DecodeBase58Check(encoded, decoded, len + InsecureRandRange(257 - len));
        BOOST_CHECK(ok);
        BOOST_CHECK(data == decoded);
=======
private:
    std::string exp_addrType;
public:
    TestAddrTypeVisitor(const std::string &exp_addrType) : exp_addrType(exp_addrType) { }
    bool operator()(const CKeyID &id) const
    {
        return (exp_addrType == "pubkey");
    }
    bool operator()(const CScriptID &id) const
    {
        return (exp_addrType == "script");
    }
    bool operator()(const CNoDestination &no) const
    {
        return (exp_addrType == "none");
    }
};

// Visitor to check address payload
class TestPayloadVisitor : public boost::static_visitor<bool>
{
private:
    std::vector<unsigned char> exp_payload;
public:
    TestPayloadVisitor(std::vector<unsigned char> &exp_payload) : exp_payload(exp_payload) { }
    bool operator()(const CKeyID &id) const
    {
        uint160 exp_key(exp_payload);
        return exp_key == id;
    }
    bool operator()(const CScriptID &id) const
    {
        uint160 exp_key(exp_payload);
        return exp_key == id;
    }
    bool operator()(const CNoDestination &no) const
    {
        return exp_payload.size() == 0;
    }
};

// Goal: check that parsed keys match test payload
BOOST_AUTO_TEST_CASE(base58_keys_valid_parse)
{
    Array tests = read_json("base58_keys_valid.json");
    std::vector<unsigned char> result;
    CBitcoinSecret secret;
    CBitcoinAddress addr;
    // Save global state
    bool fTestNet_stored = fTestNet;

    BOOST_FOREACH(Value& tv, tests)
    {
        Array test = tv.get_array();
        std::string strTest = write_string(tv, false);
        if (test.size() < 3) // Allow for extra stuff (useful for comments)
        {
            BOOST_ERROR("Bad test: " << strTest);
            continue;
        }
        std::string exp_base58string = test[0].get_str();
        std::vector<unsigned char> exp_payload = ParseHex(test[1].get_str());
        const Object &metadata = test[2].get_obj();
        bool isPrivkey = find_value(metadata, "isPrivkey").get_bool();
        bool isTestnet = find_value(metadata, "isTestnet").get_bool();
        fTestNet = isTestnet; // Override testnet flag
        if(isPrivkey)
        {
            bool isCompressed = find_value(metadata, "isCompressed").get_bool();
            // Must be valid private key
            // Note: CBitcoinSecret::SetString tests isValid, whereas CBitcoinAddress does not!
            BOOST_CHECK_MESSAGE(secret.SetString(exp_base58string), "!SetString:"+ strTest);
            BOOST_CHECK_MESSAGE(secret.IsValid(), "!IsValid:" + strTest);
            CKey privkey = secret.GetKey();
            BOOST_CHECK_MESSAGE(privkey.IsCompressed() == isCompressed, "compressed mismatch:" + strTest);
            BOOST_CHECK_MESSAGE(privkey.size() == exp_payload.size() && std::equal(privkey.begin(), privkey.end(), exp_payload.begin()), "key mismatch:" + strTest);

            // Private key must be invalid public key
            addr.SetString(exp_base58string);
            BOOST_CHECK_MESSAGE(!addr.IsValid(), "IsValid privkey as pubkey:" + strTest);
        }
        else
        {
            std::string exp_addrType = find_value(metadata, "addrType").get_str(); // "script" or "pubkey"
            // Must be valid public key
            BOOST_CHECK_MESSAGE(addr.SetString(exp_base58string), "SetString:" + strTest);
            BOOST_CHECK_MESSAGE(addr.IsValid(), "!IsValid:" + strTest);
            BOOST_CHECK_MESSAGE(addr.IsScript() == (exp_addrType == "script"), "isScript mismatch" + strTest);
            CTxDestination dest = addr.Get();
            BOOST_CHECK_MESSAGE(boost::apply_visitor(TestAddrTypeVisitor(exp_addrType), dest), "addrType mismatch" + strTest);

            // Public key must be invalid private key
            secret.SetString(exp_base58string);
            BOOST_CHECK_MESSAGE(!secret.IsValid(), "IsValid pubkey as privkey:" + strTest);
        }
    }
    // Restore global state
    fTestNet = fTestNet_stored;
}

// Goal: check that generated keys match test vectors
BOOST_AUTO_TEST_CASE(base58_keys_valid_gen)
{
    Array tests = read_json("base58_keys_valid.json");
    std::vector<unsigned char> result;
    // Save global state
    bool fTestNet_stored = fTestNet;

    BOOST_FOREACH(Value& tv, tests)
    {
        Array test = tv.get_array();
        std::string strTest = write_string(tv, false);
        if (test.size() < 3) // Allow for extra stuff (useful for comments)
        {
            BOOST_ERROR("Bad test: " << strTest);
            continue;
        }
        std::string exp_base58string = test[0].get_str();
        std::vector<unsigned char> exp_payload = ParseHex(test[1].get_str());
        const Object &metadata = test[2].get_obj();
        bool isPrivkey = find_value(metadata, "isPrivkey").get_bool();
        bool isTestnet = find_value(metadata, "isTestnet").get_bool();
        fTestNet = isTestnet; // Override testnet flag
        if(isPrivkey)
        {
            bool isCompressed = find_value(metadata, "isCompressed").get_bool();
            CKey key;
            key.Set(exp_payload.begin(), exp_payload.end(), isCompressed);
            assert(key.IsValid());
            CBitcoinSecret secret;
            secret.SetKey(key);
            BOOST_CHECK_MESSAGE(secret.ToString() == exp_base58string, "result mismatch: " + strTest);
        }
        else
        {
            std::string exp_addrType = find_value(metadata, "addrType").get_str();
            CTxDestination dest;
            if(exp_addrType == "pubkey")
            {
                dest = CKeyID(uint160(exp_payload));
            }
            else if(exp_addrType == "script")
            {
                dest = CScriptID(uint160(exp_payload));
            }
            else if(exp_addrType == "none")
            {
                dest = CNoDestination();
            }
            else
            {
                BOOST_ERROR("Bad addrtype: " << strTest);
                continue;
            }
            CBitcoinAddress addrOut;
            BOOST_CHECK_MESSAGE(boost::apply_visitor(CBitcoinAddressVisitor(&addrOut), dest), "encode dest: " + strTest);
            BOOST_CHECK_MESSAGE(addrOut.ToString() == exp_base58string, "mismatch: " + strTest);
        }
    }

    // Visiting a CNoDestination must fail
    CBitcoinAddress dummyAddr;
    CTxDestination nodest = CNoDestination();
    BOOST_CHECK(!boost::apply_visitor(CBitcoinAddressVisitor(&dummyAddr), nodest));

    // Restore global state
    fTestNet = fTestNet_stored;
}

// Goal: check that base58 parsing code is robust against a variety of corrupted data
BOOST_AUTO_TEST_CASE(base58_keys_invalid)
{
    Array tests = read_json("base58_keys_invalid.json"); // Negative testcases
    std::vector<unsigned char> result;
    CBitcoinSecret secret;
    CBitcoinAddress addr;

    BOOST_FOREACH(Value& tv, tests)
    {
        Array test = tv.get_array();
        std::string strTest = write_string(tv, false);
        if (test.size() < 1) // Allow for extra stuff (useful for comments)
        {
            BOOST_ERROR("Bad test: " << strTest);
            continue;
        }
        std::string exp_base58string = test[0].get_str();

        // must be invalid as public and as private key
        addr.SetString(exp_base58string);
        BOOST_CHECK_MESSAGE(!addr.IsValid(), "IsValid pubkey:" + strTest);
        secret.SetString(exp_base58string);
        BOOST_CHECK_MESSAGE(!secret.IsValid(), "IsValid privkey:" + strTest);
>>>>>>> 0.8
    }
}

BOOST_AUTO_TEST_SUITE_END()
