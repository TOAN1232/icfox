import urllib.request
import json
import time

repo = "TOAN1232/icfox"
url = f"https://api.github.com/repos/{repo}/actions/runs"
req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json"})

while True:
    try:
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read())
            runs = data.get("workflow_runs", [])
            if runs:
                r = runs[0]
                print(f"ID: {r['id']}, Status: {r['status']}, Conclusion: {r.get('conclusion','N/A')}, Branch: {r['head_branch']}, Trigger: {r['event']}, Created: {r['created_at']}")
                if r['status'] == 'completed':
                    if r.get('conclusion') == 'success':
                        print("BUILD SUCCESS!")
                        # Get artifact info
                        art_url = f"https://api.github.com/repos/{repo}/actions/runs/{r['id']}/artifacts"
                        art_req = urllib.request.Request(art_url, headers={"Accept": "application/vnd.github+json"})
                        with urllib.request.urlopen(art_req) as art_resp:
                            art_data = json.loads(art_resp.read())
                            artifacts = art_data.get("artifacts", [])
                            for a in artifacts:
                                print(f"  Artifact: {a['name']}, Size: {a['size_in_bytes']} bytes, Download URL: {a['archive_download_url']}")
                    else:
                        # Get job logs
                        job_url = f"https://api.github.com/repos/{repo}/actions/runs/{r['id']}/jobs"
                        job_req = urllib.request.Request(job_url, headers={"Accept": "application/vnd.github+json"})
                        with urllib.request.urlopen(job_req) as job_resp:
                            jobs = json.loads(job_resp.read()).get("jobs", [])
                            for job in jobs:
                                print(f"  Job: {job['name']}, Conclusion: {job.get('conclusion','N/A')}")
                                for step in job.get("steps", []):
                                    if step.get("conclusion") == "failure":
                                        print(f"    Failed step: {step['name']}")
                    break
            else:
                print("No runs found")
    except Exception as e:
        print(f"Error: {e}")
    time.sleep(10)