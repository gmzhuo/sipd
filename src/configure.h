#pragma once
#include <core/service.h>

class configureTR181:public service{
public:
	configureTR181();
	virtual ~configureTR181();
protected:
	virtual void apply(const char *pathname);
	virtual void del(const char *pathname);
	virtual void onEvent(const char *type, blobMessage &msg);
};

extern void addConfigureService();