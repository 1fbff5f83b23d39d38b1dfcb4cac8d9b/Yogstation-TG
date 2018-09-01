// bombinator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <WS2tcpip.h>
#include <winsock.h>
#include <Windows.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <thread>

#include "queue.hpp"

#pragma comment(lib, "ws2_32")

void begin();
bool initialized = false;
SOCKET comm_socket = INVALID_SOCKET;
HANDLE r_t;
HANDLE s_t;
bool running = TRUE;
#define MSG(x) MessageBox(NULL, x, "heck", NULL)

SOCKET connect_to(std::string addr, std::string port) {
	WSADATA wsadata;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != 0) {
		return INVALID_SOCKET;
	}
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	iResult = getaddrinfo(addr.c_str(), port.c_str(), &hints, &result);
	if (iResult != 0) {
		WSACleanup();
		return INVALID_SOCKET;
	}
	ptr = result;
	SOCKET sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (sock == INVALID_SOCKET) {
		freeaddrinfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}
	iResult = ::connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(sock);
		WSACleanup();
		return INVALID_SOCKET;
	}
	return sock;
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	::split(s, delim, std::back_inserter(elems));
	return elems;
}

Queue<std::string> input_queue;
Queue<std::string> output_queue;

extern "C" __declspec(dllexport) char *get_instruction(int argc, char *argv[]) {
	std::string input = "";
	if (!input_queue.queue_.empty())
		input = input_queue.pop();
	static char buf[4096];
	strcpy_s(buf, input.c_str());
	return buf;
}

extern "C" __declspec(dllexport) char *return_data(int argc, char *argv[]) {
	output_queue.push(std::string(argv[0]));
	static char buf[1] = "";
	return buf;
}

extern "C" __declspec(dllexport) char *initialize(int argc, char *argv[]) {
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)begin, NULL, 0, NULL);
	static char buf[1] = "";
	return buf;
}

extern "C" __declspec(dllexport) char *destroy(int argc, char *argv[]) {
	running = FALSE;
	output_queue.push("lol");
	static char buf[1] = "";
	return buf;
}

bool send_data(std::string data) {
	int iResult = 0;
	while (data.size() > 0) {
		iResult = send(comm_socket, data.c_str(), data.size(), 0);
		if (iResult == -1) return false;
		data.erase(0, iResult);
	}
	return true;
}

std::string recv_data() {
	std::vector<char> recvBuf(1024);
	int iResult = recv(comm_socket, recvBuf.data(), recvBuf.size(), 0);
	if (iResult == 0 || iResult == -1) {
		return "";
	}
	recvBuf.resize(iResult);
	std::string s(recvBuf.begin(), recvBuf.end());
	return s;
}

void recv_loop() {
	while (running) {
		std::string raw_data = recv_data();
		if (!raw_data.size()) {
			running = FALSE;
			output_queue.push("lol");
			break;
		}
		std::vector<std::string> data = split(raw_data, '\n');
		for (unsigned int i = 0; i < data.size(); i++) {
			input_queue.push(data.at(i));
		}
	}
}

void send_loop() {
	while (running) {
		if (!send_data(output_queue.pop())) {
			running = FALSE;
		}
	}
}

void begin() {
	running = FALSE;
	initialized = TRUE;
	shutdown(comm_socket, SD_BOTH);
	closesocket(comm_socket);
	comm_socket = INVALID_SOCKET;
	while (comm_socket == INVALID_SOCKET) {
		comm_socket = connect_to("127.0.0.1", "5678");
	}
	TerminateThread(r_t, 0);
	TerminateThread(s_t, 0);
	running = TRUE;
	r_t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recv_loop, NULL, 0, NULL);
	s_t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)send_loop, NULL, 0, NULL);
}


BOOL __declspec(dllexport) APIENTRY DllMain(HMODULE hinstDLL, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		shutdown(comm_socket, SD_BOTH);
		closesocket(comm_socket);
		WSACleanup();
		break;
	}
	return TRUE;
}