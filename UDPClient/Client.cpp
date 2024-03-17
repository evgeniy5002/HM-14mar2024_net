
#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

SOCKET client_socket;
string name = "";
string color = "";

struct Server_Msg {
	string name;
	string msg;
	short color;

	Server_Msg() : name(""), msg(""), color(7) {}
};


vector<string> parse_string(const std::string& text) {
	vector<string> parsed_file;

	size_t position = text.find(";");
	size_t current_position = 0;

	if (position == std::string::npos) {
		parsed_file.push_back(text.substr(current_position));
	}

	while (position != std::string::npos) {
		parsed_file.push_back(text.substr(current_position, position - current_position));

		current_position = position + 1;

		position = text.find(";", current_position);
	}

	parsed_file.push_back(text.substr(current_position));

	return parsed_file;
}

DWORD WINAPI Sender(void* param)
{
	string msg;
	while (true) {
		cin.ignore();
		char query[DEFAULT_BUFLEN];
		cin.getline(query, DEFAULT_BUFLEN);
		string q = query;
		msg = ("MSG;" + name + ";" + q + ";" + color);

		send(client_socket, msg.c_str(), msg.size(), 0);
	}
}

DWORD WINAPI Receiver(void* param)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	while (true) {
		char response[DEFAULT_BUFLEN];
		int recv_bytes = recv(client_socket, response, DEFAULT_BUFLEN, 0);

		if (recv_bytes != 0) {
			response[recv_bytes] = '\0';
			string resp = response;

			if (resp.find("MSG") != string::npos) {
				vector<string> vec = parse_string(resp);

				Server_Msg msg;
				msg.name = vec[1];
				msg.msg = vec[2];
				msg.color = stoi(vec[3]);

				SetConsoleTextAttribute(handle, msg.color);

				cout << "--- msg[" << msg.name << ": " << msg.msg << "] :) \n";

				SetConsoleTextAttribute(handle, 7);
			}
			else {
				cout << "nc: " << response << "\n";
			}
		}
	}
}

BOOL ExitHandler(DWORD whatHappening)
{
	switch (whatHappening)
	{
	case CTRL_C_EVENT: // closing console by ctrl + c
	case CTRL_BREAK_EVENT: // ctrl + break
	case CTRL_CLOSE_EVENT: // closing the console window by X button
		return(TRUE);
		break;
	default:
		return FALSE;
	}
}




int main()
{
	cout << "Input your name: ";
	cin >> name;
	cout << "\n";
	cout << "Input your color [1-15]: ";
	cin >> color;
	cout << "\n";

	system("title Client");

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	addrinfo hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	addrinfo* result = nullptr;
	iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 2;
	}

	addrinfo* ptr = nullptr;
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (client_socket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 3;
		}

		iResult = connect(client_socket, ptr->ai_addr, (int)ptr->ai_addrlen);

		if (iResult == SOCKET_ERROR) {
			closesocket(client_socket);
			client_socket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(result);

	if (client_socket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 5;
	}

	send(client_socket, name.c_str(), name.size(), 0);

	CreateThread(0, 0, Sender, 0, 0, 0);
	CreateThread(0, 0, Receiver, 0, 0, 0);

	Sleep(INFINITE);
}