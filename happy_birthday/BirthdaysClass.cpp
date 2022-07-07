#include "stdafx.h"

BirthdaysClass::BirthdaysClass(){

}

/**
* Добавляет день рождения, распределяет их по месяцам
* 
*/
bool BirthdaysClass::add_birthday(SYSTEMTIME date, const String& label){
	if(date.wMonth > 12)
		return false;
	return birthday_list[date.wMonth-1].add_birthday(date, label);
}

/**
* @param month_number - номер месяца, количество дней рождений в котором будет возвращено; 0 - возвращает количество всех дней рождений
*/
UINT32 BirthdaysClass::get_count_birthdays(UINT8 month_number){
	if(month_number){
		if(month_number > 12)
			return 0;
		return birthday_list[month_number-1].size();
	}

	UINT32 count=0;
	for(UINT8 i=0; i<12; i++){
		count += birthday_list[i].size();
	}
	return count;
}

/**
* производит разбор файла, извлекает построчно данные, строка имеет формат
* бирка\tдата\n
* @param input - дескриптор открытого файла с правами на чтение
* @returns количество добавленных дней рождений
*/
int BirthdaysClass::parse_data_from_file(HANDLE input){
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

BirthdayList BirthdaysClass::get_birthdays(UINT8 month, UINT8 day){
	BirthdayList ret;
	if(month > 12 || month == 0)
		return ret;
	return birthday_list[month-1].get_birthdays(day);
}

BirthdaysClass::~BirthdaysClass(){

}
/**
* @param str - строка в которой ищутся символы конца
* @param end_char_pos - сюда поместится индекс конечного символа (если такой не будет найден будет иметь значение размера строки)
* @returns \0, \r, \n, DELIMITER_CHAR
* @see DELIMITER_CHAR
*/
char BirthdaysClass::get_end_char_pos(std::string& str, DWORD& end_char_pos){
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
DWORD BirthdaysClass::get_not_end_char_pos(std::string& str, DWORD start_pos){
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
* @param date - структура которая будет заполнена датой, если парсинг будет удачным
* @returns успешность парсинга
*/
bool BirthdaysClass::str_to_systemtime(std::string str, SYSTEMTIME& date){
	if(str.size() != 10)
		return false;

	const char* str_p = str.c_str();
	int year, month, day;
	UINT8 year_shift, month_shift, day_shift;

	if(isdigit(str[2])){
		//yyyy.mm.dd
		str[4] = '\0';
		str[7] = '\0';
		year_shift = 0;
		month_shift = 5;
		day_shift = 8;
	}else{
		//dd.mm.yyyy
		str[2] = '\0';
		str[5] = '\0';
		year_shift = 6;
		month_shift = 3;
		day_shift = 0;
	}

	if(!StrToIntExA(str_p+year_shift, STIF_DEFAULT, &year)){
		return false;
	}
	if(!StrToIntExA(str_p+month_shift, STIF_DEFAULT, &month)){
		return false;
	}
	if(!StrToIntExA(str_p+day_shift, STIF_DEFAULT, &day)){
		return false;
	}
		
	date = {0};

	date.wYear = year;
	date.wMonth = month;
	date.wDay = day;
}
