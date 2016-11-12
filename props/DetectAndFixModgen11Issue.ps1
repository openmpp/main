# Copyright (c) 2016 OpenM++
# This code is licensed under MIT license (see LICENSE.txt for details)

$msBuildDirectory = "$env:LOCALAPPDATA\Microsoft\MSBuild\v4.0";

if (Test-Path $msBuildDirectory -PathType Container)
{
    [string]$modgen11Path = Get-ItemProperty -Path HKLM:\SOFTWARE\Wow6432Node\StatCan\Modgen\11 -Name Path -ErrorAction SilentlyContinue

    $BadIncludePath = '<IncludePath>$(VCInstallDir)include;$(VCInstallDir)atlmfc\include;$(WindowsSdkDir)include;$(FrameworkSDKDir)\include;';
    $GoodIncludePath = '<IncludePath>$(IncludePath);';

    $BadLibraryPath = '<LibraryPath>$(VCInstallDir)lib;$(VCInstallDir)atlmfc\lib;$(WindowsSdkDir)lib;$(FrameworkSDKDir)\lib;';
    $GoodLibraryPath = '<LibraryPath>$(LibraryPath);';

    if (-not $modgen11Path)
    {
        $progFiles = $env:ProgramFiles;

        if (${env:ProgramFiles(x86)})
        {
            $progFiles = ${env:ProgramFiles(x86)};
        }

        $modgen11Path = "$progFiles\StatCan\Modgen 11";
    }

    $BadIncludePath += $modgen11Path + "</IncludePath>";
    $GoodIncludePath += $modgen11Path + "</IncludePath>";
    $BadLibraryPath += $modgen11Path + "</LibraryPath>";
    $GoodLibraryPath += $modgen11Path + "</LibraryPath>";

    $issueFound = $false;
    $fixingIssues = $false;
    $messageShown = $false;

    Get-ChildItem $msBuildDirectory -Filter *.user.props | ForEach-Object -Process {
        $fileContents = Get-Content $_.FullName
        
        for($lineIndex = 0; $lineIndex -lt $fileContents.Length; $lineIndex++)
        {
            $line = $fileContents[$lineIndex];

            if ($line.Contains($BadIncludePath))
            {
                if (-not $messageShown)
                {
                    echo "Fixing Modgen 11 configuration issue...";
                    #$choice = [System.Windows.Forms.MessageBox]::Show("A Modgen 11-caused issue was found in your Visual Studio configuration. Would you like to repare it?","", [System.Windows.Forms.MessageBoxButtons]::YesNo);

                    #if ($choice -eq [System.Windows.Forms.DialogResult]::Yes)
                    #{
                        $fixingIssues = $true;
                    #}

                    $messageShown = $true;
                }

                $issueFound = $true;

                $fileContents[$lineIndex] = $line.Replace($BadIncludePath, $GoodIncludePath)
            }
            elseif ($fixingIssues -and $line.Contains($BadLibraryPath))
            {
                $fileContents[$lineIndex] = $line.Replace($BadLibraryPath, $GoodLibraryPath)
            }
        }

        if ($fixingIssues)
        {
            $fileContents | Set-Content $_.FullName 
        }
    }

	if ($issueFound)
	{
		exit 1;
	}
}