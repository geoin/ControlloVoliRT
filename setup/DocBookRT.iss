; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "DocBook RT"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Geoin S.R.L."
#define MyAppURL "http://www.geoin.it"
#define MyAppExeName "DocBookRT"
#define vcfiles "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT"
; cartella da cui prelevare i files
#define MySourceDir "C:\DocBookRT" 
; cartella in cui registare il file per il setup
#define MyBaseDir "C:\ControlloVoliRT"
; cartella di default in cui installare i dati
#define MyOutDir "C:\DocBookRT"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{76CDED91-EB9E-45F1-BF76-BC81D5BBDA5C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={#MyOutDir}
DefaultGroupName=ControlloVoliRT\{#MyAppName}
OutputDir={#MyBaseDir}\setup
OutputBaseFilename={#MyAppExeName}_setup
SetupIconFile={#MyBaseDir}\icon\ControlloVoliRT.ico
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\{#MyAppExeName}
WizardImageFile={#MyBaseDir}\icon\Regione.bmp
UsePreviousAppDir=yes
WizardImageStretch=no
WizardSmallImageFile={#MyBaseDir}\icon\Regione.bmp
BackColor=$EBEAFC
BackColor2=$5BA17D
BackColorDirection=lefttoright
CloseApplications=yes 
MinVersion=5.1
PrivilegesRequired=admin
ChangesAssociations=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "Spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Registry]
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"; Check: NeedsAddPath('{app}')
;Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}\fop-1.0"; Check: NeedsAddPath('{app}\fop-1.0') 
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: expandsz; ValueName: "DOCBOOKRT"; ValueData: "{app}\docbookx.dtd" 

[Files]
Source: "{#MySourceDir}\docbook-xsl-1.78.1\*"; DestDir: "{app}\docbook-xsl-1.78.1"; Flags: recursesubdirs
Source: "{#MySourceDir}\fop-1.0\*"; DestDir: "{app}\fop-1.0"; Flags: recursesubdirs
Source: "{#MySourceDir}\lib\*"; DestDir: "{app}\lib"; Flags: recursesubdirs
Source: "{#MySourceDir}\docbookx.dtd"; DestDir: "{app}"; Flags: recursesubdirs

[Code]
// procedura per verificare se un valore � gi� stato settato
function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKEY_LOCAL_MACHINE,
    'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
    'Path', OrigPath)
  then begin
    Result := True;
    exit;
  end;
  // look for the path with leading and trailing semicolon
  // Pos() returns 0 if not found
  Result := Pos(';' + Param + ';', ';' + OrigPath + ';') = 0;
end;

procedure DeinitializeSetup();
var
  fileName : string;
  xls : string;
  proc_xls: string;
  proc_fop: string;
  tmp: string;
  prgData: string;
  lines : TArrayOfString;
begin
  filename := ExpandConstant('{app}\pdf_convert.bat');
  xls := ExpandConstant('{app}\docbook-xsl-1.78.1\fo\docbook.xsl');
  proc_xls := ExpandConstant('{app}\lib\xsltproc');
  proc_fop := ExpandConstant('{app}\fop-1.0\fop');
  tmp := ExpandConstant('{app}\tmp.fo');
  prgData := GetEnv('ProgramData');
  SetArrayLength(lines, 5);
  lines[0] := '@echo off';
  lines[1] := 'set path=' +  prgData +'\Oracle\Java\javapath;%PATH%;';
  lines[2] := '@echo on';

  lines[3] := proc_xls + ' ' + xls + ' %1.xml > ' + tmp;
  lines[4] := proc_fop + ' ' + tmp + ' %1.pdf';
  SaveStringsToFile(filename,lines,false);
  exit;
end;

