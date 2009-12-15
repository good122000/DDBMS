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
	char flage;   //0���һ����Ƭ��1���з�Ƭ
	char type;
	unsigned len;
	char data[6400];
};

#pragma pack(pop)

#endif    //DEFINE_INCLUDE
