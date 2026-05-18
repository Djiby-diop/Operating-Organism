import subprocess
import time
import urllib.request
import urllib.error
import json
import uuid
import hmac
import hashlib
import os
import shutil
from datetime import datetime, timezone

# Helper to compute HMAC
def compute_hmac(gossip, key):
    threats_str = ",".join(gossip["observed_threats"])
    sent_ts_ms = int(datetime.fromisoformat(gossip["sent_at"].replace("Z", "+00:00")).timestamp() * 1000)
    payload = f"{gossip['gossip_id']}|{gossip['ttl_hops']}|{gossip['from']['peer_id']}|{gossip['from']['address']}|{gossip['from']['role']}|{gossip['observed_organisms']}|{sent_ts_ms}|{threats_str}"
    return hmac.new(key.encode(), payload.encode(), hashlib.sha256).hexdigest()

def create_gossip(gossip_id, ttl, peer_id, address, key):
    now = datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")
    gossip = {
        "gossip_id": gossip_id,
        "ttl_hops": ttl,
        "from": {
            "peer_id": peer_id,
            "address": address,
            "role": "tester"
        },
        "observed_threats": [],
        "observed_organisms": 1,
        "sent_at": now
    }
    if key:
        gossip["hmac_sha256"] = compute_hmac(gossip, key)
    return gossip

def http_get(url):
    req = urllib.request.Request(url)
    try:
        with urllib.request.urlopen(req) as response:
            return json.loads(response.read().decode())
    except urllib.error.HTTPError as e:
        return json.loads(e.read().decode())
    except Exception:
        return None

def http_post(url, data):
    req = urllib.request.Request(url, data=json.dumps(data).encode(), headers={'Content-Type': 'application/json'})
    try:
        with urllib.request.urlopen(req) as response:
            return json.loads(response.read().decode()), response.status
    except urllib.error.HTTPError as e:
        return json.loads(e.read().decode()), e.code
    except Exception as e:
        return None, 500

def main():
    print(">>> Building colony-server...")
    subprocess.run(["cargo", "build"], check=True)

    server_bin = "target/debug/colony-server"
    if os.name == 'nt':
        server_bin += ".exe"

    nodes = []
    base_port = 8081
    secret_key = "mesh_secret_test_key"

    # Cleanup temp dirs
    for i in range(1, 4):
        if os.path.exists(f"temp_node{i}"):
            shutil.rmtree(f"temp_node{i}")
        os.makedirs(f"temp_node{i}")

    print(">>> Starting 3 Mesh Nodes...")
    try:
        # Node 1
        env1 = os.environ.copy()
        env1["COLONY_BIND"] = f"127.0.0.1:{base_port}"
        env1["COLONY_DATA_DIR"] = "temp_node1"
        env1["COLONY_NODE_ID"] = "node1"
        env1["COLONY_GOSSIP_HMAC_KEY"] = secret_key
        # High dedup window to test dedup effectively
        env1["COLONY_GOSSIP_DEDUP_WINDOW_S"] = "60"
        p1 = subprocess.Popen([server_bin], env=env1, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        nodes.append(p1)

        # Node 2 (bootstraps to Node 1)
        env2 = os.environ.copy()
        env2["COLONY_BIND"] = f"127.0.0.1:{base_port+1}"
        env2["COLONY_DATA_DIR"] = "temp_node2"
        env2["COLONY_NODE_ID"] = "node2"
        env2["COLONY_BOOTSTRAP_PEERS"] = f"node1|127.0.0.1:{base_port}|relay"
        env2["COLONY_GOSSIP_HMAC_KEY"] = secret_key
        p2 = subprocess.Popen([server_bin], env=env2, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        nodes.append(p2)

        # Node 3 (bootstraps to Node 2)
        env3 = os.environ.copy()
        env3["COLONY_BIND"] = f"127.0.0.1:{base_port+2}"
        env3["COLONY_DATA_DIR"] = "temp_node3"
        env3["COLONY_NODE_ID"] = "node3"
        env3["COLONY_BOOTSTRAP_PEERS"] = f"node2|127.0.0.1:{base_port+1}|relay"
        env3["COLONY_GOSSIP_HMAC_KEY"] = secret_key
        p3 = subprocess.Popen([server_bin], env=env3, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        nodes.append(p3)

        print("WAITING for servers to initialize...")
        time.sleep(3)

        # Let's ensure they are up
        resp = http_get(f"http://127.0.0.1:{base_port}/mesh/metrics")
        assert resp is not None, "Node 1 is not up"

        print("\n=== Test 1: TTL & Relay ===")
        # Send a gossip to Node 1. It should relay to Node 2 (and then Node 3)
        g_id = str(uuid.uuid4())
        gossip = create_gossip(g_id, ttl=3, peer_id="tester", address="127.0.0.1:9999", key=secret_key)
        
        resp_json, status = http_post(f"http://127.0.0.1:{base_port}/mesh/gossip", gossip)
        assert resp_json["ack"]["status"] == "ok"
        
        # Wait for propagation
        time.sleep(2)

        # Check Node 2 received it
        m2 = http_get(f"http://127.0.0.1:{base_port+1}/mesh/metrics")["counters"]
        print(f"Node 2 Inbound Total: {m2['inbound_total']}")
        assert m2["inbound_total"] >= 1, "Node 2 did not receive the relayed gossip"

        # Check Node 3 received it
        m3 = http_get(f"http://127.0.0.1:{base_port+2}/mesh/metrics")["counters"]
        print(f"Node 3 Inbound Total: {m3['inbound_total']}")
        assert m3["inbound_total"] >= 1, "Node 3 did not receive the relayed gossip"
        
        print("[OK] TTL & Relay PASSED")

        print("\n=== Test 2: Deduplication ===")
        # Resend the SAME gossip to Node 1
        resp_json, status = http_post(f"http://127.0.0.1:{base_port}/mesh/gossip", gossip)
        ack = resp_json["ack"]
        assert ack["status"] == "duplicate", f"Expected duplicate, got {ack['status']}"
        assert ack["duplicate"] == True
        
        m1 = http_get(f"http://127.0.0.1:{base_port}/mesh/metrics")["counters"]
        assert m1["inbound_duplicate"] >= 1, "Node 1 duplicate counter did not increment"
        print("[OK] Deduplication PASSED")

        print("\n=== Test 3: Auth Failure ===")
        bad_gossip = create_gossip(str(uuid.uuid4()), ttl=3, peer_id="tester", address="127.0.0.1:9999", key="wrong_key")
        resp_json, status = http_post(f"http://127.0.0.1:{base_port}/mesh/gossip", bad_gossip)
        ack = resp_json["ack"]
        assert ack["status"] == "auth_failed", f"Expected auth_failed, got {ack['status']}"
        
        m1 = http_get(f"http://127.0.0.1:{base_port}/mesh/metrics")["counters"]
        assert m1["inbound_auth_failed"] >= 1, "Node 1 auth_failed counter did not increment"
        print("[OK] Auth Failure PASSED")

        print("\n=== Test 4: TTL Drop ===")
        ttl_drop_gossip = create_gossip(str(uuid.uuid4()), ttl=0, peer_id="tester", address="127.0.0.1:9999", key=secret_key)
        resp_json, status = http_post(f"http://127.0.0.1:{base_port}/mesh/gossip", ttl_drop_gossip)
        ack = resp_json["ack"]
        assert ack["status"] == "dropped_ttl", f"Expected dropped_ttl, got {ack['status']}"
        assert ack["ttl_hops_remaining"] == 0
        
        m1 = http_get(f"http://127.0.0.1:{base_port}/mesh/metrics")["counters"]
        assert m1["inbound_ttl_dropped"] >= 1, "Node 1 ttl_dropped counter did not increment"
        print("[OK] TTL Drop PASSED")

        print("\n*** ALL TESTS PASSED! ***")

    finally:
        print("Cleaning up nodes...")
        for p in nodes:
            p.kill()

if __name__ == "__main__":
    main()
