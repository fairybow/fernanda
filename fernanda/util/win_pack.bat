SET qt_ver="6.4.1"
SET qt_compiler="msvc2019_64"
SET qt="%SystemDrive%\Qt\%qt_ver%\%qt_compiler%"
SET qwin="%qt%\plugins\platforms\qwindows.dll"
SET qwinstyle="%qt%\plugins\styles\qwindowsvistastyle.dll"
SET qt_core="%qt%\bin\Qt6Core.dll"
SET qt_gui="%qt%\bin\Qt6Gui.dll"
SET qt_widgets="%qt%\bin\Qt6Widgets.dll"
SET qt_xml="%qt%\bin\Qt6Xml.dll"

SET vs_fernanda="%SystemDrive%\Dev\fernanda\x64\Release\fernanda.exe"

SET data="fernanda\data"
SET platforms="%data%\platforms"
SET styles="%data%\styles"

md %platforms%
md %styles%

echo f|xcopy /v /y /f "%vs_fernanda%" "%data%\fernanda.exe"
echo f|xcopy /v /y /f "%qwin%" "%platforms%\qwindows.dll"
echo f|xcopy /v /y /f "%qwinstyle%" "%styles%\qwindowsvistastyle.dll"
echo f|xcopy /v /y /f "%qt_core%" "%data%\Qt6Core.dll"
echo f|xcopy /v /y /f "%qt_gui%" "%data%\Qt6Gui.dll"
echo f|xcopy /v /y /f "%qt_widgets%" "%data%\Qt6Widgets.dll"
echo f|xcopy /v /y /f "%qt_xml%" "%data%\Qt6Xml.dll"

powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('fernanda\fernanda.lnk');$s.TargetPath='fernanda\data\fernanda.exe';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('fernanda\fernanda (dev).lnk');$s.TargetPath='fernanda\data\fernanda.exe';$s.Arguments='-dev';$s.Save()"