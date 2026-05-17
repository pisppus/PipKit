#Requires -Version 5.1
param(
    [switch]$NoRun,
    [switch]$Debug,
    [switch]$Clean,
    [int]$Jobs = [Environment]::ProcessorCount
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$root            = Split-Path -Parent $PSScriptRoot
$buildDir        = Join-Path $root ".sim"
$objDir          = Join-Path $buildDir "obj"
$desktopInclude  = Join-Path $root "lib\PipKit\PipCore\Host\Desktop\include"
$includeDir      = Join-Path $root "include"
$libDir          = Join-Path $root "lib\PipKit"
$vsBootstrapCmd  = Join-Path $buildDir "_vsdev-bootstrap.cmd"
$vsEnvFile       = Join-Path $buildDir "_vsdev-env.txt"
$linkArgFile     = Join-Path $buildDir "link-args.rsp"
$flagHashFile    = Join-Path $buildDir "flags.hash"

$exeName = if ($Debug) { "pipgui-sim-debug.exe" } else { "pipgui-sim.exe" }
$exe     = Join-Path $buildDir $exeName

function Write-Step  ([string]$msg) { Write-Host "  >> $msg" -ForegroundColor Cyan }
function Write-Ok    ([string]$msg) { Write-Host "  OK $msg" -ForegroundColor Green }
function Write-Warn  ([string]$msg) { Write-Host "  !! $msg" -ForegroundColor Yellow }
function Write-Fatal ([string]$msg) { Write-Host "`n  FAIL $msg`n" -ForegroundColor Red; exit 1 }

$buildStart = [Diagnostics.Stopwatch]::StartNew()
Write-Host ""
Write-Host "  simulator" -ForegroundColor DarkGray
Write-Host "  $(if ($Debug) { 'debug' } else { 'release' })  jobs=$Jobs  root=$root" -ForegroundColor DarkGray
Write-Host ""

if ($Clean) {
    Write-Step "Cleaning build directory..."
    if (Test-Path $objDir) { Remove-Item $objDir -Recurse -Force }
    if (Test-Path $exe)    { Remove-Item $exe    -Force }
    if (Test-Path $flagHashFile) { Remove-Item $flagHashFile -Force }
    Write-Ok "Clean done."
}

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
New-Item -ItemType Directory -Force -Path $objDir   | Out-Null

function Resolve-VcVars64 {
    $vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $installPaths = @(
            & $vswhere -latest -products * `
                -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
                -property installationPath 2>$null
            & $vswhere -products * `
                -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
                -property installationPath 2>$null
        ) | Where-Object { $_ } | Select-Object -Unique

        foreach ($ip in $installPaths) {
            $bat = Join-Path $ip "VC\Auxiliary\Build\vcvars64.bat"
            if (Test-Path $bat) { return $bat }
        }
    }

    foreach ($searchRoot in @(
        "C:\Program Files\Microsoft Visual Studio",
        "C:\Program Files (x86)\Microsoft Visual Studio"
    )) {
        if (!(Test-Path $searchRoot)) { continue }
        $bat = Get-ChildItem $searchRoot -Recurse -Filter "vcvars64.bat" -ErrorAction SilentlyContinue |
               Sort-Object FullName -Descending | Select-Object -First 1 -ExpandProperty FullName
        if ($bat) { return $bat }
    }
    return $null
}

Write-Step "Locating Visual Studio C++ tools..."
$vcvars = Resolve-VcVars64
if (!$vcvars -or !(Test-Path $vcvars)) {
    Write-Fatal "vcvars64.bat not found. Install 'Desktop development with C++' workload in Visual Studio."
}
Write-Ok "Found: $vcvars"

Write-Step "Initializing VS build environment..."

$cleanPath = (@(
    [Environment]::GetEnvironmentVariable("Path", "Machine"),
    [Environment]::GetEnvironmentVariable("Path", "User")
) | Where-Object { $_ } | ForEach-Object { $_.Trim(';') }) -join ';'

$vsVarsToClear = @(
    "INCLUDE","LIB","LIBPATH",
    "DevEnvDir","ExtensionSdkDir",
    "Framework40Version","FrameworkDir","FrameworkDir32",
    "FrameworkVersion","FrameworkVersion32","FrameworkVersion64",
    "UCRTVersion","UniversalCRTSdkDir",
    "VCIDEInstallDir","VCINSTALLDIR","VCToolsInstallDir",
    "VSINSTALLDIR","VisualStudioVersion",
    "WindowsLibPath","WindowsSdkBinPath","WindowsSdkDir",
    "WindowsSdkVerBinPath","WindowsSDKLibVersion","WindowsSDKVersion",
    "__VSCMD_PREINIT_PATH","__VSCMD_ARG_APP_PLAT","__VSCMD_ARG_HOST_ARCH",
    "__VSCMD_ARG_NO_LOGO","__VSCMD_ARG_TGT_ARCH","__VSCMD_VER"
)

$bootstrapLines = [System.Collections.Generic.List[string]]@(
    "@echo off",
    "setlocal EnableExtensions",
    ('set "PATH=' + ($cleanPath -replace '"', '') + '"')
)
foreach ($v in $vsVarsToClear) {
    $bootstrapLines.Add("set `"$v=`"")
}
$bootstrapLines.Add("call `"$vcvars`" >nul 2>&1")
$bootstrapLines.Add("if %errorlevel% neq 0 exit /b %errorlevel%")
$bootstrapLines.Add("set > `"$vsEnvFile`"")

[System.IO.File]::WriteAllLines($vsBootstrapCmd, $bootstrapLines, [System.Text.Encoding]::ASCII)

$null = & cmd.exe /d /s /c "`"$vsBootstrapCmd`""
if ($LASTEXITCODE -ne 0 -or !(Test-Path $vsEnvFile)) {
    Write-Fatal "Failed to initialize VS build environment (exit $LASTEXITCODE)."
}

foreach ($line in [System.IO.File]::ReadAllLines($vsEnvFile)) {
    if ($line -match '^([^=]+)=(.*)$') {
        [Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], 'Process')
    }
}
Remove-Item $vsEnvFile, $vsBootstrapCmd -Force -ErrorAction SilentlyContinue

$_clCmd   = Get-Command cl.exe   -ErrorAction SilentlyContinue
$_linkCmd = Get-Command link.exe -ErrorAction SilentlyContinue
$cl   = if ($_clCmd)   { $_clCmd.Source }   else { $null }
$link = if ($_linkCmd) { $_linkCmd.Source } else { $null }
if (!$cl -or !$link) {
    Write-Fatal "cl.exe / link.exe not found after VS env init."
}
Write-Ok "Compiler : $cl"
Write-Ok "Linker   : $link"

$running = Get-Process -ErrorAction SilentlyContinue |
           Where-Object { $_.Path -and [System.StringComparer]::OrdinalIgnoreCase.Equals($_.Path, $exe) }
if ($running) {
    Write-Warn "Terminating running simulator (PID $($running.Id))..."
    $running | Stop-Process -Force
    try { $running | Wait-Process -Timeout 10 -ErrorAction Stop } catch {}
    Write-Ok "Simulator stopped."
}

$commonArgs = [string[]]@(
    "/nologo",
    "/std:c++17",
    "/EHsc",
    "/DWIN32_LEAN_AND_MEAN",
    "/DNOMINMAX",
    "/FIArduino.h",
    "/I$desktopInclude",
    "/I$includeDir",
    "/I$libDir"
)

if ($Debug) {
    $commonArgs += @("/Od", "/Zi", "/DDEBUG", "/MDd")
} else {
    $commonArgs += @("/O2", "/GL", "/DNDEBUG", "/MD")
}

$linkFlags = [string[]]@(
    "/nologo",
    "/SUBSYSTEM:WINDOWS"
)
if ($Debug) {
    $linkFlags += "/DEBUG"
} else {
    $linkFlags += "/LTCG"
}

$flagString   = ($commonArgs + $linkFlags) -join " "
$flagHash     = ([System.Security.Cryptography.SHA256]::Create().ComputeHash(
                    [System.Text.Encoding]::UTF8.GetBytes($flagString)) |
                 ForEach-Object { $_.ToString("x2") }) -join ""
$prevHash     = if (Test-Path $flagHashFile) { Get-Content $flagHashFile -Raw } else { "" }
$flagsChanged = ($flagHash.Trim() -ne $prevHash.Trim())
if ($flagsChanged) {
    Write-Warn "Compiler flags changed - forcing full recompile."
}

$libSources = Get-ChildItem (Join-Path $root "lib\PipKit") -Recurse -Filter "*.cpp" |
              Where-Object {
                  $_.FullName -notmatch [regex]::Escape("\Platforms\ESP32\") -and
                  $_.FullName -notmatch [regex]::Escape("\Displays\ST7789\")  -and
                  $_.FullName -notmatch [regex]::Escape("\Displays\ILI9488\")
              } | Sort-Object { $_.FullName }

$appSources = Get-ChildItem (Join-Path $root "src") -File -Filter "*.cpp" |
              Sort-Object { $_.FullName }

$sources = @($libSources) + @($appSources)

function Get-DepTicks ([string]$depFile) {
    if (!(Test-Path $depFile)) { return -1 }
    $maxTick = 0L
    foreach ($line in [System.IO.File]::ReadAllLines($depFile)) {
        $p = $line.Trim()
        if (!$p) { continue }
        if (!(Test-Path $p)) { return -1 }
        $t = (Get-Item $p).LastWriteTimeUtc.Ticks
        if ($t -gt $maxTick) { $maxTick = $t }
    }
    return $maxTick
}

$rootPrefix  = (Resolve-Path $root).Path
$compileWork = [System.Collections.Generic.List[hashtable]]@()
$objectFiles = [System.Collections.Generic.List[string]]@()

foreach ($src in $sources) {
    $relative = $src.FullName.Substring($rootPrefix.Length).TrimStart('\/')
    $objName  = ($relative -replace '[:\\/]', '__') -replace '\.cpp$', '.obj'
    $objPath  = Join-Path $objDir $objName
    $depPath  = $objPath -replace '\.obj$', '.d'
    $objectFiles.Add($objPath)

    $needsCompile = $flagsChanged -or !(Test-Path $objPath)
    if (!$needsCompile) {
        $objTicks     = (Get-Item $objPath).LastWriteTimeUtc.Ticks
        $depTicks     = Get-DepTicks $depPath
        $needsCompile = ($objTicks -lt $src.LastWriteTimeUtc.Ticks) -or
                        ($depTicks -eq -1) -or
                        ($objTicks -lt $depTicks)
    }

    if ($needsCompile) {
        $compileWork.Add(@{
            Src      = $src.FullName
            Obj      = $objPath
            Dep      = $depPath
            Relative = $relative
        })
    }
}

$totalFiles   = $sources.Count
$toCompile    = $compileWork.Count
$skippedCount = $totalFiles - $toCompile

Write-Host ""
if ($skippedCount -gt 0) {
    Write-Host "  $toCompile to compile, $skippedCount up-to-date  ($totalFiles total)" -ForegroundColor DarkGray
} else {
    Write-Host "  $toCompile files to compile" -ForegroundColor DarkGray
}
Write-Host ""

$compileFailed = $false

if ($toCompile -gt 0) {
    $effectiveJobs = [Math]::Max(1, [Math]::Min($Jobs, $toCompile))

    if ($effectiveJobs -eq 1 -or $toCompile -eq 1) {
        foreach ($work in $compileWork) {
            Write-Step "Compiling $($work.Relative)"
            $raw      = & $cl @commonArgs /showIncludes /c $work.Src "/Fo$($work.Obj)" 2>&1
            $includes = [System.Collections.Generic.List[string]]@()
            foreach ($line in $raw) {
                $s = "$line"
                if ($s -match ':\s+([A-Za-z]:\\.+)$') {
                    $p = $Matches[1].Trim()
                    if ($p -and (Test-Path $p)) { $includes.Add($p) }
                } else {
                    Write-Host $s
                }
            }
            if ($LASTEXITCODE -ne 0) { Write-Fatal "Compile failed: $($work.Relative)" }
            [System.IO.File]::WriteAllLines($work.Dep, [string[]]$includes)
        }
    } else {
        Write-Step "Parallel compile ($effectiveJobs workers)..."

        $clBase = '"' + $cl + '" ' + (($commonArgs | ForEach-Object {
            if ($_ -match '\s') { '"' + $_ + '"' } else { $_ }
        }) -join ' ')

        $pool = [RunspaceFactory]::CreateRunspacePool(1, $effectiveJobs)
        $pool.Open()

        $compileScript = {
            param([string]$ClBase, [string]$Src, [string]$Obj, [string]$Dep)
            $raw      = cmd /c "$ClBase /showIncludes /c `"$Src`" `"/Fo$Obj`"" 2>&1
            $includes = [System.Collections.Generic.List[string]]@()
            $userOut  = [System.Collections.Generic.List[string]]@()
            foreach ($line in $raw) {
                $s = "$line"
                if ($s -match ':\s+([A-Za-z]:\\.+)$') {
                    $p = $Matches[1].Trim()
                    if ($p) { $includes.Add($p) }
                } else {
                    $userOut.Add($s)
                }
            }
            if ($LASTEXITCODE -eq 0) {
                [System.IO.File]::WriteAllLines($Dep, [string[]]$includes)
            }
            [pscustomobject]@{
                ExitCode = $LASTEXITCODE
                Output   = ($userOut | Out-String).Trim()
                Src      = $Src
            }
        }

        $pending = [System.Collections.Generic.List[hashtable]]@()
        foreach ($work in $compileWork) {
            $ps = [PowerShell]::Create()
            $ps.RunspacePool = $pool
            $null = $ps.AddScript($compileScript).AddParameters(@{
                ClBase = $clBase
                Src    = $work.Src
                Obj    = $work.Obj
                Dep    = $work.Dep
            })
            $pending.Add(@{
                PS       = $ps
                Handle   = $ps.BeginInvoke()
                Relative = $work.Relative
                Src      = $work.Src
                Done     = $false
            })
        }

        $doneCount = 0
        while ($doneCount -lt $pending.Count) {
            foreach ($entry in $pending) {
                if ($entry.Done -or -not $entry.Handle.IsCompleted) { continue }
                $entry.Done = $true
                $doneCount++
                $result = $entry.PS.EndInvoke($entry.Handle)
                $entry.PS.Dispose()

                if ($result.ExitCode -ne 0) {
                    Write-Host "  FAILED: $($entry.Relative)" -ForegroundColor Red
                    $errLines = $result.Output -split "`r?`n" | Where-Object {
                        $_ -and
                        $_ -notmatch '^\s*$' -and
                        $_ -notmatch 'Microsoft \(R\)' -and
                        $_ -notmatch '^C\) ' -and
                        $_ -notmatch ('^' + [regex]::Escape([IO.Path]::GetFileNameWithoutExtension($entry.Src)) + '\s*$')
                    }
                    if ($errLines) { Write-Host ($errLines -join "`n") -ForegroundColor DarkRed }
                    $compileFailed = $true
                } else {
                    Write-Ok "Compiled $($entry.Relative)"
                }
            }
            if ($doneCount -lt $pending.Count) { Start-Sleep -Milliseconds 30 }
        }

        $pool.Close()
        $pool.Dispose()

        if ($compileFailed) {
            Write-Fatal "One or more source files failed to compile. Aborting."
        }
    }

    Set-Content -Path $flagHashFile -Value $flagHash -NoNewline
}

$needsLink = !(Test-Path $exe) -or $flagsChanged
if (!$needsLink) {
    $exeTicks = (Get-Item $exe).LastWriteTimeUtc.Ticks
    foreach ($obj in $objectFiles) {
        if (!(Test-Path $obj) -or ((Get-Item $obj).LastWriteTimeUtc.Ticks -gt $exeTicks)) {
            $needsLink = $true
            break
        }
    }
}

if ($needsLink) {
    Write-Step "Linking $exeName..."

    $rspLines = [System.Collections.Generic.List[string]](@("/OUT:`"$exe`"") + $linkFlags + @(
        "user32.lib", "gdi32.lib", "windowscodecs.lib",
        "mfplat.lib", "mfreadwrite.lib", "mfuuid.lib", "ole32.lib"
    ))
    foreach ($obj in $objectFiles) {
        $rspLines.Add($(if ($obj -match '\s') { "`"$obj`"" } else { $obj }))
    }
    [System.IO.File]::WriteAllLines($linkArgFile, $rspLines, [System.Text.Encoding]::ASCII)

    & $link "@$linkArgFile"
    if ($LASTEXITCODE -ne 0) { Write-Fatal "Link failed (exit $LASTEXITCODE)." }
    Write-Ok "Linked: $exeName"
} else {
    Write-Ok "Link skipped - executable is up to date."
}

$buildStart.Stop()
$elapsed = $buildStart.Elapsed

Write-Host ""
Write-Host "  done in $($elapsed.ToString('mm\:ss\.f'))  ->  $exe" -ForegroundColor DarkGray
Write-Host ""

if (!$NoRun) {
    Write-Host "  launching..." -ForegroundColor DarkGray
    Start-Process -FilePath $exe -WorkingDirectory $buildDir
}