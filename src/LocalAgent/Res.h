#ifndef RES_INCLUDE
#define RES_INCLUDE

#include "StdDef.h"
#include "tinyxml.h"

typedef TiXmlDocument CPRes;


class QORes;

class QueryRes
{
public:
	QueryRes(std::string const data);
	void Jion(QueryRes const & x);
	void Union(QueryRes const & x);
	void Projection(std::string const x[]);
	bool Empty();
	unsigned Rows();
	unsigned Columns();
private:
	unsigned rows;
	unsigned columns;
	TiXmlDocument doc_;
	//std::vector<std::string >* table;
};

class SettingInfo
{
	SettingInfo(std::string const & xml);
	virtual ~SettingInfo();
	bool Read(std::string const & xml);
	bool Write(std::string const & xml);
protected:
	TiXmlDocument doc_;
};


#endif    //RES_INCLUDE
