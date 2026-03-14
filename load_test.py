#!/usr/bin/env python3
"""
Load testing script for concurrent backend server.
Tests with different concurrent client counts and measures performance.
"""

import socket
import threading
import time
import statistics
from typing import List, Tuple

HOST = "localhost"
PORT = 8080
TEST_DURATION = 10  # seconds per test

class LoadTestClient:
    """Single client that sends commands and measures latency."""
    
    def __init__(self, client_id: int):
        self.client_id = client_id
        self.latencies = []  # List of latencies in ms
        self.successful = 0
        self.failed = 0
        self.running = False
    
    def run(self, duration: int):
        """Send commands for specified duration."""
        self.running = True
        start_time = time.time()
        command_index = 0
        
        while time.time() - start_time < duration:
            try:
                # Rotate through different commands
                commands = [
                    f"SET key{self.client_id}_{command_index} value{command_index}\n",
                    f"GET key{self.client_id}_{command_index}\n",
                    f"DELETE key{self.client_id}_{command_index}\n",
                    "PING\n",
                ]
                command = commands[command_index % len(commands)]
                
                # Measure latency for this request
                req_start = time.time()
                
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect((HOST, PORT))
                sock.sendall(command.encode())
                response = sock.recv(1024)
                sock.close()
                
                req_latency_ms = (time.time() - req_start) * 1000
                self.latencies.append(req_latency_ms)
                
                if response:
                    self.successful += 1
                else:
                    self.failed += 1
                
                command_index += 1
                
            except Exception as e:
                self.failed += 1
        
        self.running = False
    
    def get_stats(self) -> dict:
        """Return latency statistics."""
        if not self.latencies:
            return {}
        
        return {
            "latencies": self.latencies,
            "successful": self.successful,
            "failed": self.failed,
            "min": min(self.latencies),
            "max": max(self.latencies),
            "avg": statistics.mean(self.latencies),
            "p99": self._percentile(self.latencies, 99),
        }
    
    def _percentile(self, data: List[float], percentile: int) -> float:
        """Calculate percentile of latency data."""
        if not data:
            return 0
        sorted_data = sorted(data)
        index = (percentile / 100.0) * len(sorted_data)
        if index >= len(sorted_data):
            return sorted_data[-1]
        return sorted_data[int(index)]


def run_load_test(num_clients: int, duration: int) -> dict:
    """Run load test with specified number of concurrent clients."""
    print(f"\n{'='*70}")
    print(f"Load Test: {num_clients} concurrent clients, {duration}s duration")
    print(f"{'='*70}")
    
    clients = [LoadTestClient(i) for i in range(num_clients)]
    threads = []
    
    # Start all client threads
    test_start = time.time()
    for client in clients:
        thread = threading.Thread(target=client.run, args=(duration,))
        thread.start()
        threads.append(thread)
    
    # Wait for all clients to finish
    for thread in threads:
        thread.join()
    
    total_time = time.time() - test_start
    
    # Aggregate statistics
    all_latencies = []
    total_successful = 0
    total_failed = 0
    
    for client in clients:
        stats = client.get_stats()
        all_latencies.extend(stats.get("latencies", []))
        total_successful += stats.get("successful", 0)
        total_failed += stats.get("failed", 0)
    
    total_requests = total_successful + total_failed
    rps = total_requests / total_time if total_time > 0 else 0
    
    # Calculate aggregated latency stats
    if all_latencies:
        min_lat = min(all_latencies)
        max_lat = max(all_latencies)
        avg_lat = statistics.mean(all_latencies)
        p99_lat = sorted(all_latencies)[int(0.99 * len(all_latencies))]
    else:
        min_lat = max_lat = avg_lat = p99_lat = 0
    
    # Print results
    print(f"\nResults:")
    print(f"  Total Requests:    {total_requests}")
    print(f"  Successful:        {total_successful}")
    print(f"  Failed:            {total_failed}")
    print(f"  Throughput (RPS):  {rps:.2f}")
    print(f"  Latency (ms):")
    print(f"    Min:             {min_lat:.2f}")
    print(f"    Avg:             {avg_lat:.2f}")
    print(f"    P99:             {p99_lat:.2f}")
    print(f"    Max:             {max_lat:.2f}")
    
    return {
        "num_clients": num_clients,
        "total_requests": total_requests,
        "successful": total_successful,
        "failed": total_failed,
        "rps": rps,
        "min_latency": min_lat,
        "avg_latency": avg_lat,
        "p99_latency": p99_lat,
        "max_latency": max_lat,
    }


def main():
    """Run load tests with increasing client counts."""
    print("\n" + "="*70)
    print("CONCURRENT BACKEND SERVER - LOAD TESTING")
    print("="*70)
    print(f"Server: {HOST}:{PORT}")
    print(f"Test Duration: {TEST_DURATION}s per test")
    
    test_configs = [
        10,    # Light load
        50,    # Medium load
        100,   # Heavy load
        200,   # Very heavy load
    ]
    
    results = []
    
    for num_clients in test_configs:
        try:
            result = run_load_test(num_clients, TEST_DURATION)
            results.append(result)
        except ConnectionRefusedError:
            print(f"\nError: Could not connect to server at {HOST}:{PORT}")
            print("Make sure the server is running: ./server")
            return
        except Exception as e:
            print(f"\nError during load test: {e}")
            return
    
    # Print summary table
    print(f"\n\n{'='*70}")
    print("SUMMARY TABLE")
    print(f"{'='*70}")
    print(f"{'Clients':<12} {'RPS':<12} {'Avg Lat(ms)':<14} {'P99 Lat(ms)':<14} {'Success Rate':<12}")
    print("-" * 70)
    
    for result in results:
        clients = result["num_clients"]
        rps = result["rps"]
        avg_lat = result["avg_latency"]
        p99_lat = result["p99_latency"]
        success_rate = 100.0 * result["successful"] / result["total_requests"] if result["total_requests"] > 0 else 0
        
        print(f"{clients:<12} {rps:<12.2f} {avg_lat:<14.2f} {p99_lat:<14.2f} {success_rate:<12.1f}%")
    
    print("=" * 70)


if __name__ == "__main__":
    main()
