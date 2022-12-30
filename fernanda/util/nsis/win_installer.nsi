!include "MUI.nsh"
!include "${NSISDIR}\Contrib\Modern UI\System.nsh"
!include "FileAssociation.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"

; ---------- Qt ----------
!define Q_VERSION "6.4.1"
!define Q_COMPILER "msvc2019_64"

; ---------- App ----------
!define APP "Fernanda"
!define PUB "@fairybow"
Name "${APP}"
Outfile "fernanda-setup-x64-windows.exe"

; ---------- Directories ----------
InstallDir "$PROGRAMFILES64\${APP}"
DirText "Choose a directory"
!define DATA "$INSTDIR\Data"
!define PLATFORMS "${DATA}\Platforms"
!define STYLES "${DATA}\Styles"
!define F_DIR "C:\Dev\fernanda"
!define Q_DIR "C:\Qt\${Q_VERSION}\${Q_COMPILER}"
!define Q_BIN "${Q_DIR}\bin"
!define Q_PLUGINS "${Q_DIR}\plugins"

; ---------- Files ----------
!define F_ICON "${F_DIR}\fernanda\res\icons\fernanda.ico"
!define F_EXE "${DATA}\fernanda.exe"
!define UN_F_EXE "$INSTDIR\Uninstall.exe"

; ---------- Registry ----------
!define F_REG "SOFTWARE\${APP}"
!define UN_F_REG "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APP}"

; ---------- MUI ----------
!define MUI_WELCOMEPAGE
!define MUI_LICENSEPAGE
!define MUI_DIRECTORYPAGE
!define MUI_ABORTWARNING
!define MUI_UNINSTALLER
!define MUI_UNCONFIRMPAGE
!define MUI_FINISHPAGE
!define MUI_ICON "${F_ICON}"
!define MUI_UNICON "${F_ICON}"
!insertmacro MUI_LANGUAGE "English"

; ---------- Functions ----------
Function CheckExists
	${If} ${FileExists} "$INSTDIR\*"
		MessageBox MB_YESNO `"$INSTDIR" already exists. Overwrite?` IDYES YES
		Abort
	YES:
		RMDir /r "$INSTDIR"
	${EndIf}
FunctionEnd

; ---------- Installation ----------
Section "Install"

	Call CheckExists

	; Write docs
	SetOutPath "$INSTDIR"
	File "${F_DIR}\LICENSE"
	File "${F_DIR}\README.md"

	; Write data
	SetOutPath "${DATA}"
	File "${F_DIR}\x64\Release\fernanda.exe"
	File "${Q_BIN}\Qt6Core.dll"
	File "${Q_BIN}\Qt6Gui.dll"
	File "${Q_BIN}\Qt6Widgets.dll"
	File "${Q_BIN}\Qt6Xml.dll"

	; Write platforms and styles
	SetOutPath "${PLATFORMS}"
	File "${Q_PLUGINS}\platforms\qwindows.dll"
	SetOutPath "${STYLES}"
	File "${Q_PLUGINS}\styles\qwindowsvistastyle.dll"

	; Write shortcuts
	CreateShortCut "$INSTDIR\Fernanda.lnk" "${F_EXE}"
	CreateShortCut "$INSTDIR\Fernanda (dev).lnk" "${F_EXE}" "-dev"

	; Register file extension
	${unregisterExtension} ".story" "Fernanda Story File"
	${registerExtension} "${F_EXE}" ".story" "Fernanda Story File"

	; Create desktop shortcut
	CreateShortCut "$DESKTOP\${APP}.lnk" "${F_EXE}" ""
 
	; Create start menu items
	CreateDirectory "$SMPROGRAMS\${APP}"
	CreateShortCut "$SMPROGRAMS\${APP}\Uninstall.lnk" "${UN_F_EXE}" "" "${UN_F_EXE}" 0
	CreateShortCut "$SMPROGRAMS\${APP}\${APP}.lnk" "${F_EXE}" "" "${F_EXE}" 0
 
	; Register uninstall info
	WriteRegStr HKLM "${UN_F_REG}" "DisplayIcon" "${F_ICON}"
	WriteRegStr HKLM "${UN_F_REG}" "DisplayName" "${APP}"
	WriteRegStr HKLM "${UN_F_REG}" "UninstallString" "${UN_F_EXE}"
	WriteRegStr HKLM "${UN_F_REG}" "Publisher" "${PUB}"
	${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
	IntFmt $0 "0x%08X" $0
	WriteRegDWORD HKLM "${UN_F_REG}" "EstimatedSize" "$0"
 
	WriteUninstaller "${UN_F_EXE}"

SectionEnd

; ---------- Uninstallation ----------
Section "Uninstall"

	${unregisterExtension} ".story" "Fernanda Story File"

	RMDir /r "$INSTDIR\*.*"
	RMDir "$INSTDIR"

	Delete "$DESKTOP\${APP}.lnk"
	Delete "$SMPROGRAMS\${APP}\*.*"
	RmDir  "$SMPROGRAMS\${APP}"

	DeleteRegKey HKEY_LOCAL_MACHINE "${F_REG}"
	DeleteRegKey HKEY_LOCAL_MACHINE "${UN_F_REG}"
 
SectionEnd