#include <windows.h>
//#include <strsafe.h>
//#include <commctrl.h>
#include <string>
#include <tchar.h>

#define WM_SHELLNOTIFY (WM_APP + 1)

NOTIFYICONDATA icon_struct;
//f0ad928e-9908-497f-8f95-bc42c9e1805c
const GUID appGUID = {0xf0ad928e, 0x9908, 0x497f, {0x8f, 0x95, 0xbc, 0x42, 0xc9, 0xe1, 0x80, 0x5c}};


// ќбработка сообщений
LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){

		//// ѕроверка по таймеру
		//case WM_TIMER:
		//	bool check = FileExists(TEXT("P:\\"));
		//	if(State != check){
		//		if(State){
		//			Icon.hIcon = LoadIcon(NULL, IDI_SHIELD);
		//		}else{
		//			Icon.hIcon = LoadIcon(NULL, IDI_WARNING);
		//		}
		//		Icon.uFlags = NIF_ICON;
		//		Shell_NotifyIcon(NIM_MODIFY, &Icon);
		//		State = check;
		//	}
		//	break;
			
		// —ообщение от значка
		case WM_SHELLNOTIFY:
			if(LOWORD(lParam) == WM_CONTEXTMENU){
				if(MessageBox(NULL, TEXT("«авершить работу?"), TEXT("Tray"), MB_YESNO) == IDYES){
					DestroyWindow(window);
				}
			}
			break;
		
		// —тандартна€ обработка сообщений
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(window, message, wParam, lParam);
	}
	return 0;
}


// √лавна€ функци€
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd){
	//–егистраци€ класса окна
	WNDCLASSEX main = {0};
	main.cbSize = sizeof(WNDCLASSEX);
	main.hInstance = hInstance;
	main.lpszClassName = TEXT("Main");
	main.lpfnWndProc = WndProc;
	RegisterClassEx(&main);

	// —оздание главного окна
	HWND window = CreateWindowEx(0, TEXT("Main"), NULL, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

	// —оздание значка
	icon_struct = {0};
	icon_struct.cbSize = sizeof(NOTIFYICONDATA);
	icon_struct.hWnd = window;
	icon_struct.uVersion = NOTIFYICON_VERSION_4;
	icon_struct.uID = 1;

	//используем UUID (только дл€ win7)
	//ќбрабатываем событи€
	//выводим иконку в трей
	//выводим подсказку по наведению
	//стандартный вывод подсказки
	//вывод баллона
	icon_struct.uFlags = NIF_GUID | NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_INFO;

	icon_struct.guidItem = appGUID;

	//если uVersion = 0 или NOTIFYICON_VERSION (не наш случай)
	//ѕараметр wParam будет определен идентификатором значка, который генерирует сообщение.
	//ѕараметр lParam "реальный" идентификатор сообщени€ Windows

	//наш случай
	//LOWORD(lParam) содержит событи€ уведомлений, такие как NIN_BALLOONSHOW, NIN_POPUPOPEN или WM_CONTEXTMENU.
	//HIWORD(lParam) содержит идентификатор значка. ƒлина идентификаторов значков ограничена 16 битами.
	//GET_X_LPARAM(wParam) возвращает координату прив€зки X дл€ событий уведомлений NIN_POPUPOPEN, NIN_SELECT, NIN_KEYSELECT и всех сообщений мыши между WM_MOUSEFIRST и WM_MOUSELAST. ≈сли какое-либо из этих сообщений генерируетс€ клавиатурой, параметр wParam устанавливаетс€ в верхнем левом углу целевого значка. ƒл€ всех остальных сообщений параметр wParam не определен.
	//GET_Y_LPARAM(wParam) возвращает координату прив€зки Y дл€ событий уведомлений и сообщений, как определено дл€ прив€зки X.
	icon_struct.uCallbackMessage = WM_SHELLNOTIFY;

	//LoadIconMetric(hInstance, MAKEINTRESOURCE(102), LIM_SMALL, &(icon_struct.hIcon));
	icon_struct.hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(103), IMAGE_ICON, SM_CXICON, SM_CYICON, LR_DEFAULTCOLOR);
	//icon_struct.hIcon = LoadIcon(NULL, IDI_SHIELD);

	//—трока с нулевым символом в конце. макс 128 символов включа€ 0
	//StringCchCopy(icon_struct.szTip, ARRAYSIZE(icon_struct.szTip), TEXT("Test application"));
	lstrcpyn(icon_struct.szTip, TEXT("Test application"), sizeof(icon_struct.szTip)/sizeof(icon_struct.szTip[0]));

	Shell_NotifyIcon(NIM_ADD, &icon_struct);
	Shell_NotifyIcon(NIM_SETVERSION, &icon_struct);

	// ÷икл обработки сообщений
	MSG message;
	while(GetMessage(&message, NULL, 0, 0)){
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	
	// ”даление значка
	Shell_NotifyIcon(NIM_DELETE, &icon_struct);
	//очистка пам€ти
	DestroyIcon(icon_struct.hIcon);
	return 0;
}