import urllib.request
import json
import gzip

repo = "TOAN1232/icfox"
run_id = 28421255647

# Try the raw log download URL format
url = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}/logs"
req = urllib.request.Request(url, headers={
    "Accept": "application/vnd.github+json",
    "User-Agent": "Python"
})
try:
    with urllib.request.urlopen(req) as response:
        raw = response.read()
        try:
            text = gzip.decompress(raw).decode("utf-8", errors="replace")
        except:
            text = raw.decode("utf-8", errors="replace")
        print(text[:5000])
except Exception as e:
    print(f"Logs download error: {e}")
    # Try with redirect following
    import http.client
    print(f"Status: {e.code if hasattr(e, 'code') else 'N/A'}")
    print(f"Headers: {e.headers if hasattr(e, 'headers') else 'N/A'}")