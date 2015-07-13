
set Bin=%~dp0..\bin
set Libs=%~dp0..\libs

set Rar="E:\Program Files\WinRAR\winrar.exe"
set DX=E:\phone\android-sdks\build-tools\17.0.0\dx.bat

dir %Bin%\javahook.jar

rd /s /q %Bin%\temp
mkdir %Bin%\temp

%Rar% X -y %Bin%\javahook.jar %Bin%\temp
%Rar% X -y %Libs%\commons-lang3-3.1.jar %Bin%\temp

cd %Bin%\temp

%Rar% a -r hook.zip

call %DX% --dex --output=%Bin%\hook.jar %Bin%\temp\hook.zip

dir %Bin%\*.jar

::pause