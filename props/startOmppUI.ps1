# Start openM++ UI: 
#  start oms web-service listen on localhost:free_tcp_port
#  open default browser with http://localhost:free_tcp_port
#
$publish_dir = "$env:PUBLISH_DIR"
$oms_url_tickle = "$env:OMS_URL_TICKLE"
$start_ompp_ui_log = "$env:START_OMPP_UI_LOG"

if (!"$env:OM_ROOT") {
  $env:OM_ROOT = Resolve-Path -Path "$publish_dir/../../../.."
  Write-Host "OM_ROOT: $env:OM_ROOT"
}

"Start-Process bin/oms.exe -PassThru -WindowStyle Minimized -ArgumentList ""-l localhost:0 -oms.ModelDir ""$publish_dir"" -oms.ModelLogDir ""$publish_dir"" -oms.LogRequest""" | Out-File -FilePath $start_ompp_ui_log

$oms_proc = Start-Process bin/oms.exe -PassThru -WindowStyle Minimized -ArgumentList "-l localhost:0 -oms.ModelDir ""$publish_dir"" -oms.ModelLogDir ""$publish_dir"" -oms.LogRequest"

if (!$oms_proc -or $oms_proc.ExitCode -ne $null) {
  Write-Host "Failed to start oms web-service"
  exit 1
}

# get oms port: it would fail if process start failed
($oms_tcp = Get-NetTCPConnection -OwningProcess $oms_proc.Id) 2>&1 >> ($start_ompp_ui_log)

if (!$oms_tcp -or $oms_tcp.LocalPort -eq $null) {
 Write-Host "Failed to start oms web-service"
 exit 2
}

($oms_url = "http://localhost:$($oms_tcp.LocalPort)") | Out-File -FilePath $oms_url_tickle

"Starting openM++ UI: $oms_url" | Out-File -Append -FilePath $start_ompp_ui_log
Write-Host "Starting openM++ UI: $oms_url"

start $oms_url
