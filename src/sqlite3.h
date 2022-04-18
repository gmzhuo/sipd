#pragma once
#include <string>
#include <list>

struct sqlite3;

typedef struct deviceInfo{
	std::string m_devType;
	std::string m_devID;
}deviceInfo_t;

class sqlite3DB {
protected:
	sqlite3DB();
	virtual ~sqlite3DB();
	sqlite3DB(const sqlite3DB&) = delete;
	sqlite3DB& operator=(const sqlite3DB&) = delete;
public:
	void addUser(const char *username, const char *password);
	void removeUser(const char *username);
	void addDeviceInfo(const char *username, const char *devType, const char *devID);
	void removeDeviceInfo(const char *username, const char *devType, const char *devID);
	std::string getUserPassword(const char *username) const;
	std::list<deviceInfo_t> getUserDevice(const char *username) const;
public:
	static sqlite3DB& getInstance()
	{
		static sqlite3DB db;
		return db;
	}
protected:
	sqlite3 *m_db;
};