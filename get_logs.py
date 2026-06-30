import urllib.request
import json
import gzip

repo = "TOAN1232/icfox"
run_id = 28421383271

# First get jobs to find the job ID
url = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}/jobs"
req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
try:
    with urllib.request.urlopen(req) as resp:
        jobs = json.loads(resp.read())
        for job in jobs.get("jobs", []):
            job_id = job["id"]
            print(f"Job ID: {job_id}, Name: {job['name']}")
            
            # Try to get the step annotations which contain errors
            annotations_url = f"https://api.github.com/repos/{repo}/check-runs/{job_id}/annotations"
            ann_req = urllib.request.Request(annotations_url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
            try:
                with urllib.request.urlopen(ann_req) as ann_resp:
                    annotations = json.loads(ann_resp.read())
                    if annotations:
                        for ann in annotations:
                            print(f"\n--- Annotation ---")
                            print(f"Path: {ann.get('path')}")
                            print(f"Start line: {ann.get('start_line')}")
                            print(f"Message: {ann.get('message')}")
                    else:
                        print("No annotations found")
            except Exception as e:
                print(f"Annotations error: {e}")
            
            # Try to get log by redirecting to the actual log URL
            log_url = f"https://api.github.com/repos/{repo}/actions/jobs/{job_id}/logs"
            log_req = urllib.request.Request(log_url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
            try:
                with urllib.request.urlopen(log_req) as log_resp:
                    raw = log_resp.read()
                    try:
                        text = gzip.decompress(raw).decode("utf-8", errors="replace")
                    except:
                        text = raw.decode("utf-8", errors="replace")
                    # Show the last 100 lines (where error would be)
                    lines = text.split('\n')
                    print(f"\n--- Full log ({len(lines)} lines) ---")
                    for line in lines[-100:]:
                        print(line)
            except urllib.error.HTTPError as e:
                print(f"Log HTTP error: {e.code} - {e.reason}")
                # Try to follow redirect manually
                if e.code == 302 or e.code == 301:
                    redirect_url = e.headers.get("Location")
                    if redirect_url:
                        print(f"Redirect to: {redirect_url}")
                        try:
                            with urllib.request.urlopen(redirect_url) as redir_resp:
                                raw = redir_resp.read()
                                try:
                                    text = gzip.decompress(raw).decode("utf-8", errors="replace")
                                except:
                                    text = raw.decode("utf-8", errors="replace")
                                lines = text.split('\n')
                                print(f"\n--- Redirected log ({len(lines)} lines) ---")
                                for line in lines[-100:]:
                                    print(line)
                        except Exception as e2:
                            print(f"Redirect fetch error: {e2}")
            except Exception as e:
                print(f"Log error: {e}")
except Exception as e:
    print(f"Error: {e}")