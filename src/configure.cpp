#include "configure.h"
#include <core/servdcontext.h>

DECLARE_MODULE_GET_LOCK();

configureTR181::configureTR181()
{
	m_pathname = "Device.Service.SIPProxy.Config.";
}

configureTR181::~configureTR181()
{
}

void configureTR181::apply(const char *pathname)
{
}

void configureTR181::del(const char *pathname)
{
}

void configureTR181::onEvent(const char *type, blobMessage &msg)
{
}

void addConfigureService()
{
	ADD_MODULE_SERVICE(configureTR181);
}