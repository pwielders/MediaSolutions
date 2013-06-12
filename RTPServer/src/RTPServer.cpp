// RTPServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RTSPServer.h"

using namespace Solutions;

void help ()
{
	std::cout << "Root directory for media: "<< Generics::System::WorkingDirectory() << std::endl << std::endl;
	std::cout << "Commands:" << std::endl;
	std::cout << "<start> Start the server" << std::endl;
	std::cout << "<stop>  Stop the server" << std::endl;
	std::cout << "<exit>  Leave the application" << std::endl;
}

int _tmain(int /* argc */, _TCHAR* /*argv[] */)
{
	// Turn tracing on..
	// TraceUnit::Instance().Output(NodeId("127.0.0.1", 5555));

	// We want to see the RTSP exchange.
	// TraceUnit::Instance().SetCategories(true, "ProtocolSuite", "RTSPRequest");
	// TraceUnit::Instance().SetCategories(true, "ProtocolSuite", "RTSPResponse");

	// We want to see session flow.
	//TraceUnit::Instance().SetCategories(true, "ProtocolSuite", "TraceSessionFlow");
	
	RTSPServer theServer(Generics::NodeId(_T("0.0.0.0"), 554), _T("MediaSolutions - Streaming Media v0.1"), RTSPServer::AGENT_SETUP_ALLOWED);

	// -----------------------------------------------------------------------
	// Assign all sources types that we support on this server..
	// -----------------------------------------------------------------------
	theServer.Register (new Source::FactoryType<Source::FileMP3Audio>());
	theServer.Register (new Source::FactoryType<Source::FileAVIMedia>());
	theServer.Register (new Source::FactoryType<Source::FileMPGMedia>());
	theServer.Register (new Source::FactoryType<Source::FileMPEGMedia>());
	theServer.Register (new Source::FactoryType<Source::FileTSMedia>());

	// -----------------------------------------------------------------------
	// Assign all sinks types that we support on this server..
	// -----------------------------------------------------------------------
	theServer.Register (new Sink::FactoryType<Sink::RTPSinkUDP>());

	help ();

	String command; 

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
			else if (command != _T("exit"))
			{
				help();
			}

    } while (command != _T("exit"));


	// Stop the server, if it was not stopped !!
	theServer.Stop();

	// All singletons are derived from the generics singleton, as we are done with all stuff, trigegr them to DIE !!!!
	Generics::Singleton::Dispose();

	return 0;
}