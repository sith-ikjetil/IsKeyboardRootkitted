ECHO CREATING DIGITAL SIGNATURE
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64\signtool"  sign /sm /a /fd sha256 /t  "http://timestamp.comodoca.com/authenticode" .\..\x64\Release\KKS.IsKeyboardRktApp.exe
