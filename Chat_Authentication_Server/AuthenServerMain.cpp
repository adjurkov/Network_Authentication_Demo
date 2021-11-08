#include "DBHelper.h"

DBHelper database;



int main(int argc, char** argv)
{
	database.Connect("127.0.0.1", "root", "root");
	database.CreateAccount("ana_djurkovic@hotmail.com", "password");
	system("Pause");

	return 0;
}