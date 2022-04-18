#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>

#define TIME_PORT	27015
string handle_request(char* recvBuff, int bytesRecv);
void checkTimeLap();

enum operations {
	GetTime, GetTimeWithoutDate, GetTimeSinceEpoch, GetClientToServerDelayEstimation, MeasureRTT, GetTimeWithoutDateOrSeconds,
	GetYear, GetMonthAndDay, GetSecondsSinceBeginingOfMonth, GetWeekOfYear, GetDaylightSavings, GetTimeWithoutDateInCity, MeasureTimeLap, Error
};
operations str2op(char* recvBuff);

clock_t timelap = 0;

void main()
{
	// Initialize Winsock (Windows Sockets).

	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}

	// Server side:
	// Create and bind a socket to an internet address.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called m_socket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							datagram sockets (SOCK_DGRAM), 
	//							and the UDP/IP protocol (IPPROTO_UDP).
	SOCKET m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The "if" statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == m_socket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a server to communicate on a network, it must first bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigdned long (4 bytes) data type).
	// INADDR_ANY means to listen on all interfaces.
	// inet_addr (Internet address) is used to convert a string (char *) into unsigned int.
	// inet_ntoa (Internet address) is the reverse function (converts unsigned int to char *)
	// The IP address 127.0.0.1 is the host itself, it's actually a loop-back.
	serverService.sin_addr.s_addr = INADDR_ANY;	//inet_addr("127.0.0.1");
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order (which is big-endian).
	serverService.sin_port = htons(TIME_PORT);

	// Bind the socket for client's requests.

	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(m_socket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(m_socket);
		WSACleanup();
		return;
	}

	// Waits for incoming requests from clients.

	// Send and receive data.
	sockaddr client_addr;
	int client_addr_len = sizeof(client_addr);
	int bytesSent = 0;
	int bytesRecv = 0;
	char sendBuff[255];
	char recvBuff[255];

	// Get client's requests and answer them.
	// The recvfrom function receives a datagram and stores the source address.
	// The buffer for data to be received and its available size are 
	// returned by recvfrom. The fourth argument is an idicator 
	// specifying the way in which the call is made (0 for default).
	// The two last arguments are optional and will hold the details of the client for further communication. 
	// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).
	cout << "Time Server: Wait for clients' requests.\n";

	while (true)
	{
		checkTimeLap();
		bytesRecv = recvfrom(m_socket, recvBuff, 255, 0, &client_addr, &client_addr_len);
		if (SOCKET_ERROR == bytesRecv)
		{
			cout << "Time Server: Error at recvfrom(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return;
		}

		// Answer client's request by the current time.
		recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";
		string res = handle_request(recvBuff, bytesRecv);
		strcpy(sendBuff, res.c_str());

		// Sends the answer to the client, using the client address gathered
		// by recvfrom. 
		bytesSent = sendto(m_socket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr*)&client_addr, client_addr_len);
		if (SOCKET_ERROR == bytesSent)
		{
			cout << "Time Server: Error at sendto(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return;
		}

		cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(m_socket);
	WSACleanup();
}



string handle_request(char* recvBuff, int bytesRecv)
{
	string sendBuff;
	time_t _time;
	time(&_time);
	struct tm* timer = localtime(&_time);;

	char tempBuff[255];

	switch ( str2op(recvBuff) ) 
	{
	case GetTime:
		strftime(tempBuff, 256, "%c", timer);
		break;

	case GetTimeWithoutDate:
		strftime(tempBuff, 256, "%H:%M:%S", timer);
		break;

	case GetTimeSinceEpoch:
		_itoa(_time, tempBuff, 10);
		break;

	case GetClientToServerDelayEstimation: 
		sprintf(tempBuff, "%ld", GetTickCount());
		break;

	case GetTimeWithoutDateOrSeconds:
		strftime(tempBuff, 256, "%H:%M", timer);
		break;

	case GetYear:
		strftime(tempBuff, 256, "%Y", timer);
		break;

	case GetMonthAndDay:
		strftime(tempBuff, 256, "%d/%m", timer);
		break;

	case GetSecondsSinceBeginingOfMonth:
	{
		int result = timer->tm_mday * 24 * 60 * 60 + timer->tm_hour * 60 * 60 + timer->tm_min * 60 + timer->tm_sec;
		_itoa(result, tempBuff, 10);
	}
		break;

	case GetWeekOfYear:
		strftime(tempBuff, 256, "%U", timer);
		break;

	case GetDaylightSavings:
	{
		int dayLight = timer->tm_isdst;
		_itoa(dayLight, tempBuff, 10);
	}
		break;

	case GetTimeWithoutDateInCity:
	{
		int add;
		if (strstr(recvBuff, "Tokyo") != NULL) add = 9;
		else if (strstr(recvBuff, "Melbourne") != NULL) add = 10;
		else if (strstr(recvBuff, "San Francisco") != NULL) add = -7;
		else if (strstr(recvBuff, "Porto") != NULL) add = 1;
		else add = 0;

		timer->tm_mday += (int)(timer->tm_hour + add) / 24;
		timer->tm_hour = (timer->tm_hour + add) % 24;
		strftime(tempBuff, 256, "%c", timer);	}
		break;

	case MeasureTimeLap: 
		if (timelap == 0)
		{
			timelap = clock();
			strcpy(tempBuff, "measuring timer started");
		}
		else
		{
			double duration = (clock() - timelap) / (double)CLOCKS_PER_SEC;
			sprintf(tempBuff, "time has passed %f seconds", duration);
			timelap = 0;
		}
		break;

	default:
		strcpy(tempBuff, "invalid");
		
		
	}

	sendBuff = tempBuff;
	return sendBuff;
}


operations str2op(char* recvBuff)
{
	if (strcmp(recvBuff, "GetTime") == 0) return GetTime;
	else if (strcmp(recvBuff, "GetTimeWithoutDate") == 0) return GetTimeWithoutDate;
	else if (strcmp(recvBuff, "GetTimeSinceEpoch") == 0) return GetTimeSinceEpoch;
	else if (strcmp(recvBuff, "GetClientToServerDelayEstimation") == 0) return GetClientToServerDelayEstimation;
	else if (strcmp(recvBuff, "MeasureRTT") == 0) return MeasureRTT;
	else if (strcmp(recvBuff, "GetTimeWithoutDateOrSeconds") == 0) return GetTimeWithoutDateOrSeconds;
	else if (strcmp(recvBuff, "GetYear") == 0) return GetYear;
	else if (strcmp(recvBuff, "GetMonthAndDay") == 0) return GetMonthAndDay;
	else if (strcmp(recvBuff, "GetSecondsSinceBeginingOfMonth") == 0) return GetSecondsSinceBeginingOfMonth;
	else if (strcmp(recvBuff, "GetWeekOfYear") == 0) return GetWeekOfYear;
	else if (strcmp(recvBuff, "GetDaylightSavings") == 0) return GetDaylightSavings;
	else if (strstr(recvBuff, "GetTimeWithoutDateInCity") != NULL) return GetTimeWithoutDateInCity;
	else if (strcmp(recvBuff, "MeasureTimeLap") == 0) return MeasureTimeLap;
	else return Error;
}



void checkTimeLap()
{
	if (timelap != 0)
	{
		double duration = (double)(clock()  - timelap) / CLOCKS_PER_SEC;
		if (duration >= 180) timelap = 0;
	}
}