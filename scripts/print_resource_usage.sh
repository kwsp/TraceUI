# Print resource usage of TraceUI
ps aux | awk '/TraceUI$/ {
  cmd=$11; sub(/.*\//, "", cmd);  # strip path, keep binary name only
  printf "PID: %s | CPU: %s%% | MEM: %s%% | VSZ: %.1f MB | RSS: %.1f MB | Process: %s \n", $2, $3, $4, $5/1024, $6/1024, cmd
}'
