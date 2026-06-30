import urllib.request
import json

repo = "TOAN1232/icfox"
run_id = 28421383271

# Get the check suite URL from the run
url = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}"
req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
with urllib.request.urlopen(req) as resp:
    run_data = json.loads(resp.read())
    print("Run conclusion:", run_data.get("conclusion"))
    
# Get jobs with steps info
url2 = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}/jobs"
req2 = urllib.request.Request(url2, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
with urllib.request.urlopen(req2) as resp2:
    jobs = json.loads(resp2.read())
    for job in jobs.get("jobs", []):
        for step in job.get("steps", []):
            if step.get("conclusion") == "failure":
                print(f"\nFailed step: {step['name']}")
                print(f"Step number: {step.get('number')}")
                print(f"Started at: {step.get('started_at')}")
                print(f"Completed at: {step.get('completed_at')}")