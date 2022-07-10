#include "stdafx.h"

std::wstring string_to_wstring(const std::string& str){
	UINT32 wstr_len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	if(!wstr_len)
		return std::wstring();
	HANDLE h_process_heap = GetProcessHeap();
	LPWSTR wstr = (LPWSTR)HeapAlloc(h_process_heap, 0, wstr_len * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wstr, wstr_len);
	std::wstring ret(wstr);
	HeapFree(h_process_heap, 0, wstr);
	return ret;
}



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
	SYSTEMTIME current_time;


	BirthdaysClass birthdays_list;
	/**
	* 
	*/
	UINT8 birthday_type_print;


	//��������� �������� ����������� �������
	bool args_parse_result;

	/**
	* ������� �������
	* --help -h /? -?
	*/
	bool is_help;
	/**
	* ������� ��������� ����� ����������� ���� ����������� ����� ��������� ���������� ���
	* ���� < 0 ��������� �� ���������
	* /c[ ���������� �������� � ���]
	* -c[ ���������� �������� � ���]
	* --close[ ���������� �������� � ���]
	*/
	int close_after_sec;

public:
	/**
	* ������� ����� ���������, ������� � ������� � ������� ������� App::start
	* @param hInstance - 
	* @param comand_args - ��������� �� ������ � �����������
	*/
	App(HINSTANCE hInstance, LPTSTR comand_args)
		: icon_struct({0}),
		main_win_c({0}),
		window(nullptr),
		hInstance(hInstance),
		context_menu(nullptr),
		current_time({0}),

		birthday_type_print(0),

		is_help(false),
		close_after_sec(-1)
	{
		args_parse_result = this->parse_comand_args(comand_args);
		if(!args_parse_result || is_help)
			return;
		GetLocalTime(&current_time);
		window = create_window();
		this->init_icon_struct();
		this->init_context_menu();
	}

	/**
	* ������� ������ �� ����
	* @see App::icon_struct
	*/
	void hide_icon(){
		Shell_NotifyIcon(NIM_DELETE, &icon_struct);
	}

	/**
	* ��������� ��������� �������
	* ��� ������� ����������� �� ������ �������� ���������� (args_parse_result == false)
	* @see App::args_parse_result
	* ��� ������� ������� (is_help == true)
	* @see App::is_help
	* @return ��� ���������� ������ ����������
	*/
	int start(){
		if(!args_parse_result){
			MessageBox(NULL, TEXT("������ ������������ ����������"), TEXT("������"), MB_OK | MB_ICONERROR | MB_TASKMODAL);
			return 1;
		}
		if(is_help){
			MessageBox(
				NULL,
				TEXT("Description\n\n")//todo
				TEXT("app.exe   [/c[ �������]]   [/?]\n\n")
				TEXT("  /c (-c, --close)\t���������� �������� ��������� ����� ��������� ���������� ������(�����) (�� ��������� ")
				TEXT(STR(DEFAULT_CLOSE_AFTER_SEC))
				TEXT(" ���) ����� ����������� ���� �����������, ���� �� �������� �������\n")
				TEXT("  /? (-?, -h, --help)\t����� �������\n"),
				TEXT("�������"),
				MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
			);
			return 0;
		}

		HANDLE h_birthdays_file = open_birthdays_file();
		if(h_birthdays_file != NULL && h_birthdays_file != INVALID_HANDLE_VALUE){
			//FILETIME last_write_time = {0};
			//GetFileTime(h_birthdays_file, NULL, NULL, &last_write_time);
			//todo ��������� � ��������� ����� ��������� ��� ��������� ��������
			//���� �� ����� ��������� ��� ���� ��� ��� ����� ������� �����?
			birthdays_list.parse_data_from_file(h_birthdays_file);

			CloseHandle(h_birthdays_file);
		}

		if(birthdays_list.get_count_birthdays(current_time.wMonth)){
			birthday_type_print = 1;
			SetTimer(window, BALLON_TYPES_INTERVAL_TIMER, BALLON_TYPES_INTERVAL_SEC*1000, NULL);
		}else{
			if(close_after_sec >= 0){
				SetTimer(window, CLOSE_TIMER, close_after_sec*1000, NULL);
			}
		}

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

	/**
	* ���������� ����������� ���� context_menu � ��������� �����������
	*/
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

	/**
	* ������������ ������ �������� ������������ ����
	* @param menu_item_ind - ������������� �������� ����
	* CM_EXIT - ����� �� ����������
	* 
	*/
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

	void show_birthdays_ballons(){
		//���� �������� � ���� ������
		if(birthday_type_print == 1){
			BirthdayList b_list = birthdays_list.get_birthdays(current_time.wMonth, 0);
			lstrcpyn(icon_struct.szInfoTitle, TEXT("� ���� ������ ���� �������� ���������:"), szInfoTitle_LENGTH);
			String out;
			String birthday_greeting;
			UINT32 len = b_list.size();
			for(UINT32 i=0; i<len; i++){
				birthday_greeting = b_list.get_greeting_with_day_age(i, current_time);
				if(birthday_greeting.size() >= szInfo_LENGTH-1){
					continue;
				}
				if(birthday_greeting.size() + out.size() >= szInfo_LENGTH-2){
					break;
				}
				if(out.size()){
					out += TEXT("\n");
				}
				out += birthday_greeting;
			}

			lstrcpyn(icon_struct.szInfo, out.c_str(), szInfo_LENGTH);
			Shell_NotifyIcon(NIM_MODIFY, &icon_struct);
		}

		KillTimer(window, BALLON_TYPES_INTERVAL_TIMER);
	}

	/**
	* ������� ������
	*/
	~App(){
		if(!args_parse_result || is_help)
			return;
		//������� ������
		DestroyIcon(icon_struct.hIcon);
		DestroyIcon(icon_struct.hBalloonIcon);
		DestroyMenu(context_menu);
	}

private:
	/**
	* ������������ ����� ���� main_win_c
	* @see App::main_win_c
	* ������� ����
	* @return ���������� ���������� ����
	*/
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

	/**
	* ���������� ������� ���������� ��������� ������
	* @param comand_args - ��������� �� ������ � �����������
	* @see App::is_help
	* @see App::close_after_sec
	* @return ������� �� ���������� ���������
	*/
	bool parse_comand_args(LPTSTR comand_args){
		#ifdef UNICODE
			LPWSTR *arg_list;
			int arg_c;

			arg_list = CommandLineToArgvW(comand_args, &arg_c);
			if(arg_list == NULL){
			  return false;
			}
			for(int i=0; i<arg_c; i++){
				if(
					lstrcmp(arg_list[i], TEXT("-h")) == 0 ||
					lstrcmp(arg_list[i], TEXT("-?")) == 0 ||
					lstrcmp(arg_list[i], TEXT("/?")) == 0 ||
					lstrcmp(arg_list[i], TEXT("--help")) == 0
				){
					is_help = true;
					continue;
				}

				if(
					lstrcmp(arg_list[i], TEXT("-c")) == 0 ||
					lstrcmp(arg_list[i], TEXT("/c")) == 0 ||
					lstrcmp(arg_list[i], TEXT("--close")) == 0
				){
					if(i+1 == arg_c){
						close_after_sec = DEFAULT_CLOSE_AFTER_SEC;
						continue;
					}
					if(StrToIntEx(arg_list[i+1], STIF_DEFAULT, &close_after_sec)){
						i++;
					}else{
						close_after_sec = DEFAULT_CLOSE_AFTER_SEC;
					}
				}
			}

			LocalFree(arg_list);
			return true;
		#else
			#error "parse_comand_args ANSI undefined"
			return false;
		#endif //UNICODE
	}

	/**
	* ���������� ������������� ��������� icon_struct
	* @see App::icon_struct
	*/
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

		/*
		//������ � ������� �������� � �����. ���� 128 �������� ������� 0
		//StringCchCopy(icon_struct.szTip, ARRAYSIZE(icon_struct.szTip), TEXT("Test application"));
		lstrcpyn(icon_struct.szTip, TEXT("Test application"), szTip);
	
	
		lstrcpyn(icon_struct.szInfoTitle, TEXT("Title"), szInfoTitle_LENGTH);
		lstrcpyn(icon_struct.szInfo, TEXT("text"), szInfo_LENGTH);
		*/

		icon_struct.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
		icon_struct.hBalloonIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(103), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, LR_DEFAULTCOLOR);
	}
	
	/**
	* ���������� ������������� ������������ ���� context_menu
	* @see App::context_menu
	*/
	void init_context_menu(){
		context_menu = CreatePopupMenu();
		AppendMenu(context_menu, MFT_STRING | MF_CHECKED, CM_EXEC, TEXT("��������"));
		AppendMenu(context_menu, MFT_STRING, CM_EXIT, TEXT("&�����"));
	}

	/**
	* ��������� ���� �� ������������ ���� (%USERPROFILE%\Documents\Happy Birthday\birthdays.csv)
	* ��� �� ����������� ����
	* @param user_path - ��������� �� ������ ���� � �����
	* @return NULL ��� �� ��� ���������� CreateFile
	* INVALID_HANDLE_VALUE
	*/
	HANDLE open_birthdays_file(LPCTSTR user_path = NULL){

		if(user_path){
			return CreateFile(user_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		}


		HKEY hKey = nullptr;
		HANDLE h_process_heap = GetProcessHeap();

		//������� ���� � ����� ��� ��������� ��� �������� ����� �� �������
		if(RegOpenKeyEx(
			HKEY_CURRENT_USER,
			TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"),
			0,
			KEY_QUERY_VALUE,
			&hKey
		) != ERROR_SUCCESS){
			return NULL;
		}

		//������� ������ ������ � �������
		DWORD type, data_size;
		if(RegQueryValueEx(hKey, TEXT("Personal"), NULL, &type, NULL, &data_size) != ERROR_SUCCESS){
			RegCloseKey(hKey);
			return NULL;
		}

		if(type != REG_EXPAND_SZ && type != REG_SZ || data_size == 0){
			RegCloseKey(hKey);
			return NULL;
		}

		LPTSTR path = (LPTSTR)HeapAlloc(h_process_heap, 0, data_size + sizeof(TEXT("\\Happy Birthday\\birthdays.csv")));
		if(path == NULL){
			RegCloseKey(hKey);
			return NULL;
		}

		//������� ������ �� �������
		if(RegQueryValueEx(hKey, TEXT("Personal"), NULL, &type, (LPBYTE)path, &data_size) != ERROR_SUCCESS){
			HeapFree(h_process_heap, 0, path);
			RegCloseKey(hKey);
			return NULL;
		}
		
		RegCloseKey(hKey);

		//������� ���� ����
		if(lstrcat(path, TEXT("\\Happy Birthday\\birthdays.csv")) == NULL){
			HeapFree(h_process_heap, 0, path);
			return NULL;
		}

		//������� ��������� ����������
		data_size = ExpandEnvironmentStrings(path, NULL, 0);
		if(data_size == 0){
			HeapFree(h_process_heap, 0, path);
			return NULL;
		}

		LPTSTR full_path = (LPTSTR)HeapAlloc(h_process_heap, 0, data_size * sizeof(TCHAR));
		if(full_path == NULL){
			HeapFree(h_process_heap, 0, path);
			return NULL;
		}

		data_size = ExpandEnvironmentStrings(path, full_path, data_size);
		if(data_size == 0){
			HeapFree(h_process_heap, 0, path);
			HeapFree(h_process_heap, 0, full_path);
			return NULL;
		}

		//������� ���� �� ������
		HANDLE ret = CreateFile(full_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		HeapFree(h_process_heap, 0, path);
		HeapFree(h_process_heap, 0, full_path);

		return ret;
	}
};

App* application = nullptr;


/**
* ������������ ������� ����������
* @see App::create_window
* @param window - 
* @param message - 
* WM_SHELLNOTIFY - ��������� �� ������ (App::show_context_menu)
* WM_COMMAND - ������� ������ ������ ���� (App::context_menu_click)
* WM_DESTROY - �������� ������(App::hide_icon), ����� �� ���������
*/
LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
		//��������� �� ������
		case WM_SHELLNOTIFY:
			if(LOWORD(lParam) == WM_CONTEXTMENU){
				KillTimer(window, CLOSE_TIMER);
				application->show_context_menu(GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
			}
			break;

		//������� ������ ������ ����
		case WM_COMMAND:
			application->context_menu_click(LOWORD(wParam));
			break;
		
		//������� ��������
		case WM_TIMER:
			switch(wParam){
				case CLOSE_TIMER:
					KillTimer(window, CLOSE_TIMER);
					DestroyWindow(window);
					break;
 
				case BALLON_TYPES_INTERVAL_TIMER:
					application->show_birthdays_ballons();
					break; 
			}
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



/**
* ����� �����
*/
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd){

	App program(hInstance, lpCmdLine);

	application = &program;

	return program.start();
}