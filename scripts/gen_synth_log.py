#!/usr/bin/env python3
import random
import time
from datetime import datetime, timedelta

ENDPOINTS = ["/api/items", "/api/checkout", "/health", "/login", "/static/app.js", "/search"]
STATUS = [200, 200, 200, 200, 404, 500, 502]
UAS = ["Mozilla/5.0", "curl/8.0", "PostmanRuntime/7.0"]

def fmt(dt):
  # nginx-like: 10/Oct/2000:13:55:36 -0700
  return dt.strftime("%d/%b/%Y:%H:%M:%S -0300")

def main():
  import argparse
  ap = argparse.ArgumentParser()
  ap.add_argument("--out", default="synthetic.log")
  ap.add_argument("--lines", type=int, default=100000)
  ap.add_argument("--start", default="2025-01-01 00:00:00")
  args = ap.parse_args()

  dt = datetime.strptime(args.start, "%Y-%m-%d %H:%M:%S")

  with open(args.out, "w", encoding="utf-8") as f:
    for i in range(args.lines):
      dt += timedelta(seconds=random.randint(0, 2))
      ep = random.choice(ENDPOINTS)
      st = random.choice(STATUS)
      ua = random.choice(UAS)
      method = "GET" if random.random() < 0.85 else "POST"
      q = "" if random.random() < 0.7 else f"?q={random.randint(1,999)}"
      latency = max(0.001, random.gauss(0.12, 0.08))
      if st >= 500:
        latency *= random.uniform(2.0, 6.0)
      line = f'127.0.0.1 - - [{fmt(dt)}] "{method} {ep}{q} HTTP/1.1" {st} {random.randint(10,5000)} "-" "{ua}" {latency:.3f}\n'
      f.write(line)

if __name__ == "__main__":
  main()
