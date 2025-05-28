#include "AbstractFactory.h"

void CptBaseObjectService::RegisterFCptObject(const ST_ComponentBaseInfo& info)
{
}

void CptBaseObjectService::RegisterVCptObject(const ST_ComponentBaseInfo& info)
{
}

void CptBaseObjectService::RegisterLCptObject(const ST_ComponentBaseInfo& info)
{
}
void CptBaseObject::PrintText()
{
	if (m_cptBaseInfo == nullptr)
	{
		std::cout << "errCptInfo : CptInfo is Nullptr"<<std::endl;
	}
	else
	{
		std::cout << "CptInfo : uid is: " << m_cptBaseInfo->cptUID << "  CptName is : " << m_cptBaseInfo->cptName << std::endl;
	}
}
CptBaseObject::CptBaseObject()
{
}

CptBaseObject::~CptBaseObject()
{
}
