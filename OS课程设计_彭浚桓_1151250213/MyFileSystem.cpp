// MyFileSystem.cpp : 定义控制台应用程序的入口点。
//


#include "MyFileSystem.h"//定义基本结构
#include <iostream>
#include <sstream>

using namespace std;

void usage();
Cmd* Cmd::instance = nullptr;

int main(int args, char* argv[]) {
	Disc* disc = nullptr;


	cout << "输入 help 获取帮助" << ",输入 Exit 退出" << endl;
	string input_cmd;
	Cmd* cmd = Cmd::getInstance(disc);
	while (true) {
		cout << cmd->getCwd() << "  >>";
		getline(cin, input_cmd);
		if (input_cmd._Equal("Exit") || input_cmd._Equal("exit"))
			break;
		if (input_cmd._Equal("help")) {
			usage();
			continue;
		}

		auto flag = cmd->parse(input_cmd);
		if (!flag) {
			cout << "Error" << endl;
		}
	}
	return 0;
}

bool Cmd::parse(string cmd) {
	string* strings = split(cmd);

	if (strings[0]._Equal("Format")) {
		Format();
	}
	else if (disc == nullptr) {
		cout << "硬盘未初始化！！！请输入 help 查看命令！！" << endl;
		return false;
	}
	else if (strings[0]._Equal("MKfile")) {
		Mk(strings[1], false);
	}
	else if (strings[0]._Equal("MKdir")) {
		Mk(strings[1], true);
	}
	else if (strings[0]._Equal("Cd")) {
		Cd(strings[1]);
	}
	else if (strings[0]._Equal("Delfile")) {
		DelFile(strings[1]);
	}
	else if (strings[0]._Equal("Dir")) {
		Dir();
	}
	else if (strings[0]._Equal("Copy")) {
		Copy(strings[1], strings[2]);
	}
	else if (strings[0]._Equal("Open")) {
		Open(strings[1]);
	}
	else if (strings[0]._Equal("Attrib")) {
		Attrib(strings[1], strings[2]);
	}
	else if (strings[0]._Equal("Viewinodemap")) {
		ViewINodeMap();
	}
	else if (strings[0]._Equal("Viewblockmap")) {
		ViewBlockMap();
	}
	else if (strings[0]._Equal("test")) {
		//创建大量目录以检测程序是否正确
		for (int i = 0; i < 70; i++) {
			string temp;

			std::stringstream ss;

			ss << i;
			ss >> temp;
			Mk(temp, true);
		}
	}
	else {
		cout << strings[0] << "找不到命令！请重新输入" << endl;
	}
	return true;
}

bool Cmd::Format() {
	cout << "正在初始化硬盘……请等待" << endl;
	disc = new Disc;
	for (int i = 0; i < 1024; i++) {
		disc->blockBitMap.blocks[i] = false;
		if (i < 512)
			disc->i_nodeBitMap.i_node_bitmap[i % 512] = false;
	}
	if (disc->dataBlocks != nullptr) {
		cout << "初始化成功……" << endl;
		cwd = "Root/";
		return true;
	}
	return false;
}


bool Cmd::Mk(string dir, bool isDir) {
	int i_node_num;
	if (cwd_inode != -1) {//非根目录
		I_NODE* parentINode = &(disc->i_node_s[cwd_inode]);
		if (parentINode->isFull(disc->dataBlocks)) {
			cout << "该目录下已满，不能再添加任何" << (isDir ? "目录" : "文件") << endl;
			return false;
		}


		if (disc->i_nodeBitMap.getAnINodeNum(i_node_num)) {//找到空i-node
			disc->i_nodeBitMap.i_node_bitmap[i_node_num] = true;//更改对应i-node的状态
			disc->i_node_s[i_node_num].init();//初始化i-node——主要是对时间的更改
			parentINode->addChild(i_node_num, disc->dataBlocks, disc->blockBitMap, dir, isDir);//完成父节点到子节点的连接,true--目录
		}
		else {
			cout << "空间不足，无法创建" << (isDir ? "目录" : "文件") << "！";
			disc->i_nodeBitMap.i_node_bitmap[i_node_num] = false;//不能创建相应的数据，i-node对应位应该置为false

			return false;
		}
	}
	else//根节点
	{
		int j;
		if (disc->rootDirectory.getAnVoidDirecoryEntry(j))//获取可用根目录
		{
			if (disc->i_nodeBitMap.getAnINodeNum(i_node_num)) {
				disc->i_nodeBitMap.i_node_bitmap[i_node_num] = true;
				disc->i_node_s[i_node_num].init();
			}

			disc->rootDirectory.direcoryEntries[j].init(dir._Myptr(), isDir, i_node_num);
		}
		else {
			cout << "根目录已满，不能再向其添加目录" << endl;
		}
	}
	return true;
}

bool Cmd::Cd(string path) {
	if (path._Equal(".."))
	{
		if(iNodeManager->getParent(cwd_inode)) {
			string temp = cwd;
			int i = temp.find_last_of('/');
			temp = string(temp,0, i);
			i = temp.find_last_of('/');
			temp = string(temp, 0, i+1);
			cwd = temp;
		}

		return true;
	}

	if (cwd_inode == -1)//根目录
	{

		for (size_t i = 0; i < 4; i++) {
			DirecoryEntry direcoryEntry = disc->rootDirectory.direcoryEntries[i];
			if (path._Equal(direcoryEntry.fileName)) {//同名
				if (direcoryEntry.flag == 1) {//目录
					iNodeManager->addInode(cwd_inode);
					cwd_inode = direcoryEntry.i_node_number;
					
					cwd = cwd + path + "/";

					return true;
				}
				cout << path << "不是目录" << endl;
				return false;

			}
		}
		cout << path << "不存在" << endl;
		return false;
	}

	int childINode;
	bool isDir;
	I_NODE parentINode = disc->i_node_s[cwd_inode];
	isDir = parentINode.existChild(path, disc->dataBlocks, childINode);
	if (childINode != -1) {//不等于-1则说明存在
		if (isDir)//true表示目录
		{
			iNodeManager->addInode(cwd_inode);
			cwd_inode = childINode;
			cwd = cwd + path + "/";
			return true;
		}
		cout << path << "不是目录" << endl;
		return false;
	}
	cout << path << "不存在" << endl;
	return true;
}

bool Cmd::DelFile(string cmd) {
	return true;
}

bool Cmd::DelDir(string cmd) {
	return true;
}

bool Cmd::Dir() {
	return true;
}

bool Cmd::Copy(string orign_path, string goal_path) {
	return true;
}

bool Cmd::Open(string cmd) {
	return true;
}

bool Cmd::Attrib(string file_path, string operation) {
	return true;
}

bool Cmd::ViewINodeMap() {
	return true;
}

bool Cmd::ViewBlockMap() {
	return true;
}


void usage() {
	cout << "Format\t\t\t初始化磁盘，划定结构" << endl <<
		"Mkfile [file path]\t\t创建文件" << endl <<
		"Mkdir [dir path]\t\t创建目录" << endl <<
		"Cd [dir path]\t\t改变当前目录" << endl <<
		"Delfile [file path]\t删除文件（注意只读属性）" << endl <<
		"Deldir [dir path]\t删除目录（注意只读属性）" << endl <<
		"Dir\t\t\t列文件目录 （列出名字和建立时间，注意隐藏属性）" << endl <<
		"Copy [origin file path] [goal file path]\t\t\t复制文件到某一路经" << endl <<
		"Open [file path]\t打开并编辑文件（注意只读属性）" << endl <<
		"Attrib [+r|-r] [+h|-h]  [file path]\t\t更改文件属性，加只读，减只读，加隐藏，减隐藏" << endl <<
		"Viewinodemap\t\t显示当前inode位示图状况" << endl <<
		"Viewblockmap\t\t显示当前block位示图状况" << endl;
}


bool DirecoryEntry::init(char fileName_[11], int flag_, int i_node_number_){
	strcpy_s(this->fileName, fileName_);
	this->flag = flag_;
	this->i_node_number = i_node_number_;
	return true;
}

bool RootDirectory::getAnVoidDirecoryEntry(int& j){
	for (int i = 0; i < 4; i++){
		if (direcoryEntries[i].fileName[0] == '#'){
			j = i;
			return true;
		}
	}
	return false;
}

bool I_NodeBitmap::getAnINodeNum(int& i){
	i = 0;
	// ReSharper disable once CppPossiblyErroneousEmptyStatements
	while (i < 1024 && i_node_bitmap[i++] != false);
	i -= 1;//抵消掉最后一次i++

		   //如果存在，则对应位是false，则返回=！false->true
		   //如果不存在，则i最后等于1023，其对应位为true，则返回=！true -> false

	return !i_node_bitmap[i];
}

bool BlockBitmap::getABlockNum(int& i){
	i = 0;
	// ReSharper disable once CppPossiblyErroneousEmptyStatements
	while (i < 1024 && blocks[i++] != false);
	i -= 1;
	return !blocks[i];
}

void I_NODE::init(){
	time_t t = time(nullptr);
	char temp[1];
	strftime(temp, sizeof(temp), "%H", localtime(&t));
	hour = *temp;
	strftime(temp, sizeof(temp), "%M", localtime(&t));
	minutes = *temp;
}

bool I_NODE::isFull(DataBlock dataBlocks[]){
	//一级数据块为空——没有full
	if (directAddress[0] == -1){
		return false;
	}
	//一级数据块为空——没有full
	if (directAddress[1] == -1){
		return false;
	}
	//一级索引块为空——没有full
	if (firstClassIndexAddress == -1){
		return false;
	}


	if (dataBlocks[directAddress[0]].directoryBlock.direcoryEntry[0].flag == 1)
		if (dataBlocks[directAddress[0]].directoryBlock.direcoryEntry[3].fileName[0] == -51)
			return false;
	if (dataBlocks[directAddress[1]].directoryBlock.direcoryEntry[0].flag == 1)
		if (dataBlocks[directAddress[1]].directoryBlock.direcoryEntry[3].fileName[0] == -51)
			return false;
	for (int i = 0; i < 16; i++){
		int j = dataBlocks[firstClassIndexAddress].indexBlock.indexs[i];
		if (j != -1){
			DataBlock datablock = dataBlocks[j];
			if (datablock.directoryBlock.direcoryEntry[0].flag == 1){
				if (datablock.directoryBlock.direcoryEntry[3].fileName[0] == -51){
					return false;
				}
			}
		} else{ return false; }
	}
	return true;
}
bool I_NODE::addChild(int childINodeNum, DataBlock dataBlocks[], BlockBitmap& blockBitMap, string path, bool isDir){
	//TODO 在这里添加关于增加子节点的代码
	for (int i = 0; i < 2; i++){
		if (directAddress[i] == -1){
			//直接地址为空——申请数据块
			if (blockBitMap.getABlockNum(directAddress[i])){
				blockBitMap.blocks[directAddress[i]] = true;
				DirecoryEntry childDirectory;
				childDirectory.init(path._Myptr(), isDir, childINodeNum);
				dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[0] = childDirectory;
				return true;
			}
			//没有空余数据块作为一级数据块

			return false;
		}
		//直接地址不为空，需要判断是否还存在空的目录项
		if (dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[0].flag == 1){
			//先判断该数据块是否是目录文件数据块

			//如果不是，则不能作为目录文件数据块

			int z;
			bool flag;
			for (z = 0, flag = false; z < 4; z++){

				if (dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[z].fileName[0] == -51){
					flag = true;
					break;//存在空的目录项
				}
			}
			if (flag){
				DirecoryEntry childDirectory;
				childDirectory.init(path._Myptr(), isDir, childINodeNum);
				dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[z] = childDirectory;
				return true;
			}
		}

	}

	if (firstClassIndexAddress == -1)//直接索引满，一级索引为空，要先申请一个索引块
	{
		int i;
		if (blockBitMap.getABlockNum(i)){//找到一个空数据块来作为索引块
			blockBitMap.blocks[i] = true;//更改对应状态位的值（false->true）
			for (size_t m = 0; m < BLOCK_SIZE / 4; m++){
				dataBlocks[i].indexBlock.indexs[m] = -1;
			}

			int j;
			if (blockBitMap.getABlockNum(j))//找到一个空数据块作为目录文件数据块
			{
				dataBlocks[i].indexBlock.indexs[0] = j;//将目录文件数据块与索引块连接起来
				blockBitMap.blocks[j] = true;
				DirecoryEntry childDirectory;
				childDirectory.init(path._Myptr(), isDir, childINodeNum);
				dataBlocks[j].directoryBlock.direcoryEntry[0] = childDirectory;
				firstClassIndexAddress = i;//更新一级索引块位置
			} else//没有空余数据块作为目录文件数据块
			{
				blockBitMap.blocks[i] = false;
				return false;
			}
		} else//没有空余数据块作为索引块
		{
			return false;
		}
	} else if (firstClassIndexAddress != -1)//一级索引块不为空
	{
		//首先要找到一个非空的二级数据块
		for (size_t i = 0; i < BLOCK_SIZE / 4; i++){
			int j;
			if ((j = dataBlocks[firstClassIndexAddress].indexBlock.indexs[i]) != -1){
				//然后再看有没有空的目录项
				if (dataBlocks[j].directoryBlock.direcoryEntry[0].flag == 1){
					//先判断是否是目录数据块
					int x;
					bool flag = false;
					for (x = 1; x < 4; x++){
						if (dataBlocks[j].directoryBlock.direcoryEntry[x].fileName[0] == -51){
							flag = true;
							break;
						}
					}

					if (flag){
						DirecoryEntry childDirectory;
						childDirectory.init(path._Myptr(), isDir, childINodeNum);
						dataBlocks[j].directoryBlock.direcoryEntry[x] = childDirectory;
						return true;
					}
				}


			} else//二级数据块为空，则需要申请一个数据块
			{
				int y;
				if (blockBitMap.getABlockNum(y)){
					blockBitMap.blocks[y] = true;
					DirecoryEntry childDirectory;
					childDirectory.init(path._Myptr(), 1, childINodeNum);
					dataBlocks[y].directoryBlock.direcoryEntry[0] = childDirectory;

					//应该在一级索引块【i】的位置上放置新的数据块
					dataBlocks[firstClassIndexAddress].indexBlock.indexs[i] = y;
					return true;
				}

				return false;
			}
		}
	}
	return false;
}
bool I_NODE::existChild(string child, DataBlock dataBlocks[], int& inodeNum){
	//依据
	inodeNum = -1;
	for (size_t j = 0; j < 2; j++){
		if (directAddress[j] != -1){
			DataBlock datablock = dataBlocks[directAddress[j]];
			if (datablock.directoryBlock.direcoryEntry[0].flag == 1){//判断是否是目录文件块
				for (int i = 0; i < 4; i++){
					DirecoryEntry directEntry = datablock.directoryBlock.direcoryEntry[i];
					if (child._Equal(directEntry.fileName)){
						inodeNum = directEntry.i_node_number;
						if (directEntry.flag == 1)//目录
						{
							return true;
						}
						return false; //一般文件
					}
				}
			}

		}
	}
	if (firstClassIndexAddress != -1)//一级索引不为空
	{
		DataBlock datablock = dataBlocks[firstClassIndexAddress];
		for (size_t i = 0; i < 16; i++){
			int j;
			if ((j = datablock.indexBlock.indexs[i]) != -1){//拿到非空二级数据块
				if (dataBlocks[j].directoryBlock.direcoryEntry[0].flag == 1){//判断是否是目录文件数据块
					for (int k = 0; k < 4; k++){
						DirecoryEntry directEntry = dataBlocks[j].directoryBlock.direcoryEntry[k];
						if (child._Equal(directEntry.fileName)){
							inodeNum = directEntry.i_node_number;
							if (directEntry.flag == 1)//目录
							{
								return true;
							}
							return false; //一般文件
						}
					}
				}
			}
		}
	}

	return true;
}

bool InodeLinkManager::getParent(int &cwd_inode){
	if (tail->val==-2) {
		cout << "根目录，不可退回上级" << endl;
		return false;
	}
	InodeLink *temp = head;
	while (temp->next!=tail) {
		temp = temp->next;
	}
	int parentInodeNum = tail->val;
	tail = temp;
	tail->next = nullptr;
	cwd_inode = parentInodeNum;
	return true;
}

int InodeLinkManager::addInode(int inode_number){
	tail->next = new InodeLink(inode_number);
	tail = tail->next;
	return 0;
}
