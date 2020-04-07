# Stop oms web-service if it MODEL_NAME.oms_url.tickle contains oms URL
#
$oms_url_tickle = "$env:OMS_URL_TICKLE"
$stop_oms_log = "$env:STOP_OMS_LOG"

($oms_url = Get-Content -TotalCount 1 -Path $oms_url_tickle) | Out-File -FilePath $stop_oms_log

@"
Stop oms web-service:

If error is:
   'Unable to connect to the remote server'
   then oms web-service not running on $oms_url
If error is:
   'The underlying connection was closed: An unexpected error occurred on a receive'
   then oms web-service shutdown succeeded

"@ | Out-File -Append -FilePath $stop_oms_log

Write-Host "Stop oms web-service: $oms_url"

$oms_shutdown_url = "$oms_url/api/admin/shutdown"
try {
  Invoke-WebRequest -Method PUT -Uri $oms_shutdown_url
} catch {
  $_  | Out-File -Append -FilePath $stop_oms_log
}
