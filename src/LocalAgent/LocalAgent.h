#ifndef LOCALAGNET_INCLUDE
#define LOCALAGNET_INCLUDE
#define OPTIMIZE
#include <string>
#include <map>
#include <vector>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include "tinyxml.h"


//****************************************************************************

class CPRes:
	public TiXmlDocument
{
public:
private:
};

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

enum relation
{
	Union,
	Product,
	Selection,
	Projection,
	Join,
	E,
	G,
	L,
	GE,
	LE
};

#define DATABUFFERSIZE 6400

#pragma pack(push,1)

struct datapack
{
	char flage;   //0最后一个分片，1还有分片
	//char type;
	unsigned total;
	unsigned len;
	char data[DATABUFFERSIZE];
};

#pragma pack(pop)

//**********************************************************************

class Data:
	public boost::enable_shared_from_this<Data>
{
public:
	Data(unsigned size)
	{
		size_=size;
		used=0;
		data_=new char[size];
		memset(data_,0,size);
	}
	Data()
	{
		size_=0;
		used=0;
		data_=NULL;
	}
	Data(const Data& data)
	{
		size_=data.size_;
		used=data.used;
		data_=new char[size_];
		memcpy(data_,data.data_,size_);
	}
	~Data()
	{
		delete[] data_;
	}
	char const *  GetData()
	{
		return data_;
	}
	bool AddData(char* data,unsigned size)
	{
		if(size_-used<size) return false;
		memcpy(&data_[used],data,size);
		used=used+size;
		return true;
	}
	void Assigen(unsigned size)
	{
		delete[] data_;
		data_=new char[size];
		memset(data_,0,size);
		size_=size;
		used=0;
	}

private:
	unsigned size_;
	unsigned used;
	char* data_;
};

typedef boost::shared_ptr<Data> DataPtr;

//**********************************************************************************

class Client:
	public boost::enable_shared_from_this<Client>
{
public:
	
	Client(std::string const & address, std::string const & port);
	~Client();
	void Write(char* data,unsigned size,unsigned total,Data& res);
	//void Read(datapack
	void Read(Data& res);
	void Close();
	char const * GetData();
	void Run();
private:
	void HandleConnect(boost::system::error_code const & error,boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	void DoWrite(char* data,unsigned size,unsigned total,Data& res);
	void HandleWrite(boost::system::error_code const & error,std::size_t count,char* data,unsigned size,unsigned total,Data& res);
	void HandleKeepRead(boost::system::error_code const & error,std::size_t count,Data& res);
	void DoRead(Data& res);
	void HandleReadData(boost::system::error_code const & error,std::size_t count,Data& res);
	void DoClose();
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::socket socket_;
	boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::resolver::query query_;
    boost::asio::ip::tcp::resolver::iterator iterator_;
	datapack buffer_;
	Data data_;
};

typedef boost::shared_ptr<Client> ClientPtr;

//******************************************************************************************

class Session:
	public boost::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_service& io_service);
	~Session();
	boost::asio::ip::tcp::socket& Socket();
	void Start(boost::function<bool (Data& ) > handler);
	void Write(char* data,unsigned size,unsigned total);
	//void Read(boost::function<bool (Data*& ) > handler);
	void Close();
	void Print();
private:
	//void DoWrite(char* data,unsigned size,unsigned total);
	void HandleWrite(boost::system::error_code const & error,std::size_t,char* data,unsigned size,unsigned total);
	void HandleKeepRead(boost::system::error_code const & error,std::size_t count,boost::function<bool (Data& ) > handler);
	//void DoRead();
	void HandleReadData(boost::system::error_code const & error,std::size_t count,boost::function<bool (Data& ) > handler);
	void DoClose();
	boost::asio::ip::tcp::socket socket_;
	datapack buffer_;
	Data data_;

};

typedef boost::shared_ptr<Session> SessionPtr;


//******************************************************************************************

class Server
{
public:
	Server(unsigned thread_pool_size,unsigned short port,boost::function<bool (Data& ) > handler);
	//void HandleAccept(Session& session,const boost::system::error_code& error);
	void HandleAccept(SessionPtr session,const boost::system::error_code& error,boost::function<bool (Data& ) > handler);
	void Run();
	void Stop();
private:
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::endpoint endpoint_;
	unsigned thread_pool_size_;
};

class GDD;

class LocalAgent;

class Location
{
public:
	Location(std::string const & tablename,std::string const & var,std::string const & low,std::string const & le,std::string const & up,std::string const & ue);
	void Print();
	friend class GDD;
	friend class LocalAgent;
private:
	bool Insert(std::string const & tablename,std::string const & var,std::string const & low,std::string const & le,std::string const & up,std::string const & ue,std::string const & site);
	std::string tablename;
	std::string var;
	std::string low;
	std::string lowequal;
	std::string up;
	std::string upequal;
	bool requestAll;
	//bool isRequest;
	TiXmlDocument doc_;
};

class GDD
{
public:
	GDD(std::string const & xml);
	~GDD();
	bool Load(std::string const & xml);
	bool Save();
	bool GetAllSiteInfo(std::vector<std::pair<std::string ,std::string > >& info);
	bool GetSiteSetting(std::string const & name,std::string& address,std::string& port);
	bool GetTableLocation(Location& location);
	bool GetSiteRange(TiXmlElement const * site,std::string& var,std::string& low,std::string& le,std::string& up,std::string& ue);
	bool IsFragFeild(TiXmlElement const * table,std::string const & var);
	
private:
	TiXmlElement const * GetTableInfo(std::string const & name);  
	int StringToInt(std::string const & str);
	bool vaild;
	TiXmlDocument doc_;

};

//********************************************************************************


//typedef bool (* ComFunc)(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);

class CProcessor
{
	typedef bool (CProcessor::* ComFunc)(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	
public:
	CProcessor(std::string const & xml);
	bool SplitCom(std::string const & com,CPRes& res);
	//void AddExpression(std::string const & expression);
private:
	bool DefineSite(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool CreateDB(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool CreateTable(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool Fragment(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool Allocate(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool Insert(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool Delete(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool Import(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool Select(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool DropTable(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool DropDB(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res);
	bool AddSelectFeild(CPRes& res,std::string const & table,std::string const & feild);
	

	typedef std::map<std::string,std::pair<ComFunc,std::vector<boost::regex > > > regexmap;
	regexmap regexs;
	
};

//***********************************************************************************

//class ResultProcessor
//{
//
//};

/*

class GlobalManager
{
public:
	GlobalManager(GDD& gdd);
	bool SetGDD(GDD& gdd);
	bool DistributeQuery(QORes const & qores,QueryRes& queryres);
private:
	
	GDD& gdd_;
	CProcessor cprocessor_;
	QueryOptimize qoptimize_;
	ResultProcessor rprocessor_;
};

*/

//************************************************************

class LDD
{
public:
	LDD(std::string const & xml);
	LDD(){};
	~LDD();
	bool Load(std::string const & xml);
	bool Save();
	bool GetSqlSetting(std::string const & name,std::string& value);
	bool GetAppSetting(std::string const & name,std::string& value);
private:
	bool vaild;
	TiXmlDocument doc_;
};

class LocalManager
{
public:
	LocalManager(LDD& ldd);
	//bool Query

private:
	LDD& ldd_;

};

//************************************************************

class LocalAgent
{
	typedef bool (LocalAgent::* RevCmdFunc)(SessionPtr senssion,CPRes& res);
	typedef std::map<std::string,RevCmdFunc > RevFuncmap;
	typedef bool (LocalAgent::* SendCmdFunc)(CPRes& res);
	typedef std::map<std::string,SendCmdFunc > SendFuncmap;
	typedef std::list<ClientPtr> ClientList;
	typedef std::list<DataPtr> DataList;
public:
	LocalAgent();
	bool Start();
private:
	//bool DistributeCmd(CPRes& res);
	bool ExecuteCmd(CPRes& res);
	bool QueryOptimize(CPRes& res);
	//bool AddQueryInfo(CPRes& res);
	bool ReceiveCmd(); 
	//bool ExecuteCmd(std::string cmd);
	void StartLocalServer();
	void StartRevLocalCmd(std::istream& in);
	bool SendDefineSite(CPRes& res);
	bool SendCreateDB(CPRes& res);
	bool SendCreateTable(CPRes& res);
	bool SendFragment(CPRes& res);
	//bool SendAllocate(CPRes& res);
	bool SendInsert(CPRes& res);
	bool SendDelete(CPRes& res);
	bool SendImport(CPRes& res);
	bool SendSelect(CPRes& res);
	bool SendDropTable(CPRes& res);
	bool SendDropDB(CPRes& res);
	bool RevDefineSite(SessionPtr senssion,CPRes& res);
	bool RevCreateDB(SessionPtr senssion,CPRes& res);
	bool RevCreateTable(SessionPtr senssion,CPRes& res);
	bool RevFragment(SessionPtr senssion,CPRes& res);
	//bool RevAllocate(SessionPtr senssion,CPRes& res);
	bool RevInsert(SessionPtr senssion,CPRes& res);
	bool RevDelete(SessionPtr senssion,CPRes& res);
	bool RevImport(SessionPtr senssion,CPRes& res);
	bool RevSelect(SessionPtr senssion,CPRes& res);
	bool RevDropTable(SessionPtr senssion,CPRes& res);
	bool RevDropDB(SessionPtr senssion,CPRes& res);
	void ResultProcess(DataList& datalist,CPRes& res);
	//bool AddSelectFeild(CPRes& res,std::string const & table,std::string const & feild);
	void BroadCast(char const * data,unsigned size);
	CProcessor cprocessor_;
	GDD gdd_;
	LDD ldd_;
	SendFuncmap sendfuncmap_;
	RevFuncmap revfuncmap_;
};

#endif //LOCALAGNET_INCLUDE