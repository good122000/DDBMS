#include "Res.h"

QueryRes::QueryRes(const std::string data)
{
	doc_.Parse(data.c_str());
}

void QueryRes::Jion(const QueryRes &x)
{
	//ºŸ…Ëµ›‘ˆ≈≈–Ú
	std::vector<std::string >* temp=new std::vector<std::string >[rows+x.rows+1];
	memset(temp,0,sizeof(temp)*(rows+x.rows+1));
	temp[0].assign(columns+x.columns);
	std::vector<std::string >::iterator iter1,iter2=temp[0].begin();
	for(iter1=table[0].begin();iter1!=table[0].end();++iter1)
	{
		(*iter2)=(*iter1);
		++iter2;
	}
	for(iter1=x.table[0].begin();iter1!=x.table[0].end();++iter1)
	{
		(*iter2)=(*iter1);
		++iter2;
	}
	int xrow=1,temprow=0;
	for(int i=1;i<=rows && xrow<=x.rows;)
	{
		int cmp=table[i][0].compare(x.table[xrow][0]);
		if(cmp==0)
		{
			temp[temprow+1].assign(columns+x.columns);
			for(iter1=table[i].begin();iter1!=table[i].end();++iter1)
			{
				(*iter2)=(*iter1);
				++iter2;
			}
			for(iter1=x.table[xrow].begin();iter1!=x.table[xrow].end();++iter1)
			{
				(*iter2)=(*iter1);
				++iter2;
			}
			i++;
			xrow++;
			temprow++;
		}
		else if(cmp>0)
		{
			xrow++;
		}
		else i++;
	}
	rows=temprow;
	columns=columns+x.columns;
	delete[] table;
	table=temp;
}

unsigned QueryRes::Rows()
{
	return rows;
}

unsigned QueryRes::Columns()
{
	return columns;
}

void QueryRes::Union(const QueryRes &x)
{
	std::vector<std::string >* temp=new std::vector<std::string >[rows+x.rows+1];
	memset(temp,0,sizeof(temp)*(rows+x.rows+1));
	temp[0]=table[0];;
	int xrow=1,temprow=0,i;
	for(i=1;i<=rows && xrow<=x.rows;)
	{
		int cmp=table[i][0].compare(x.table[xrow][0]);
		if(cmp==0)
		{
			xrow++;
		}
		else if(cmp>0)
		{
			temp[temprow+1]=x.table[xrow];
			xrow++;
			temprow++;
		}
		else
		{
			temp[temprow+1]=table[i];
			i++;
			temprow++;
		}
	}
	while(i<=rows)
	{
		temp[temprow+1]=table[i];
		i++;
		temprow++;
	}
	while(xrow<=x.rows)
	{
		temp[temprow+1]=x.table[xrow];
		xrow++;
		temprow++;
	}
	rows=temprow;
	delete[] table;
	table=temp;

}


void QueryRes::Projection(std::string const x[])
{
	
	
}

bool QueryRes::Empty()
{
	return rows>0 ? true : false ;
}