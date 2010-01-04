#include "LocalAgent.h"
#include <limits>
#include <fstream>
#include <sstream>
#include <iostream>

#define NEWE(p) new TiXmlElement((p))
#define NEWD(p1,p2,p3) new TiXmlDeclaration((p1),(p2),(p3))
#define NEWC(p) new TiXmlComment((p))
#define NEWT(p) new TiXmlText((p))

//*******************************************************************************************************

MyQueryRes::MyQueryRes(QueryRes &res,GDD* gdd):gdd_(gdd)
{
	
	feilds_=res.feilds_;
	SetKeyFlag();
	tablename_=res.tablename_;
	rows_=res.rows_;
	columns_=res.columns_;
	//gdd_.GetKeyFeilds(tablename_,keyfeilds_);
	TiXmlElement* root=res.RootElement();
	TiXmlElement* row=root->FirstChildElement("Row");
	std::vector<std::string >::const_iterator iter;
	//for(iter=feilds_.begin();iter!=feilds_.end();++iter)
	//{
	//	column_.insert(ColumnPair(*iter,std::vector<std::string >()));
	//}
	assign(rows_,MyRow(keyflag_));
	int index=0;
	while(row!=NULL)
	{
		iter=feilds_.begin();
		//MyRow myrow;
		TiXmlElement* feild=row->FirstChildElement();
		while(feild!=NULL && iter!=feilds_.end())
		{
			
			//myrow->insert(MyPair(*iter,feild->GetText()));
			(*this)[index].push_back(feild->GetText());
			//column_[*iter].push_back(feild->GetText());
			feild=feild->NextSiblingElement();
			iter++;
		}
		row=row->NextSiblingElement("Row");
		index++;
	}
	
}

MyQueryRes::MyQueryRes(GDD* gdd,std::vector<std::string >& feilds):gdd_(gdd),feilds_(feilds)
{
}

void MyQueryRes::SetKeyFlag()
{
	std::set<std::string > key;
	gdd_->GetKeyFeilds(tablename_,key);
	//std::vector<std::string >::const_iterator iter;
	for(std::size_t i=0;i<feilds_.size();++i)
	{
		if(key.find(feilds_[i])!=key.end())
		{
			keyflag_[i]=1;
		}
		else keyflag_[i]=0;
	}

	//gdd_.GetKeyFeilds(key);
}

bool MyQueryRes::IsKey(int index)
{
	return keyflag_[index]==1 ? true : false ;
}

void MyQueryRes::Join(MyQueryRes& res)
{
	int count=0;
}




void MyQueryRes::Projection(std::vector<std::string> &feilds)
{
}

void MyQueryRes::Union(MyQueryRes& res)
{
}

//*******************************************************************************************************

QueryRes::QueryRes(DataPtr data):TiXmlDocument()
{
	Parse(data->GetData());
	TiXmlElement* root=RootElement();
	std::string yes="yes",type="data";
	if(yes.compare(root->Attribute("success"))==0) success_=true;
	else success_=false;
	if(type.compare(root->Attribute("type"))==0)
	{
		nodata_=false;
		tablename_=root->Attribute("table");
		root->Attribute("lows",&rows_);
		root->Attribute("columns",&columns_);
	}
	else nodata_=true;
	
}

QueryRes::QueryRes(std::vector<std::string >& feilds,bool success,std::string& type,std::string& table,int rows,int columns):TiXmlDocument()
{
	feilds_=feilds;
	success_=success;
	rows_=rows;
	columns_=columns;
	tablename_=table;
	if(type.compare("data")==0)
	{
		nodata_=false;
	}
	else
	{
		nodata_=true;
	}
	TiXmlDeclaration* decl=NEWD("1.0","","");
	LinkEndChild(decl);
	TiXmlComment* comment=NEWC("data");
	LinkEndChild(comment);
	TiXmlElement* root=NEWE("Data");
	LinkEndChild(root);
	if(success_) root->SetAttribute("success","yes");
	else  root->SetAttribute("success","no");
	root->SetAttribute("type",type);
	root->SetAttribute("table",table);
	root->SetAttribute("lows",rows);
	root->SetAttribute("columns",columns);


}

QueryRes::QueryRes(bool success,std::string& type,std::string& table,int rows,int columns)
{
	success_=success;
	rows_=rows;
	columns_=columns;
	tablename_=table;
	if(type.compare("data")==0)
	{
		nodata_=false;
	}
	else
	{
		nodata_=true;
	}
	TiXmlDeclaration* decl=NEWD("1.0","","");
	LinkEndChild(decl);
	TiXmlComment* comment=NEWC("data");
	LinkEndChild(comment);
	TiXmlElement* root=NEWE("Data");
	LinkEndChild(root);
	if(success_) root->SetAttribute("success","yes");
	else  root->SetAttribute("success","no");
	root->SetAttribute("type",type);
	root->SetAttribute("table",table);
	root->SetAttribute("lows",rows);
	root->SetAttribute("columns",columns);

}

void QueryRes::Insert(std::vector<std::string>& value)
{
	if(value.size()!=feilds_.size()) return;
	TiXmlElement* root=RootElement();
	TiXmlElement* row=NEWE("Row");
	root->LinkEndChild(row);
	std::vector<std::string>::const_iterator iter1=feilds_.begin();
	std::vector<std::string>::const_iterator iter2=value.begin();
	for(;iter2!=value.end(),iter1!=feilds_.end();++iter1,++iter2)
	{
		
		TiXmlElement* feild=NEWE(*iter1);
		row->LinkEndChild(feild);
		feild->LinkEndChild(NEWT(*iter2));
	}

}

/*
void QueryRes::ConvertToMyQueryRes(MyQueryRes &res)
{
	typedef std::pair<std::string ,std::string > MyPair;
	res.feilds_=feilds_;
	res.tablename_=tablename_;
	res.rows_=rows_;
	
	res.columns_=columns_;
	TiXmlElement* root=RootElement();
	
	TiXmlElement* row=root->FirstChildElement("Row");
	std::vector<std::string >::const_iterator iter;
	while(row!=NULL)
	{
		iter=feilds_.begin();
		MyRowPtr myrow(new MyRow());
		TiXmlElement* feild=row->FirstChildElement();
		while(feild!=NULL && iter!=feilds_.end())
		{
			myrow->insert(MyPair(*iter,feild->GetText()));
			
			feild=feild->NextSiblingElement();
			iter++;
		}
		res.push_back(myrow);
		row=row->NextSiblingElement("Row");
	}


}
*/

//*******************************************************************************************************

RevCmd::RevCmd(Data &data):TiXmlDocument()
{
	Parse(data.GetData());
	cmdtype_=RootElement()->Value();;
}

const std::string& RevCmd::GetCmdType()
{
	return cmdtype_;
}

//*******************************************************************************************************

Client::Client(const std::string &address, const std::string &port,char* data,unsigned total,DataPtr res):
	  io_service_(),socket_(io_service_),resolver_(io_service_),
		  query_(address,port),data_(),iterator_(),total_(total),sent_(total),result_(res)
{
 	senddata_=new char[total];
	memset(senddata_,0,total);
	memcpy(senddata_,data,total);
	memset(&buffer_,0,sizeof(buffer_));
	iterator_=resolver_.resolve(query_);
	boost::asio::ip::tcp::endpoint endpoint = *iterator_;
	
    socket_.async_connect(endpoint,
        boost::bind(&Client::HandleConnect, this,
		boost::asio::placeholders::error,boost::ref(++iterator_ )));
	memset(&buffer_,0,sizeof(buffer_));
	//std::cout<<endpoint.address().to_string()<<std::endl;
	//int x;

}


void Client::HandleConnect(const boost::system::error_code &error, boost::asio::ip::tcp::resolver::iterator& endpoint_iterator)
{
	if(!error)
	{
		Write(senddata_,sent_,total_,result_);
	}
	else if(endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
	{
		socket_.close();
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
          boost::bind(&Client::HandleConnect, this,
		  boost::asio::placeholders::error,boost::ref(++endpoint_iterator)));
	}
	
    

}


void Client::Run()
{
	io_service_.run();
}

void Client::Write(char *data, unsigned int size, unsigned int total,DataPtr res)
{
	data_.Assigen(total);
	data_.AddData(data,total);
	io_service_.post(boost::bind(&Client::DoWrite,this,(char*)data_.GetData(),size,total,res));
}

void Client::Read(DataPtr res)
{
	io_service_.post(boost::bind(&Client::DoRead,this,res));

}

void Client::Close()
{
	io_service_.post(boost::bind(&Client::DoClose,this));
}



void Client::DoRead(DataPtr res)
{
	memset(&buffer_,0,sizeof(buffer_));
	boost::asio::async_read(socket_,
		boost::asio::buffer(&buffer_,sizeof(buffer_)),
		boost::bind(&Client::HandleKeepRead, this,
		boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,res));
}

void Client::HandleKeepRead(const boost::system::error_code &error,std::size_t count,DataPtr res)
{
	if (!error)
    {
		data_.Assigen(buffer_.total);
		data_.AddData(buffer_.data,buffer_.len);
		//memset(&buffer_,0,sizeof(buffer_));
		if(buffer_.flage==1)
		{
			memset(&buffer_,0,sizeof(buffer_));
			boost::asio::async_read(socket_,
				boost::asio::buffer(&buffer_, sizeof(buffer_)),
			boost::bind(&Client::HandleReadData, this,
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,res));
		}
		else
		{
			*res=data_;
			//handler(data_);
			//DoClose();
		}
		
    }
	if (error)
	{
		// Initiate graceful connection closure.
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		
	}
    


}

void Client::HandleReadData(const boost::system::error_code &error,std::size_t count,DataPtr res)
{
	if (!error)
    {
		if(!data_.AddData(buffer_.data,buffer_.len)) DoClose();
		//memset(&buffer_,0,sizeof(buffer_));
		if(buffer_.flage==1)
		{
			memset(&buffer_,0,sizeof(buffer_));
			boost::asio::async_read(socket_,
				boost::asio::buffer(&buffer_, sizeof(buffer_)),
			boost::bind(&Client::HandleReadData, this,
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,res));
		}
		else
		{
			*res=data_;
			//handler(data_);
			//DoClose();
		}
		
    }
	if (error)
	{
		// Initiate graceful connection closure.
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		
	}
   
}

void Client::DoWrite(char *data, unsigned int size, unsigned int total,DataPtr res)
{
	memset(&buffer_,0,sizeof(buffer_));
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

void Client::HandleWrite(const boost::system::error_code &error,std::size_t count, char *data, unsigned int size, unsigned int total,DataPtr res)
{
	if(!error)
	{
		memset(&buffer_,0,sizeof(buffer_));
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
	if (error)
	{
		// Initiate graceful connection closure.
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		
	}

}

void Client::DoClose()
{
	socket_.close();
	//io_service_.stop();
}

char const * Client::GetData()
{
	 return data_.GetData();
}

Client::~Client()
{
	//std::cout<<"Client closed"<<std::endl;
	delete [] senddata_;
	//DoClose();
}


//********************************************************************


Server::Server(unsigned thread_pool_size,unsigned short port,boost::function<bool (SessionPtr, Data &cmd ) > handler):
	  io_service_(),thread_pool_size_(thread_pool_size),endpoint_(boost::asio::ip::tcp::v4(),port),acceptor_(io_service_)
		 
{
	acceptor_.open(endpoint_.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint_);
	acceptor_.listen();

	SessionPtr newsession(new Session(io_service_,*this));
	acceptor_.async_accept(newsession->Socket(),
		boost::bind(&Server::HandleAccept, this, newsession,
		boost::asio::placeholders::error,handler));

}

Server::Server(): io_service_(),acceptor_(io_service_),endpoint_()
{
}

void Server::CreateServer(unsigned int thread_pool_size, unsigned short port, boost::function<bool(SessionPtr,Data&cmd)> handler)
{
	thread_pool_size_=thread_pool_size;
	endpoint_=boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port);
  
	acceptor_.open(endpoint_.protocol());

	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint_);
	acceptor_.listen();

	SessionPtr newsession(new Session(io_service_,*this));
	acceptor_.async_accept(newsession->Socket(),
		boost::bind(&Server::HandleAccept, this, newsession,boost::asio::placeholders::error,handler));

}

void Server::HandleAccept(SessionPtr session, const boost::system::error_code &error,boost::function<bool (SessionPtr, Data &cmd ) > handler)
{
	if (!error)
    {
	
      session->Start(handler);
	  sessions_.insert(session);
      //chat_session_ptr new_session(new chat_session(io_service_, room_));
	  SessionPtr newsession(new Session(io_service_,*this));
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
			boost::bind(&boost::asio::io_service::run, boost::ref(io_service_))));
		threads.push_back(thread);
	}
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
}

void Server::Close(SessionPtr s)
{
	sessions_.erase(s);
	//s->Stop();
	//io_service_.stop();
}

void Server::CloseAll()
{
	std::for_each(sessions_.begin(), sessions_.end(),
      boost::bind(&Session::Close, _1));
	sessions_.clear();
	io_service_.stop();

}

//***********************************************************************************

Session::Session(boost::asio::io_service &io_service,Server& server):socket_(io_service),server_(server)
{
	memset(&buffer_,0,sizeof(buffer_));
	senddata_=NULL;
	//total_=0;
	//sent_=0;
}

boost::asio::ip::tcp::socket& Session::Socket()
{
	return socket_;
}

void Session::Write(char* data,unsigned size,unsigned total)
{
	senddata_=new char[total];
	memcpy(senddata_,data,total);
	memset(&buffer_,0,sizeof(buffer_));
	if(size<=DATABUFFERSIZE)
	{
		buffer_.total=total;
		buffer_.len=size;
		buffer_.flage=0;
		memcpy(buffer_.data,&senddata_[total-size],size);
	}
	else
	{
		buffer_.total=total;
		buffer_.len=DATABUFFERSIZE;
		buffer_.flage=1;
		memcpy(buffer_.data,&senddata_[total-size],DATABUFFERSIZE);

	}
	 
	boost::asio::async_write(socket_,
		 boost::asio::buffer(&buffer_,sizeof(buffer_)),
		 boost::bind(&Session::HandleWrite, this,
		 boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,senddata_,size-buffer_.len,total));
}

void Session::HandleWrite(const boost::system::error_code &error,std::size_t count, char *data, unsigned int size, unsigned int total)
{
	if(!error)
	{
		if(size==0)
		{
			
			boost::system::error_code ignored_ec;
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
			server_.Close(shared_from_this());
			return;
		}
		memset(&buffer_,0,sizeof(buffer_));
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
	if (error)
	{
		// Initiate graceful connection closure.
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		server_.Close(shared_from_this());
	}

}



void Session::Start(boost::function<bool (SessionPtr, Data &cmd ) > handler)
{
	//std::cout<<"new senssion"<<std::endl;
	memset(&buffer_,0,sizeof(buffer_));
	boost::asio::async_read(socket_,
		boost::asio::buffer(&buffer_, sizeof(buffer_)),
		boost::bind(
          &Session::HandleKeepRead, shared_from_this(),
          boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,handler));
}

void Session::HandleKeepRead(const boost::system::error_code &error,std::size_t,boost::function<bool (SessionPtr, Data &cmd ) > handler)
{
	if (!error)
    {
		data_.Assigen(buffer_.total);
		data_.AddData(buffer_.data,buffer_.len);
		//memset(&buffer_,0,sizeof(buffer_));
		if(buffer_.flage==1)
		{
			memset(&buffer_,0,sizeof(buffer_));
			boost::asio::async_read(socket_,
				boost::asio::buffer(buffer_.data, DATABUFFERSIZE),
			boost::bind(&Session::HandleReadData, shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,handler));
		}
		else
		{
			//命令处理，发送查询数据
			handler(shared_from_this(),data_);
			//DoClose();
		}
		
    }
   if (error)
	{
		// Initiate graceful connection closure.
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		server_.Close(shared_from_this());
	}

}

void Session::HandleReadData(boost::system::error_code const & error,std::size_t,boost::function<bool (SessionPtr, Data &cmd ) > handler)
{
	if (!error)
    {
		if(!data_.AddData(buffer_.data,buffer_.len)) DoClose();
		//memset(&buffer_,0,sizeof(buffer_));
		if(buffer_.flage==1)
		{
			memset(&buffer_,0,sizeof(buffer_));
			boost::asio::async_read(socket_,
				boost::asio::buffer(buffer_.data, DATABUFFERSIZE),
			boost::bind(&Session::HandleReadData, shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,handler));
		}
		else
		{
			//命令处理，发送查询数据
			handler(shared_from_this(),data_);
			//DoClose();
		}
		
    }
	if (error)
	{
		// Initiate graceful connection closure.
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		server_.Close(shared_from_this());
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
	//std::cout<<"session closed"<<std::endl;
	delete [] senddata_;
	//DoClose();
}

void Session::Print()
{
	std::string str;
	
	//cout<<data_.GetData()<<endl;
}

//*****************************************************************************



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
		TiXmlElement* element=NEWE("Feild");
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
	try
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
	catch(std::exception& e)
	{
		std::cerr<<e.what()<<std::endl;
		return false;
	}
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
	std::string name=com.substr(0,com.find(" "));
	regexmap::iterator iter;
	iter=regexs.find(name);
	if(iter==regexs.end()) return false;
	ComFunc func=iter->second.first;
	return (this->*func)(com,iter->second.second,res);
}
//*******************************************************************************

LDD::LDD(const std::string &xml):doc_(xml)
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

bool LDD::SetAppSetting(const std::string &name, std::string &value)
{
	if(!vaild) return false;
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* element=root->FirstChildElement("AppSetting");
	TiXmlElement* son=element->FirstChildElement(name);
	if(son==NULL) return false;
	son->FirstChild()->ToText()->SetValue(value);
	return true;
}

bool LDD::GetAllMemoryTable(std::vector<std::string >& tables)
{
	if(!vaild) return false;
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* element=root->FirstChildElement("SqlSetting");
	TiXmlElement* son=element->FirstChildElement("Table");
	std::string engine="memory";
	while(son!=NULL)
	{
		if(engine.compare(son->Attribute("engine"))==0)
			tables.push_back(son->GetText());
		son=son->NextSiblingElement("Table");
	}
	
	return true;

}

bool LDD::SetSqlSetting(const std::string &name, std::string &value)
{
	if(!vaild) return false;
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* element=root->FirstChildElement("SqlSetting");
	TiXmlElement* son=element->FirstChildElement(name);
	if(son==NULL) return false;
	son->FirstChild()->ToText()->SetValue(value);
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

bool Location::GetDes(GDD& gdd, std::list<DesInfo >& deslist)
{
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* table=root->FirstChildElement("Table");
	while(table!=NULL)
	{
		std::string site=table->Attribute("site");
		std::string address;
		std::string port;
		gdd.GetSiteSetting(site,address,port);
		deslist.push_back(DesInfo(address,port));
		table=table->NextSiblingElement("Table");
	}
	return true;

}

void Location::Print()
{
	using namespace std;
	cout<<doc_<<endl;
}
//**********************************************************************

GDD::GDD(const std::string &xml):doc_(xml)
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
		//TiXmlElement* son=element->FirstChildElement("Name");
		if(name.compare(element->Attribute("name"))==0)
		{
			address=element->Attribute("address");;
			port=element->Attribute("port");
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

bool GDD::GetTableFeilds(std::string& tablename,std::vector<std::string >& feilds)
{
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* tableinfo=root->FirstChildElement("TableInfo");
	TiXmlElement* table=tableinfo->FirstChildElement("Table");
	while(table!=NULL)
	{
		if(tablename.compare(table->Attribute("name"))==0)
		{
			TiXmlElement* feild=table->FirstChildElement("Feild");
			while(feild!=NULL)
			{
				feilds.push_back(feild->GetText());
				feild=feild->NextSiblingElement("Feild");
			}
			return true;
		}
		table=table->NextSiblingElement("Table");
	}
	return false;
}

bool GDD::GetKeyFeilds(std::string& tablename,std::set<std::string> &feilds)
{
	TiXmlElement* root=doc_.RootElement();
	TiXmlElement* tableinfo=root->FirstChildElement("TableInfo");
	TiXmlElement* table=tableinfo->FirstChildElement("Table");
	std::string key="yes";
	while(table!=NULL)
	{
		if(tablename.compare(table->Attribute("name"))==0)
		{
			TiXmlElement* feild=table->FirstChildElement("Feild");
			while(feild!=NULL)
			{
				if(key.compare(feild->Attribute("key"))==0)
				{
					feilds.insert(feild->GetText());
				}
				feild=feild->NextSiblingElement("Feild");
			}
			return true;
		}
		table=table->NextSiblingElement("Table");
	}
	return false;

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

LocalAgent::LocalAgent():gdd_(".\\AppData\\GDD.xml"),ldd_(".\\AppData\\LDD.xml"),cprocessor_(".\\AppData\\regex.xml"),server_(),conn_()
{
	typedef std::pair<std::string, SendCmdFunc > SendPair;
	typedef std::pair<std::string, RevCmdFunc > RevPair;
	sendfuncmap_.insert(SendPair("DefineSite",&LocalAgent::SendDefineSite));
	sendfuncmap_.insert(SendPair("CreateDB",&LocalAgent::SendCreateDB));
	sendfuncmap_.insert(SendPair("CreateTable",&LocalAgent::SendCreateTable));
	sendfuncmap_.insert(SendPair("Fragment",&LocalAgent::SendFragment));
	//sendfuncmap_.insert(SendPair("Allocate",&LocalAgent::SendAllocate));
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
	//revfuncmap_.insert(RevPair("Allocate",&LocalAgent::RevAllocate));
	revfuncmap_.insert(RevPair("Insert",&LocalAgent::RevInsert));
	revfuncmap_.insert(RevPair("Delete",&LocalAgent::RevDelete));
	revfuncmap_.insert(RevPair("Import",&LocalAgent::RevImport));
	revfuncmap_.insert(RevPair("Select",&LocalAgent::RevSelect));
	revfuncmap_.insert(RevPair("DropTable",&LocalAgent::RevDropTable));
	revfuncmap_.insert(RevPair("DropDB",&LocalAgent::RevDropDB));
	//std::string address,port,
	if(ConnectToSql()) connected=true;
	else connected=false;
}

bool LocalAgent::ConnectToSql()
{
	try
	{
		std::string address,port,user,pass,db,server;
		ldd_.GetSqlSetting("Address",address);
		ldd_.GetSqlSetting("Port",port);
		ldd_.GetSqlSetting("UserName",user);
		ldd_.GetSqlSetting("PassWord",pass);
		ldd_.GetSqlSetting("DataBase",db);
		return connected=conn_.connect(db.c_str(),server.c_str(),user.c_str(),pass.c_str(),atoi(port.c_str()));
	}
	catch(std::exception& e)
	{
		std::cerr<<e.what()<<std::endl;
		return false;
	}
}

void LocalAgent::Start()
{
	
	
	boost::thread thread1(boost::bind(&LocalAgent::StartLocalServer,this));
	boost::thread thread2(boost::bind(&LocalAgent::StartRevLocalCmd,this,boost::ref(std::cin)));
	thread2.join();
	thread1.join();
		
}

void LocalAgent::StartLocalServer()
{
	//boost::shared_ptr<boost::thread> thread(new boost::thread(
	//		boost::bind(&boost::asio::io_service::run, &io_service_)));
	std::string lisenning;
	std::string size;
	ldd_.GetAppSetting("RevPort",lisenning);
	ldd_.GetAppSetting("ThreadPollSize",size);
	//Server server(atoi(size.c_str()),atoi(lisenning.c_str()),boost::bind(&LocalAgent::ExecuteCmd,this,_1,_2));
	try
	{
		server_.CreateServer(atoi(size.c_str()),atoi(lisenning.c_str()),bind(&LocalAgent::ExecuteCmd,this,_1,_2));
		server_.Run();
	}
	catch(std::exception& e)
	{
		std::cerr<<e.what()<<std::endl;
	}
	
}

void LocalAgent::StartRevLocalCmd(std::istream &in)
{
	
	while(true)
	{
		std::string cmd;
		std::getline(in,cmd);
		if(cmd.compare("exit")==0)
		{
			server_.CloseAll();
			return ;

		}
		CPRes res;
		if(!cprocessor_.SplitCom(cmd,res))
		{
			std::cerr<<"no such cmd!"<<std::endl;

		}
		//std::cout<<res<<std::endl;
		else
		{
			std::string cmdtype=res.RootElement()->Value();
			SendFuncmap::iterator iter=sendfuncmap_.find(cmdtype);
			if(iter!=sendfuncmap_.end())
			{
				try
				{
					SendCmdFunc func=iter->second;
					(this->*func)(res);
				}
				catch(std::exception& e)
				{
					std::cerr<<e.what()<<std::endl;
				}
			}
		}
		
	}
}



bool LocalAgent::ExecuteCmd(SessionPtr senssion, Data &cmd)
{
	//std::cout<<cmd.GetData()<<std::endl;
	RevCmd revcmd(cmd);
	std::string cmdtype=revcmd.GetCmdType();
	RevFuncmap::iterator iter=revfuncmap_.find(cmdtype);
	if(iter!=revfuncmap_.end())
	{
		try
		{
			RevCmdFunc func=iter->second;
			return (this->*func)(senssion,revcmd);
		}
		catch(std::exception& e)
		{
			std::cerr<<e.what()<<std::endl;
			return false;
		}
	}
	return false;
}

bool LocalAgent::SendDefineSite(CPRes& res)
{
	//set gdd
	DataList datalist;
	std::string data;
	data<<res;
	TiXmlElement* root=res.RootElement();
	DataPtr result(new Data());
	datalist.push_back(result);
	ClientPtr client(new Client(root->FirstChildElement("Address")->GetText(),root->FirstChildElement("Port")->GetText(),(char*)data.c_str(),data.size(),result));
	boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
	
	//client->Write((char*)data.c_str(),data.size(),data.size(),*result);
	thread->join();
	ResultProcess(datalist,res);
	//std::for_each(datalist.begin(),datalist.end(),boost::bind(Release<Data>,_1));
	return true;
}

bool LocalAgent::SendCreateDB(CPRes& res)
{
	//set gdd
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
		DataPtr result(new Data());
		ClientPtr client(new Client(iter->first,iter->second,(char*)data.c_str(),data.size(),result));
		
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
		
		//client->Write((char*)data.c_str(),data.size(),data.size(),*result);
		clientlist.push_back(client);
		datalist.push_back(result);
		threads.push_back(thread);
		iter++;
	}
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
	ResultProcess(datalist,res);
	//std::for_each(datalist.begin(),datalist.end(),boost::bind(Release<Data>,_1));
	//返回结果处理
	return true;

}

bool LocalAgent::SendCreateTable(CPRes& res)
{
	//set gdd
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
		DataPtr result(new Data());
		ClientPtr client(new Client(iter->first,iter->second,(char*)data.c_str(),data.size(),result));
		
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
		threads.push_back(thread);
		
		//client->Write((char*)data.c_str(),data.size(),data.size(),*result);
		clientlist.push_back(client);
		datalist.push_back(result);
		iter++;
	}
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
	//返回结果处理
	ResultProcess(datalist,res);
	//std::for_each(datalist.begin(),datalist.end(),boost::bind(Release<Data>,_1));
	return true;

}

bool LocalAgent::SendFragment(CPRes& res)
{
	//set gdd
	return true;

}


bool LocalAgent::SendInsert(CPRes &res)
{
	//std::cout<<res<<std::endl;
	TiXmlElement* root=res.RootElement();
	std::string table=root->Attribute("tablename");
	TiXmlElement* element=root->FirstChildElement("Feild");
	std::vector<std::string > feilds;
	gdd_.GetTableFeilds(table,feilds);
	//std::string feild=element->Attribute("name");
	std::string feild=feilds[0];
	std::string value=element->GetText();
	Location location(table,feild,value,"yes",value,"yes");
	gdd_.GetTableLocation(location);
	root=location.doc_.RootElement();
	element=root->FirstChildElement("Table");
	std::string address,port;
	gdd_.GetSiteSetting(element->Attribute("site"),address,port);
	std::string data;
	data<<res;
	DataPtr result(new Data());
	DataList datalist;
	datalist.push_back(result);
	ClientPtr client(new Client(address,port,(char*)data.c_str(),data.size(),result));
	
	boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
	
	//client->Write((char*)data.c_str(),data.size(),data.size(),*result);
	thread->join();
	ResultProcess(datalist,res);
	//std::for_each(datalist.begin(),datalist.end(),boost::bind(Release<Data>,_1));
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
		DataPtr result(new Data());
		ClientPtr client(new Client(address,port,(char*)data.c_str(),data.size(),result));
	   
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
		threads.push_back(thread);
		 
	    //client->Write((char*)data.c_str(),data.size(),data.size(),*result);
		clientlist.push_back(client);
		datalist.push_back(result);
		element=element->NextSiblingElement("Table");

	}
	
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
	ResultProcess(datalist,res);
	//std::for_each(datalist.begin(),datalist.end(),boost::bind(Release<Data>,_1));
	//返回结果处理
	return true;
}

bool LocalAgent::SendImport(CPRes &res)
{
	using namespace std;
	TiXmlElement* root=res.RootElement();
	std::string file,database;
	file=root->FirstChildElement("File")->GetText();
	database=root->FirstChildElement("DataBase")->GetText();
	ifstream in(file.c_str());
	//if(!in.open((file.c_str()))) return false;
	if(!in) return false;
	
	while(!in.eof())
	{
		string str;
		getline(in,str);
		if(str.empty()) break;
		stringstream sstr(str);
		string table;
		unsigned lines,feilds;
		sstr>>table>>lines>>feilds;
		while(lines!=0 && !in.eof())
		{
			unsigned feildnum=feilds;
			CPRes insert;
			TiXmlDeclaration* decl=NEWD("1.0","","");
	        insert.LinkEndChild(decl);
	        TiXmlComment* comment=NEWC("insert");
	        insert.LinkEndChild(comment);
			TiXmlElement* insertroot=NEWE("Insert");
			insertroot->SetAttribute("tablename",table);
			insert.LinkEndChild(insertroot);
			getline(in,str);
			//sstr.clear();
			//sstr.str(str);
			string value;
			std::size_t subbeg=0;
			std::size_t subend=0;
			while(feildnum!=0)
			{
				subend=str.find('\t',subbeg);
				//sstr>>value;
				TiXmlElement* feild=NEWE("Feild");
				feild->LinkEndChild(NEWT(str.substr(subbeg,subend-subbeg)));
				insertroot->LinkEndChild(feild);
				subbeg=subend+1;
				feildnum--;
			}
			//std::cout<<insert<<std::endl;
			SendInsert(insert);
			lines--;
		}

	}
	return true;
}

bool LocalAgent::SendSelect(CPRes &res)
{
	
	//SubSelectionList sublist;
	DataList datalist;
	ClientList clientlist;
	std::vector<boost::shared_ptr<boost::thread> > threads;
	
	TiXmlElement* root=res.RootElement();
	TiXmlElement* table=root->FirstChildElement("Table");
	while(table!=NULL)
	{
		SubSelection sub(gdd_,std::string(table->Attribute("name")),res);
		//sublist.push_back(sub);
		DesList& deslist=sub.GetDesList();
		DesList::iterator iter=deslist.begin();
		std::string data;
		data<<sub;
		while(iter!=deslist.end())
		{
			DataPtr result(new Data());
			ClientPtr client(new Client(iter->first,iter->second,(char*)data.c_str(),data.size(),result));
			boost::shared_ptr<boost::thread> thread(new boost::thread(
				boost::bind(&Client::Run, client)));
			threads.push_back(thread);
			//client->Write((char*)data.c_str(),data.size(),data.size(),*result);
			clientlist.push_back(client);
			datalist.push_back(result);
			iter++;
		}
		table=table->NextSiblingElement("Table");
	}

	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
	ResultProcess(datalist,res);
	
	//std::for_each(datalist.begin(),datalist.end(),boost::bind(Release<Data>,_1));

	//返回结果处理
	return true;
}

bool LocalAgent::SendDropDB(CPRes &res)
{
	std::string data;
	data<<res;
	std::vector<DesInfo > des;
	gdd_.GetAllSiteInfo(des);
	DataList datalist;
	ClientList clientlist;
	std::vector<boost::shared_ptr<boost::thread> > threads;
	for(std::size_t i=0;i<des.size();++i)
	{
		DataPtr result(new Data());
		ClientPtr client(new Client(des[i].first,des[i].second,(char*)data.c_str(),data.size(),result));
		
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
		//client->Write((char*)data.c_str(),data.size(),data.size(),*result);
		clientlist.push_back(client);
		datalist.push_back(result);
		threads.push_back(thread);
	}
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();

	//返回结果处理
	ResultProcess(datalist,res);
	//std::for_each(datalist.begin(),datalist.end(),boost::bind(Release<Data>,_1));

	return true;
}

bool LocalAgent::SendDropTable(CPRes &res)
{

	std::string data;
	data<<res;
	std::vector<DesInfo > des;
	gdd_.GetAllSiteInfo(des);
	DataList datalist;
	ClientList clientlist;
	std::vector<boost::shared_ptr<boost::thread> > threads;
	for(std::size_t i=0;i<des.size();++i)
	{
		DataPtr result(new Data());
		ClientPtr client(new Client(des[i].first,des[i].second,(char*)data.c_str(),data.size(),result));
		
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&Client::Run, client)));
		//client->Write((char*)data.c_str(),data.size(),data.size(),*result);
		clientlist.push_back(client);
		datalist.push_back(result);
		threads.push_back(thread);
	}
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();

	//返回结果处理
	ResultProcess(datalist,res);
	//std::for_each(datalist.begin(),datalist.end(),boost::bind(Release<Data>,_1));

	return true;
}

bool LocalAgent::CreateTable(std::string& tablename,std::vector<std::pair<char,std::string> >& feilds,const std::string& engine=std::string("InnoDB"))
{
	/*
	CREATE TABLE `course_memory` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` char(80) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
*/
	//std::string create="create table ";
	//create+="'"+tablename+"'"
	return true;

}

bool LocalAgent::ToSelectString(TiXmlDocument* xmlselect,std::string& strselect)
{
	TiXmlElement* root=xmlselect->RootElement();
	TiXmlElement* table=root->FirstChildElement("Table");
	strselect="select ";
	std::vector<std::string > tables;
	while(table!=NULL)
	{
		std::string tablename=table->Attribute("name");
		tablename+="_memory";
		tables.push_back(tablename);
		TiXmlElement* feild=table->FirstChildElement("Feild");
		while(feild!=NULL)
		{
			if(feild->Attribute("type")==NULL)
			{
				feild=feild->NextSiblingElement("Feild");
				continue;
				
			}
			std::string feildtext=feild->GetText();
			if(feildtext.compare("*")==0)
			{
				strselect+=feildtext+" ";
				break;
			}
			strselect+=tablename+"."+feildtext+",";
			feild=feild->NextSiblingElement("Feild");
		}
		table=table->NextSiblingElement("Table");
	}
	strselect[strselect.size()-1]=' ';
	strselect+="from ";
	for(std::size_t i=0;i<tables.size();i++)
	{
		strselect+=tables[i]+",";
	}
	strselect[strselect.size()-1]=' ';
	//strselect+="where ";
	TiXmlElement* condition=root->FirstChildElement("Condition");
	if(condition!=NULL) strselect+="where ";
	while(condition!=NULL)
	{
		std::string type="value";
		TiXmlElement* left=condition->FirstChildElement("Left");
		TiXmlElement* op=condition->FirstChildElement("Operator");
		TiXmlElement* right=condition->FirstChildElement("Right");
		strselect+=left->Attribute("table");
		strselect+="_memory";
		strselect+=".";
		strselect+=left->GetText();
		strselect+=op->GetText();
		if(type.compare(right->Attribute("type"))==0)
		{
			strselect+=right->GetText();
		}
		else
		{
			strselect+=right->Attribute("table");
			strselect+="_memory";
			strselect+=".";
			strselect+=right->GetText();

		}
		strselect+=" and ";
		condition=condition->NextSiblingElement("Condition");
	}
	std::string fstr="and";
	std::size_t found=strselect.rfind(fstr);
	if (found!=std::string::npos)
		strselect.replace (found,fstr.length(),";");
	return true;

	//strselect[strselect.refind_last_of("and")-2]=';';
	//TiXmlElement* feild=table->FirstChildElement("Feild");



}

bool LocalAgent::ClearMemoryDB()
{
	std::vector<std::string > tables;
	ldd_.GetAllMemoryTable(tables);
	try
	{
		mysqlpp::Query sqldelete = conn_.query();
		for(std::vector<std::string >::iterator iter=tables.begin();iter!=tables.end();++iter)
		{
			sqldelete.exec("delete from "+(*iter));
		}
	}
	catch(std::exception& e)
	{
		std::cout<<e.what()<<std::endl;
		return false;
	}
	return true;
	
}

void LocalAgent::ResultProcess(LocalAgent::DataList &datalist, CPRes &res)
{
	
	ClearMemoryDB();
	DataList::iterator iter=datalist.begin();
	while(iter!=datalist.end())
	{
		QueryRes queryres(*iter);
		if(queryres.Empty())
		{
			if(queryres.Ok()) std::cout<<"Ok!"<<std::endl;
			else std::cout<<"Failed!"<<std::endl;
			iter++;
			continue;
		}
		try
		{
			TiXmlElement* root=queryres.RootElement();
			std::string tablename=root->Attribute("table");
			std::string memtablename=tablename+"_memory";
			//mysqlpp::Query sqldelete = conn_.query();
			//sqldelete.exec("delete from "+memtablename);
			std::vector<std::string > feilds;
			gdd_.GetTableFeilds(tablename,feilds);
			TiXmlElement* row=root->FirstChildElement("Row");
			while(row!=NULL)
			{
				std::string insert="insert into ";
				insert+=memtablename+" values(";
				for(std::size_t i=0;i<feilds.size();i++)
				{
					TiXmlElement* feild=row->FirstChildElement(feilds[i]);
					if(feild!=NULL)
					{
						insert+="'";
						insert+=feild->GetText();
						insert+="'";
						
					}
					else
					{
						insert+="NULL";

					}
					insert+=",";
				}
				insert[insert.size()-1]=' ';
				insert+=")";
				mysqlpp::Query sqlquery = conn_.query(insert);
				bool ok=sqlquery.exec(insert);
				row=row->NextSiblingElement("Row");
			}
		}
		catch(std::exception& e)
		{
			std::cerr<<e.what()<<std::endl;
			return;
		}
		iter++;
		
	}
	std::string cmd=res.GetCmd();
	if(cmd.compare("Select")==0)
	{
		std::string query;
		ToSelectString(&res,query);
		mysqlpp::Query sqlquery = conn_.query(query);
		if (mysqlpp::StoreQueryResult queryres = sqlquery.store())
		{
			//mysqlpp::Field test;
			Print(queryres);
		}
	}

}

void LocalAgent::Print(mysqlpp::StoreQueryResult& res)
{
	using namespace std;
	cout.setf(ios::left);
	mysqlpp::Fields feilds=res.fields();
	for(std::size_t i=0;i<feilds.size();i++)
	{
		cout<<setw(feilds[i].length())<<feilds[i].name();

	}
	cout<<endl;
	for(std::size_t i=0;i<res.num_rows();i++)
	{
		for(std::size_t j=0;j<res.num_fields();j++)
		{
			cout<<setw(feilds[j].length())<<res[i][j];
		}
		cout<<endl;
	}
	

	
}

bool LocalAgent::RevDefineSite(SessionPtr senssion, RevCmd &cmd)
{
	//QueryRes res(true,std::string("nodata"));
	//std::string data;
	//data<<res;
	//senssion->Write((char*)data.c_str(),data.size(),data.size());
	return true;
	
}

bool LocalAgent::RevCreateDB(SessionPtr senssion, RevCmd &cmd)
{
	//QueryRes res(true,std::string("nodata"));
	//std::string data;
	//data<<res;
	//senssion->Write((char*)data.c_str(),data.size(),data.size());
	return true;
}

bool LocalAgent::RevCreateTable(SessionPtr senssion, RevCmd &cmd)
{
	//QueryRes res(true,std::string("nodata"));
	//std::string data;
	//data<<res;
	//senssion->Write((char*)data.c_str(),data.size(),data.size());
	return true;
}

bool LocalAgent::RevFragment(SessionPtr senssion, RevCmd &cmd)
{
	//QueryRes res(true,std::string("nodata"));
	//std::string data;
	//data<<res;
	//senssion->Write((char*)data.c_str(),data.size(),data.size());
	return true;
}

bool LocalAgent::RevInsert(SessionPtr senssion, RevCmd &cmd)
{
	try
	{
		
		std::string insert="insert into ";
		TiXmlElement* root=cmd.RootElement();
		std::string tablename=root->Attribute("tablename");
		insert+=tablename+" values(";
		//TiXmlElement* table=root->FirstChildElement("Insert");
		TiXmlElement* feild=root->FirstChildElement("Feild");
		//std::string all=feild->GetText();
		//std::string tablename=table->Attribute("name");
		//std::vector<std::string > feilds;
		while(true)
		{
			insert+=feild->GetText();
			//feilds.push_back(feild->GetText());
			feild=feild->NextSiblingElement("Feild");
			if(feild!=NULL) 
			{
				insert+=",";
			}
			else
			{
				insert+=")";
				break;
			}
		}
		std::string data;
		mysqlpp::Query sqlquery = conn_.query(insert);
		if(sqlquery.exec(insert))
		{
			QueryRes queryres(true,std::string("nodata"));
			data<<queryres;
		}
		else
		{
			QueryRes queryres(false,std::string("nodata"));
			data<<queryres;
		}
		senssion->Write((char*)data.c_str(),data.size(),data.size());
	}
	catch(std::exception& e)
	{
		std::cerr<<e.what()<<std::endl;
		return false;
	}
	return true;
	
}

bool LocalAgent::RevDelete(SessionPtr senssion, RevCmd &cmd)
{
	//QueryRes res(true,std::string("nodata"));
	//std::string data;
	//data<<res;
	//senssion->Write((char*)data.c_str(),data.size(),data.size());
	return true;
}
bool LocalAgent::RevImport(SessionPtr senssion, RevCmd &cmd)
{
	//QueryRes res(true,std::string("nodata"));
	//std::string data;
	//data<<res;
	//senssion->Write((char*)data.c_str(),data.size(),data.size());
	return true;
}
bool LocalAgent::RevSelect(SessionPtr senssion, RevCmd &cmd)
{
	//QueryRes res(true,std::string("nodata"));
	//std::string data;
	//data<<res;
	//RevCmd
	
	std::string query="select ";
	TiXmlElement* root=cmd.RootElement();
	TiXmlElement* table=root->FirstChildElement("Table");
	TiXmlElement* feild=table->FirstChildElement("Feild");
	std::string all=feild->GetText();
	std::string tablename=table->Attribute("name");
	std::vector<std::string > feilds;
	while(true)
	{
		query+=feild->GetText();
		feilds.push_back(feild->GetText());
		feild=feild->NextSiblingElement("Feild");
		if(feild!=NULL) 
		{
			query+=",";
		}
		else
		{
			query+=" ";
			break;
		}
	}
	
	query+="from "+tablename;;
	TiXmlElement* condition=root->FirstChildElement("Condition");
	if(condition!=NULL)
	{
		query+=" where ";
		query+=condition->FirstChildElement("Left")->GetText();
		query+=condition->FirstChildElement("Operator")->GetText();
		query+=condition->FirstChildElement("Right")->GetText();
	}
	if(all.compare("*")==0)
	{
		feilds.clear();
		
		gdd_.GetTableFeilds(tablename,feilds);
		//feilds=temp;
	}
	
	mysqlpp::Query sqlquery = conn_.query(query);
	try
	{
		if (mysqlpp::StoreQueryResult res = sqlquery.store())
		{
			std::string type;
			if(res.num_rows()>0) type="data";
			else type="nodata";
			QueryRes queryres(feilds,true,type,tablename,res.num_rows(),res.num_fields());
			mysqlpp::StoreQueryResult::const_iterator it;
			for(it=res.begin();it!=res.end();++it)
			{
				mysqlpp::Row row=*it;
				mysqlpp::Row::const_iterator iter=row.begin();
				std::vector<std::string > temp;
				for(;iter!=row.end();++iter)
				{
					temp.push_back(iter->c_str());
				}
				queryres.Insert(temp);
			}
			std::string data;
			data<<queryres;
			//std::cout<<queryres<<std::endl;
			senssion->Write((char*)data.c_str(),data.size(),data.size());
		}
		
	}
	catch(std::exception& e)
	{

		std::cerr<<e.what()<<std::endl;
		QueryRes queryres(feilds,false,std::string("nodata"),tablename);
		std::string data;
		data<<queryres;
		std::cout<<queryres<<std::endl;
		senssion->Write((char*)data.c_str(),data.size(),data.size());

		return false;
	}
	return true;
}
bool LocalAgent::RevDropTable(SessionPtr senssion, RevCmd &cmd)
{
	//QueryRes res(true,std::string("nodata"));
	//std::string data;
	//data<<res;
	//senssion->Write((char*)data.c_str(),data.size(),data.size());
	return true;
}
bool LocalAgent::RevDropDB(SessionPtr senssion, RevCmd &cmd)
{
	//QueryRes res(true,std::string("nodata"));
	//std::string data;
	//data<<res;
	//senssion->Write((char*)data.c_str(),data.size(),data.size());
	return true;
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

//*******************************************************************************

SubSelection::SubSelection(GDD& gdd,std::string& tablename, CPRes &res):TiXmlDocument(),gdd_(gdd),tablename_(tablename)
{
	TiXmlDeclaration* decl=NEWD("1.0","","");
	LinkEndChild(decl);
	TiXmlComment* comment=NEWC("select");
	LinkEndChild(comment);
	TiXmlElement* root=NEWE("Select");
	LinkEndChild(root);
	TiXmlElement* table=NEWE("Table");
	root->LinkEndChild(table);
	table->SetAttribute("name",tablename);
	TiXmlElement* restable=res.RootElement()->FirstChildElement("Table");
	while(restable!=NULL)
	{
		//TiXmlElement* resfeild=
		if(tablename.compare(restable->Attribute("name"))==0)
		{
#ifdef OPTIMIZE
			TiXmlElement* resfeild= restable->FirstChildElement("Feild");
			while(resfeild!=NULL)
			{
				TiXmlElement* feild=NEWE("Feild");
			    table->LinkEndChild(feild);
			    feild->LinkEndChild(NEWT(resfeild->GetText()));
				resfeild=resfeild->NextSiblingElement("Feild");
			}
#else
			TiXmlElement* feild=NEWE("Feild");
			table->LinkEndChild(feild);
			feild->LinkEndChild(NEWT("*"));
#endif
			TiXmlElement* condition=GetCondition(tablename,res);
			if(condition!=NULL)
			{
				root->LinkEndChild(condition);
			}
			vaild=true;
			GetDes();
			return;
		}
		restable=restable->NextSiblingElement("Table");
	}
	vaild=false;

	
}

TiXmlElement* SubSelection::GetCondition(std::string tablename, CPRes &res)
{
	std::string type="value";
	TiXmlElement* root=res.RootElement();
	TiXmlElement* rescondition=root->FirstChildElement("Condition");
	while(rescondition!=NULL)
	{
		TiXmlElement* resleft=rescondition->FirstChildElement("Left");
		TiXmlElement* resop=rescondition->FirstChildElement("Operator");
		TiXmlElement* resright=rescondition->FirstChildElement("Right");
		if(tablename.compare(resleft->Attribute("table"))==0 && type.compare(resright->Attribute("type"))==0)
		{
			TiXmlElement* condition=NEWE("Condition");
			TiXmlElement* left=NEWE("Left");
			condition->LinkEndChild(left);
			left->LinkEndChild(NEWT(resleft->GetText()));
			TiXmlElement* op=NEWE("Operator");
			condition->LinkEndChild(op);
			op->LinkEndChild(NEWT(resop->GetText()));
			TiXmlElement* right=NEWE("Right");
			condition->LinkEndChild(right);
			right->LinkEndChild(NEWT(resright->GetText()));
			return condition;
		}
		rescondition=rescondition->NextSiblingElement("Condition");
	}
	return NULL;
}

bool SubSelection::GetDes()
{
	std::string g=">";
	std::string ge=">=";
	std::string l="<";
	std::string le="<=";
	std::string e="=";
	TiXmlElement* root=RootElement();
	TiXmlElement* condition=root->FirstChildElement("Condition");
	if(condition==NULL)
	{
		Location location(tablename_,"all","MIN","no","MAX","no");
	    gdd_.GetTableLocation(location);
	    location.GetDes(gdd_,deslist_);
	    return true;

	}
	TiXmlElement* left=condition->FirstChildElement("Left");
	TiXmlElement* op=condition->FirstChildElement("Operator");
	TiXmlElement* right=condition->FirstChildElement("Right");
	std::string low,lowe,up,upe;
	do
	{
		if(g.compare(op->GetText())==0)
		{
			low=right->GetText();
			lowe="no";
			up="MAX";
			upe="no";
			break;
		}
		if(ge.compare(op->GetText())==0)
		{
			low=right->GetText();
			lowe="yes";
			up="MAX";
			upe="no";
			break;
		}
		if(l.compare(op->GetText())==0)
		{
			low="MIN";
			lowe="no";
			up=right->GetText();
			upe="no";
			break;
		}
		if(le.compare(op->GetText())==0)
		{
			low="MIN";
			lowe="yes";
			up=right->GetText();
			upe="no";
			break;
		}
		if(e.compare(op->GetText())==0)
		{
			low=right->GetText();
			lowe="yes";
			up=right->GetText();
			upe="yes";
			break;
		}
	}while(false);
	Location location(tablename_,left->GetText(),low,lowe,up,upe);
	gdd_.GetTableLocation(location);
	location.GetDes(gdd_,deslist_);
	return true;
}

DesList& SubSelection::GetDesList()
{
	return deslist_;
}

//*******************************************************************************


		   
