#include "LocalAgent.h"
#include <limits>
#include <fstream>
#include <sstream>

//*******************************************************************************************************

Client::Client(const std::string &address, const std::string &port):
	  io_service_(),socket_(io_service_),resolver_(io_service_),query_(address,port),data_()
{
	
	iterator_=resolver_.resolve(query_);
	boost::asio::ip::tcp::endpoint endpoint = *iterator_;
    socket_.async_connect(endpoint,
        boost::bind(&Client::HandleConnect, this,
          boost::asio::placeholders::error,++iterator_ ));
	memset(&buffer_,0,sizeof(buffer_));
	//int x;

}


void Client::HandleConnect(const boost::system::error_code &error, boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
	
    if (error && endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      socket_.close();
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
          boost::bind(&Client::HandleConnect, this,
            boost::asio::placeholders::error, ++endpoint_iterator));
	}

}


void Client::Run()
{
	io_service_.run();
}

void Client::Write(char *data, unsigned int size, unsigned int total,Data& res)
{
	data_.Assigen(total);
	data_.AddData(data,total);
	io_service_.post(boost::bind(&Client::DoWrite,this,(char*)data_.GetData(),size,total,res));
}

void Client::Read(Data& res)
{
	io_service_.post(boost::bind(&Client::DoRead,this,res));

}

void Client::Close()
{
	io_service_.post(boost::bind(&Client::DoClose,this));
}



void Client::DoRead(Data& res)
{
	boost::asio::async_read(socket_,
		boost::asio::buffer(buffer_.data,DATABUFFERSIZE),
		boost::bind(&Client::HandleKeepRead, this,
		boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,res));
}

void Client::HandleKeepRead(const boost::system::error_code &error,std::size_t count,Data& res)
{
	if (!error)
    {
		data_.Assigen(buffer_.total);
		data_.AddData(buffer_.data,buffer_.len);
		if(buffer_.flage==1)
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(buffer_.data, DATABUFFERSIZE),
			boost::bind(&Client::HandleReadData, this,
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,res));
		}
		else
		{
			//handler(data_);
			DoClose();
		}
		
    }
    else
    {
      DoClose();
    }


}

void Client::HandleReadData(const boost::system::error_code &error,std::size_t count,Data& res)
{
	if (!error)
    {
		if(!data_.AddData(buffer_.data,buffer_.len)) DoClose();
		if(buffer_.flage==1)
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(buffer_.data, DATABUFFERSIZE),
			boost::bind(&Client::HandleReadData, this,
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,res));
		}
		else
		{
			//handler(data_);
			DoClose();
		}
		
    }
    else
    {
      DoClose();
    }

}

void Client::DoWrite(char *data, unsigned int size, unsigned int total,Data& res)
{
	if(size<=DATABUFFERSIZE)
	{
		buffer_.total=total;
		buffer_.len=size;
		buffer_.flage=0;
		memcpy(buffer_.data,&data[total-size],size);
	}
	else
	{
		buffer_.total=total;
		buffer_.len=DATABUFFERSIZE;
		buffer_.flage=1;
		memcpy(buffer_.data,&data[total-size],DATABUFFERSIZE);

	}
	 
	boost::asio::async_write(socket_,
		 boost::asio::buffer(&buffer_,sizeof(buffer_)),
		 boost::bind(&Client::HandleWrite, this,
		 boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,data,size-buffer_.len,total,res));

}

void Client::HandleWrite(const boost::system::error_code &error,std::size_t count, char *data, unsigned int size, unsigned int total,Data& res)
{
	if(!error)
	{
		if(size==0)
		{
			Read(res);
			return;
		}
		if(size<DATABUFFERSIZE)
		{
			buffer_.total=total;
		    buffer_.len=size;
		    buffer_.flage=0;
		    memcpy(buffer_.data,&data[total-size],size);
		}
		else
		{
			buffer_.total=total;
			buffer_.len=DATABUFFERSIZE;
			buffer_.flage=1;
			memcpy(buffer_.data,&data[total-size],DATABUFFERSIZE);
		}
		boost::asio::async_write(socket_,
			boost::asio::buffer(&buffer_,sizeof(buffer_)),
            boost::bind(&Client::HandleWrite, this,
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,data,size-buffer_.len,total,res));
	}
	else
	{
		DoClose();
	}
}

void Client::DoClose()
{
	socket_.close();
	io_service_.stop();
}

char const * Client::GetData()
{
	 return data_.GetData();
}

Client::~Client()
{
	DoClose();
}


//********************************************************************


Server::Server(unsigned thread_pool_size,unsigned short port,boost::function<bool (Data& ) > handler):
	  io_service_(),thread_pool_size_(thread_pool_size),endpoint_(boost::asio::ip::tcp::v4(),port),acceptor_(io_service_)
		 
{
	acceptor_.open(endpoint_.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint_);
	acceptor_.listen();

	SessionPtr newsession(new Session(io_service_));
	acceptor_.async_accept(newsession->Socket(),
		boost::bind(&Server::HandleAccept, this, newsession,
		boost::asio::placeholders::error,handler));

}

void Server::HandleAccept(SessionPtr session, const boost::system::error_code &error,boost::function<bool (Data& ) > handler)
{
	if (!error)
    {
      session->Start(handler);
      //chat_session_ptr new_session(new chat_session(io_service_, room_));
	  SessionPtr newsession(new Session(io_service_));
      acceptor_.async_accept(newsession->Socket(),
		  boost::bind(&Server::HandleAccept, this, newsession,
          boost::asio::placeholders::error,handler));
    }
}

void Server::Run()
{
	std::vector<boost::shared_ptr<boost::thread> > threads;
	for (std::size_t i = 0; i < thread_pool_size_; ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&boost::asio::io_service::run, &io_service_)));
		threads.push_back(thread);
	}
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
}

void Server::Stop()
{
	io_service_.stop();
}

//***********************************************************************************

Session::Session(boost::asio::io_service &io_service):socket_(io_service)
{
	memset(&buffer_,0,sizeof(buffer_));
}

boost::asio::ip::tcp::socket& Session::Socket()
{
	return socket_;
}

void Session::Write(char* data,unsigned size,unsigned total)
{
	if(size<=DATABUFFERSIZE)
	{
		buffer_.total=total;
		buffer_.len=size;
		buffer_.flage=0;
		memcpy(buffer_.data,&data[total-size],size);
	}
	else
	{
		buffer_.total=total;
		buffer_.len=DATABUFFERSIZE;
		buffer_.flage=1;
		memcpy(buffer_.data,&data[total-size],DATABUFFERSIZE);

	}
	 
	boost::asio::async_write(socket_,
		 boost::asio::buffer(&buffer_,sizeof(buffer_)),
		 boost::bind(&Session::HandleWrite, this,
		 boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,data,size-buffer_.len,total));
}

void Session::HandleWrite(const boost::system::error_code &error,std::size_t count, char *data, unsigned int size, unsigned int total)
{
	if(!error)
	{
		if(size==0)
		{
			
			DoClose();
			return;
		}
		if(size<DATABUFFERSIZE)
		{
			buffer_.total=total;
		    buffer_.len=size;
		    buffer_.flage=0;
		    memcpy(buffer_.data,&data[total-size],size);
		}
		else
		{
			buffer_.total=total;
			buffer_.len=DATABUFFERSIZE;
			buffer_.flage=1;
			memcpy(buffer_.data,&data[total-size],DATABUFFERSIZE);
		}
		boost::asio::async_write(socket_,
			boost::asio::buffer(&buffer_,sizeof(buffer_)),
            boost::bind(&Session::HandleWrite, this,
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,data,size-buffer_.len,total));
	}
	else
	{
		DoClose();
	}
}



void Session::Start(boost::function<bool (Data& ) > handler)
{
	//cout<<"New session"<<endl;
	boost::asio::async_read(socket_,
		boost::asio::buffer(&buffer_, sizeof(buffer_)),
		boost::bind(
          &Session::HandleKeepRead, shared_from_this(),
          boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,handler));
}

void Session::HandleKeepRead(const boost::system::error_code &error,std::size_t,boost::function<bool (Data& ) > handler)
{
	if (!error)
    {
		data_.Assigen(buffer_.total);
		data_.AddData(buffer_.data,buffer_.len);
		if(buffer_.flage==1)
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(buffer_.data, DATABUFFERSIZE),
			boost::bind(&Session::HandleReadData, shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,handler));
		}
		else
		{
			//命令处理，发送查询数据
			if(handler(data_))
			{

			}
			else DoClose();
		}
		
    }
    else
    {
		DoClose();
      
    }

}

void Session::HandleReadData(boost::system::error_code const & error,std::size_t,boost::function<bool (Data& ) > handler)
{
	if (!error)
    {
		if(!data_.AddData(buffer_.data,buffer_.len)) DoClose();
		if(buffer_.flage==1)
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(buffer_.data, DATABUFFERSIZE),
			boost::bind(&Session::HandleReadData, shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,handler));
		}
		else
		{
			//命令处理，发送查询数据
			if(handler(data_))
			{

			}
			else DoClose();
		}
		
    }
    else
    {
      DoClose();
    }

}

void Session::Close()
{
	DoClose();
}

void Session::DoClose()
{
	socket_.close();
}

Session::~Session()
{
	DoClose();
}

void Session::Print()
{
	//cout<<data_.GetData()<<endl;
}

//*****************************************************************************

#define NEWE(p) new TiXmlElement((p))
#define NEWD(p1,p2,p3) new TiXmlDeclaration((p1),(p2),(p3))
#define NEWC(p) new TiXmlComment((p))
#define NEWT(p) new TiXmlText((p))

bool CProcessor::DefineSite(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("definesite");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("DefineSite");
	res.LinkEndChild(root);
	TiXmlElement* element=NEWE("SiteName");
	element->LinkEndChild(NEWT(what[1].str()));
	root->LinkEndChild(element);
	element=NEWE("Address");
	element->LinkEndChild(NEWT(what[2].str()));
	root->LinkEndChild(element);
	element=NEWE("Port");
	element->LinkEndChild(NEWT(what[3].str()));
	root->LinkEndChild(element);
	return true;
}

bool CProcessor::CreateDB(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("createdb");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("CreateDB");
	res.LinkEndChild(root);
	TiXmlElement* element=NEWE("DBName");
	element->LinkEndChild(NEWT(what[1].str()));
	root->LinkEndChild(element);
	return true;
}

bool CProcessor::CreateTable(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("createtable");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("CreateTable");
	root->SetAttribute("name",what[1].str());
	res.LinkEndChild(root);
	std::string substr=what[2].str();
	std::string::const_iterator at=substr.begin();
	std::string::const_iterator end=substr.end();
	while(boost::regex_search(at,end,what,reg[1]))
	{
		TiXmlElement* element=NEWE("Field");
		element->SetAttribute("key",(what[6].str().compare("key")==0 ? "yes" : "no"));
		element->SetAttribute("type",what[2].str());
		element->SetAttribute("count",(what[4].str().compare("")==0 ? "1" : what[4].str()));
		element->LinkEndChild(NEWT(what[1].str()));
		root->LinkEndChild(element);
		at=what[0].second;

	}
	return true;
}

bool CProcessor::Fragment(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	return true;
}
bool CProcessor::Allocate(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("allocate");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("Allocate");
	res.LinkEndChild(root);
	TiXmlElement* element=NEWE("FragName");
	element->LinkEndChild(NEWT(what[1].str()));
	root->LinkEndChild(element);
	element=NEWE("DesName");
	element->LinkEndChild(NEWT(what[2].str()));
	root->LinkEndChild(element);
	return true;
}

bool CProcessor::Insert(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("insert");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("Insert");
	root->SetAttribute("tablename",what[1].str());
	res.LinkEndChild(root);
	std::string substr1=what[2].str();
	std::string substr2=what[4].str();
	std::string::const_iterator at=substr1.begin();
	std::string::const_iterator end=substr1.end();
	while(boost::regex_search(at,end,what,reg[1]))
	{
		TiXmlElement* element=NEWE("Field");
		element->SetAttribute("name",what[0].str());
		root->LinkEndChild(element);
		at=what[0].second;
	}
	at=substr2.begin();
	end=substr2.end();
	TiXmlElement* element=root->FirstChildElement();
	while(boost::regex_search(at,end,what,reg[2]))
	{
		element->LinkEndChild(NEWT(what[0].str()));
		at=what[0].second;
		element=element->NextSiblingElement();
	}
	return true;
}

bool CProcessor::Delete(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("delete");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("Delete");
	res.LinkEndChild(root);
	TiXmlElement* element=NEWE("Condition");
	root->LinkEndChild(element);
	TiXmlElement* son=NEWE("Left");
	son->LinkEndChild(NEWT(what[2].str()));
	element->LinkEndChild(son);
	son=NEWE("Operator");
	son->LinkEndChild(NEWT(what[3].str()));
	element->LinkEndChild(son);
	son=NEWE("Right");
	son->LinkEndChild(NEWT(what[4].str()));
	element->LinkEndChild(son);
	return true;
}

bool CProcessor::Import(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("import");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("Import");
	res.LinkEndChild(root);
	TiXmlElement* element=NEWE("File");
	element->LinkEndChild(NEWT(what[1].str()));
	root->LinkEndChild(element);
	element=NEWE("DataBase");
	element->LinkEndChild(NEWT(what[2].str()));
	root->LinkEndChild(element);
	return true;
}

bool CProcessor::AddSelectFeild(CPRes &res,const std::string &table,const std::string &feild)
{
	TiXmlElement* root=res.RootElement();
	TiXmlElement* stable=root->FirstChildElement("Table");
	while(stable!=NULL)
	{
		if(table.compare(stable->Attribute("name"))==0)
		{
			TiXmlElement* sfeild=stable->FirstChildElement("Feild");
			std::string all="*";
			if(all.compare(sfeild->GetText())==0) return true;
			while(sfeild!=NULL)
			{
				if(feild.compare(sfeild->GetText())!=0) sfeild=sfeild->NextSiblingElement("Feild");
				else return true;
			}
			TiXmlElement* tempfeild=NEWE("Feild");
			tempfeild->LinkEndChild(NEWT(feild));
			stable->LinkEndChild(tempfeild);
			return true;
		}
		else stable=stable->NextSiblingElement("Table");
	}
	return false;

}

bool CProcessor::Select(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("select");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("Select");
	res.LinkEndChild(root);
	std::string substr1=what[1].str();
	std::string substr2=what[2].str();
	std::string substr3=what[4].str();
	std::string::const_iterator at=substr2.begin();
	std::string::const_iterator end=substr2.end();
	while(boost::regex_search(at,end,what,reg[2]))
	{
		TiXmlElement* element=NEWE("Table");
		element->SetAttribute("name",what[0].str());
		root->LinkEndChild(element);
		at=what[0].second;
	}
	at=substr1.begin();
	end=substr1.end();
	while(boost::regex_search(at,end,what,reg[1]))
	{
		if(what[0].str().compare("*")==0)
		{
			TiXmlElement* element=root->FirstChildElement("Table");
			while(element!=NULL)
			{
				TiXmlElement* son=NEWE("Feild");
				son->SetAttribute("type","projection");
				son->LinkEndChild(NEWT("*"));
				element->LinkEndChild(son);
				element=element->NextSiblingElement("Table");
			}
		}
		else
		{
			
			if(!(what[1].matched)) return false;
			TiXmlElement* element=root->FirstChildElement("Table");
			while(element!=NULL)
			{
				std::string name=element->Attribute("name");
				if(name.compare(what[2].str())==0)
				{
					TiXmlElement* son=NEWE("Feild");
					son->SetAttribute("type","projection");
					son->LinkEndChild(NEWT(what[3].str()));
					element->LinkEndChild(son);
					break;
				}
				else
				{
					element=element->NextSiblingElement("Table");
				}
			}
		}
		at=what[0].second;
	}
	if(substr3.compare("")!=0)
	{
		at=substr3.begin();
		end=substr3.end();
		while(boost::regex_search(at,end,what,reg[3]))
		{
			TiXmlElement* element=NEWE("Condition");
			TiXmlElement* son=NEWE("Left");
			son->SetAttribute("type","feild");
			son->SetAttribute("table",what[2].str());
			son->LinkEndChild(NEWT(what[3].str()));
			element->LinkEndChild(son);
			AddSelectFeild(res,what[2].str(),what[3].str());
			son=NEWE("Operator");
			son->LinkEndChild(NEWT(what[4].str()));
			element->LinkEndChild(son);
			son=NEWE("Right");
			if(what[5].matched)
			{
				son->SetAttribute("type","Feild");
				son->SetAttribute("table",what[6].str());
			}
			else
			{
				son->SetAttribute("type","value");
			}
			son->LinkEndChild(NEWT(what[7].str()));
			element->LinkEndChild(son);
			AddSelectFeild(res,what[6].str(),what[7].str());
			root->LinkEndChild(element);
			at=what[0].second;
		}
	}
	return true;
}
bool CProcessor::DropTable(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("droptable");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("DropTable");
	TiXmlElement* element=NEWE("TableName");
	element->LinkEndChild(NEWT(what[1].str()));
	root->LinkEndChild(element);
	res.LinkEndChild(root);
	return true;
}

bool CProcessor::DropDB(std::string const & com,std::vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("dropdb");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("DropDB");
	TiXmlElement* element=NEWE("DBName");
	element->LinkEndChild(NEWT(what[1].str()));
	root->LinkEndChild(element);
	res.LinkEndChild(root);
	return true;
}


CProcessor::CProcessor(const std::string& xml)
{
	typedef std::pair<ComFunc,std::vector<boost::regex > > Pair1;
	typedef std::pair<std::string,Pair1 > Pair2;
	
	TiXmlDocument doc(xml);
	doc.LoadFile();
	//TiXmlHandle hdoc(&doc);
	TiXmlElement* root=doc.FirstChildElement();//hdoc.FirstChildElement().ToElement();
	int expressions;
	root->Attribute("count",&expressions);
	TiXmlElement* element=root->FirstChildElement();
	while(element!=NULL)
	{
		std::string name=element->Attribute("name");
		std::vector<boost::regex > regs;
		TiXmlElement* son=element->FirstChildElement();
		while(son!=NULL)
		{
			boost::regex reg(son->GetText(),boost::regex::icase);
			regs.push_back(reg);
			son=son->NextSiblingElement();
		}
		do
		{
			if(name.compare("definesite")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::DefineSite,regs)));
				break;
			}
			if(name.compare("createdb")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::CreateDB,regs)));
				break;
			}
			if(name.compare("createtable")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::CreateTable,regs)));
				break;
			}
			if(name.compare("definesite")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::DefineSite,regs)));
				break;
			}
			if(name.compare("fragment")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::Fragment,regs)));
				break;
			}
			if(name.compare("allocate")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::Allocate,regs)));
				break;
			}
			if(name.compare("insert")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::Insert,regs)));
				break;
			}
			if(name.compare("delete")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::Delete,regs)));
				break;
			}
			if(name.compare("import")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::Import,regs)));
				break;
			}
			if(name.compare("select")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::Select,regs)));
				break;
			}
			if(name.compare("droptable")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::DropTable,regs)));
				break;
			}
			if(name.compare("dropdb")==0)
			{
				regexs.insert(Pair2(name,Pair1(&CProcessor::DropDB,regs)));
				break;
			}
		}while(false);
		element=element->NextSiblingElement();
	}
	
}

bool CProcessor::SplitCom(const std::string &com, CPRes &res)
{
	std::string name=com.substr(0,com.find_first_of(" "));
	regexmap::iterator iter;
	iter=regexs.find(name);
	if(iter==regexs.end()) return false;
	ComFunc func=iter->second.first;
	return (this->*func)(com,iter->second.second,res);
}
//*******************************************************************************

LDD::LDD(const std::string &xml)
{
	
	if(doc_.LoadFile()) vaild=true;
	else vaild=false;
}

LDD::~LDD()
{
	if(vaild) doc_.SaveFile();
}

bool LDD::GetAppSetting(const std::string &name, std::string &value)
{
	if(!vaild) return false;
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* element=root->FirstChildElement("AppSetting");
	TiXmlElement* son=element->FirstChildElement(name);
	if(son==NULL) return false;
	value=son->GetText();
	return true;
}

bool LDD::GetSqlSetting(const std::string &name, std::string &value)
{
	if(!vaild) return false;
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* element=root->FirstChildElement("SqlSetting");
	TiXmlElement* son=element->FirstChildElement(name);
	if(son==NULL) return false;
	value=son->GetText();
	return true;
}

bool LDD::Load(const std::string &xml)
{
	if(doc_.LoadFile(xml)) return vaild=true;
	else return vaild=false;
	
}

bool LDD::Save()
{
	if(vaild) return doc_.SaveFile();
	return false;
}

//**********************************************************************

Location::Location(const std::string &tablename, const std::string &var, const std::string &low, const std::string &le, const std::string &up, const std::string &ue)
{
	//doc_();
	TiXmlDeclaration* decl=NEWD("1.0","","");
	doc_.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("location");
	doc_.LinkEndChild(comment);
	TiXmlElement* root=NEWE("Location");
	doc_.LinkEndChild(root);
	this->tablename=tablename;
	this->var=var;
	this->low=low;
	this->lowequal=le;
	this->up=up;
	this->upequal=ue;
	if(var.compare("all")==0) requestAll=true;
	else requestAll=false;
}

bool Location::Insert(const std::string &tablename, const std::string &var, const std::string &low, const std::string &le, const std::string &up, const std::string &ue, const std::string &site)
{
	TiXmlElement* table=NEWE("Table");
	table->SetAttribute("name",tablename);
	table->SetAttribute("site",site);
	doc_.RootElement()->LinkEndChild(table);
	TiXmlElement* range=NEWE("Range");
	range->SetAttribute("var",var);
	table->LinkEndChild(range);
	TiXmlElement* Low=NEWE("Low");
	Low->SetAttribute("equal",le);
	Low->LinkEndChild(NEWT(low));
	range->LinkEndChild(Low);
	TiXmlElement* Up=NEWE("Up");
	Up->SetAttribute("equal",ue);
	Up->LinkEndChild(NEWT(up));
	range->LinkEndChild(Up);
	return true;
	

}

void Location::Print()
{
	using namespace std;
	cout<<doc_<<endl;
}
//**********************************************************************

GDD::GDD(const std::string &xml)
{
	
	if(doc_.LoadFile(xml)) vaild=true;
	else vaild=false;
}

GDD::~GDD()
{
	if(vaild) doc_.SaveFile();
}

bool GDD::Load(const std::string &xml)
{
	if(doc_.LoadFile(xml)) return vaild=true;
	else return vaild=false;
}

bool GDD::GetAllSiteInfo(std::vector<std::pair<std::string ,std::string > >& info)
{
	typedef std::pair<std::string ,std::string > Pair;
	
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* element=root->FirstChildElement("SiteSetting");
	element=element->FirstChildElement("Site");
	while(element!=NULL)
	{
		info.push_back(Pair(element->Attribute("address"),element->Attribute("port")));
		element=element->NextSiblingElement("Site");
		//TiXmlElement* son=element->FirstChildElement("Name");
		//if(name.compare(son->GetText())==0)
		//{
		//	address=son->NextSiblingElement("Address")->GetText();
		//	port=son->NextSiblingElement("Port")->GetText();
		//	return true;
		//}
		//element=element->NextSiblingElement("Site");
	}
	return true;

}

bool GDD::GetSiteSetting(const std::string &name, std::string &address, std::string &port)
{
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* element=root->FirstChildElement("SiteSetting");
	element=element->FirstChildElement("Site");
	while(element!=NULL)
	{
		TiXmlElement* son=element->FirstChildElement("Name");
		if(name.compare(son->GetText())==0)
		{
			address=son->NextSiblingElement("Address")->GetText();
			port=son->NextSiblingElement("Port")->GetText();
			return true;
		}
		element=element->NextSiblingElement("Site");
	}
	return false;
}

TiXmlElement const * GDD::GetTableInfo(const std::string &name)
{
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* element=root->FirstChildElement("TableSetting");
	element=element->FirstChildElement("Table");
	while(element!=NULL)
	{
		if(name.compare(element->Attribute("name"))==0)
		{
			return element;
		}
		element=element->NextSiblingElement("Table");
	}
	return NULL;
}

bool GDD::GetSiteRange(const TiXmlElement *site,std::string &var,std::string &low,std::string &le,std::string &up,std::string &ue)
{
	const TiXmlElement *range=site->FirstChildElement("Range");
	var=range->Attribute("var");
	low=range->FirstChildElement("Low")->GetText();
	le=range->FirstChildElement("Low")->Attribute("equal");
	up=range->FirstChildElement("Up")->GetText();
	ue=range->FirstChildElement("Up")->Attribute("equal");
	return true;
}

bool GDD::IsFragFeild(const TiXmlElement *table,const std::string &var)
{
	const TiXmlElement* site=table->FirstChildElement("Site");
	while(site!=NULL)
	{
		TiXmlElement const * range=site->FirstChildElement("Range");
		if(var.compare(range->Attribute("var"))==0) return true;
		site=site->NextSiblingElement("Site");
	}
	return false;
}

bool GDD::GetTableLocation(Location &location)
{
	TiXmlElement const * gddtable=GetTableInfo(location.tablename);
	TiXmlElement const * site=gddtable->FirstChildElement("Site");
	if(location.requestAll || !IsFragFeild(gddtable,location.var))
	{
		while(site!=NULL)
		{
			location.Insert(location.tablename,"all","MIN","no","MAX","no",site->Attribute("name"));
			site=site->NextSiblingElement("Site");
		}
		return true;
	}
	else
	{
		std::string var,low,le,up,ue;
		while(site!=NULL)
		{
			GetSiteRange(site,var,low,le,up,ue);
			int gddlow=StringToInt(low);
			int gddup=StringToInt(up);
			int llow=StringToInt(location.low);
			int lup=StringToInt(location.up);
			if(llow>=gddup)
			{
				site=site->NextSiblingElement("Site");
				continue;
			}
			if(lup<gddup || up.compare("MAX")==0)
			{
				return location.Insert(location.tablename,location.var,location.low,location.lowequal,location.up,location.upequal,site->Attribute("name"));
			}
			else
			{
				location.Insert(location.tablename,location.var,location.low,location.lowequal,up,ue,site->Attribute("name"));
				location.low=up;
				location.lowequal="yes";
				site=site->NextSiblingElement("Site");
			}
		}
		return false;
	}
	
}




int GDD::StringToInt(const std::string &str)
{
#undef min()
#undef max()
	if(str.compare("MIN")==0) return std::numeric_limits<int>::min();
	if(str.compare("MAX")==0) return std::numeric_limits<int>::max();
	return atoi(str.c_str());
}



//************************************************************************

LocalAgent::LocalAgent():gdd_(".\\AppData\\GDD.xml"),ldd_(".\\AppData\\LDD.xml"),cprocessor_(".\\AppData\\regex.xml")
{
	typedef std::pair<std::string, SendCmdFunc > SendPair;
	typedef std::pair<std::string, RevCmdFunc > RevPair;
	sendfuncmap_.insert(SendPair("DefineSite",&LocalAgent::SendDefineSite));
	sendfuncmap_.insert(SendPair("CreateDB",&LocalAgent::SendCreateDB));
	sendfuncmap_.insert(SendPair("CreateTable",&LocalAgent::SendCreateTable));
	sendfuncmap_.insert(SendPair("Fragment",&LocalAgent::SendFragment));
	sendfuncmap_.insert(SendPair("Allocate",&LocalAgent::SendAllocate));
	sendfuncmap_.insert(SendPair("Insert",&LocalAgent::SendInsert));
	sendfuncmap_.insert(SendPair("Delete",&LocalAgent::SendDelete));
	sendfuncmap_.insert(SendPair("Import",&LocalAgent::SendImport));
	sendfuncmap_.insert(SendPair("Select",&LocalAgent::SendSelect));
	sendfuncmap_.insert(SendPair("DropTable",&LocalAgent::SendDropTable));
	sendfuncmap_.insert(SendPair("DropDB",&LocalAgent::SendDropDB));

	revfuncmap_.insert(RevPair("DefineSite",&LocalAgent::RevDefineSite));
	revfuncmap_.insert(RevPair("CreateDB",&LocalAgent::RevCreateDB));
	revfuncmap_.insert(RevPair("CreateTable",&LocalAgent::RevCreateTable));
	revfuncmap_.insert(RevPair("Fragment",&LocalAgent::RevFragment));
	revfuncmap_.insert(RevPair("Allocate",&LocalAgent::RevAllocate));
	revfuncmap_.insert(RevPair("Insert",&LocalAgent::RevInsert));
	revfuncmap_.insert(RevPair("Delete",&LocalAgent::RevDelete));
	revfuncmap_.insert(RevPair("Import",&LocalAgent::RevImport));
	revfuncmap_.insert(RevPair("Select",&LocalAgent::RevSelect));
	revfuncmap_.insert(RevPair("DropTable",&LocalAgent::RevDropTable));
	revfuncmap_.insert(RevPair("DropDB",&LocalAgent::RevDropDB));
}

void LocalAgent::StartLocalServer()
{
	//boost::shared_ptr<boost::thread> thread(new boost::thread(
	//		boost::bind(&boost::asio::io_service::run, &io_service_)));
	std::string lisenning;
	std::string size;
	ldd_.GetAppSetting("RevPort",lisenning);
	ldd_.GetAppSetting("ThreadPollSize",size);
	//Server server(atoi(size.c_str()),atoi(lisenning.c_str()));
	//server.Run();
	//Server server(10,

}

void LocalAgent::StartRevLocalCmd(std::istream &in)
{
	std::string cmd;
	while(true)
	{
		std::getline(in,cmd);
		CPRes res;
		cprocessor_.SplitCom(cmd,res);
	}
}

bool LocalAgent::QueryOptimize(CPRes &res)
{
	return true;

}

bool LocalAgent::ExecuteCmd(CPRes &res)
{
	return true;
}

bool LocalAgent::SendDefineSite(CPRes& res)
{
	std::string data;
	data<<res;
	TiXmlElement* root=res.RootElement();
	ClientPtr client(new Client(root->FirstChildElement("Address")->GetText(),root->FirstChildElement("Port")->GetText()));
	DataPtr result(new Data());
	client->Write((char*)data.c_str(),data.size(),data.size(),*result);
	boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
	thread->join();
	return true;
}

bool LocalAgent::SendCreateDB(CPRes& res)
{
	std::string data;
	data<<res;
	DataList datalist;
	ClientList clientlist;
	std::vector<std::pair<std::string,std::string > > sites;
	gdd_.GetAllSiteInfo(sites);
	std::vector<std::pair<std::string,std::string > >::iterator iter=sites.begin();
	std::vector<boost::shared_ptr<boost::thread> > threads;
	while(iter!=sites.end())
	{
		ClientPtr client(new Client(iter->first,iter->second));
		DataPtr result(new Data());
		client->Write((char*)data.c_str(),data.size(),data.size(),*result);
		clientlist.push_back(client);
		datalist.push_back(result);
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
		threads.push_back(thread);
		iter++;
	}
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();

	//返回结果处理
	return true;

}

bool LocalAgent::SendCreateTable(CPRes& res)
{
	std::string data;
	data<<res;
	DataList datalist;
	ClientList clientlist;
	std::vector<std::pair<std::string,std::string > > sites;
	gdd_.GetAllSiteInfo(sites);
	std::vector<std::pair<std::string,std::string > >::iterator iter=sites.begin();
	std::vector<boost::shared_ptr<boost::thread> > threads;
	while(iter!=sites.end())
	{
		ClientPtr client(new Client(iter->first,iter->second));
		DataPtr result(new Data());
		client->Write((char*)data.c_str(),data.size(),data.size(),*result);
		clientlist.push_back(client);
		datalist.push_back(result);
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
		threads.push_back(thread);
		iter++;
	}
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
	//返回结果处理

	return true;

}

bool LocalAgent::SendFragment(CPRes& res)
{
	return true;

}


bool LocalAgent::SendInsert(CPRes &res)
{
	TiXmlElement* root=res.RootElement();
	std::string table=root->Attribute("tablename");
	TiXmlElement* element=root->FirstChildElement("Feild");
	std::string feild=element->Attribute("name");
	std::string value=element->GetText();
	Location location(table,feild,value,"yes",value,"yes");
	gdd_.GetTableLocation(location);
	root=location.doc_.RootElement();
	element=root->FirstChildElement("Table");
	std::string address,port;
	gdd_.GetSiteSetting(element->Attribute("site"),address,port);
	std::string data;
	data<<res;
	ClientPtr client(new Client(address,port));
	DataPtr result(new Data());
	client->Write((char*)data.c_str(),data.size(),data.size(),*result);
	boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
	thread->join();

	//返回结果处理
	return true;

}

bool LocalAgent::SendDelete(CPRes &res)
{
	std::string data;
	data<<res;
	DataList datalist;
	ClientList clientlist;
	TiXmlElement* root=res.RootElement();
	std::string table=root->Attribute("tablename");
	TiXmlElement* element=root->FirstChildElement("Condition");
	TiXmlElement* son=element->FirstChildElement("Left");
	std::string feild=son->GetText();
	son=son->NextSiblingElement("Right");
	std::string value=son->GetText();
	Location location(table,feild,value,"yes",value,"yes");
	gdd_.GetTableLocation(location);
	root=location.doc_.RootElement();
	element=root->FirstChildElement("Table");
	std::vector<boost::shared_ptr<boost::thread> > threads;
	while(element!=NULL)
	{
		std::string address,port;
	    gdd_.GetSiteSetting(element->Attribute("site"),address,port);
		ClientPtr client(new Client(address,port));
	    DataPtr result(new Data());
	    client->Write((char*)data.c_str(),data.size(),data.size(),*result);
		clientlist.push_back(client);
		datalist.push_back(result);
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
		threads.push_back(thread);
		element=element->NextSiblingElement("Table");

	}
	
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();

	//返回结果处理
	return true;
}

bool LocalAgent::SendImport(CPRes &res)
{
	using namespace std;
	TiXmlElement* root=res.RootElement();
	std::string file,database;
	file=root->FirstChildElement("File")->GetText();
	database=root->FirstChildElement("Database")->GetText();
	ifstream in(file);
	while(!in.eof())
	{
		string str;
		getline(in,str);
		if(str.empty()) break;
		stringstream sstr(str);
		string table;
		unsigned lines,feilds;
		sstr>>table>>lines>>feilds;
		while(line!=0 && !in.eof())
		{
			CPRes insert;
			TiXmlDeclaration* decl=NEWD("1.0","","");
	        insert.LinkEndChild(decl);
	        TiXmlComment* comment=NEWC("insert");
	        insert.LinkEndChild(comment);
			TiXmlElement* insertroot=NEWE("Insert");
			insertroot->SetAttribute("tablename",table);
			insert.LinkEndChild(insertroot);
			getline(in,str);
			sstr.clear();
			sstr.str(str);
			string value;
			while(feilds!=0)
			{
				sstr>>value;
				TiXmlElement* feild=NEWE("Feild");
				feild->LinkEndChild(NEWT(value));
				insertroot->LinkEndChild(feild);
				feilds--;
			}
			SendInsert(insertres);
			lines--;
		}

	}
	return true;
}

bool LocalAgent::SendSelect(CPRes &res)
{
#ifdef OPTIMIZE
	TiXmlElement* root=res.RootElement();


#else
#endif
	return true;
}

bool LocalAgent::SendDropDB(CPRes &res)
{
	return true;
}

bool LocalAgent::SendDropTable(CPRes &res)
{
	return true;
}

void LocalAgent::BroadCast(const char *data,unsigned size)
{

}


/*

bool  LocalAgent::AddQueryInfo(CPRes &res)
{
	TiXmlElement* root=res.RootElement();
	TiXmlElement* table=root->FirstChildElement("Table");
	while(table!=NULL)
	{
		TiXmlElement* feild=table->FirstChildElement("Feild");
		while(feild!=NULL)
		{
			feild->SetAttribute("type","projection");
			feild=feild->NextSiblingElement("Feild");
		}
		table=table->NextSiblingElement("Table");
	}

}
*/


		   
