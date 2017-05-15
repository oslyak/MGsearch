#include <iostream>
#include <fstream>
#include <iterator>

#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Timespan.h"
#include "Poco/Exception.h"
#include "Poco/RegularExpression.h"
#include "Poco/String.h"

using std::string;
using std::vector;
using std::cin;
using std::cout;
using std::endl;

using Poco::Net::SocketAddress;
using Poco::Net::DatagramSocket;
using Poco::Timespan;
using Poco::RegularExpression;

void MakeSsdpRequest(vector<string>& responses,string st = "") {
	if (st.empty()) st = "upnp:rootdevice";
	//if (st.empty()) st = "ssdp:all";

	string message = "M-SEARCH * HTTP/1.1\r\n"
		"HOST: 239.255.255.250:1900\r\n"
		"ST:" + st + "\r\n"
		"MAN: \"ssdp:discover\"\r\n"
		"MX:1\r\n\r\n";

	DatagramSocket dgs;
	SocketAddress destAddress("239.255.255.250", 1900);
	dgs.sendTo(message.data(), message.size(), destAddress);
	dgs.setSendTimeout(Timespan(1, 0));
	dgs.setReceiveTimeout(Timespan(2, 0));
	char buffer[1024];
	try {
		// Здесь можно и бесконечный цикл, так как отвалимся по timeout. Но на всякий ограничиваю 1000 пакетами, так как, если кто-то решит отвечать постоянно, timeout не наступит.
		for (int i = 0; i < 1000; i++) {
			int n = dgs.receiveBytes(buffer, sizeof(buffer));
			buffer[n] = '\0';
			responses.push_back(string(buffer));
		}
	}
	catch (Poco::TimeoutException) { }
}

string ParseIP(string str) {
	try {
		RegularExpression re("(location:.*://)([a-zA-Z_0-9\\.]*)([:/])", RegularExpression::RE_CASELESS);
		vector<string> vec;
		re.split(str, 0, vec);
		if (vec.size() > 2) return vec[2];
	}
	catch (const Poco::RegularExpressionException) { cout << "RegularExpressionException" << endl; }
	return "";
}

string GetDir(string path) {
	// Get the last position of '/'

	// get '/' or '\\' depending on unix/mac or windows.
	#if defined(_WIN32) || defined(WIN32)
		int pos = path.rfind('\\');
	#else
		int pos = path.rfind('/');
	#endif

	return path.substr(0, pos + 1);
}

int main(int numArgs, char *args[])
{
	vector<string> ips, responses;
	MakeSsdpRequest(responses, "urn:help-micro.kiev.ua:device:fp:1");

	for (string response : responses) {
		// Проверяю статус ответа.
		if ((response.find("HTTP/1.1 200 OK", 0) == 0) && (response.find("help-micro.kiev.ua:device", 0) > 0))  {
			string ip = ParseIP(response);
			if (!ip.empty()) ips.push_back(ip);
		}
	}

	sort(ips.begin(), ips.end());
	ips.erase(unique(ips.begin(), ips.end()), ips.end());
	for (string ip : ips) {
		cout << "IP: " << ip << endl;
	}

	std::ofstream output_file(GetDir(args[0]) + "mg-ips.txt");
	std::ostream_iterator<std::string> output_iterator(output_file, "\r\n");
	std::copy(ips.begin(), ips.end(), output_iterator);

	return  0;
}