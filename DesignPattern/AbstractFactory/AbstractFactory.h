#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
struct ST_ComponentBaseInfo
{
    unsigned long cptUID;
    std::string cptName;
};

class CptBaseObject
{
  public:
    CptBaseObject();
    virtual ~CptBaseObject();
    virtual ST_ComponentBaseInfo *CreateComponentObject(unsigned long uid,const std::string& cptName) = 0;
    virtual void PrintText() = 0;
  private:
    ST_ComponentBaseInfo* m_cptBaseInfo = nullptr;
};

class CptBaseObjectService
{
    void RegisterFCptObject(const ST_ComponentBaseInfo& info);
    void RegisterVCptObject(const ST_ComponentBaseInfo& info);
    void RegisterLCptObject(const ST_ComponentBaseInfo& info);
private:
    std::unordered_map<unsigned long, CptBaseObject*> m_fCptMap;
    std::unordered_map<unsigned long, CptBaseObject*> m_vCptMap;
    std::unordered_map<unsigned long, CptBaseObject*> m_lCptMap;
};