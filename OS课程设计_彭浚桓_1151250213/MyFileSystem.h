#pragma once

#include <string>
#include <time.h>

struct TxtBlock;
using namespace std;

// TODO: 在此处引用程序需要的其他头文件

//一些常量的定义
const int BLOCK_SIZE = 64; //磁盘块的大小——64字节
const int DIRECORY_ENTRY = 16;//目录项的大小——16字节

struct Disc;
struct RootDirectory;
struct I_NodeBitmap;
struct BlockBitmap;
struct I_NODE;
union DataBlock;

struct DirecoryEntry;
struct IndexBlock;
struct TxtBlock;
struct DirectoryFileBlock;

//目录项
struct DirecoryEntry
{
	char fileName[11];//文件或目录名
	unsigned char flag;//0 表示一般文件，1 表示目录
	int i_node_number;//i-node 编号——指向文件或者目录的索引节点
	DirecoryEntry(){
		fileName[0] = '\0';
		flag = -1;
		i_node_number = -1;
	}
	bool init(char fileName_[11],int flag_,int i_node_number_)
	{

		strcpy(this->fileName, fileName_);
		this->flag = flag_;
		this->i_node_number = i_node_number_;
		return true;
	}

};

//索引数据块
struct IndexBlock
{
	int indexs[16]{-1};//每个盘块号64字节，可以存放16个索引，即16个blockbitmap的下标
	
};

//文本文件数据块
struct TxtBlock
{
	char txt[BLOCK_SIZE];
};

//目录文件数据块
struct DirectoryFileBlock
{
	DirecoryEntry direcoryEntry[4];
};

union DataBlock
{
	IndexBlock indexBlock;//索引块
	TxtBlock txtBlock;//文本文件数据块
	DirectoryFileBlock directoryBlock;//目录文件数据块
	DataBlock(void){

	}
};


//根目录——最多四个目录项
struct RootDirectory
{
	DirecoryEntry direcoryEntries[4];
	bool getAnVoidDirecoryEntry(int &j)
	{
		for (int i = 0; i < 4; i++){
			if(direcoryEntries[i].fileName[0]=='\0')
			{
				j = i;
				return true;
			}
		}
		return false;
	}
	
};

//i-node位图
struct I_NodeBitmap
{
	bool i_node_bitmap[512]{false};
	//获取一个能够使用的i-node的下标

	/**
	 * \brief 获取一个能够使用的i-node下标
	 * \param i 用于存储下标的整形
	 * \return 如果存在->true
	 */
	bool getAnINodeNum(int& i)
	{
		i = 0;
		// ReSharper disable once CppPossiblyErroneousEmptyStatements
		while (i < 1024 && i_node_bitmap[i++] != false);
		i -= 1;//抵消掉最后一次i++

		//如果存在，则对应位是false，则返回=！false->true
		//如果不存在，则i最后等于1023，其对应位为true，则返回=！true -> false

		return !i_node_bitmap[i];
	}
};

//数据块位图——描述磁盘的状态
struct BlockBitmap
{
	bool blocks[512 * 2]{false};

	
	/**
	 * \brief 获取一个能够使用的数据块的下标
	 * \param i 
	 * \return 如果存在->true
	 */
	bool getABlockNum(int& i)
	{
		i = 0;
		// ReSharper disable once CppPossiblyErroneousEmptyStatements
		while (i < 1024 && blocks[i++] != false);
		return !blocks[i];
	}
};

//i-node 长度为16字节（1+1+1+1+2*4+4）
//在文件超出两个盘块时，将使用一级索引地址，就是说文件最大为18个盘块
struct I_NODE
{
	unsigned char isReadOnly;// 是否只读
	unsigned char isHide;// 是否隐藏
	unsigned char hour;// 建立文件系统时间的小时
	unsigned char minutes;// 建立文件系统时间的分钟
	int directAddress[2];// 直接盘块地址——指向一个数据块
	int firstClassIndexAddress;// 一级索引地址——指向一个索引块

	I_NODE()
	{
		isReadOnly = '0';
		isHide = '0';

		time_t t = time(nullptr);
		char temp[1];
		strftime(temp, sizeof(temp), "%H", localtime(&t));
		hour = *temp;
		strftime(temp, sizeof(temp), "%M", localtime(&t));
		minutes = *temp;
		directAddress[0] = -1;
		directAddress[1] = -1;
		firstClassIndexAddress = -1;
	}

	void init()
	{
		time_t t = time(nullptr);
		char temp[1];
		strftime(temp, sizeof(temp), "%H", localtime(&t));
		hour = *temp;
		strftime(temp, sizeof(temp), "%M", localtime(&t));
		minutes = *temp;
	}
	

	//判断当前i-node能否继续添加子i-node(子目录或者子文件）
	bool isFull(DataBlock dataBlocks[])
	{
		if (directAddress[0]!=-1 && directAddress[1]!=-1 && firstClassIndexAddress!=-1 && dataBlocks[firstClassIndexAddress].indexBlock.indexs[15]!=-1)//非空
		{
			return true;
		}
		return false;
	}

	bool addChild(int childINodeNum, DataBlock dataBlocks[], BlockBitmap blockBitMap)
	{
		//TODO 在这里添加关于增加子节点的代码
		if (directAddress[0]==-1)
		{
			directAddress[0] = childINodeNum;
		}else if(directAddress[1]==-1)
		{
			directAddress[1] = childINodeNum;
		}else if (firstClassIndexAddress!=-1)
		{
			for (int i = 0;i<15;i++)
			{
				if(dataBlocks[firstClassIndexAddress].indexBlock.indexs[i] == -1)
				{
					dataBlocks[firstClassIndexAddress].indexBlock.indexs[i] = childINodeNum;
				}
				
			}
		}else if (firstClassIndexAddress == -1)//直接索引满，一级索引为空，要先申请一个索引块
		{
			int i;
			if (blockBitMap.getABlockNum(i)){//找到一个空数据块来作为索引块
				blockBitMap.blocks[i] = true;//更改对应状态位的值（false->true）
				dataBlocks[i].indexBlock.indexs[0] = childINodeNum;//将子节点存入索引块中
				firstClassIndexAddress = i;//更新一级索引块位置
			}else
			{
				return false;
			}
		}
		return true;
	}

};

//磁盘
struct Disc
{
	 RootDirectory rootDirectory;
	 I_NodeBitmap i_nodeBitMap;//描述512个i-node的状态
	 BlockBitmap blockBitMap;//描述1024块磁盘块的状态
	 I_NODE i_node_s[512];//512个i-node占用磁盘128块
	 DataBlock dataBlocks[1024];

};


class Cmd
{
public:

	Cmd(Disc* disc);
	~Cmd();
	bool parse(string cmd);
	static Cmd* getInstance(Disc* disc);
	static string* split(string s, char c);

	Disc* disc;

private:

	int cwd_inode;
	string cwd;
public:
	int getCwdINode() const
	{
		return cwd_inode;
	}

	void setCwdINode(int cwd_inode)
	{
		this->cwd_inode = cwd_inode;
	}

	string getCwd() const
	{
		return cwd;
	}

	void setCwd(const string& cwd)
	{
		this->cwd = cwd;
	}

private:
	Cmd();
	static Cmd* instance;
	bool Format();//初始化磁盘，划定结构
	bool MkFile(string filepath);//创建文件
	bool MkDir(string dir);//创建目录
	bool Cd(string dir);//改变当前目录
	bool DelFile(string filepath);//删除文件（注意只读属性）
	bool DelDir(string dir);//删除目录
	bool Dir();//列文件目录
	bool Copy(string orign_path, string goal_path);//复制文件到某一路径
	bool Open(string filepath);//打开并编辑文件
	bool Attrib(string file_path, string operation);//更改文件属性（是否为只读、是否被隐藏）
	bool ViewINodeMap();//显示当前i-node位示图状况
	bool ViewBlockMap();//显示当前block位示图状况
};

inline Cmd::Cmd() : disc(nullptr), cwd_inode(-1){
}


inline Cmd::Cmd(Disc* disc):cwd_inode(-1)
{
	
	this->disc = disc;
}

inline Cmd::~Cmd()
{
}

inline Cmd* Cmd::getInstance(Disc* disc)
{
	if (instance == nullptr)
	{
		instance = new Cmd(disc);
	}

	return instance;
}

//完成命令的分割——默认以空格作为分隔符
inline string* Cmd::split(string s, char c = ' ')
{
	string* strings = new string[3];
	string temps(s);
	// ReSharper disable once CppInitializedValueIsAlwaysRewritten
	for (auto i = 0, j = 0;; j++)
	{
		if ((i = temps.find_first_of(c)) != string::npos)
		{
			strings[j] = string(temps, 0, i);
			temps = string(temps, i + 1);
		}
		else
		{
			strings[j] = string(temps, 0, i);
			break;
		}
	}
	return strings;
}
