#include <windows.h>
//#include <strsafe.h>
//#include <commctrl.h>
#include <string>
#include <tchar.h>

#define WM_SHELLNOTIFY (WM_APP + 1)

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

class App{
public:
	//f0ad928e-9908-497f-8f95-bc42c9e1805c
	const GUID appGUID = {0xf0ad928e, 0x9908, 0x497f, {0x8f, 0x95, 0xbc, 0x42, 0xc9, 0xe1, 0x80, 0x5c}};
private:
	NOTIFYICONDATA icon_struct;
	WNDCLASSEX main_win_c;
	HWND window;
	HINSTANCE hInstance;

public:
	App(HINSTANCE hInstance) : hInstance(hInstance), main_win_c({0}), window(nullptr), icon_struct({0}){
		window = create_window();
		this->init_icon_struct();
	}

	void hide_icon(){
		// ”даление значка
		Shell_NotifyIcon(NIM_DELETE, &icon_struct);
	}

	int start(){
		Shell_NotifyIcon(NIM_ADD, &icon_struct);
		Shell_NotifyIcon(NIM_SETVERSION, &icon_struct);

		// ÷икл обработки сообщений
		MSG message;
		while(GetMessage(&message, NULL, 0, 0)){
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		return 0;
	}

	~App(){
		//очистка пам€ти
		DestroyIcon(icon_struct.hIcon);
		DestroyIcon(icon_struct.hBalloonIcon);
	}

private:
	HWND create_window(){
		//–егистраци€ класса окна
		main_win_c.cbSize = sizeof(WNDCLASSEX);
		main_win_c.hInstance = hInstance;
		main_win_c.lpszClassName = TEXT("Main");
		main_win_c.lpfnWndProc = WndProc;
		RegisterClassEx(&main_win_c);

		// —оздание главного окна
		return CreateWindowEx(0, TEXT("Main"), NULL, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	}

	void init_icon_struct(){
		// —оздание значка
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
	
	
		lstrcpyn(icon_struct.szInfoTitle, TEXT("Title"), sizeof(icon_struct.szInfoTitle)/sizeof(icon_struct.szInfoTitle[0]));
		lstrcpyn(icon_struct.szInfo, TEXT("text"), sizeof(icon_struct.szInfo)/sizeof(icon_struct.szInfo[0]));

		icon_struct.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
		icon_struct.hBalloonIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(103), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, LR_DEFAULTCOLOR);
	}
};

App* application = nullptr;


// ќбработка сообщений
LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
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
			application->hide_icon();
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(window, message, wParam, lParam);
	}
	return 0;
}


// √лавна€ функци€
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd){

	App program(hInstance);

	application = &program;

	return program.start();
}