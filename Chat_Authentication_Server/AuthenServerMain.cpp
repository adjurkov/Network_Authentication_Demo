#include <gen/authentication.pb.h>
#include <mysql/jdbc.h>

using namespace sql::mysql;

void ConnectToMySQLServer()
{
	try 
	{
		// Returns a mySQL driver pointer
		MySQL_Driver* driver = sql::mysql::get_driver_instance();
	}
	catch (sql::SQLException e)
	{
		printf("Failed to get driver instance with error %s\n", e.what());
		return;
	}
	printf("Successfully retrieved the driver instance\n");

}

int main(int argc, char** argv)
{
	// To create a mySQL connection, we need a driver
	ConnectToMySQLServer();
	system("Pause");

	return 0;
}