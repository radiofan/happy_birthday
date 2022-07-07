#include "stdafx.h"

BirthdayList::BirthdayList(){

}

UINT32 BirthdayList::size() const{
	return dates.size();
}

/**
* Добавляет день рождения,
* 
*/
bool BirthdayList::add_birthday(SYSTEMTIME date, const String &label){

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

BirthdayList BirthdayList::get_birthdays(UINT8 day) const{
	if(day == 0)
		return (*this);

	BirthdayList ret;
	UINT32 len=dates.size();
	for(UINT32 i=0; i<len; i++){
		if(dates[i].wDay == day){
			ret.add_birthday(dates[i], labels[i]);
		}
	}
	return ret;
}

const SYSTEMTIME& BirthdayList::get_birthday_date(UINT32 ind) const{
	//todo
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