/*
* File: messages.cpp
* Author: m79lol, iskinmike
*
*/

#include <string>
#include <vector>
#include <windows.h>
#include <process.h>

#include <boost\interprocess\managed_shared_memory.hpp>
#include <boost\interprocess\shared_memory_object.hpp>

#include "messagesf.h"

CRITICAL_SECTION *G_CS_FromMemory;

bool Is_ReadFromSharedMemory = false;

struct VectorAndCS{
	std::vector<std::pair<HANDLE*, std::string> *>* Vector;
	CRITICAL_SECTION *CS;
};

std::vector<std::pair<HANDLE*, std::string> *> *BoxOfMessages;

int extractString(std::string str, char first, char second){
	std::string temp("");

	int beg = 0;
	int end = 0;
	int i = 1;
	if (first == '%') { i++; }
	beg = str.find(first) + i;
	end = str.find(second);

	temp.assign(str, beg, end - beg);

	return std::stoi(temp);
};
int extractObj_id(std::string str){
	return extractString(str, ':', '&');
};

void testStringSuccess(std::string str){
	if (str.find("fail") != std::string::npos) {
		throw std::exception();
	}
};

void initFunctionsDll(){
	boost::interprocess::shared_memory_object shm_obj(boost::interprocess::open_only, "PostmansSharedMemory", boost::interprocess::read_write);
	boost::interprocess::mapped_region region(shm_obj, boost::interprocess::read_only);

	VectorAndCS **VACSFSM = static_cast<VectorAndCS**>(region.get_address());
	G_CS_FromMemory = (*VACSFSM)->CS;
	BoxOfMessages = (*VACSFSM)->Vector;
};

std::string createMessage(std::string params){
	HANDLE WaitRecivedMessage;
	WaitRecivedMessage = CreateEvent(NULL, true, false, NULL);

	std::pair<HANDLE*, std::string> pairParams(&WaitRecivedMessage, params);
	if (!Is_ReadFromSharedMemory){
		initFunctionsDll();
		Is_ReadFromSharedMemory = true;
	}
	EnterCriticalSection(G_CS_FromMemory); //CRITICAL_SECTION
	(*BoxOfMessages).push_back(&pairParams); //push_back
	LeaveCriticalSection(G_CS_FromMemory);
	if (params != "destroy") {
		WaitForSingleObject(WaitRecivedMessage, INFINITE);
	}
	CloseHandle(WaitRecivedMessage);

	testStringSuccess(pairParams.second);
	return pairParams.second;
};

void deleteCreatedObject(int obj_id){
	std::string params("delete:");

	params.append(std::to_string(obj_id));

	createMessage(params);
};

int createCube(int x, int y, int z, int d_x, int d_y, int d_z, std::string color){
	std::string params("obj:cube,");

	params.append(std::to_string(x));
	params.append(",");
	params.append(std::to_string(y));
	params.append(",");
	params.append(std::to_string(z));
	params.append(",");
	params.append(std::to_string(d_x));
	params.append(",");
	params.append(std::to_string(d_y));
	params.append(",");
	params.append(std::to_string(d_z));
	params.append(",");
	params.append(color);

	std::string temp;
	temp = createMessage(params);

	int d = extractObj_id(temp);
	return d;
};

int createSphere(int x, int y, int z, int Radius, std::string color){
	std::string params("obj:sphere,");

	params.append(std::to_string(x));
	params.append(",");
	params.append(std::to_string(y));
	params.append(",");
	params.append(std::to_string(z));
	params.append(",");
	params.append(std::to_string(Radius));
	params.append(",");
	params.append(color);

	std::string temp;
	temp = createMessage(params);

	int d = extractObj_id(temp);
	return d;
};
