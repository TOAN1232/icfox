import urllib.request
import json
import gzip

# First get the failed run details
run_id = 28420961251
url = f"https://api.github.com/repos/TOAN1232/icfox/actions/runs/{run_id}/jobs"
req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json"})
try:
    with urllib.request.urlopen(req) as response:
        data = json.loads(response.read())
        jobs = data.get("jobs", [])
        for job in jobs:
            print(f"Job: {job['name']}, Status: {job['status']}, Conclusion: {job.get('conclusion','N/A')}")
            for step in job.get("steps", []):
                print(f"  Step: {step['name']}, Status: {step['status']}, Conclusion: {step.get('conclusion','N/A')}")
                if step.get("conclusion") == "failure" and step.get("number", 0) > 0:
                    # Get logs for failed step
                    log_url = f"https://api.github.com/repos/TOAN1232/icfox/actions/jobs/{job['id']}/logs"
                    log_req = urllib.request.Request(log_url, headers={"Accept": "application/vnd.github+json"})
                    try:
                        with urllib.request.urlopen(log_req) as log_response:
                            log_data = log_response.read()
                            # Try gzip decompress
                            try:
                                log_text = gzip.decompress(log_data).decode("utf-8", errors="replace")
                            except:
                                log_text = log_data.decode("utf-8", errors="replace")
                            print(f"  LOGS (first 3000 chars):\n{log_text[:3000]}")
                    except Exception as e:
                        print(f"  Could not fetch logs: {e}")
except Exception as e:
    print(f"Error: {e}")