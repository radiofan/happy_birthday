#include "stdafx.h"

const TCHAR* week_day[7] = {TEXT("ВС"), TEXT("ПН"), TEXT("ВТ"), TEXT("СР"), TEXT("ЧТ"), TEXT("ПТ"), TEXT("СБ")};

BirthdayList::BirthdayList(){

}

/**
* склонение слов после числа
* 
* @param number - число
* @param title_1 - вариант для склонения для 1
* @param title_2 - вариант для склонения для 2
* @param title_10 - вариант для склонения для 10
* 
* @returns строки типа "1 книга", "2 книги", "10 книг"
*/
String BirthdayList::num_decline(INT32 number, const String& title_1, const String& title_2, const String& title_10){
	UINT8 cases[6] = {2, 0, 1, 1, 1, 2};
	UINT32 intnum = abs(number);

	UINT8 title_index = (intnum % 100 > 4 && intnum % 100 < 20) ? 2 : cases[min(intnum % 10, 5)];
	switch(title_index){
		case 0:
			return std::toString(number) + TEXT(" ") + title_1;
		case 1:
			return std::toString(number) + TEXT(" ") + title_2;
		case 2:
			return std::toString(number) + TEXT(" ") + title_10;
	}
	return String();
}

UINT32 BirthdayList::size() const{
	return dates.size();
}

/**
* Добавляет день рождения,
* 
*/
bool BirthdayList::add_birthday(SYSTEMTIME date, const String& label){

	UINT32 size = dates.size();
	UINT32 insert_pos = 0;

	for(; insert_pos<size; insert_pos++){
		if(date.wMonth <= dates[insert_pos].wMonth && date.wDay <= dates[insert_pos].wDay)
			break;
	}

	dates.insert(dates.begin()+insert_pos, date);
	labels.insert(labels.begin()+insert_pos, label);

	return true;
}

BirthdayList BirthdayList::get_birthdays(UINT8 day) const{
	if(day == 0)
		return (*this);

	BirthdayList ret;
	UINT32 len=dates.size();
	for(UINT32 i=0; i<len; i++){
		if(dates[i].wDay > day){
			break;
		}else if(dates[i].wDay == day){
			ret.add_birthday(dates[i], labels[i]);
		}
	}
	return ret;
}

const SYSTEMTIME& BirthdayList::get_date(UINT32 ind) const{
	return dates[ind];
}

const String& BirthdayList::get_label(UINT32 ind) const{
	return labels[ind];
}

String BirthdayList::get_greeting_with_day_age(UINT32 ind, const SYSTEMTIME& current_time) const{
	if(ind >= dates.size()){
		return String();
	}

	String ret(labels[ind] + TEXT(" "));

	//получим день недели день рождения в текущем году
	FILETIME tmp = {0};
	SYSTEMTIME date = dates[ind];
	date.wYear = current_time.wYear;
	SystemTimeToFileTime(&date, &tmp);
	FileTimeToSystemTime(&tmp, &date);

	ret += std::toString(dates[ind].wDay) + TEXT(" (") + week_day[date.wDayOfWeek] + TEXT(") ")
		+  BirthdayList::num_decline(current_time.wYear - dates[ind].wYear, String(TEXT("год")), String(TEXT("года")), String(TEXT("лет")));
	return ret;
}

BirthdayList::~BirthdayList(){
	/*
	UINT32 size = labels.size();
	HANDLE h_process_heap = GetProcessHeap();
	for(UINT32 i=0; i<size; i++){
		HeapFree(h_process_heap, 0, (LPVOID)labels[i]);
	}
	*/
}