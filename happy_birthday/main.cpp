#include <windows.h>
#include <windowsx.h>
//#include <strsafe.h>
//#include <commctrl.h>
#include <string>
#include <tchar.h>

#define WM_SHELLNOTIFY (WM_APP + 1)
#define CM_EXIT 15
#define CM_EXEC 14

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
	HMENU context_menu;

public:
	App(HINSTANCE hInstance) : hInstance(hInstance), main_win_c({0}), window(nullptr), icon_struct({0}), context_menu(nullptr){
		window = create_window();
		this->init_icon_struct();
		this->init_context_menu();
	}

	void hide_icon(){
		// �������� ������
		Shell_NotifyIcon(NIM_DELETE, &icon_struct);
	}

	int start(){
		Shell_NotifyIcon(NIM_ADD, &icon_struct);
		Shell_NotifyIcon(NIM_SETVERSION, &icon_struct);

		// ���� ��������� ���������
		MSG message;
		while(GetMessage(&message, NULL, 0, 0)){
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		return 0;
	}

	bool show_context_menu(INT16 x, INT16 y){
 
		SetForegroundWindow(window);
		bool ret = TrackPopupMenu(
			context_menu,
			//TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, 
			TPM_LEFTBUTTON, 
			x, 
			y,
			0,
			window,
			NULL
		);
		return ret;
	}

	void context_menu_click(INT16 menu_item_ind){
		switch(menu_item_ind){
			case CM_EXIT:
				DestroyWindow(window);
				break;
			case CM_EXEC:
				ModifyMenu(context_menu, CM_EXEC, MF_BYCOMMAND | MF_UNCHECKED | MFT_STRING, NULL, TEXT("��������"));
				break;
		}
	}

	~App(){
		//������� ������
		DestroyIcon(icon_struct.hIcon);
		DestroyIcon(icon_struct.hBalloonIcon);
		DestroyMenu(context_menu);
	}

private:
	HWND create_window(){
		//����������� ������ ����
		main_win_c.cbSize = sizeof(WNDCLASSEX);
		main_win_c.hInstance = hInstance;
		main_win_c.lpszClassName = TEXT("Main");
		main_win_c.lpfnWndProc = WndProc;
		RegisterClassEx(&main_win_c);

		// �������� �������� ����
		return CreateWindowEx(0, TEXT("Main"), NULL, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	}

	void init_icon_struct(){
		// �������� ������
		icon_struct.cbSize = sizeof(NOTIFYICONDATA);
		icon_struct.hWnd = window;
		icon_struct.uVersion = NOTIFYICON_VERSION_4;
		icon_struct.uID = 1;

		//���������� UUID (������ ��� win7)
		//������������ �������
		//������� ������ � ����
		//������� ��������� �� ���������
		//����������� ����� ���������
		//����� �������
		icon_struct.uFlags = NIF_GUID | NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_INFO;

		icon_struct.guidItem = appGUID;

		//���� uVersion = 0 ��� NOTIFYICON_VERSION (�� ��� ������)
		//�������� wParam ����� ��������� ��������������� ������, ������� ���������� ���������.
		//�������� lParam "��������" ������������� ��������� Windows

		//��� ������
		//LOWORD(lParam) �������� ������� �����������, ����� ��� NIN_BALLOONSHOW, NIN_POPUPOPEN ��� WM_CONTEXTMENU.
		//HIWORD(lParam) �������� ������������� ������. ����� ��������������� ������� ���������� 16 ������.
		//GET_X_LPARAM(wParam) ���������� ���������� �������� X ��� ������� ����������� NIN_POPUPOPEN, NIN_SELECT, NIN_KEYSELECT � ���� ��������� ���� ����� WM_MOUSEFIRST � WM_MOUSELAST. ���� �����-���� �� ���� ��������� ������������ �����������, �������� wParam ��������������� � ������� ����� ���� �������� ������. ��� ���� ��������� ��������� �������� wParam �� ���������.
		//GET_Y_LPARAM(wParam) ���������� ���������� �������� Y ��� ������� ����������� � ���������, ��� ���������� ��� �������� X.
		icon_struct.uCallbackMessage = WM_SHELLNOTIFY;

		//LoadIconMetric(hInstance, MAKEINTRESOURCE(102), LIM_SMALL, &(icon_struct.hIcon));
		icon_struct.hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(103), IMAGE_ICON, SM_CXICON, SM_CYICON, LR_DEFAULTCOLOR);
		//icon_struct.hIcon = LoadIcon(NULL, IDI_SHIELD);

		//������ � ������� �������� � �����. ���� 128 �������� ������� 0
		//StringCchCopy(icon_struct.szTip, ARRAYSIZE(icon_struct.szTip), TEXT("Test application"));
		lstrcpyn(icon_struct.szTip, TEXT("Test application"), sizeof(icon_struct.szTip)/sizeof(icon_struct.szTip[0]));
	
	
		lstrcpyn(icon_struct.szInfoTitle, TEXT("Title"), sizeof(icon_struct.szInfoTitle)/sizeof(icon_struct.szInfoTitle[0]));
		lstrcpyn(icon_struct.szInfo, TEXT("text"), sizeof(icon_struct.szInfo)/sizeof(icon_struct.szInfo[0]));

		icon_struct.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
		icon_struct.hBalloonIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(103), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, LR_DEFAULTCOLOR);
	}

	void init_context_menu(){
		context_menu = CreatePopupMenu();
		AppendMenu(context_menu, MFT_STRING | MF_CHECKED, CM_EXEC, TEXT("��������"));
		AppendMenu(context_menu, MFT_STRING, CM_EXIT, TEXT("&�����"));
	}
};

App* application = nullptr;


// ��������� ���������
LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
		//��������� �� ������
		case WM_SHELLNOTIFY:
			if(LOWORD(lParam) == WM_CONTEXTMENU){
				application->show_context_menu(GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
			}
			break;

		//������� ������ ������ ����
		case WM_COMMAND:
			application->context_menu_click(LOWORD(wParam));
			break;
			
		//����� �� ���������
		case WM_DESTROY:
			application->hide_icon();
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(window, message, wParam, lParam);
	}
	return 0;
}


// ������� �������
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd){

	App program(hInstance);

	application = &program;

	return program.start();
}