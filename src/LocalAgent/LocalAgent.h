#ifndef LOCAL_AGENT_INCLUDE
#define LOCAL_AGENT_INCLUDE

#include "StdDef.h"
#include "BoostDef.h"
#include "Res.h"



class GDD
	: public SettingInfo
{
public:
	GDD(std::string const & xml):SettingInfo(xml);

};

typedef bool (* ComFunc)(string const & com,boost::regex const & reg,CPRes& res);

class CProcessor
{
public:
	CProcessor(std::string const xml);
	bool SplitCom(std::string const & com,CPRes& res);
	//void AddExpression(std::string const & expression);

private:
	//std::set<string > expressions;
	typedef std::map<std::string,std::vector<boost::regex > > regexmap;
	regexmap regexs;
	
};

class QueryOptimize
{
public:
	bool Optimize(CPRes const & cpres,QORes& qores,GDD const & gdd);

};

class ResultProcessor
{

};

class GlobalQueryManager
{
public:
	GlobalQueryManager(GDD& gdd);
	bool SetGDD(GDD& gdd);
	bool DistributeQuery(QORes const & qores,QueryRes& queryres);
private:
	
	GDD& gdd_;
	CProcessor cprocessor_;
	QueryOptimize qoptimize_;
	ResultProcessor rprocessor_;
};

//************************************************************

class LDD
	: public SettingInfo
{
public:
	LDD(std::string const & xml):SettingInfo(xml);

};

class LocalQueryManager
{
public:
	LocalQueryManager(LDD& ldd);
	//bool Query

private:
	LDD& ldd_;

};

//************************************************************

class LocalAgent
{
};

#endif  //LOCAL_AGENT_INCLUDE
