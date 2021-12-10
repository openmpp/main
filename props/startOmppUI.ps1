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
$env:OM_CFG_INI_ALLOW = "true"
$env:OM_CFG_INI_ANY_KEY = "true"

"Start-Process bin/oms.exe -PassThru -WindowStyle Minimized -ArgumentList ""-l localhost:0 -oms.ModelDir ""$publish_dir"" -oms.ModelLogDir ""$publish_dir"" -oms.UrlSaveTo ""$oms_url_tickle"" -oms.HomeDir ""$publish_dir"" -oms.LogRequest""" | Out-File -FilePath $start_ompp_ui_log

$oms_proc = Start-Process bin/oms.exe -PassThru -WindowStyle Minimized -ArgumentList "-l localhost:0 -oms.ModelDir ""$publish_dir"" -oms.ModelLogDir ""$publish_dir"" -oms.UrlSaveTo ""$oms_url_tickle"" -oms.HomeDir ""$publish_dir"" -oms.LogRequest"

Start-Sleep -s 1

if (!$oms_proc -or $oms_proc.ExitCode -ne $null) {
  Write-Host "Failed to start oms web-service"
  exit 1
}

($oms_url = Get-Content -TotalCount 1 -Path $oms_url_tickle) | Out-File -Append -FilePath $start_ompp_ui_log

"Starting openM++ UI: $oms_url" | Out-File -Append -FilePath $start_ompp_ui_log
Write-Host "Starting openM++ UI: $oms_url"

start $oms_url
