#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h> 
#include <string>
#include <string.h>

#define TIME_PORT	27015

enum operations {
	Error, GetTime, GetTimeWithoutDate, GetTimeSinceEpoch, GetClientToServerDelayEstimation, MeasureRTT, GetTimeWithoutDateOrSeconds,
	GetYear, GetMonthAndDay, GetSecondsSinceBeginingOfMonth, GetWeekOfYear, GetDaylightSavings, GetTimeWithoutDateInCity, MeasureTimeLap, Exit
};

void displayMenu();
string getInput();
bool socketError(int bytes, string at);
string op2str(int op);
void handleRRT();
void handleDelayEstimation();
string send(string input);
string receive();

SOCKET connSocket;
sockaddr_in server;

void main()
{
	// Initialize Winsock (Windows Sockets).
	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Client: Error at WSAStartup()\n";
		return;
	}

	// Client side:
	// Create a socket and connect to an internet address.
	connSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == connSocket)
	{
		cout << "Time Client: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a client to communicate on a network, it must connect to a server.
	// Need to assemble the required data for connection in sockaddr structure.
	// Create a sockaddr_in object called server. 
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(TIME_PORT);
	
	displayMenu();

	while (true)
	{
		
		string input = getInput();
		if (input == "Exit") break;
		system("cls");
		displayMenu();

		try
		{
			if (input == op2str(GetClientToServerDelayEstimation)) handleDelayEstimation();
			else if (input == op2str(MeasureRTT)) handleRRT();
			else
			{
				send(input);
				receive();
			}
		}
		catch (runtime_error e)
		{
			cout << e.what();
			return;
		}
	}

	// Closing connections and Winsock.
	cout << "Time Client: Closing Connection.\n";
	closesocket(connSocket);
	system("pause");
}

int readInput(int min, int max)
{
	int num;
	while (true)
	{
		try {
			string input;
			cin >> input;
			num = stoi(input);
			if (num >= min && num <= max)
				break;
		}
		catch (exception e) {}
		cout << "invalid input" << endl;
	}
	return num;
}


void displayMenu()
{
	cout << "please choose server operation:\n\t1.Get Time\n\t2.Get Time Without Date\n\t3.Get Time Since Epoch\n\t"
		"4.Get Client To Server Delay Estimation\n\t5.Measure RTT\n\t6.Get Time Without Date Or Seconds\n\t7.Get Year\n\t8.Get Month And Day\n\t"
		"9.Get Seconds Since Begining Of Month\n\t10.Get Week Of Year\n\t11.Get Day Light Savings\n\t"
		<< "12.Get Time Without Date In City\n\t13.Measure Time Lap\n\t14.Exit\n\n";
}

bool socketError(int bytes, string at)
{
	if (SOCKET_ERROR == bytes)
	{
		closesocket(connSocket);
		WSACleanup();
		throw runtime_error("Time Client : Error at " + at + " : " + to_string(WSAGetLastError()));
	}
	return false;
}


void handleRRT()
{
	long int sendTime = 0, recvTime = 0;
	double sum = 0;

	for (int i = 1; i < 100; i++)
	{
		sendTime = GetTickCount();
		send(op2str(GetClientToServerDelayEstimation));
		string recv = receive();
		recvTime = GetTickCount();

		double duration = (double)(recvTime - sendTime) / 1000.0;
		sum += duration;
	}
	sum = sum / 100;
	system("cls");
	displayMenu();
	cout << "RRT Measure: " << sum << " seconds" << endl;
}

void handleDelayEstimation()
{
	int prevVal = 0, curVal = 0, sum = 0;

	for (int i = 0; i < 100; i++)
		send(op2str(GetClientToServerDelayEstimation));

	for (int i = 0; i < 100; i++)
	{
		string recv = receive();
		curVal = stoi(recv);
		if(i !=0) sum += curVal - prevVal;
		prevVal = curVal;
	}
	double retVal = (double)sum / 99;
	system("cls");
	displayMenu();
	cout << "Estimate Delay: " << retVal << endl;
}

string op2str(int op)
{
	string res;
	if (op == GetTime) return "GetTime";
	else if (op == GetTimeWithoutDate) return "GetTimeWithoutDate";
	else if (op == GetTimeSinceEpoch) return "GetTimeSinceEpoch";
	else if (op == GetClientToServerDelayEstimation) return "GetClientToServerDelayEstimation";
	else if (op == MeasureRTT) return "MeasureRTT";
	else if (op == GetTimeWithoutDateOrSeconds) return "GetTimeWithoutDateOrSeconds";
	else if (op == GetYear) return "GetYear";
	else if (op == GetMonthAndDay) return "GetMonthAndDay";
	else if (op == GetSecondsSinceBeginingOfMonth) return "GetSecondsSinceBeginingOfMonth";
	else if (op == GetWeekOfYear) return "GetWeekOfYear";
	else if (op == GetDaylightSavings) return "GetDaylightSavings";
	else if (op == GetTimeWithoutDateInCity) return "GetTimeWithoutDateInCity";
	else if (op == MeasureTimeLap) return "MeasureTimeLap";
	else if (op == Exit) return "Exit";


	else return "";
}


string getCity()
{
	string city;
	int input;
	cout << "please enter the number of the required city:\n\t1.Tokyo\n\t2.Melbourne\n\t3.San Francisco\n\t4.Porto\n\t5.other\n";

	input = readInput(1, 5);

	if (input == 1) return " Tokyo";
	else if (input == 2) return " Melbourne";
	else if (input == 3) return " San Francisco";
	else if (input == 4) return " Porto";
	else if (input == 5) return " other";
}

string getInput()
{
	string op;
	string city;
	int num = readInput(GetTime, Exit);
	if (num == GetTimeWithoutDateInCity)  city = getCity();

	op = op2str(num) + city;
	return op;
}


string send(string input)
{
	int bytesSent = 0;
	char sendBuff[255];

	strcpy(sendBuff, input.c_str());
	bytesSent = sendto(connSocket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr*)&server, sizeof(server));
	socketError(bytesSent, "sendto()");
	cout << "Time Client: Sent: " << bytesSent << "/" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

	return input;
}

string receive()
{
	int bytesRecv = 0;
	char recvBuff[255];

	bytesRecv = recv(connSocket, recvBuff, 255, 0);
	socketError(bytesRecv, "recv()");
	recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
	cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n\n";

	return recvBuff;
}