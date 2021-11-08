#include "DBHelper.h"

DBHelper::DBHelper(void)
	: m_IsConnected(false)
	, m_IsConnecting(false)
	, m_Connection(0)
	, m_Driver(0)
	, m_ResultSet(0)
{
}

bool DBHelper::IsConnected(void)
{
	return m_IsConnected;
}

CreateAccountWebResult DBHelper::CreateAccount(const string& email, const string& password)
{
	sql::Statement* stmt = m_Connection->createStatement();
	try
	{
		m_ResultSet = stmt->executeQuery("SELECT * FROM `web_auth`;");
	}
	catch (SQLException e)
	{
		printf("Failed to retrieved web_auth data!\n");
		return CreateAccountWebResult::INTERNAL_SERVER_ERROR;
	}

	while (m_ResultSet->next())
	{
		int32_t id = m_ResultSet->getInt(sql::SQLString("id"));
		printf("id: %d\n", id);
		int32_t id_bycolumn = m_ResultSet->getInt(1);
		printf("id_bycolumn(1): %d\n", id_bycolumn);

		SQLString email = m_ResultSet->getString("email");
		printf("email: %s\n", email.c_str());
	}


	printf("Successfully retrieved web_auth data!\n");
	return CreateAccountWebResult::SUCCESS;
}

void DBHelper::Connect(const string& hostname, const string& username, const string& password)
{
	if (m_IsConnecting)
		return;

	m_IsConnecting = true;
	try
	{
		m_Driver = mysql::get_driver_instance();
		m_Connection = m_Driver->connect(hostname, username, password);
		m_Connection->setSchema("authenticationservice");
	}
	catch (SQLException e)
	{
		printf("Failed to connect to database with error: %s\n", e.what());
		m_IsConnecting = false;
		return;
	}
	m_IsConnected = true;
	m_IsConnecting = false;

	printf("Successfully connected to database!\n");
}