#ifndef LOCALAGNET_INCLUDE
#define LOCALAGNET_INCLUDE
#define OPTIMIZE
#include <string>
#include <map>
#include <vector>
#include <set>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <mysql++.h>
#include "tinyxml.h"


//****************************************************************************

template <typename T >
inline void Release(T *p)
{
	delete [] p;
}

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

#define DATABUFFERSIZE 1200

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
		data_=new char[size_+1];
		memset(data_,0,size_+1);
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
		data_=new char[size_+1];
		data_[size_]='\0';
		memcpy(data_,data.data_,size_);
	}
	virtual ~Data()
	{
		delete[] data_;
	}
	Data& operator=(const Data& data)
	{
		delete[] data_;
		size_=data.size_;
		used=data.used;
		data_=new char[size_+1];
		data_[size_]='\0';
		memcpy(data_,data.data_,size_);
		return *this;
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
		size_=size;
		used=0;
		data_=new char[size_+1];
		memset(data_,0,size_+1);
		
	}

private:
	unsigned size_;
	unsigned used;
	char* data_;
};

typedef boost::shared_ptr<Data> DataPtr;

//**********************************************************************************

class CPRes:
	public TiXmlDocument,
	public boost::enable_shared_from_this<CPRes>
{
public:
	CPRes():TiXmlDocument(){};
	virtual ~CPRes(){};
	std::string GetCmd()
	{
		TiXmlElement* root=RootElement();
		return root->Value();
	}
private:
};


typedef boost::shared_ptr<CPRes> CPResPtr;

class Condition:
	public TiXmlElement
{
public:
	virtual ~Condition(){};
private:
};

class QueryRes;

class MyRow:
	public std::vector<std::string >
{
public:
	MyRow(const std::vector<char >& key):keyflag_(key)
	{
	}
	MyRow(const MyRow& row)
	{
		*this=row;
	}
	MyRow& operator=(const MyRow& row)
	{
		std::vector<std::string >::operator=(row);
		keyflag_=row.keyflag_;
		return *this;
	}
	virtual ~MyRow(){};
	bool operator<(const MyRow& row)
	{
		std::size_t pos=0;
		for(;pos<this->size();++pos)
		{
			if(keyflag_[pos]==1)
			{
				int cmp=(*this)[pos].compare(row[pos]);
				if(cmp<0) return true;
				else if(cmp>0) return false;

			}

		}
		return false;
	}
	
	

private:
	std::vector<char > keyflag_;
	 //std::map<std::string ,std::vector<std::string > >& column_;
};

//typedef boost::shared_ptr<MyRow> MyRowPtr;

//typedef std::vector<MyRowPtr> MyQueryRes;
class GDD;

class LocalAgent;

class MyQueryRes:
	public std::vector<MyRow >,public boost::enable_shared_from_this<MyQueryRes >
{
	typedef std::pair<std::string ,std::string > RowPair;
	typedef std::pair<std::string ,std::vector<std::string > > ColumnPair;
	typedef std::vector<MyRow > Row;
	typedef std::map<std::string ,std::vector<std::string > > Column;
public:
	MyQueryRes(){};
	MyQueryRes(QueryRes& res,GDD* gdd);
	MyQueryRes(GDD* gdd,std::vector<std::string >& feilds);
	MyQueryRes(const MyQueryRes& res):std::vector<MyRow >(res),gdd_(res.gdd_)
	{
		//std::vector<MyRow >::operator =(res);
		feilds_=res.feilds_;
		rows_=res.rows_;
		columns_=res.columns_;
		keyflag_=res.keyflag_;
		feildlen_=res.feildlen_;
		tablename_=res.tablename_;
		gdd_=res.gdd_;
	}
	MyQueryRes& operator=(const MyQueryRes& res)
	{
		std::vector<MyRow >::operator =(res);
		feilds_=res.feilds_;
		rows_=res.rows_;
		columns_=res.columns_;
		keyflag_=res.keyflag_;
		feildlen_=res.feildlen_;
		tablename_=res.tablename_;
		gdd_=res.gdd_;

	}
	void CopyAttr(const MyQueryRes& res)
	{
		feilds_=res.feilds_;
		rows_=res.rows_;
		columns_=res.columns_;
		keyflag_=res.keyflag_;
		feildlen_=res.feildlen_;
		tablename_=res.tablename_;
		gdd_=res.gdd_;
	}

	
	virtual ~MyQueryRes(){};
	void Join(MyQueryRes& res);
	void Union(MyQueryRes& res);
	void Projection(std::vector<std::string >& feilds);
	friend class QueryRes;
	friend class LocalAgent;
private:
	bool IsKey(int index);
	void SetKeyFlag();
	int rows_;
	int columns_;
	//Row row_;
	//std::vector<MyRow >::iterator iter_;
	//Column column_;
	std::vector<std::string > feilds_;
	//std::set<std::string > keyfeilds_;
	std::vector<char > keyflag_;  //1 key,0 not key
	std::vector<int> feildlen_;
	std::string tablename_;
	GDD* gdd_;
};

typedef boost::shared_ptr<MyQueryRes> MyQueryResPtr;

class QueryRes:
	public TiXmlDocument,public boost::enable_shared_from_this<QueryRes>
{
public:
	QueryRes(DataPtr data);
	QueryRes(std::vector<std::string >& feilds,bool success,std::string& type,std::string& table=std::string(""),int rows=0,int columns=0);
	QueryRes(bool success,std::string& type,std::string& table=std::string(""),int rows=0,int columns=0);
	virtual ~QueryRes(){};
	void Insert(std::vector<std::string >& value);
	//void ConvertToMyQueryRes(MyQueryRes& res);
	//void Join(QueryRes const & x);
	//void Union(QueryRes const & x);
	//void Projection(std::string const x[]);
	//bool Empty();
	//int Rows();
	//int Columns();
	bool Empty()
	{
		return nodata_;
	}
	bool Ok()
	{
		return success_;
	}
	friend class MyQueryRes;
private:
	bool success_;
	bool nodata_;
	int rows_;
	int columns_;
	std::string tablename_;

	std::vector<std::string > feilds_;
};

typedef boost::shared_ptr<QueryRes> QueryResPtr;

class RevCmd:
	public TiXmlDocument
{
public:
	RevCmd(Data& data);
	RevCmd():TiXmlDocument(){};
	virtual ~RevCmd(){};
	const std::string& GetCmdType(); 
private:
	std::string cmdtype_;

};



//**********************************************************************************

class Client:
	public boost::enable_shared_from_this<Client>
{
public:
	
	Client(std::string const & address, std::string const & port,char* data,unsigned total,DataPtr res);
	virtual ~Client();
	void Write(char* data,unsigned size,unsigned total,DataPtr res);
	//void Read(datapack
	void Read(DataPtr res);
	void Close();
	char const * GetData();
	void Run();
private:
	void HandleConnect(boost::system::error_code const & error,boost::asio::ip::tcp::resolver::iterator& endpoint_iterator);
	void DoWrite(char* data,unsigned size,unsigned total,DataPtr res);
	void HandleWrite(boost::system::error_code const & error,std::size_t count,char* data,unsigned size,unsigned total,DataPtr res);
	void HandleKeepRead(boost::system::error_code const & error,std::size_t count,DataPtr res);
	void DoRead(DataPtr res);
	void HandleReadData(boost::system::error_code const & error,std::size_t count,DataPtr res);
	void DoClose();
	char* senddata_;
	unsigned total_;
	unsigned sent_;
	DataPtr result_;
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

class Server;

class Session;

typedef boost::shared_ptr<Session> SessionPtr;

class Session:
	public boost::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_service& io_service,Server& server);
	virtual ~Session();
	boost::asio::ip::tcp::socket& Socket();
	void Start(boost::function<bool (SessionPtr, Data &cmd ) > handler);
	void Write(char* data,unsigned size,unsigned total);
	//void Read(boost::function<bool (Data*& ) > handler);
	//void Stop();
	void Close();
	void Print();
private:
	//void DoWrite(char* data,unsigned size,unsigned total);
	void HandleWrite(boost::system::error_code const & error,std::size_t,char* data,unsigned size,unsigned total);
	void HandleKeepRead(boost::system::error_code const & error,std::size_t count,boost::function<bool (SessionPtr, Data &cmd ) > handler);
	//void DoRead();
	void HandleReadData(boost::system::error_code const & error,std::size_t count,boost::function<bool (SessionPtr, Data &cmd ) > handler);
	void DoClose();
	boost::asio::ip::tcp::socket socket_;
	datapack buffer_;
	Data data_;
	char* senddata_;
	Server& server_;
	//unsigned total_;
	//unsigned sent_;

};




//******************************************************************************************

class Server
{
public:
	Server();
	Server(unsigned thread_pool_size,unsigned short port,boost::function<bool (SessionPtr, Data &cmd ) > handler);
	//void HandleAccept(Session& session,const boost::system::error_code& error);
	void CreateServer(unsigned thread_pool_size,unsigned short port,boost::function<bool (SessionPtr, Data &cmd ) > handler);
	void HandleAccept(SessionPtr session,const boost::system::error_code& error,boost::function<bool (SessionPtr, Data &cmd ) > handler);
	void Run();
	void CloseAll();
	void Close(SessionPtr s);
private:
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::endpoint endpoint_;
	unsigned thread_pool_size_;
	std::set<SessionPtr> sessions_;
};

class GDD;

class LocalAgent;

typedef std::pair<std::string ,std::string > DesInfo;
typedef std::list<DesInfo > DesList;

class Location
{
	
public:
	Location(std::string const & tablename,std::string const & var,std::string const & low,std::string const & le,std::string const & up,std::string const & ue);
	bool GetDes(GDD& gdd, std::list<DesInfo >& deslist);
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
	//bool Save();
	bool GetTableFeilds(std::string& tablename,std::vector<std::string >& feilds);
	bool GetKeyFeilds(std::string& tablename,std::set<std::string >& feilds);
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
	bool SetSqlSetting(std::string const & name,std::string& value);
	bool SetAppSetting(std::string const & name,std::string& value);
	bool GetAllMemoryTable(std::vector<std::string >& tables);

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

enum SubSelectionFlag
{
	opt,
	noneopt
};

class SubSelection:
	public TiXmlDocument,
	public boost::enable_shared_from_this<SubSelection>
{
	
	typedef std::list<DesInfo > DesList;
public:
	SubSelection(GDD& gdd,std::string& tablename,CPRes& res);
	DesList& GetDesList();
	bool IsVaild(){return vaild;}

private:
	TiXmlElement* GetCondition(std::string tablename,CPRes& res);
	bool GetDes();
	bool vaild;
	std::string tablename_;
	GDD& gdd_;
	DesList deslist_;
};

typedef boost::shared_ptr<SubSelection> SubSelectionPtr;

//************************************************************


class LocalAgent
{
	
	typedef bool (LocalAgent::* RevCmdFunc)(SessionPtr senssion, RevCmd &cmd);
	typedef std::map<std::string,RevCmdFunc > RevFuncmap;
	typedef bool (LocalAgent::* SendCmdFunc)(CPRes& res);
	typedef std::map<std::string,SendCmdFunc > SendFuncmap;
	typedef std::list<ClientPtr> ClientList;
	typedef std::list<DataPtr> DataList;
	
	typedef std::list<SubSelectionPtr > SubSelectionList;
public:
	LocalAgent();
	void Start();
	
private:
	
	bool ExecuteCmd(SessionPtr senssion, Data &cmd);
	
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
	bool RevDefineSite(SessionPtr senssion, RevCmd &cmd);
	bool RevCreateDB(SessionPtr senssion, RevCmd &cmd);
	bool RevCreateTable(SessionPtr senssion, RevCmd &cmd);
	bool RevFragment(SessionPtr senssion, RevCmd &cmd);
	//bool RevAllocate(SessionPtr senssion,CPRes& res);
	bool RevInsert(SessionPtr senssion, RevCmd &cmd);
	bool RevDelete(SessionPtr senssion, RevCmd &cmd);
	bool RevImport(SessionPtr senssion, RevCmd &cmd);
	bool RevSelect(SessionPtr senssion, RevCmd &cmd);
	bool RevDropTable(SessionPtr senssion, RevCmd &cmd);
	bool RevDropDB(SessionPtr senssion, RevCmd &cmd);
	void ResultProcess(DataList& datalist,CPRes& res);
	bool ConnectToSql();
	bool CreateTable(std::string& tablename,std::vector<std::pair<char,std::string> >& feilds,const std::string& engine);
	bool ToSelectString(TiXmlDocument* xmlselect,std::string& strselect);
	void Print(mysqlpp::StoreQueryResult& res);
	bool ClearMemoryDB();
	//bool AddSelectFeild(CPRes& res,std::string const & table,std::string const & feild);
	Server server_;
	CProcessor cprocessor_;
	mysqlpp::Connection conn_;
	boost::mutex mutex_;
	GDD gdd_;
	LDD ldd_;
	SendFuncmap sendfuncmap_;
	RevFuncmap revfuncmap_;
	bool connected;
};

#endif //LOCALAGNET_INCLUDE