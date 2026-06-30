import urllib.request
import json

repo = "TOAN1232/icfox"
run_id = 28421255647

# Try annotations API
url = f"https://api.github.com/repos/{repo}/check-runs?check_name=build"
req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
try:
    with urllib.request.urlopen(req) as response:
        data = json.loads(response.read())
        print(json.dumps(data, indent=2)[:5000])
except Exception as e:
    print(f"Error: {e}")

# Try getting the run directly with more detail
url2 = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}"
req2 = urllib.request.Request(url2, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
try:
    with urllib.request.urlopen(req2) as response:
        data2 = json.loads(response.read())
        print("\nRun keys:", list(data2.keys()))
        # Check for annotations_url
        if "check_suite_url" in data2:
            print(f"Check suite URL: {data2['check_suite_url']}")
        # Try getting the workflow job annotations
        if "workflow_id" in data2:
            print(f"Workflow ID: {data2['workflow_id']}")
except Exception as e:
    print(f"Error: {e}")