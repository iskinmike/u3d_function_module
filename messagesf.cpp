/*
* File: messages.cpp
* Author: m79lol, iskinmike
*
*/
#include <stdlib.h>
#include <string>
#include <vector>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/shared_memory_object.hpp>

#include "messagesf.h"
#include "define_section.h"
#include "stringC11.h"

PTR_DEFINE_ATOM(G_CS_FromMemory);

bool Is_ReadFromSharedMemory = false;

struct VectorAndCS{
	std::vector<std::pair<PTR_EVENT_HANDLE, std::string> *>* Vector;
	PTR_DEFINE_ATOM(CS);
};

std::vector<std::pair<PTR_EVENT_HANDLE, std::string> *> *BoxOfMessages;

int extractString(std::string str, char first, char second){
	std::string temp("");

	int beg = 0;
	int end = 0;
	int i = 1;
	if (first == '%') { i++; }
	beg = str.find(first) + i;
	end = str.find(second);

	temp.assign(str, beg, end - beg);

	return strtod(temp.c_str(), NULL);
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

	DEFINE_EVENT(WaitRecivedMessage);
	DEFINE_ATOM(WaitMessageMutex);

	std::pair<PTR_EVENT_HANDLE, std::string> pairParams(&WaitRecivedMessage, params);
	if (!Is_ReadFromSharedMemory){
		initFunctionsDll();
		Is_ReadFromSharedMemory = true;
	}
	ATOM_LOCK(*G_CS_FromMemory);
	(*BoxOfMessages).push_back(&pairParams);
	ATOM_UNLOCK(*G_CS_FromMemory); 
	if (params != "destroy") {
		EVENT_WAIT(WaitRecivedMessage,WaitMessageMutex);
	}
	DESTROY_EVENT(WaitRecivedMessage);

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
