//#include "general_server.h"
#include "select_server.h"
//#include "general_noblock_server.h"

int main(int argc, char **argv)
{
	//general_server();
	//general_noblock_server();
	select_server();
	return 0;
}