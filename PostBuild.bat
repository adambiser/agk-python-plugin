Copy "%1" "..\..\AGKPlugin\PythonPlugin\Windows.dll"
cd ..
cd ..
IF EXIST "UpdateAGKFolder.bat" (
	CALL UpdateAGKFolder.bat
)

REM Copy to each example project.
FOR /D %%G in ("Examples\*") DO (
	Copy "AGKPlugin\PythonPlugin\Windows.dll" "%%G\Plugins\PythonPlugin\Windows.dll"
)
