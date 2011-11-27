; Script generated by the HM NIS Edit Script Wizard.

SetCompressor /SOLID lzma

!include 'LogicLib.nsh'


; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "DXGL"
!define PRODUCT_PUBLISHER "William Feely"
!define PRODUCT_WEB_SITE "https://www.williamfeely.info/wiki/DXGL"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\dxglcfg.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!include "version.nsh"

; MUI2
!include "MUI2.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "..\COPYING.txt"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\dxglcfg.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Configure DXGL"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\ReadMe.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------


!define HKEY_CURRENT_USER 0x80000001
!define RegOpenKeyEx     "Advapi32::RegOpenKeyEx(i, t, i, i, *i) i"
!define RegQueryValueEx  "Advapi32::RegQueryValueEx(i, t, i, *i, i, *i) i"
!define RegCloseKey      "Advapi32::RegCloseKey(i) i"
!define REG_MULTI_SZ     7
!define INSTPATH         "InstallPaths"
!define KEY_QUERY_VALUE          0x0001
!define KEY_ENUMERATE_SUB_KEYS   0x0008
!define ROOT_KEY         ${HKEY_CURRENT_USER}
Var SUBKEY

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "DXGL-${PRODUCT_VERSION}-win32.exe"
InstallDir "$PROGRAMFILES\DXGL"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "..\Release\dxgltest.exe"
  CreateDirectory "$SMPROGRAMS\DXGL"
  CreateShortCut "$SMPROGRAMS\DXGL\DXGL Test.lnk" "$INSTDIR\dxgltest.exe"
  File "..\Release\dxglcfg.exe"
  CreateShortCut "$SMPROGRAMS\DXGL\Configure DXGL.lnk" "$INSTDIR\dxglcfg.exe"
  File "..\Release\ddraw.dll"
  File "..\ReadMe.txt"
  File "..\COPYING.txt"
  
  StrCpy $8 0
  SetPluginUnload alwaysoff
  regloop:
    EnumRegKey $SUBKEY HKCU "Software\DXGL" $8
    StrCmp $SUBKEY "" regdone
    StrCpy $SUBKEY "Software\DXGL\$SUBKEY"
    IntOp $8 $8 + 1
    ;REG_MULTI_SZ reader based on code at http://nsis.sourceforge.net/REG_MULTI_SZ_Reader
    StrCpy $0 ""
    StrCpy $1 ""
    StrCpy $2 ""
    StrCpy $3 ""
    System::Call "${RegOpenKeyEx}(${ROOT_KEY},'$SUBKEY',0, \
                  ${KEY_QUERY_VALUE}|${KEY_ENUMERATE_SUB_KEYS},.r0) .r3"
    StrCmp $3 0 readvalue
    Goto regloop
    readvalue:
    System::Call "${RegQueryValueEx}(r0,'${INSTPATH}',0,.r1,0,.r2) .r3"
    StrCmp $3 0 checksz
    goto readdone
    checksz:
    StrCmp $1 ${REG_MULTI_SZ} checkempty
    Goto readdone
    checkempty:
    StrCmp $2 0 0 multiszalloc
    Goto readdone
    multiszalloc:
    System::Alloc $2
    Pop $1
    StrCmp $1 0 0 multiszget
    Goto readdone
    multiszget:
    System::Call "${RegQueryValueEx}(r0, '${INSTPATH}', 0, n, r1, r2) .r3"
    StrCmp $3 0 multiszprocess
    System::Free $1
    Goto readdone
    multiszprocess:
    StrCpy $4 $1
    IntOp $6 $4 + $2
    !ifdef NSIS_UNICODE
    IntOp $6 $6 - 2
    !else
    IntOp $6 $6 - 1
    !endif
    szloop:
      System::Call "*$4(&t${NSIS_MAX_STRLEN} .r3)"
      StrLen $5 $3
      IntOp $5 $5 + 1
      !ifdef NSIS_UNICODE
      IntOp $5 $5 * 2
      !endif
      IntOp $4 $4 + $5
      ;copy file here
      DetailPrint "Installing ddraw.dll to $3"
      CopyFiles $INSTDIR\ddraw.dll $3
      IntCmp IntCmp $4 $6 0 szloop
      System::Free $1

    readdone:
    StrCmp $0 0 regloop
    System::Call "${RegCloseKey}(r0)"
    goto regloop
  regdone:
  SetPluginUnload manual
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\DXGL\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\DXGL\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\dxglcfg.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\dxglcfg.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\COPYING.txt"
  Delete "$INSTDIR\ReadMe.txt"
  Delete "$INSTDIR\ddraw.dll"
  Delete "$INSTDIR\dxglcfg.exe"
  Delete "$INSTDIR\dxgltest.exe"

  Delete "$SMPROGRAMS\DXGL\Uninstall.lnk"
  Delete "$SMPROGRAMS\DXGL\Website.lnk"
  Delete "$SMPROGRAMS\DXGL\Configure DXGL.lnk"
  Delete "$SMPROGRAMS\DXGL\DXGL Test.lnk"

  RMDir "$SMPROGRAMS\DXGL"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

  StrCpy $8 0
  SetPluginUnload alwaysoff
  regloop:
    EnumRegKey $SUBKEY HKCU "Software\DXGL" $8
    StrCmp $SUBKEY "" regdone
    StrCpy $SUBKEY "Software\DXGL\$SUBKEY"
    IntOp $8 $8 + 1
    ;REG_MULTI_SZ reader based on code at http://nsis.sourceforge.net/REG_MULTI_SZ_Reader
    StrCpy $0 ""
    StrCpy $1 ""
    StrCpy $2 ""
    StrCpy $3 ""
    System::Call "${RegOpenKeyEx}(${ROOT_KEY},'$SUBKEY',0, \
                  ${KEY_QUERY_VALUE}|${KEY_ENUMERATE_SUB_KEYS},.r0) .r3"
    StrCmp $3 0 readvalue
    Goto regloop
    readvalue:
    System::Call "${RegQueryValueEx}(r0,'${INSTPATH}',0,.r1,0,.r2) .r3"
    StrCmp $3 0 checksz
    goto readdone
    checksz:
    StrCmp $1 ${REG_MULTI_SZ} checkempty
    Goto readdone
    checkempty:
    StrCmp $2 0 0 multiszalloc
    Goto readdone
    multiszalloc:
    System::Alloc $2
    Pop $1
    StrCmp $1 0 0 multiszget
    Goto readdone
    multiszget:
    System::Call "${RegQueryValueEx}(r0, '${INSTPATH}', 0, n, r1, r2) .r3"
    StrCmp $3 0 multiszprocess
    System::Free $1
    Goto readdone
    multiszprocess:
    StrCpy $4 $1
    IntOp $6 $4 + $2
    !ifdef NSIS_UNICODE
    IntOp $6 $6 - 2
    !else
    IntOp $6 $6 - 1
    !endif
    szloop:
      System::Call "*$4(&t${NSIS_MAX_STRLEN} .r3)"
      StrLen $5 $3
      IntOp $5 $5 + 1
      !ifdef NSIS_UNICODE
      IntOp $5 $5 * 2
      !endif
      IntOp $4 $4 + $5
      ;copy file here
      DetailPrint "Removing ddraw.dll from $3"
      Delete $3\ddraw.dll
      IntCmp IntCmp $4 $6 0 szloop
      System::Free $1

    readdone:
    StrCmp $0 0 regloop
    System::Call "${RegCloseKey}(r0)"
    goto regloop
  regdone:
  SetPluginUnload manual

  SetAutoClose true
SectionEnd