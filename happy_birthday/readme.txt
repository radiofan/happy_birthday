работа с иконками в трее
https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shell_notifyicona
https://docs.microsoft.com/ru-RU/windows/win32/api/shellapi/ns-shellapi-notifyicondataa

работа с меню
http://www.vsokovikov.narod.ru/New_MSDN_API/Menu/fn_appendmenu.htm
http://narovol.narod.ru/_tbkp/New_MSDN_API/Menu/fn_modifymenu.htm

создание установщика приложения
https://marketplace.visualstudio.com/items?itemName=VisualStudioClient.MicrosoftVisualStudio2017InstallerProjects
https://www.cyberforum.ru/cpp-builder/thread15180.html
https://www.cyberforum.ru/win-api/thread130524.html

запуск программ для редактирования файла
https://docs.microsoft.com/ru-ru/windows/win32/shell/launch
https://docs.microsoft.com/ru-RU/windows/win32/api/shellapi/nf-shellapi-shellexecutea

описание кодировок
https://habr.com/ru/post/478636/
BOM
UTF-8		EF BB BF		1110 1111  1011 1011  1011 1111
UTF-16 (BE)	FE FF			1111 1110  1111 1111
UTF-16 (LE)	FF FE			1111 1111  1111 1110
UTF-32 (BE)	00 00 FE FF		0000 0000  0000 0000  1111 1110  1111 1111
UTF-32 (LE)	FF FE 00 00		1111 1111  1111 1110  0000 0000  0000 0000

utf8
1 байт
0iii iiii
2 байта
110i iiii  10ii iiii
3 байта
1110 iiii  10ii iiii  10ii iiii
4 байта
1111 0iii  10ii iiii  10ii iiii  10ii iiii

максимальный символ utf-8
f4 8f bf bf					1111 0100  1000 1111  1011 1111  1011 1111