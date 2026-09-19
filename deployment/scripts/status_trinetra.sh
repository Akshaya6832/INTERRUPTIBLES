#!/bin/sh
# Engineering student note: status_trinetra.sh handles deployment, startup, configuration, and target integration.
# Comments are kept simple so the build/deployment steps are easy to trace.
echo "=== TRINETRA processes ==="
pidin | grep TRINETRA || true
echo
echo "=== TRINETRA service logs ==="
for f in /var/trinetra/logs/TRINETRA_*.log; do
    if [ -f "$f" ]; then
        echo "--- $f ---"
        tail -n 5 "$f"
    fi
done
