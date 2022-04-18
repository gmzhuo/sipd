#include "sqlite3.h"
#include <sqlite3.h>
#include <stdexcept>

sqlite3DB::sqlite3DB():
	m_db(NULL)
{
	char *zErrMsg = 0;

	auto rc = sqlite3_open("/var/state/sipd.db", &m_db);
	if( rc || (!m_db)) {
		throw std::runtime_error("failed to open /var/state/sipd.db");
	}

	const char *sql = "CREATE TABLE SIPUser("
			"Username VARCHAR(32) PRIMARY KEY,"
			"Password VARCHAR(32)"
		");";

	sqlite3_exec(m_db , sql , 0 , 0 , &zErrMsg );

	sql = "CREATE TABLE SIPUserDevice("
			"Username VARCHAR(32),"
			"DevType VARCHAR(32),"
			"DevID VARCHAR(32)"
		");";

	sqlite3_exec(m_db , sql , 0 , 0 , &zErrMsg );
}

sqlite3DB::~sqlite3DB()
{
	if(m_db) {
		sqlite3_close(m_db);
	}
}

void sqlite3DB::addUser(const char *username, const char *password)
{
	char sql[2048];
	char *zErrMsg = 0;

	sprintf(sql, "INSERT INTO \"SIPUser\" VALUES(\"%s\", \"%s\");", username, password);

	sqlite3_exec( m_db , sql , 0 , 0 , &zErrMsg );
}

void sqlite3DB::removeUser(const char *username)
{
	char sql[2048];
	char *zErrMsg = 0;

	sprintf(sql, "DELETE FROM \"SIPUser\" WHERE Username = \"%s\";", username);

	sqlite3_exec( m_db , sql , 0 , 0 , &zErrMsg );
}

void sqlite3DB::addDeviceInfo(const char *username, const char *devType, const char *devID)
{
	char sql[2048];
	char *zErrMsg = 0;

	sprintf(sql, "INSERT INTO \"SIPUserDevice\" VALUES(\"%s\", \"%s\", \"%s\");", username, devType, devID);

	sqlite3_exec( m_db , sql , 0 , 0 , &zErrMsg );
}

void sqlite3DB::removeDeviceInfo(const char *username, const char *devType, const char *devID)
{
	char sql[2048];
	char *zErrMsg = 0;

	sprintf(sql, "DELETE FROM \"SIPUserDevice\" WHERE Username = \"%s\" "
			"and DevType = \"%s\" and DevID=\"%s\";",
			username, devType, devID);

	sqlite3_exec( m_db , sql , 0 , 0 , &zErrMsg );
}

std::string sqlite3DB::getUserPassword(const char *username) const
{
	char sql[2048];
	char *zErrMsg = 0;
	int nrow = 0, ncolumn = 0;
	char **azResult; //二维数组存放结果

	sprintf(sql, "SELECT Password FROM SIPUser where Username='%s';", username);
	sqlite3_get_table(m_db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );

	if(nrow == 1 && ncolumn == 1) {
		return azResult[nrow];
	}

	return "";
}

std::list<deviceInfo_t> sqlite3DB::getUserDevice(const char *username) const
{
	char sql[2048];
	char *zErrMsg = 0;
	int nrow = 0, ncolumn = 0;
	char **azResult; //二维数组存放结果

	sprintf(sql, "SELECT Devtype,DevID FROM SIPUserDevice where Username='%s';", username);
	sqlite3_get_table(m_db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );

	std::list<deviceInfo_t> devices;

	for(int j = 0; j < ncolumn; j++) {
		deviceInfo_t info;
		info.m_devType = azResult[j * nrow + nrow];
		info.m_devID = azResult[j * nrow + nrow + 1];
		devices.push_back(info);
	}

	return devices;
}
