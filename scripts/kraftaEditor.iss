[Setup]
AppName=Krafta Editor
AppVersion=1.0.0
DefaultDirName={pf}\Krafta Editor
DefaultGroupName=Krafta Editor
OutputDir=dist
OutputBaseFilename=kraftaEditor_Setup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
DisableDirPage=no

[Files]
Source: "build\bin\RELEASE\kraftaEditor.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\*"; DestDir: "{app}\assets"; Flags: recursesubdirs createallsubdirs
Source: "languages\*"; DestDir: "{app}\languages"; Flags: recursesubdirs

[Icons]
Name: "{group}\Krafta Editor"; Filename: "{app}\kraftaEditor.exe"
Name: "{commondesktop}\Krafta Editor"; Filename: "{app}\kraftaEditor.exe"

[Run]
Filename: "{app}\kraftaEditor.exe"; Description: "Run Krafta Editor"; Flags: nowait postinstall skipifsilent
