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
            print(f"Job ID: {job['id']}, Name: {job['name']}")
            # Get logs via the specific log URL
            log_url = job["logs_url"]
            log_req = urllib.request.Request(log_url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
            try:
                with urllib.request.urlopen(log_req) as log_resp:
                    raw = log_resp.read()
                    # Try decompress
                    try:
                        text = gzip.decompress(raw).decode("utf-8", errors="replace")
                    except:
                        text = raw.decode("utf-8", errors="replace")
                    print(text)
            except Exception as e:
                print(f"Log fetch error: {e}")
                # Try alternative URL
                log_url2 = f"https://api.github.com/repos/{repo}/actions/jobs/{job['id']}/logs"
                log_req2 = urllib.request.Request(log_url2, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
                try:
                    with urllib.request.urlopen(log_req2) as log_resp2:
                        raw2 = log_resp2.read()
                        try:
                            text2 = gzip.decompress(raw2).decode("utf-8", errors="replace")
                        except:
                            text2 = raw2.decode("utf-8", errors="replace")
                        print(text2)
                except Exception as e2:
                    print(f"Alt log fetch error: {e2}")
except Exception as e:
    print(f"Error: {e}")