#include "AbstractFactory.h"
#include "ComponentFactoryView.h"

CptBaseObject::CptBaseObject()
{
}

CptBaseObject::~CptBaseObject()
{
}

void CptBaseObject::CreateComponentObject()
{
	for (auto& tmp : m_cptMap)
	{
		ComponentObject* object = new ComponentObject(tmp.second);
	}
}


ComponentObject::ComponentObject(const ST_ComponentBaseInfo& info)
{
	std::cout << "ComponentObject : UID is : " << info.cptUID << " Name is : " << info.cptName << std::endl;
}

ComponentObject::~ComponentObject()
{
}
