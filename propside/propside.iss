; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "SimpleIDE"
#define MyDocName "SimpleIDE"
#define MyAppVersion "0-5-0"
#define MyAppPublisher "MicroCSource"
#define MyAppURL "www.MicroCSource.com"
#define MyAppExeName "bin\SimpleIDE.exe"

#define compiler "C:\propgcc"

; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ---- IMPORTANT!!! ---- Set this to your QtPath
; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define MyQtPath "C:\Qt\2010.05"
#define MyGccPath "C:\msys\opt\parallax"
#define MyGccMingwPath "C:\mingw"

#define MyAppBin "{app}\bin"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppID={{4FA91D9B-6633-4229-B3BE-DF96DFD916F3}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=..\propside-build-desktop
OutputBaseFilename=Simple-IDE_{#MyAppVersion}_setup
Compression=lzma/Max
SolidCompression=true
AlwaysShowDirOnReadyPage=true
UserInfoPage=false
UsePreviousUserInfo=false
ChangesEnvironment=true
LicenseFile=.\IDE_LICENSE.txt

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1
Name: modifypath; Description: "&Add Propeller-GCC directory to your environment PATH";

[Files]
Source: "..\propside-build-desktop\debug\SimpleIDE.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "IDE_LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LGPL_2_1.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LGPL_EXCEPTION.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\ctags58\README"; DestDir: "{app}\ctags-readme.txt"; Flags: ignoreversion
Source: "..\ctags58\COPYING"; DestDir: "{app}\ctags-license.txt"; Flags: ignoreversion
Source: "..\icons\24x24-free-application-icons\readme.txt"; DestDir: "{app}\bin\aha-soft-license.txt"; Flags: ignoreversion

Source: "{#MyQtPath}\mingw\bin\libgcc_s_dw2-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MyQtPath}\mingw\bin\mingwm10.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MyQtPath}\qt\bin\QtCored4.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MyQtPath}\qt\bin\QtGuid4.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\propside-demos\*"; DestDir: "{code:GetDataDir}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\ctags58\ctags.exe"; DestDir: "{code:GetCompilerDir}\bin"; Flags: ignoreversion
Source: "{#MyGccPath}\*"; DestDir: "{code:GetCompilerDir}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyGccMingwPath}\bin\libi*"; DestDir: "{code:GetCompilerDir}\bin"; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
;don't run: the environment variable will not be set until program restart.
;Filename: {app}\{#MyAppExeName}; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; Flags: skipifsilent NoWait PostInstall; 

[Registry]
Root: HKCU; SubKey: Software\MicroCSource; Flags: UninsDeleteKey; 
Root: HKCU; Subkey: "Software\MicroCSource\SimpleIDE"; Flags: UninsDeleteKey; 
Root: HKCU; Subkey: "Software\MicroCSource\SimpleIDE"; ValueType: string; ValueName: Compiler; ValueData: {code:GetCompilerDir}\bin\propeller-elf-gcc.exe; Flags: UninsDeleteKey; 
Root: HKCU; Subkey: "Software\MicroCSource\SimpleIDE"; ValueType: string; ValueName: Includes; ValueData: {code:GetCompilerDir}\propeller-load\; Flags: UninsDeleteKey; 
Root: HKCU; Subkey: "Software\MicroCSource\SimpleIDE"; ValueType: string; ValueName: LastFileName; ValueData: {code:GetDataDir}\hello\hello.c; Flags: UninsDeleteKey; 
Root: HKCU; Subkey: "Software\MicroCSource\SimpleIDE"; ValueType: string; ValueName: Workspace; ValueData: {code:GetDataDir}; Flags: UninsDeleteKey;

[Code]
var
  DataDirPage: TInputDirWizardPage;
  CompilerPage: TInputDirWizardPage;
  
procedure InitializeWizard;
begin
  { Create the pages }
  CompilerPage := CreateInputDirPage(wpSelectDir,
    'Select Compiler Directory', 'Where is Propeller-GCC installed?',
    'Select the folder where Propeller-GCC tools will be installed.    Please do not use a folder having spaces in the folder name.',
    False, '');
  CompilerPage.Add('');
  CompilerPage.Values[0] := GetPreviousData('CompilerDir', ExpandConstant('{#compiler}'));

  DataDirPage := CreateInputDirPage(wpSelectDir,
    'Select Workspace Directory', 'Where should source files be installed?',
    'Select the folder where Setup will install source files, then click Next.',
    False, '');
  DataDirPage.Add('');
  DataDirPage.Values[0] := GetPreviousData('DataDir', ExpandConstant('{userdocs}')+'\SimpleIDE');
end;

function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo,
  MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
var
  S: String;
begin
  { Fill the 'Ready Memo' with the normal settings and the custom settings }
  S := '';
  S := S + MemoGroupInfo + Newline + Newline;
  S := S + MemoDirInfo + Newline + Newline;
  S := S + 'SimpleIDE source folder:' + Newline + Space + DataDirPage.Values[0] + NewLine + NewLine;
  S := S + 'Propeller-GCC folder:' + Newline + Space + CompilerPage.Values[0] + NewLine + NewLine;
  Result := S;
end;

function GetDataDir(Param: String): String;
begin
  { Return the selected DataDir }
  Result := DataDirPage.Values[0];
end;

function GetCompilerDir(Param: String): String;
begin
  { Return the selected CompilerDir }
  Result := CompilerPage.Values[0];
end;

const
    ModPathName = 'modifypath';
    ModPathType = 'user';

function ModPathDir(): TArrayOfString;
begin
    setArrayLength(Result, 1);
    Result[0] := ExpandConstant('{#compiler}\bin');
end;
#include "modpath.iss"
