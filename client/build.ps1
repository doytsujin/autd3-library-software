﻿#
# File: build.ps1
# Project: client
# Created Date: 25/08/2019
# Author: Shun Suzuki
# -----
# Last Modified: 21/02/2020
# Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
# -----
# Copyright (c) 2019-2020 Hapis Lab. All rights reserved.
# 
#

Param(
    [string]$BUILD_DIR = "\build",
    [ValidateSet(2017 , 2019)]$VS_VERSION = 2019,
    [string]$ARCH = "x64",
    [switch]$DISABLE_MATLAB = $FALSE,
    [switch]$ENABLE_TEST,
    [string]$TOOL_CHAIN = ""
)

$ROOT_DIR = $PSScriptRoot
$PROJECT_DIR = Join-Path $ROOT_DIR $BUILD_DIR

function ColorEcho($color, $PREFIX, $message) {
    Write-Host $PREFIX -ForegroundColor $color -NoNewline
    Write-Host ":", $message
}

function FindCMake() {
    $cmake_reg = Get-ChildItem HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall | 
    ForEach-Object { Get-ItemProperty $_.PsPath } | 
    Where-Object DisplayName -match "CMake"
    if ($cmake_reg) {
        return Join-Path $cmake_reg.InstallLocation \bin
    }
    else {
        return "NULL"
    }
}

function ReplaceContent([string]$filepath, [string]$rep1, [string]$rep2) {
    $file = $(Get-Content $filepath) -replace $rep1, $rep2
    $file > $filepath
}

if ($ENABLE_TEST -and ($TOOL_CHAIN -eq "")) {
    ColorEcho "Red" "Error" "Please specify vcpkg tool chain file. ex. -TOOL_CHAIN ""-DCMAKE_TOOLCHAIN_FILE=C:[...]\vcpkg\scripts\buildsystems\vcpkg.cmake"""
    $host.UI.RawUI.ReadKey() | Out-Null
    Exit
}

$IDE_NAME = "Visual Studio "
if ($VS_VERSION -eq 2017) {
    if ( ($ARCH -eq "x86") -or ($ARCH -eq "Win32")) {
        $IDE_NAME = $IDE_NAME + "15 2017"
    }
    else {
        $IDE_NAME = $IDE_NAME + "15 2017 Win64"
    }
}
elseif ($VS_VERSION -eq 2019) {
    $IDE_NAME = $IDE_NAME + "16 2019"
}
else {
    ColorEcho "Red" "Error" "This Library only support Visual Studio 2017 or 2019."
}

ColorEcho "Green" "INFO" "Use", $IDE_NAME, "with", $ARCH, "architecture."

if (-not (Get-Command cmake -ea SilentlyContinue)) {
    ColorEcho "Green" "INFO" "CMake not found in PATH. Looking for CMake..."
    $cmake_path = FindCMake
    if ($cmake_path -eq "NULL") {
        ColorEcho "Red" "Error" "CMake not found. Install CMake or set CMake binary folder to PATH."
        $host.UI.RawUI.ReadKey() | Out-Null
        exit
    }
    else {
        $env:Path = $env:Path + ";" + $cmake_path
    }
}
$cmake_version = 0
foreach ($line in cmake -version) {
    $ary = $line -split " "
    $cmake_version = $ary[2]
    break
}
ColorEcho "Green" "INFO" "Find CMake", $cmake_version

ColorEcho "Green" "INFO" "Create project directory if not exists."
if (Test-Path $PROJECT_DIR) { 
    ColorEcho "Yellow" "WARNING" "Directory", $PROJECT_DIR, "is already exists."
    $ans = Read-Host "Overwrite? (Y/[N])"
    if (($ans -eq "y") -or ($ans -eq "Y")) {
        Remove-Item $PROJECT_DIR -Recurse -Force
        if (Test-Path $PROJECT_DIR) {         
            ColorEcho "Red" "Error" "Cannot remove directory", $PROJECT_DIR
            $host.UI.RawUI.ReadKey() | Out-Null
            exit
        }
        else {
            ColorEcho "Green" "INFO" "Remove directory", $PROJECT_DIR         
        }
    }
    else {
        ColorEcho "Green" "INFO" "Cancled..."
        $host.UI.RawUI.ReadKey() | Out-Null
        exit
    }
}
if (New-Item -Path $PROJECT_DIR -ItemType Directory) {
    ColorEcho "Green" "INFO" "Success to create project directory", $PROJECT_DIR
}

ColorEcho "Green" "INFO" "Creating VS solution..."
Push-Location $PROJECT_DIR
$command = "cmake .. -G " + '$IDE_NAME'
if ($VS_VERSION -ne 2017) {
    $command += " -A " + $ARCH
}

if ($ENABLE_TEST) {
    $command += " -D ENABLE_TESTS=ON " + $TOOL_CHAIN
}
else {
    $command += " -D ENABLE_TESTS=OFF"
}

if ($DISABLE_MATLAB) {
    $command += " -D DISABLE_MATLAB=ON"
}
else {
    $command += " -D DISABLE_MATLAB=OFF"
}

Invoke-Expression $command
Pop-Location

ColorEcho "Green" "INFO" "Done."
$host.UI.RawUI.ReadKey() | Out-Null
exit
