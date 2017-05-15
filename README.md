# MGsearch
C++ and Poco library project. Search fiscal printers MG series on LAN by SSDP requests.

But if change one line it can be used to search equipment of any type.

Just remove second parameter from line 77:
MakeSsdpRequest(responses, "urn:help-micro.kiev.ua:device:fp:1");
