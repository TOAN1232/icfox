import urllib.request
import json
import time

print("Waiting 120 seconds for rate limit reset...")
time.sleep(120)

repo = "TOAN1232/icfox"
url = f"https://api.github.com/repos/{repo}/actions/runs"
req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
try:
    data = json.loads(urllib.request.urlopen(req).read())
    runs = data.get("workflow_runs", [])
    for r in runs[:3]:
        print(f"ID: {r['id']}, Status: {r['status']}, Conclusion: {r.get('conclusion','N/A')}")
except Exception as e:
    print(f"Error: {e}")