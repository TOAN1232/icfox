import urllib.request
import json
import time
import gzip

repo = "TOAN1232/icfox"
url = f"https://api.github.com/repos/{repo}/actions/runs"
req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})

checked_ids = set()

while True:
    try:
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read())
            runs = data.get("workflow_runs", [])
            if runs:
                r = runs[0]
                run_id = r['id']
                status = r['status']
                conclusion = r.get('conclusion', 'N/A')
                event = r['event']
                created = r['created_at']
                
                if run_id not in checked_ids:
                    print(f"\nRun ID: {run_id}, Status: {status}, Conclusion: {conclusion}, Event: {event}, Created: {created}")
                    checked_ids.add(run_id)
                    
                    if status == 'completed':
                        if conclusion == 'success':
                            print("=== BUILD SUCCESS! ===")
                            # Get artifacts
                            art_url = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}/artifacts"
                            art_req = urllib.request.Request(art_url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
                            with urllib.request.urlopen(art_req) as art_resp:
                                art_data = json.loads(art_resp.read())
                                artifacts = art_data.get("artifacts", [])
                                for a in artifacts:
                                    print(f"Artifact: {a['name']}, Size: {a['size_in_bytes']} bytes, ID: {a['id']}")
                            break
                        elif conclusion == 'failure':
                            print("=== BUILD FAILED ===")
                            # Try to get logs
                            log_url = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}/logs"
                            log_req = urllib.request.Request(log_url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
                            try:
                                with urllib.request.urlopen(log_req) as log_resp:
                                    raw = log_resp.read()
                                    try:
                                        text = gzip.decompress(raw).decode("utf-8", errors="replace")
                                    except:
                                        text = raw.decode("utf-8", errors="replace")
                                    # Find error lines
                                    for line in text.split('\n'):
                                        if 'error' in line.lower() or 'fatal' in line.lower() or ':' in line[:10]:
                                            print(f"  LOG: {line[:200]}")
                            except Exception as e:
                                print(f"  Could not fetch logs: {e}")
                            
                            # Get jobs info
                            job_url = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}/jobs"
                            job_req = urllib.request.Request(job_url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
                            with urllib.request.urlopen(job_req) as job_resp:
                                jobs = json.loads(job_resp.read()).get("jobs", [])
                                for job in jobs:
                                    print(f"  Job: {job['name']}, Conclusion: {job.get('conclusion','N/A')}")
                                    for step in job.get("steps", []):
                                        sc = step.get("conclusion", "")
                                        if sc == "failure":
                                            print(f"    Failed: {step['name']}")
                        break
                    else:
                        print(f"  Still running... (status: {status})")
            else:
                print("No runs found")
    except Exception as e:
        print(f"Monitor error: {e}")
    
    time.sleep(15)