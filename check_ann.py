import urllib.request
import json

repo = "TOAN1232/icfox"
run_id = 28421744038

# Get jobs
url = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}/jobs"
req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
with urllib.request.urlopen(req) as resp:
    jobs = json.loads(resp.read())
    for job in jobs.get("jobs", []):
        print(f"Job: {job['name']}, ID: {job['id']}")
        # Get annotations
        ann_url = f"https://api.github.com/repos/{repo}/check-runs/{job['id']}/annotations"
        ann_req = urllib.request.Request(ann_url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Python"})
        try:
            with urllib.request.urlopen(ann_req) as ann_resp:
                annotations = json.loads(ann_resp.read())
                for ann in annotations:
                    print(f"\n  Annotation:")
                    print(f"    Path: {ann.get('path')}")
                    print(f"    Line: {ann.get('start_line')}")
                    print(f"    Level: {ann.get('annotation_level')}")
                    print(f"    Message: {ann.get('message')}")
        except Exception as e:
            print(f"  Annotations error: {e}")