#!/usr/bin/env python2
# Copyright (c) 2014-2015 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

#
# Test REST interface
#


from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import *
from struct import *
from io import BytesIO
from codecs import encode
import binascii

try:
    import http.client as httplib
except ImportError:
    import httplib
try:
    import urllib.parse as urlparse
except ImportError:
    import urlparse

def deser_uint256(f):
    r = 0
    for i in range(8):
        t = unpack(b"<I", f.read(4))[0]
        r += t << (i * 32)
    return r

#allows simple http get calls
def http_get_call(host, port, path, response_object = 0):
    conn = httplib.HTTPConnection(host, port)
    conn.request('GET', path)

    if response_object:
        return conn.getresponse()

    return conn.getresponse().read().decode('utf-8')

#allows simple http post calls with a request body
def http_post_call(host, port, path, requestdata = '', response_object = 0):
    conn = httplib.HTTPConnection(host, port)
    conn.request('POST', path, requestdata)

    if response_object:
        return conn.getresponse()

    return conn.getresponse().read()

class RESTTest (BitcoinTestFramework):
    FORMAT_SEPARATOR = "."

    def setup_chain(self):
        print("Initializing test directory "+self.options.tmpdir)
        initialize_chain_clean(self.options.tmpdir, 3)

    def setup_network(self, split=False):
        self.nodes = start_nodes(3, self.options.tmpdir)
        connect_nodes_bi(self.nodes,0,1)
        connect_nodes_bi(self.nodes,1,2)
        connect_nodes_bi(self.nodes,0,2)
        self.is_network_split=False
        self.sync_all()

    def run_test(self):
        url = urlparse.urlparse(self.nodes[0].url)
        print "Mining blocks..."

        self.nodes[0].generate(1)
        self.sync_all()
        self.nodes[2].generate(100)
        self.sync_all()

        assert_equal(self.nodes[0].getbalance(), 500)

        txid = self.nodes[0].sendtoaddress(self.nodes[1].getnewaddress(), 0.1)
        self.sync_all()
        self.nodes[2].generate(1)
        self.sync_all()
        bb_hash = self.nodes[0].getbestblockhash()

        assert_equal(self.nodes[1].getbalance(), Decimal("0.1")) #balance now should be 0.1 on node 1

        # load the latest 0.1 tx over the REST API
        json_string = http_get_call(url.hostname, url.port, '/rest/tx/'+txid+self.FORMAT_SEPARATOR+"json")
        json_obj = json.loads(json_string)
        vintx = json_obj['vin'][0]['txid'] # get the vin to later check for utxo (should be spent by then)
        # get n of 0.1 outpoint
        n = 0
        for vout in json_obj['vout']:
            if vout['value'] == 0.1:
                n = vout['n']


        ######################################
        # GETUTXOS: query a unspent outpoint #
        ######################################
        json_request = '/checkmempool/'+txid+'-'+str(n)
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)

        #check chainTip response
        assert_equal(json_obj['chaintipHash'], bb_hash)

        #make sure there is one utxo
        assert_equal(len(json_obj['utxos']), 1)
        assert_equal(json_obj['utxos'][0]['value'], 0.1)


        ################################################
        # GETUTXOS: now query a already spent outpoint #
        ################################################
        json_request = '/checkmempool/'+vintx+'-0'
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)

        #check chainTip response
        assert_equal(json_obj['chaintipHash'], bb_hash)

        #make sure there is no utox in the response because this oupoint has been spent
        assert_equal(len(json_obj['utxos']), 0)

        #check bitmap
        assert_equal(json_obj['bitmap'], "0")


        ##################################################
        # GETUTXOS: now check both with the same request #
        ##################################################
        json_request = '/checkmempool/'+txid+'-'+str(n)+'/'+vintx+'-0'
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)
        assert_equal(len(json_obj['utxos']), 1)
        assert_equal(json_obj['bitmap'], "10")

        #test binary response
        bb_hash = self.nodes[0].getbestblockhash()

        binaryRequest = b'\x01\x02'
        binaryRequest += hex_str_to_bytes(txid)
        binaryRequest += pack("i", n)
        binaryRequest += hex_str_to_bytes(vintx)
        binaryRequest += pack("i", 0)

        bin_response = http_post_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'bin', binaryRequest)
        output = BytesIO()
        output.write(bin_response)
        output.seek(0)
        chainHeight = unpack("i", output.read(4))[0]
        hashFromBinResponse = hex(deser_uint256(output))[2:].zfill(65).rstrip("L")

        assert_equal(bb_hash, hashFromBinResponse) #check if getutxo's chaintip during calculation was fine
        assert_equal(chainHeight, 102) #chain height must be 102


        ############################
        # GETUTXOS: mempool checks #
        ############################

        # do a tx and don't sync
        txid = self.nodes[0].sendtoaddress(self.nodes[1].getnewaddress(), 0.1)
        json_string = http_get_call(url.hostname, url.port, '/rest/tx/'+txid+self.FORMAT_SEPARATOR+"json")
        json_obj = json.loads(json_string)
        vintx = json_obj['vin'][0]['txid'] # get the vin to later check for utxo (should be spent by then)
        # get n of 0.1 outpoint
        n = 0
        for vout in json_obj['vout']:
            if vout['value'] == 0.1:
                n = vout['n']

        json_request = '/'+txid+'-'+str(n)
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)
        assert_equal(len(json_obj['utxos']), 0) #there should be a outpoint because it has just added to the mempool

        json_request = '/checkmempool/'+txid+'-'+str(n)
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)
        assert_equal(len(json_obj['utxos']), 1) #there should be a outpoint because it has just added to the mempool

        #do some invalid requests
        json_request = '{"checkmempool'
        response = http_post_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'json', json_request, True)
        assert_equal(response.status, 500) #must be a 500 because we send a invalid json request

        json_request = '{"checkmempool'
        response = http_post_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'bin', json_request, True)
        assert_equal(response.status, 500) #must be a 500 because we send a invalid bin request

        response = http_post_call(url.hostname, url.port, '/rest/getutxos/checkmempool'+self.FORMAT_SEPARATOR+'bin', '', True)
        assert_equal(response.status, 500) #must be a 500 because we send a invalid bin request

        #test limits
        json_request = '/checkmempool/'
        for x in range(0, 20):
            json_request += txid+'-'+str(n)+'/'
        json_request = json_request.rstrip("/")
        response = http_post_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json', '', True)
        assert_equal(response.status, 500) #must be a 500 because we exceeding the limits

        json_request = '/checkmempool/'
        for x in range(0, 15):
            json_request += txid+'-'+str(n)+'/'
        json_request = json_request.rstrip("/")
        response = http_post_call(url.hostname, url.port, '/rest/getu#!/usr/bin/env python2
# Copyright (c) 2014-2015 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

#
# Test REST interface
#


from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import *
from struct import *
from io import BytesIO
from codecs import encode
import binascii

try:
    import http.client as httplib
except ImportError:
    import httplib
try:
    import urllib.parse as urlparse
except ImportError:
    import urlparse

def deser_uint256(f):
    r = 0
    for i in range(8):
        t = unpack(b"<I", f.read(4))[0]
        r += t << (i * 32)
    return r

#allows simple http get calls
def http_get_call(host, port, path, response_object = 0):
    conn = httplib.HTTPConnection(host, port)
    conn.request('GET', path)

    if response_object:
        return conn.getresponse()

    return conn.getresponse().read().decode('utf-8')

#allows simple http post calls with a request body
def http_post_call(host, port, path, requestdata = '', response_object = 0):
    conn = httplib.HTTPConnection(host, port)
    conn.request('POST', path, requestdata)

    if response_object:
        return conn.getresponse()

    return conn.getresponse().read()

class RESTTest (BitcoinTestFramework):
    FORMAT_SEPARATOR = "."

    def setup_chain(self):
        print("Initializing test directory "+self.options.tmpdir)
        initialize_chain_clean(self.options.tmpdir, 3)

    def setup_network(self, split=False):
        self.nodes = start_nodes(3, self.options.tmpdir)
        connect_nodes_bi(self.nodes,0,1)
        connect_nodes_bi(self.nodes,1,2)
        connect_nodes_bi(self.nodes,0,2)
        self.is_network_split=False
        self.sync_all()

    def run_test(self):
        url = urlparse.urlparse(self.nodes[0].url)
        print "Mining blocks..."

        self.nodes[0].generate(1)
        self.sync_all()
        self.nodes[2].generate(100)
        self.sync_all()

        assert_equal(self.nodes[0].getbalance(), 500)

        txid = self.nodes[0].sendtoaddress(self.nodes[1].getnewaddress(), 0.1)
        self.sync_all()
        self.nodes[2].generate(1)
        self.sync_all()
        bb_hash = self.nodes[0].getbestblockhash()

        assert_equal(self.nodes[1].getbalance(), Decimal("0.1")) #balance now should be 0.1 on node 1

        # load the latest 0.1 tx over the REST API
        json_string = http_get_call(url.hostname, url.port, '/rest/tx/'+txid+self.FORMAT_SEPARATOR+"json")
        json_obj = json.loads(json_string)
        vintx = json_obj['vin'][0]['txid'] # get the vin to later check for utxo (should be spent by then)
        # get n of 0.1 outpoint
        n = 0
        for vout in json_obj['vout']:
            if vout['value'] == 0.1:
                n = vout['n']


        ######################################
        # GETUTXOS: query a unspent outpoint #
        ######################################
        json_request = '/checkmempool/'+txid+'-'+str(n)
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)

        #check chainTip response
        assert_equal(json_obj['chaintipHash'], bb_hash)

        #make sure there is one utxo
        assert_equal(len(json_obj['utxos']), 1)
        assert_equal(json_obj['utxos'][0]['value'], 0.1)


        ################################################
        # GETUTXOS: now query a already spent outpoint #
        ################################################
        json_request = '/checkmempool/'+vintx+'-0'
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)

        #check chainTip response
        assert_equal(json_obj['chaintipHash'], bb_hash)

        #make sure there is no utox in the response because this oupoint has been spent
        assert_equal(len(json_obj['utxos']), 0)

        #check bitmap
        assert_equal(json_obj['bitmap'], "0")


        ##################################################
        # GETUTXOS: now check both with the same request #
        ##################################################
        json_request = '/checkmempool/'+txid+'-'+str(n)+'/'+vintx+'-0'
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)
        assert_equal(len(json_obj['utxos']), 1)
        assert_equal(json_obj['bitmap'], "10")

        #test binary response
        bb_hash = self.nodes[0].getbestblockhash()

        binaryRequest = b'\x01\x02'
        binaryRequest += hex_str_to_bytes(txid)
        binaryRequest += pack("i", n)
        binaryRequest += hex_str_to_bytes(vintx)
        binaryRequest += pack("i", 0)

        bin_response = http_post_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'bin', binaryRequest)
        output = BytesIO()
        output.write(bin_response)
        output.seek(0)
        chainHeight = unpack("i", output.read(4))[0]
        hashFromBinResponse = hex(deser_uint256(output))[2:].zfill(65).rstrip("L")

        assert_equal(bb_hash, hashFromBinResponse) #check if getutxo's chaintip during calculation was fine
        assert_equal(chainHeight, 102) #chain height must be 102


        ############################
        # GETUTXOS: mempool checks #
        ############################

        # do a tx and don't sync
        txid = self.nodes[0].sendtoaddress(self.nodes[1].getnewaddress(), 0.1)
        json_string = http_get_call(url.hostname, url.port, '/rest/tx/'+txid+self.FORMAT_SEPARATOR+"json")
        json_obj = json.loads(json_string)
        vintx = json_obj['vin'][0]['txid'] # get the vin to later check for utxo (should be spent by then)
        # get n of 0.1 outpoint
        n = 0
        for vout in json_obj['vout']:
            if vout['value'] == 0.1:
                n = vout['n']

        json_request = '/'+txid+'-'+str(n)
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)
        assert_equal(len(json_obj['utxos']), 0) #there should be a outpoint because it has just added to the mempool

        json_request = '/checkmempool/'+txid+'-'+str(n)
        json_string = http_get_call(url.hostname, url.port, '/rest/getutxos'+json_request+self.FORMAT_SEPARATOR+'json')
        json_obj = json.loads(json_string)
        assert_equal(len(json_obj['utxos']), 1) #there should be a outpoint because it has just added to the mempool

        #do some invalid requests
        json_request = '{"checkmempool'
        response = http_post_call(url.hostname, url.port, '/rest/getutxos'+self.FORMAT_SEPARATOR+'json', json_request, True)
        assert_equal(response.status, 500) #must be a 500 because we send a invalid json request

        json_request = '{"checkmempool'
        response = htt