; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Controllo Voli RT"
#define MyAppVersion "1.0.3"
#define MyAppPublisher "Geoin S.R.L."
#define MyAppURL "http://www.geoin.it"
#define MyAppExeName "ControlloVoliRT"
#define vcfiles "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT"
#define MyBaseDir "C:\ControlloVoliRT"
#define MyBinDir "C:\ControlloVoliRT\bin"
#define MyLibDir "C:\ControlloVoliRT\lib"
#define MyIconDir "C:\ControlloVoliRT\icon"
#define MyOutDir "C:\OSGeo4W\apps\qgis\"
#define Qgis GetEnv('QGIS_PREFIX_PATH')
[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{5152E2E4-7EC2-4C16-B1F9-E5C6143F7B8C}
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
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: expandsz; ValueName: "CV_QGIS_PREFIX_PATH"; ValueData: {app}\

[Files]
Source: "{#MyLibDir}\PocoFoundation.dll"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyLibDir}\PocoXML.dll"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyLibDir}\PocoUtil.dll"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyLibDir}\PocoZip.dll"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyLibDir}\gdal.dll"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyLibDir}\ziplib.dll"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyLibDir}\rtklib.dll"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyLibDir}\dem_interpolate.dll"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyLibDir}\photo_util.dll"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyBinDir}\CVLoader.exe"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyLibDir}\RT-qgis_plugin.dll"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyBinDir}\check_photo.exe"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyBinDir}\check_gps.exe"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyBinDir}\check_ta.exe"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyBinDir}\check_ortho.exe"; DestDir: "{app}/plugins"; Flags: ignoreversion
Source: "{#MyIconDir}\gps.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyIconDir}\lidar.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyIconDir}\Ortho.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyIconDir}\lidar1.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyIconDir}\Regione.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyIconDir}\volo.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyIconDir}\voloL.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyIconDir}\voloP.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyIconDir}\voloPL.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyIconDir}\tria.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyIconDir}\projects.png"; DestDir: "{app}/icons"; Flags: ignoreversion
Source: "{#MyBaseDir}\setup\DocBookRT_setup.exe"; DestDir: "{tmp}"; Flags: ignoreversion
Source: "{#vcfiles}\msvcp90.dll";  DestDir: "{sys}"; Flags: restartreplace uninsneveruninstall 32bit
Source: "{#vcfiles}\msvcr90.dll";  DestDir: "{sys}"; Flags: restartreplace uninsneveruninstall 32bit

[Run]
Filename: "{tmp}\DocBookRT_setup.exe"; Flags: waituntilterminated







