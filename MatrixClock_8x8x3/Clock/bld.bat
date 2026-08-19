set PATH=%PATH%;C:\Users\teifd2\AppData\Local\Python\pythoncore-3.14-64\Scripts\
pyinstaller pyinstaller --clean --onefile -w --name OledClock ^
  --collect-all Pillow ^
  --hidden-import=PIL._imaging ^
  --hidden-import=serial.tools.list_ports ^
  --icon=7segClock.ico ^
  --add-data "link.png;." ^
  --add-data "unlink.png;." ^
  --paths=C:\D224\Git\Arduino\8266_Oled\Clock\.venv\Lib\site-packages main.py
