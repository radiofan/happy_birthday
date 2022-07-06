#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "Shlwapi.lib")
#include <shlwapi.h>
//#include <strsafe.h>
//#include <commctrl.h>
#include <vector>
#include <string>
#include <tchar.h>
#ifdef _UNICODE
	typedef std::wstring String;
	#define str_to_tstr(X) String(X.begin(), X.end())

#else
	typedef std::string String;
	#define str_to_tstr(X) X
#endif

#define WM_SHELLNOTIFY (WM_APP + 1)
#define CM_EXIT 15
#define CM_EXEC 14
#define DEFAULT_CLOSE_AFTER_SEC 5
#define STR_(X) #X
#define STR(X) STR_(X)

#define nano100SecInWeek (__int64)10000000*60*60*24*7
#define nano100SecInDay  (__int64)10000000*60*60*24
#define nano100SecInHour (__int64)10000000*60*60
#define nano100SecInMin  (__int64)10000000*60
#define nano100SecInSec  (__int64)10000000

#define DELIMITER_CHAR '\t'

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);


class BirthdayList{
private:
	std::vector<String> labels;
	std::vector<SYSTEMTIME> dates;
public:
	BirthdayList(){

	}

	UINT32 size() const{
		return dates.size();
	}

	/**
	* ��������� ���� ��������,
	* 
	*/
	bool add_birthday(SYSTEMTIME date, const String &label){

		//������� ����, ��� ��� �� �����
		date.wHour = 0;
		date.wMinute = 0;
		date.wMilliseconds = 0;

		UINT32 size = dates.size();
		UINT32 insert_pos = 0;

		for(; insert_pos<size; insert_pos++){
			if(date.wMonth <= dates[insert_pos].wMonth && date.wDay <= dates[insert_pos].wDay)
				break;
		}

		dates.insert(dates.begin(), date);
		labels.insert(labels.begin(), label);

		return true;
	}

	~BirthdayList(){
		/*
		UINT32 size = labels.size();
		HANDLE h_process_heap = GetProcessHeap();
		for(UINT32 i=0; i<size; i++){
			HeapFree(h_process_heap, 0, (LPVOID)labels[i]);
		}
		*/
	}
	
};

class BirthdaysClass{
private:
	BirthdayList birthday_list[12];
public:
	BirthdaysClass(){

	}

	/**
	* ��������� ���� ��������, ������ label ����� ������� � ������� HeapFree ���� ���� ����� ��������
	* 
	*/
	bool add_birthday(SYSTEMTIME date, const String& label){
		return birthday_list[date.wMonth-1].add_birthday(date, label);
	}


	int parse_data_from_file(HANDLE input){


		//������, ���� ��� �� � ���� ������

		constexpr DWORD buffer_len = 10;
		CHAR buffer[buffer_len];
		DWORD data_len;

		//DWORD string_start=0, string_end=0;
		//bool is_quot = false;
		char tmp;
		std::string string_buffer = "", label;
		SYSTEMTIME date = {0};
		DWORD end_char_pos, noend_char_pos, date_finded=0;
		bool find_date = false;


		while(ReadFile(input, buffer, buffer_len-1, &data_len, NULL) && data_len != 0){
			buffer[data_len] = '\0';
			string_buffer += buffer;
		PARSE_DATA_STR:
			tmp = get_end_char_pos(string_buffer, end_char_pos);
			
			if(end_char_pos == string_buffer.size()){
				//�������� ������ �� ������, ���������� �������� ������ �� �����
				continue;
			}
			if(find_date){
				//��������� ����
				if(tmp == '\r' || tmp == '\n'){
					//������ ����� ����
					find_date = false;
					label = "";
					if(this->str_to_systemtime(string_buffer.substr(0, end_char_pos), date)){
						this->add_birthday(date, str_to_tstr(label));
						date_finded++;
					}
					goto CLEAR_BUFFER_TO_FIRST_NOEND_CHAR;
				}else{
					//������� ������ �� ��� ������, ��� ������ ������ �������� ������
					//������� ��� ��� ��������� �� ���������� �����������
					goto CLEAR_BUFFER_TO_FIRST_NOEND_CHAR;
				}
			}else{
				//��������� �����
				if(tmp == DELIMITER_CHAR){
					//������ ����� �����
					label = string_buffer.substr(0, end_char_pos);
					find_date = true;
					string_buffer = string_buffer.substr(end_char_pos+1);
					goto PARSE_DATA_STR;
				}else{
					//����� ����� �� ��� ������
					//������� ������� ������
					//� ���� ������� �������� ������
					goto CLEAR_BUFFER_TO_FIRST_NOEND_CHAR;
				}
			}

		//���������� ������� ������� �� ������� ����������� �������
		CLEAR_BUFFER_TO_FIRST_NOEND_CHAR:
			noend_char_pos = get_not_end_char_pos(string_buffer, end_char_pos+1);
			if(noend_char_pos == string_buffer.size()){
				string_buffer = "";
				continue;
			}
			string_buffer = string_buffer.substr(noend_char_pos);
			goto PARSE_DATA_STR;

		}

		if(find_date){
			if(this->str_to_systemtime(string_buffer, date)){
				this->add_birthday(date, str_to_tstr(label));
			}
		}

		return date_finded;
	}

	~BirthdaysClass(){

	}
private:
	/**
	* @param str - ������ � ������� ������ ������� �����
	* @param end_char_pos - ���� ���������� ������ ��������� ������� (���� ����� �� ����� ������ ����� ����� �������� ������� ������)
	* @returns \0, \r, \n, DELIMITER_CHAR
	* @see DELIMITER_CHAR
	*/
	char get_end_char_pos(std::string& str, DWORD& end_char_pos){
		DWORD len = str.size();
		for(DWORD i=0; i<len; i++){
			switch(str[i]){
				case DELIMITER_CHAR:
				case '\r':
				case '\n':
				case '\0':
					end_char_pos = i;
					return str[i];
			}
		}
		end_char_pos = len;
		return '\0';
	}

	/**
	* @param str - ������ � ������� ������ ������� �����
	* @param start_pos - ������� ������� � �������� ���������� ����� ����������� �������
	* @returns ������� ������� ����������� ������� ��� ����� ������ ���� ����� �� ������
	*/
	DWORD get_not_end_char_pos(std::string& str, DWORD start_pos){
		DWORD len = str.size();
		for(; start_pos<len; start_pos++){
			switch(str[start_pos]){
				case DELIMITER_CHAR:
				case '\r':
				case '\n':
				case '\0':
					continue;
				default:
					return start_pos;
			}
		}
		return len;
	}

	/**
	* @param str - ������ � ����� ������� yyyy.mm.dd ��� dd.mm.yyyy, ����� - ����� ���������� ������
	*/
	bool str_to_systemtime(std::string str, SYSTEMTIME& date){
		if(str.size() != 10)
			return false;

		const char* str_p = str.c_str();
		int year ,month, day;

		if(isdigit(str[2])){
			//yyyy.mm.dd
			str[4] = '\0';
			str[7] = '\0';
			if(!StrToIntExA(str_p, STIF_DEFAULT, &year)){
				return false;
			}
			if(!StrToIntExA(str_p+5, STIF_DEFAULT, &month)){
				return false;
			}
			if(!StrToIntExA(str_p+8, STIF_DEFAULT, &day)){
				return false;
			}
		}else{
			//dd.mm.yyyy
			str[2] = '\0';
			str[5] = '\0';
			if(!StrToIntExA(str_p, STIF_DEFAULT, &day)){
				return false;
			}
			if(!StrToIntExA(str_p+3, STIF_DEFAULT, &month)){
				return false;
			}
			if(!StrToIntExA(str_p+6, STIF_DEFAULT, &year)){
				return false;
			}
		}
		
		date = {0};

		date.wYear = year;
		date.wMonth = month;
		date.wDay = day;
	}
};


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


	BirthdaysClass birthdays_list;


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

		is_help(false),
		close_after_sec(-1)
	{
		args_parse_result = this->parse_comand_args(comand_args);
		if(!args_parse_result || is_help)
			return;
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

		//������ � ������� �������� � �����. ���� 128 �������� ������� 0
		//StringCchCopy(icon_struct.szTip, ARRAYSIZE(icon_struct.szTip), TEXT("Test application"));
		lstrcpyn(icon_struct.szTip, TEXT("Test application"), sizeof(icon_struct.szTip)/sizeof(icon_struct.szTip[0]));
	
	
		lstrcpyn(icon_struct.szInfoTitle, TEXT("Title"), sizeof(icon_struct.szInfoTitle)/sizeof(icon_struct.szInfoTitle[0]));
		lstrcpyn(icon_struct.szInfo, TEXT("text"), sizeof(icon_struct.szInfo)/sizeof(icon_struct.szInfo[0]));

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



/**
* ����� �����
*/
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd){

	App program(hInstance, lpCmdLine);

	application = &program;

	return program.start();
}