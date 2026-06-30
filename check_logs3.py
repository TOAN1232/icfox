import urllib.request
import json
import gzip

repo = "TOAN1232/icfox"
run_id = 28421255647
url = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}/jobs"
req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
try:
    with urllib.request.urlopen(req) as response:
        data = json.loads(response.read())
        jobs = data.get("jobs", [])
        for job in jobs:
            print("Job keys:", list(job.keys()))
            print(f"Name: {job['name']}, Status: {job['status']}, Conclusion: {job.get('conclusion','N/A')}")
            # Try steps to get details
            for step in job.get("steps", []):
                print(f"  Step: {step['name']}, Status: {step['status']}, Conclusion: {step.get('conclusion','N/A')}")
                if step.get("conclusion") == "failure":
                    if "logs_url" in step:
                        print(f"  Logs URL: {step['logs_url']}")
            # Try to get the check_run URL
            if "check_run_url" in job:
                print(f"Check run URL: {job['check_run_url']}")
except Exception as e:
    print(f"Error: {e}")