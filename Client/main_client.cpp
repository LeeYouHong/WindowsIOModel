//#include "general_client.h"
#include "select_client.h"
//#include "general_noblock_client.h"


int main(int argc, char **argv)
{
	//general_noblock_client();
	select_client();
	return 0;
}