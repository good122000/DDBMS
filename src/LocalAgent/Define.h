#ifndef DEFINE_INCLUDE
#define DEFINE_INCLUDE

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



#pragma pack(push,1)
struct datapack
{
	char flage;   //0最后一个分片，1还有分片
	char type;
	unsigned len;
	char data[6400];
};

#pragma pack(pop)

#endif    //DEFINE_INCLUDE
