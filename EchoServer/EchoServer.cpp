// EchoServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "EchoProtocol.h"

void help ()
{
	std::cout << "Commands:" << std::endl;
	std::cout << "<start> Start the server" << std::endl;
	std::cout << "<stop>  Stop the server" << std::endl;
	std::cout << "<exit>  Leave the application" << std::endl;
}

int _tmain(int /* argc */ , _TCHAR* /* argv[] */)
{
	String command;

	SocketServerType<EchoProtocol> theServer(NodeId(_T("0.0.0.0"), 7));

	help ();

    do
    {
			TCHAR buffer[1024];

            std::cin >> buffer; 
			
			command = String (buffer);

			if (command == _T("start"))
			{
				theServer.Start();
			}
			else if (command == _T("stop"))
			{
				theServer.Stop();
			}
			else if (command == _T("exit"))
			{
				help();
			}

    } while (command != _T("exit"));

	return 0;
}

