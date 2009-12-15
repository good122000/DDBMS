#include "LocalAgent.h"
#include "tinyxml.h"

using namespace std;

#define NEWE(p) new TiXmlElement((p))
#define NEWD(p1,p2,p3) new TiXmlDeclaration((p1),(p2),(p3))
#define NEWC(p) new TiXmlComment((p))
#define NEWT(p) new TiXmlText((p))

bool DefineSite(string const & com,vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,reg[0],boost::regex::icase)) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("definesite");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("DefineSite");
	res.LinkEndChild(root);
	TiXmlElement* element=NEWE("SiteName");
	element->LinkEndChild(NEWT(what[1].str()););
	root->LinkEndChild(element);
	element=NEWE("Address");
	element->LinkEndChild(NEWT(what[2].str()));
	root->LinkEndChild(element);
	element=NEWE("Port");
	element->LinkEndChild(NEWT(what[3].str()));
	root->LinkEndChild(element);
	return true;
}

bool CreateDB(string const & com,vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,reg[0],boost::regex::icase)) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("createdb");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("CreateDB");
	res.LinkEndChild(root);
	TiXmlElement* element=NEWE("DBName");
	element->LinkEndChild(NEWT(what[1].str()););
	root->LinkEndChild(element);
	return true;
}

bool CreateTable(string const & com,vector<boost::regex > const & reg,CPRes& res)
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
	string substr=what[2].str();
	string::const_iterator at=substr.begin();
	string::const_iterator end=substr.end();
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

bool Allocate(string const & com,vector<boost::regex > const & reg,CPRes& res)
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
	elemnet=NEWE("DesName");
	element->LinkEndChild(NEWT(what[2].str()));
	root->LinkEndChild(element);
	return true;
}

bool Insert(string const & com,vector<boost::regex > const & reg,CPRes& res)
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
	string substr1=what[2].str();
	string substr2=what[4].str();
	string::const_iterator at=substr1.begin();
	string::const_iterator end=substr1.end();
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

bool Delete(string const & com,vector<boost::regex > const & reg,CPRes& res)
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

bool Import(string const & com,vector<boost::regex > const & reg,CPRes& res)
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
	elemnet->LinkEndChild(NEWT(what[2].str()));
	root->LinkEndChild(element);
	return true;
}

bool Select(string const & com,vector<boost::regex > const & reg,CPRes& res)
{
	boost::smatch what;
	if(!boost::regex_match(com,what,reg[0])) return false;
	TiXmlDeclaration* decl=NEWD("1.0","","");
	res.LinkEndChild(decl);
	TiXmlComment* comment=NEWC("select");
	res.LinkEndChild(comment);
	TiXmlElement* root=NEWE("Select");
	res.LinkEndChild(root);
	string substr1=what[1].str();
	string substr2=what[2].str();
	string substr3=what[4].str();
	string::const_iterator at=substr2.begin();
	string::const_iterator end=substr2.end();
	while(boost::regex_search(at,end,substr2,reg[2]))
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
				son->LinkEndChild(NEWT("*"));
				element->LinkEndChild(son);
				element=element->NextSiblingElement("Table");
			}
		}
		else
		{
			if(!what[1].matched) return false;
			TiXmlElement* element=root->FirstChildElement("Table");
			while(element!=NULL)
			{
				string name=element->Attribute("name");
				if(name.compare(what[2].str())==0)
				{
					TiXmlElement* son=NEWE("Feild");
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
			root->LinkEndChild(element);
			at=what[0].second;
		}
	}
	return true;
}

bool DropTable(string const & com,vector<boost::regex > const & reg,CPRes& res)
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
}

bool DropDB(string const & com,vector<boost::regex > const & reg,CPRes& res)
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
}


CProcessor::CProcessor(const std::string xml)
{
	typedef pair<string,vector<boost::regex > > Pair;
	
	TiXmlDocument doc(xml);
	doc.LoadFile();
	//TiXmlHandle hdoc(&doc);
	TiXmlElement* root=doc.FirstChildElement();//hdoc.FirstChildElement().ToElement();
	int expressions;
	root->Attribute("count",&expressions);
	TiXmlElement* element=root->FirstChildElement();
	while(element!=NULL)
	{
		string name=element->Attribute("name");
		vector<boost::regex > regs;
		TiXmlElement* son=element->FirstChildElement();
		while(son!=NULL)
		{
			regs.push_back(son->GetText());
			son=son->NextSiblingElement();
		}
		regexs.insert(Pair(name,regs));
		element=element->NextSiblingElement();
	}
	
}

bool CProcessor::SplitCom(const std::string &com, CPRes &res)
{
	string name=com.substr(0,com.find_first_of(" "));
	regexmap::iterator=iter;
	iter=regexs.find(name);
	if(iter==regexs.end()) return false;
	

}