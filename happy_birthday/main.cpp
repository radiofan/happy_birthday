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
	* Добавляет день рождения,
	* 
	*/
	bool add_birthday(SYSTEMTIME date, const String &label){

		//обнулим часы, они нам не нужны
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
	* Добавляет день рождения, строка label будет очищена с помощью HeapFree если день будет добавлен
	* 
	*/
	bool add_birthday(SYSTEMTIME date, const String& label){
		return birthday_list[date.wMonth-1].add_birthday(date, label);
	}


	int parse_data_from_file(HANDLE input){


		//дерьмо, надо что то с этим делать

		constexpr DWORD buffer_len = 1024;
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
				//конечный символ не найден, продолжаем получать данные из файла
				continue;
			}
			if(find_date && (tmp == '\r' || tmp == '\n')){
				//найден конец даты
				find_date = false;
				label = "";
				if(this->str_to_systemtime(string_buffer.substr(0, end_char_pos), date)){
					this->add_birthday(date, str_to_tstr(label));
					date_finded++;
				}
				string_buffer = string_buffer.substr(end_char_pos+1);
			}else if(!find_date && tmp == DELIMITER_CHAR){
				//найден конец бирки
				label = string_buffer.substr(0, end_char_pos);
				find_date = true;
				string_buffer = string_buffer.substr(end_char_pos+1);
			}else{
				//был найден мусор - чистим его
				noend_char_pos = get_not_end_char_pos(string_buffer, end_char_pos+1);
				if(noend_char_pos == string_buffer.size()){
					string_buffer = "";
					continue;
				}
				string_buffer = string_buffer.substr(noend_char_pos);
			}
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
	* @param str - строка в которой ищутся символы конца
	* @param end_char_pos - сюда поместится индекс конечного символа (если такой не будет найден будет иметь значение размера строки)
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
	* @param str - строка в которой ищутся символы конца
	* @param start_pos - позиция символа с которого начинается поиск неконечного символа
	* @returns позиция первого неконечного символа или длину строки если смвол не найден
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
	* @param str - строка с датой формата yyyy.mm.dd или dd.mm.yyyy, точка - любой нецифровой символ
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


	//параметры задаются параметрами запуска
	bool args_parse_result;

	/**
	* вывести справку
	* --help -h /? -?
	*/
	bool is_help;
	/**
	* закрыть программу после отображения всех уведомлений через указанное количество сек
	* если < 0 программа не закроется
	* /c[ количество ожидания в сек]
	* -c[ количество ожидания в сек]
	* --close[ количество ожидания в сек]
	*/
	int close_after_sec;

public:
	/**
	* главный класс программы, создать и вернуть в главной функции App::start
	* @param hInstance - 
	* @param comand_args - указатель на строку с аргументами
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
	* Удаляет значок из трея
	* @see App::icon_struct
	*/
	void hide_icon(){
		Shell_NotifyIcon(NIM_DELETE, &icon_struct);
	}

	/**
	* запускает обработку событий
	* или выводит уведомление об ошибке парсинга параметров (args_parse_result == false)
	* @see App::args_parse_result
	* или выводит справку (is_help == true)
	* @see App::is_help
	* @return код завершения работы приложения
	*/
	int start(){
		if(!args_parse_result){
			MessageBox(NULL, TEXT("Ошибка распознвания параметров"), TEXT("Ошибка"), MB_OK | MB_ICONERROR | MB_TASKMODAL);
			return 1;
		}
		if(is_help){
			MessageBox(
				NULL,
				TEXT("Description\n\n")//todo
				TEXT("app.exe   [/c[ секунды]]   [/?]\n\n")
				TEXT("  /c (-c, --close)\tПроизведет закрытие программы через указанное количество секунд(целое) (по умолчанию ")
				TEXT(STR(DEFAULT_CLOSE_AFTER_SEC))
				TEXT(" сек) после отображения всех уведомлений, если не получает событий\n")
				TEXT("  /? (-?, -h, --help)\tВывод справки\n"),
				TEXT("Справка"),
				MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
			);
			return 0;
		}

		HANDLE h_birthdays_file = open_birthdays_file();
		if(h_birthdays_file != NULL && h_birthdays_file != INVALID_HANDLE_VALUE){
			//FILETIME last_write_time = {0};
			//GetFileTime(h_birthdays_file, NULL, NULL, &last_write_time);
			//todo сравнение с последней датой изменения при последнем парсинге
			//есть ли смысл сохранять кэш если его все равно парсить нужно?
			birthdays_list.parse_data_from_file(h_birthdays_file);
			

			CloseHandle(h_birthdays_file);
		}

		Shell_NotifyIcon(NIM_ADD, &icon_struct);
		Shell_NotifyIcon(NIM_SETVERSION, &icon_struct);

		// Цикл обработки сообщений
		MSG message;
		while(GetMessage(&message, NULL, 0, 0)){
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		return 0;
	}

	/**
	* отображает контекстное меню context_menu в указанных координатах
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
	* Обрабатывает выбора элемента контекстного меню
	* @param menu_item_ind - идентификатор элемента меню
	* CM_EXIT - выход из приложения
	* 
	*/
	void context_menu_click(INT16 menu_item_ind){
		switch(menu_item_ind){
			case CM_EXIT:
				DestroyWindow(window);
				break;
			case CM_EXEC:
				ModifyMenu(context_menu, CM_EXEC, MF_BYCOMMAND | MF_UNCHECKED | MFT_STRING, NULL, TEXT("Действие"));
				break;
		}
	}

	/**
	* Очищает память
	*/
	~App(){
		if(!args_parse_result || is_help)
			return;
		//очистка памяти
		DestroyIcon(icon_struct.hIcon);
		DestroyIcon(icon_struct.hBalloonIcon);
		DestroyMenu(context_menu);
	}

private:
	/**
	* регистрирует класс окна main_win_c
	* @see App::main_win_c
	* создает окно
	* @return дескриптор созданного окна
	*/
	HWND create_window(){
		//Регистрация класса окна
		main_win_c.cbSize = sizeof(WNDCLASSEX);
		main_win_c.hInstance = hInstance;
		main_win_c.lpszClassName = TEXT("Main");
		main_win_c.lpfnWndProc = WndProc;
		RegisterClassEx(&main_win_c);

		// Создание главного окна
		return CreateWindowEx(0, TEXT("Main"), NULL, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	}

	/**
	* производит парсинг аргументов командной строки
	* @param comand_args - указатель на строку с аргументами
	* @see App::is_help
	* @see App::close_after_sec
	* @return удалось ли распарсить аргументы
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
	* производит инициализацию структуры icon_struct
	* @see App::icon_struct
	*/
	void init_icon_struct(){
		// Создание значка
		icon_struct.cbSize = sizeof(NOTIFYICONDATA);
		icon_struct.hWnd = window;
		icon_struct.uVersion = NOTIFYICON_VERSION_4;
		icon_struct.uID = 1;

		//используем UUID (только для win7)
		//Обрабатываем события
		//выводим иконку в трей
		//выводим подсказку по наведению
		//стандартный вывод подсказки
		//вывод баллона
		icon_struct.uFlags = NIF_GUID | NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_INFO;

		icon_struct.guidItem = appGUID;

		//если uVersion = 0 или NOTIFYICON_VERSION (не наш случай)
		//Параметр wParam будет определен идентификатором значка, который генерирует сообщение.
		//Параметр lParam "реальный" идентификатор сообщения Windows

		//наш случай
		//LOWORD(lParam) содержит события уведомлений, такие как NIN_BALLOONSHOW, NIN_POPUPOPEN или WM_CONTEXTMENU.
		//HIWORD(lParam) содержит идентификатор значка. Длина идентификаторов значков ограничена 16 битами.
		//GET_X_LPARAM(wParam) возвращает координату привязки X для событий уведомлений NIN_POPUPOPEN, NIN_SELECT, NIN_KEYSELECT и всех сообщений мыши между WM_MOUSEFIRST и WM_MOUSELAST. Если какое-либо из этих сообщений генерируется клавиатурой, параметр wParam устанавливается в верхнем левом углу целевого значка. Для всех остальных сообщений параметр wParam не определен.
		//GET_Y_LPARAM(wParam) возвращает координату привязки Y для событий уведомлений и сообщений, как определено для привязки X.
		icon_struct.uCallbackMessage = WM_SHELLNOTIFY;

		//LoadIconMetric(hInstance, MAKEINTRESOURCE(102), LIM_SMALL, &(icon_struct.hIcon));
		icon_struct.hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(103), IMAGE_ICON, SM_CXICON, SM_CYICON, LR_DEFAULTCOLOR);
		//icon_struct.hIcon = LoadIcon(NULL, IDI_SHIELD);

		//Строка с нулевым символом в конце. макс 128 символов включая 0
		//StringCchCopy(icon_struct.szTip, ARRAYSIZE(icon_struct.szTip), TEXT("Test application"));
		lstrcpyn(icon_struct.szTip, TEXT("Test application"), sizeof(icon_struct.szTip)/sizeof(icon_struct.szTip[0]));
	
	
		lstrcpyn(icon_struct.szInfoTitle, TEXT("Title"), sizeof(icon_struct.szInfoTitle)/sizeof(icon_struct.szInfoTitle[0]));
		lstrcpyn(icon_struct.szInfo, TEXT("text"), sizeof(icon_struct.szInfo)/sizeof(icon_struct.szInfo[0]));

		icon_struct.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
		icon_struct.hBalloonIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(103), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, LR_DEFAULTCOLOR);
	}
	
	/**
	* производит инициализацию контекстного меню context_menu
	* @see App::context_menu
	*/
	void init_context_menu(){
		context_menu = CreatePopupMenu();
		AppendMenu(context_menu, MFT_STRING | MF_CHECKED, CM_EXEC, TEXT("Действие"));
		AppendMenu(context_menu, MFT_STRING, CM_EXIT, TEXT("&Выход"));
	}

	/**
	* открывает файл по стандартному пути (%USERPROFILE%\Documents\Happy Birthday\birthdays.csv)
	* или по переданному пути
	* @param user_path - указатель на строку путь к файлу
	* @return NULL или то что возвращает CreateFile
	* INVALID_HANDLE_VALUE
	*/
	HANDLE open_birthdays_file(LPCTSTR user_path = NULL){

		if(user_path){
			return CreateFile(user_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		}


		HKEY hKey = nullptr;
		HANDLE h_process_heap = GetProcessHeap();

		//получим путь к папке мои документы для текущего юзера из реестра
		if(RegOpenKeyEx(
			HKEY_CURRENT_USER,
			TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"),
			0,
			KEY_QUERY_VALUE,
			&hKey
		) != ERROR_SUCCESS){
			return NULL;
		}

		//получим размер строки в реестре
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

		//вытащим строку из реестра
		if(RegQueryValueEx(hKey, TEXT("Personal"), NULL, &type, (LPBYTE)path, &data_size) != ERROR_SUCCESS){
			HeapFree(h_process_heap, 0, path);
			RegCloseKey(hKey);
			return NULL;
		}
		
		RegCloseKey(hKey);

		//добавим свой путь
		if(lstrcat(path, TEXT("\\Happy Birthday\\birthdays.csv")) == NULL){
			HeapFree(h_process_heap, 0, path);
			return NULL;
		}

		//заменим системные переменные
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

		//откроем файл на чтение
		HANDLE ret = CreateFile(full_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		HeapFree(h_process_heap, 0, path);
		HeapFree(h_process_heap, 0, full_path);

		return ret;
	}
};

App* application = nullptr;


/**
* обрабатывает события приложения
* @see App::create_window
* @param window - 
* @param message - 
* WM_SHELLNOTIFY - Сообщение от значка (App::show_context_menu)
* WM_COMMAND - событие выбора пункта меню (App::context_menu_click)
* WM_DESTROY - скрывает значок(App::hide_icon), выход из программы
*/
LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
		//Сообщение от значка
		case WM_SHELLNOTIFY:
			if(LOWORD(lParam) == WM_CONTEXTMENU){
				application->show_context_menu(GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
			}
			break;

		//событие выбора пункта меню
		case WM_COMMAND:
			application->context_menu_click(LOWORD(wParam));
			break;
			
		//выход из программы
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
* точка входа
*/
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd){

	App program(hInstance, lpCmdLine);

	application = &program;

	return program.start();
}