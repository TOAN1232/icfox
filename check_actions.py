import urllib.request
import json

url = "https://api.github.com/repos/TOAN1232/icfox/actions/runs"
req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json"})
try:
    with urllib.request.urlopen(req) as response:
        data = json.loads(response.read())
        runs = data.get("workflow_runs", [])
        if not runs:
            print("No workflow runs found")
        for r in runs[:5]:
            print(f"ID: {r['id']}, Status: {r['status']}, Conclusion: {r.get('conclusion','N/A')}, Branch: {r['head_branch']}, Trigger: {r['event']}")
except Exception as e:
    print(f"Error: {e}")