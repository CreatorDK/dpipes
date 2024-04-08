@echo off
setlocal

for /f "delims=" %%I in ('powershell -noprofile "iex (${%~dp0Tests/scripts/openServer.ps1} | out-string)"') do (
    set server=%%~I
)

for /f "delims=" %%I in ('powershell -noprofile "iex (${%~dp0Tests/scripts/openClient.ps1} | out-string)"') do (
    set client=%%~I
)

set /P pipe="Select pipe type: [anonymous]/[named]/[all]: "
set /P test="Select test: [Test1-Test17]/[all]: "

%server% server %pipe% %test% /path %client%