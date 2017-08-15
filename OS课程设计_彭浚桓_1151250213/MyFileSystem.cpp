// MyFileSystem.cpp : 定义控制台应用程序的入口点。
//


#include "MyFileSystem.h"//定义基本结构
#include <iostream>
#include <sstream>

using namespace std;

void usage();
Cmd* Cmd::instance = nullptr;

int main(int args, char* argv[]) {
	Disk* disc = nullptr;


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
	else if (disk == nullptr) {
		cout << "硬盘未初始化！！！请输入 help 查看命令！！" << endl;
		return false;
	}
	else if (strings[0]._Equal("MKfile")) {
		Mk(cwd_inode_num,strings[1], false);
	}
	else if (strings[0]._Equal("MKdir")) {
		Mk(cwd_inode_num,strings[1], true);
	}
	else if (strings[0]._Equal("Cd")) {
		Cd(strings[1]);
	}
	else if (strings[0]._Equal("Delfile")) {
		DelFile(strings[1],false);
	}
	else if (strings[0]._Equal("Deldir")) {
		int level = 0;
		DelDir(strings[1], level);
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
		Attrib(strings[2], strings[1]);
	}
	else if (strings[0]._Equal("Viewinodemap")) {
		ViewINodeMap();
	}
	else if (strings[0]._Equal("Viewblockmap")) {
		ViewBlockMap();
	}
//	else if (strings[0]._Equal("testDir")) {
//		//创建大量目录以检测程序是否正确
//		for (int i = 0; i < 70; i++) {
//			string temp;
//
//			stringstream ss;
//
//			ss << i;
//			ss >> temp;
//			Mk(temp, true);
//		}
//	}
//	else if (strings[0]._Equal("testFile")) {
//		//创建大量文件以检测程序是否正确
//		for (int i = 0; i < 70; i++) {
//			string temp;
//
//			stringstream ss;
//
//			ss << i;
//			ss >> temp;
//			Mk(temp, false);
//		}
//	}
	else {
		cout << strings[0] << "找不到命令！请重新输入" << endl;
		usage();
	}
	return true;
}

bool Cmd::Format() {
	cout << "正在初始化硬盘……请等待" << endl;
	disk = new Disk;
	for (int i = 0; i < 1024; i++) {
		disk->blockBitMap.blocks[i] = false;
		if (i < 512)
			disk->i_nodeBitMap.i_node_bitmap[i % 512] = false;
	}
	if (disk->dataBlocks != nullptr) {
		cout << "初始化成功……" << endl;

		head = new InodeLink(-1);
		iNodeManager = new InodeLinkManager(head);
		cwd_inode_num = -1;


		cwd = "Root/";
		return true;
	}
	return false;
}


bool Cmd::Mk(string dir, bool isDir) {
bool Cmd::Mk(int inode,string dir, bool isDir) {
	if (dir._Equal("")) {
		cout << "请输入参数" << endl;
		return false;
	}
	int i_node_num;
	if (inode != -1) {//非根目录
		I_NODE* parentINode = &(disk->i_node_s[inode]);
		if (parentINode->isFull(disk->dataBlocks)) {
			cout << "该目录下已满，不能再添加任何" << (isDir ? "目录" : "文件") << endl;
			return false;
		}

		//判断是否存在同名
		if (isDir)//如果是一个目录，则判断是否有同名目录存在
		{
			int temp;
			if (parentINode->existChild(dir, disk->dataBlocks, temp)) {//为真则表示是一个目录（可能不存在）
				if (temp != -1) {//存在
					cout << "同名" << (isDir ? "目录" : "文件") << "已经存在!!!" << endl;
					return false;
				}
			}

		}
		else {//如果是创建文件，则判断是否有同名文件存在
			int temp;
			if (!parentINode->existChild(dir, disk->dataBlocks, temp)) {//为false则表示是一个目录
				if (temp != -1) {//存在
					cout << "同名" << (isDir ? "目录" : "文件") << "已经存在!!!" << endl;
					return false;
				}
			}
		}


		if (disk->i_nodeBitMap.getAnINodeNum(i_node_num)) {//找到空i-node
			disk->i_nodeBitMap.i_node_bitmap[i_node_num] = true;//更改对应i-node的状态
			disk->i_node_s[i_node_num].init();//初始化i-node——主要是对时间的更改

			disk->blockBitMap.getABlockNum(disk->i_node_s[i_node_num].directAddress[0]);
			disk->blockBitMap.blocks[disk->i_node_s[i_node_num].directAddress[0]] = true;


			parentINode->addChild(i_node_num, disk->dataBlocks, disk->blockBitMap, dir, isDir);//完成父节点到子节点的连接,true--目录
			return true;
		}
		cout << "不存在空余i-node，无法创建" << (isDir ? "目录" : "文件") << "！";
		disk->i_nodeBitMap.i_node_bitmap[i_node_num] = false;//不能创建相应的数据，i-node对应位应该置为false

		return false;
	}
	//根节点
	//判断是否存在同名
	RootDirectory root = disk->rootDirectory;
	for (size_t temp = 0; temp < 4; temp++) {
		if (dir._Equal(root.direcoryEntries[temp].fileName)) {
			cout << "同名" << (isDir ? "目录" : "文件") << "已经存在!!!" << endl;
			return false;
		}
	}
	int j;
	if (root.getAnVoidDirecoryEntry(j))//获取可用根目录
	{


		if (disk->i_nodeBitMap.getAnINodeNum(i_node_num)) {
			disk->i_nodeBitMap.i_node_bitmap[i_node_num] = true;
			disk->i_node_s[i_node_num].init();

			disk->blockBitMap.getABlockNum(disk->i_node_s[i_node_num].directAddress[0]);
			disk->blockBitMap.blocks[disk->i_node_s[i_node_num].directAddress[0]] = true;

			disk->rootDirectory.direcoryEntries[j].init(dir._Myptr(), isDir, i_node_num);
			return true;
		}

		cout << "不存在空闲i-node" << endl;
		return false;


	}

	cout << "根目录已满，不能再向其添加目录" << endl;
	return false;

}

bool Cmd::Cd(string path) {
	if (path._Equal("")) {
		cout << "请输入参数" << endl;
		return false;
	}
	if (path._Equal("..")) {
		if (iNodeManager->getParent(cwd_inode_num)) {
			string temp = cwd;
			int i = temp.find_last_of('/');
			temp = string(temp, 0, i);
			i = temp.find_last_of('/');
			temp = string(temp, 0, i + 1);
			cwd = temp;
		}

		return true;
	}

	if (cwd_inode_num == -1)//根目录
	{

		for (size_t i = 0; i < 4; i++) {
			DirectoryEntry direcoryEntry = disk->rootDirectory.direcoryEntries[i];
			if (path._Equal(direcoryEntry.fileName)) {//同名
				if (direcoryEntry.flag == 1) {//目录

					cwd_inode_num = direcoryEntry.i_node_number;
					iNodeManager->addInode(cwd_inode_num);
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
	I_NODE parentINode = disk->i_node_s[cwd_inode_num];
	isDir = parentINode.existChild(path, disk->dataBlocks, childINode);
	if (childINode != -1) {//不等于-1则说明存在
		if (isDir)//true表示目录
		{
			iNodeManager->addInode(cwd_inode_num);
			cwd_inode_num = childINode;
			cwd = cwd + path + "/";
			return true;
		}
		cout << path << "不是目录" << endl;
		return false;
	}
	cout << path << "不存在" << endl;
	return true;
}

bool Cmd::DelFile(string path,bool del) {
	//判断是否根目录
	if (cwd_inode_num == -1) {

		for (size_t i = 0; i < 4; i++) {
			if (path._Equal(disk->rootDirectory.direcoryEntries[i].fileName)) {
				if (del)
				{
					if (disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].isReadOnly == 0)
						disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].clear(disk->dataBlocks, disk->blockBitMap);
					disk->i_nodeBitMap.i_node_bitmap[disk->rootDirectory.direcoryEntries[i].i_node_number] = false;
					disk->rootDirectory.direcoryEntries[i].fileName[0] = '\0';
					disk->rootDirectory.direcoryEntries[i].flag = -1;
					disk->rootDirectory.direcoryEntries[i].i_node_number = -1;
					return true;
				}

				if (disk->rootDirectory.direcoryEntries[i].flag == 0)//如果是文件
				{
					if(disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].isReadOnly==0)
					disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].clear(disk->dataBlocks, disk->blockBitMap);
					disk->i_nodeBitMap.i_node_bitmap[disk->rootDirectory.direcoryEntries[i].i_node_number] = false;
					disk->rootDirectory.direcoryEntries[i].fileName[0] = '\0';
					disk->rootDirectory.direcoryEntries[i].flag = -1;
					disk->rootDirectory.direcoryEntries[i].i_node_number = -1;
					return true;
				}else {
					cout << "文件只读，不可删除" << endl;
				}
			}
		}


	}


	I_NODE parent_Inode = disk->i_node_s[cwd_inode_num];
	int child_Inode_num;
	DirectoryEntry* pdircoryEntry = nullptr;
	if (!parent_Inode.existChild(path, disk->dataBlocks, child_Inode_num, &pdircoryEntry)) {//如果是文件则返回false
		if (child_Inode_num != -1) {
			if(del) {
				disk->i_node_s[child_Inode_num].clear(disk->dataBlocks, disk->blockBitMap);//删除文本，同时将对应的数据块bitmap置为false
				disk->i_nodeBitMap.i_node_bitmap[child_Inode_num] = false;
				pdircoryEntry->i_node_number = -1;
				pdircoryEntry->flag = -1;
				pdircoryEntry->fileName[0] = '\0';
				return true;
			}
			if (disk->i_node_s[child_Inode_num].isReadOnly == 0 ){
				disk->i_node_s[child_Inode_num].clear(disk->dataBlocks, disk->blockBitMap);//删除文本，同时将对应的数据块bitmap置为false
				disk->i_nodeBitMap.i_node_bitmap[child_Inode_num] = false;
				pdircoryEntry->i_node_number = -1;
				pdircoryEntry->flag = -1;
				pdircoryEntry->fileName[0] = '\0';
				return true;
			}else {
				cout << "文件只读，不可删除" << endl;
			}

		}
		else {
			cout << "文件不存在" << endl;
		}
	}


	return true;
}


bool Cmd::DelDir(string path, int& level) {

	//递归删除目录，level记录层级

	if (cwd_inode_num == -1) {
		for (int i = 0; i < 4; i++) {
			//同名，且是文件夹
			if (path._Equal(disk->rootDirectory.direcoryEntries[i].fileName) && disk->rootDirectory.direcoryEntries[i].flag == 1) {
				if (disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].isReadOnly == 0){
					Cd(path);
					level++;
					DelDir(disk->rootDirectory.direcoryEntries[i].fileName, level);
					Cd("..");
					level--;
					disk->rootDirectory.direcoryEntries[i].fileName[0] = '\0';
					disk->i_nodeBitMap.i_node_bitmap[disk->rootDirectory.direcoryEntries[i].i_node_number] = false;
					disk->rootDirectory.direcoryEntries[i].flag = -1;

					disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].directAddress[0] = -1;
					disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].directAddress[1] = -1;
					disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].firstClassIndexAddress = -1;
					disk->rootDirectory.direcoryEntries[i].i_node_number = -1;
					return true;
				} else{
					cout << "只读文件夹，不可删除" << endl;
				
				}
			}
		}
	}
	else {
		if (level == 0) {
			I_NODE parent = disk->i_node_s[cwd_inode_num];
			int child_inode;
			DirectoryEntry* pDirectoryEntry;
			if (parent.existChild(path, disk->dataBlocks, child_inode, &pDirectoryEntry)) {
				//为真可能是不存在，可能是找到的是文件夹
				if (child_inode == -1) {
					cout << path << "不存在" << endl;
					return true;
				}
				//找到的是文件夹
				if (disk->i_node_s[pDirectoryEntry->i_node_number].isReadOnly == 0){


					Cd(path);
					level++;
					DelDir(path, level);
					Cd("..");
					level--;

					pDirectoryEntry->fileName[0] = '\0';
					pDirectoryEntry->flag = -1;
					disk->i_nodeBitMap.i_node_bitmap[pDirectoryEntry->i_node_number] = false;

					disk->i_node_s[pDirectoryEntry->i_node_number].directAddress[0] = -1;
					disk->i_node_s[pDirectoryEntry->i_node_number].directAddress[1] = -1;
					disk->i_node_s[pDirectoryEntry->i_node_number].firstClassIndexAddress = -1;


					pDirectoryEntry->i_node_number = -1;
					return true;
				}
				else {
					cout << "只读文件夹，不可删除" << endl;
				}
			}
			cout << path << "不存在" << endl;
			return true;
		}
		//level!=0,则删除cwd_node_num下所有目录文件

		I_NODE cwd_inode = disk->i_node_s[cwd_inode_num];

		for (int i = 0; i < 2; i++) {
			if (cwd_inode.directAddress[i] != -1) {
				DataBlock temp = disk->dataBlocks[cwd_inode.directAddress[i]];
				for (int j = 0; j < 4; j++) {
					if (temp.directoryBlock.direcoryEntry[j].flag == 1) {//目录
						Cd(temp.directoryBlock.direcoryEntry[j].fileName);
						level++;
						DelDir(temp.directoryBlock.direcoryEntry[j].fileName, level);
						Cd("..");
						level--;

						disk->dataBlocks[cwd_inode.directAddress[i]].directoryBlock.direcoryEntry[j].fileName[0] = '\0';
						disk->dataBlocks[cwd_inode.directAddress[i]].directoryBlock.direcoryEntry[j].flag = -1;
						disk->i_nodeBitMap.i_node_bitmap[disk->dataBlocks[cwd_inode.directAddress[i]].directoryBlock.direcoryEntry[j].i_node_number] = false;
						disk->dataBlocks[cwd_inode.directAddress[i]].directoryBlock.direcoryEntry[j].i_node_number = -1;

					}
					else if (temp.directoryBlock.direcoryEntry[j].flag == 0)//文件
					{
						DelFile(temp.directoryBlock.direcoryEntry[j].fileName,true);
						disk->dataBlocks[cwd_inode.directAddress[i]].directoryBlock.direcoryEntry[j].fileName[0] = '\0';
						disk->dataBlocks[cwd_inode.directAddress[i]].directoryBlock.direcoryEntry[j].flag = -1;
						disk->i_nodeBitMap.i_node_bitmap[disk->dataBlocks[cwd_inode.directAddress[i]].directoryBlock.direcoryEntry[j].i_node_number] = false;
						disk->dataBlocks[cwd_inode.directAddress[i]].directoryBlock.direcoryEntry[j].i_node_number = -1;

					}
				}
				disk->blockBitMap.blocks[cwd_inode.directAddress[i]] = false;

			}
		}
		int x;
		if ((x = cwd_inode.firstClassIndexAddress) != -1) {
			IndexBlock indexblock = disk->dataBlocks[x].indexBlock;
			for (size_t i = 0; i < 16; i++) {
				int y;
				if ((y = indexblock.indexs[i]) != -1) {
					DataBlock temp = disk->dataBlocks[y];
					for (int j = 0; j < 4; j++) {
						if (temp.directoryBlock.direcoryEntry[j].flag == 1) {//目录
							Cd(temp.directoryBlock.direcoryEntry[j].fileName);
							level++;
							DelDir(temp.directoryBlock.direcoryEntry[j].fileName, level);
							Cd("..");
							level--;

							disk->dataBlocks[y].directoryBlock.direcoryEntry[j].fileName[0] = '\0';
							disk->dataBlocks[y].directoryBlock.direcoryEntry[j].flag = -1;
							disk->i_nodeBitMap.i_node_bitmap[disk->dataBlocks[y].directoryBlock.direcoryEntry[j].i_node_number] = false;
							disk->dataBlocks[y].directoryBlock.direcoryEntry[j].i_node_number = -1;


						}
						else if (temp.directoryBlock.direcoryEntry[j].flag == 0)//文件
						{
							DelFile(temp.directoryBlock.direcoryEntry[j].fileName,true);
							disk->dataBlocks[y].directoryBlock.direcoryEntry[j].fileName[0] = '\0';
							disk->dataBlocks[y].directoryBlock.direcoryEntry[j].flag = -1;
							disk->i_nodeBitMap.i_node_bitmap[disk->dataBlocks[y].directoryBlock.direcoryEntry[j].i_node_number] = false;
							disk->dataBlocks[y].directoryBlock.direcoryEntry[j].i_node_number = -1;

						}
					}
				}
				disk->blockBitMap.blocks[y] = false;
			}
			disk->blockBitMap.blocks[cwd_inode.firstClassIndexAddress] = false;

		}
	}
	return false;
}


bool Cmd::Dir() {
	cout << "名字" << "\t" << "类型" << "\t" << "时间" << endl;
	int i;
	if (cwd_inode_num == -1) {//根目录
		for (i = 0; i < 4; i++) {
			if (disk->rootDirectory.direcoryEntries[i].fileName[0] > 0) {
				DirectoryEntry temp = disk->rootDirectory.direcoryEntries[i];
				if (temp.i_node_number > -1) {
					I_NODE child = disk->i_node_s[temp.i_node_number];
					if (!child.isHide) {
						cout << temp.fileName << "\t" << ((temp.flag == 1) ? "目录" : "文件") << "\t" << ((int)child.hour) << "：" << ((int)child.minutes) << endl;
					}
				}
			}
		}
		return true;
	}
	I_NODE inode = disk->i_node_s[cwd_inode_num];

	for (int z = 0; z < 2; z++) {


		if ((i = inode.directAddress[z]) != -1)//一级数据块不空
		{
			DataBlock dataBlock = disk->dataBlocks[i];//拿到数据块
			if (dataBlock.directoryBlock.direcoryEntry[z].flag == 1 || dataBlock.directoryBlock.direcoryEntry[0].flag == 0) {//如果是目录文件数据块
				for (size_t j = 0; j < BLOCK_SIZE / 16; j++) {
					DirectoryEntry temp = dataBlock.directoryBlock.direcoryEntry[j];
					if (temp.i_node_number > -1) {
						I_NODE child = disk->i_node_s[temp.i_node_number];
						if (child.isHide == 0) {
							cout << temp.fileName << "\t" << ((temp.flag == 1) ? "目录" : "文件") << "\t" << ((int)child.hour) << "：" << ((int)child.minutes) << endl;
						}
					}
				}
			}
		}
	}

	if ((i = inode.firstClassIndexAddress) != -1)//一级索引块不为空
	{
		IndexBlock indexBlock = disk->dataBlocks[i].indexBlock;//拿到索引块
		for (size_t z = 0; z < 16; z++) {
			if (indexBlock.indexs[z] == -1)//二级数据块空
			{
				break;
			}
			DataBlock dataBlock = disk->dataBlocks[indexBlock.indexs[z]];//拿到二级数据块
			if (dataBlock.directoryBlock.direcoryEntry[0].flag == 1 || dataBlock.directoryBlock.direcoryEntry[0].flag == 0) {//如果是目录文件数据块
				for (size_t j = 0; j < BLOCK_SIZE / 16; j++) {
					DirectoryEntry temp = dataBlock.directoryBlock.direcoryEntry[j];
					if (temp.fileName[0] > 0) {
						I_NODE child = disk->i_node_s[temp.i_node_number];
						if (child.isHide == 0) {
							cout << temp.fileName << "\t" << ((temp.flag == 1) ? "目录" : "文件") << "\t" << ((int)child.hour) << "：" << ((int)child.minutes) << endl;
						}
					}
				}
			}
		}

	}

	return true;
}

bool Cmd::Copy(string origin_path, string goal_path) {
	//TODO 文本复制函数

	if (origin_path._Equal("") || goal_path._Equal("")) {
		cout << "参数不齐！" << endl;
		return false;
	}

	//TODO 完成地址解析，获取目标目录inode

	int j = 0;
	int temp_inode_num = -1;
	int i = goal_path.find_first_of('\\');
	string temp = string(goal_path,j, i);
	j = i+1;
	if (temp._Equal("root"))
	{
		temp_inode_num = -1;
	}
	string goal = string(goal_path, j,goal_path.length());
	i = goal.find_first_of('\\');
	temp = string(goal, 0, i);
	

	for (int x = 0; x <4;x++)
	{
		if (temp._Equal(disk->rootDirectory.direcoryEntries[x].fileName))
		{
			temp_inode_num = disk->rootDirectory.direcoryEntries[x].i_node_number;
			break;
		}
	}
	
	while (true){

		goal = string(goal, j, goal.length());
		i = goal.find_first_of('\\');
		if (i==-1)
		{
			break;
		}
		temp = string(goal, 0, i);
		j = i + 1;
		int temp_int;
		if (disk->i_node_s[temp_inode_num].existChild(temp, disk->dataBlocks, temp_int)&&temp_int!=-1) {
			temp_inode_num = temp_int;
			continue;
		}
	}
	//最后temp_inode_num就是目标文件夹inode



	if (cwd_inode_num == -1) {
		int child_goal = -1, child_origin = -1;
		for (size_t i = 0; i < 4; i++) {
			if (origin_path._Equal(disk->rootDirectory.direcoryEntries[i].fileName)) {
				child_origin = disk->rootDirectory.direcoryEntries[i].i_node_number;
			}
//			if (goal_path._Equal(disk->rootDirectory.direcoryEntries[i].fileName)) {
//				child_goal = disk->rootDirectory.direcoryEntries[i].i_node_number;
//			}

		}
		if (child_origin == -1) {
			cout << "源文件不存在！" << endl;
			return false;
		}


		I_NODE origin = disk->i_node_s[child_origin];

//
//		if (child_goal == -1) {
//			Mk(goal_path, false);
//
//			for (size_t i = 0; i < 4; i++) {
//				if (goal_path._Equal(disk->rootDirectory.direcoryEntries[i].fileName)) {
//					child_goal = disk->rootDirectory.direcoryEntries[i].i_node_number;
//					break;
//				}
//			}
//		}

		disk->i_node_s[child_goal].addText(origin.getTxt(disk->dataBlocks), disk->dataBlocks, disk->blockBitMap);
		return true;
	}
	//非根目录
	I_NODE parent = disk->i_node_s[cwd_inode_num];
	//源文件必须存在
	int child_goal, child_origin;
	if (!parent.existChild(origin_path, disk->dataBlocks, child_origin)) {
		if (child_origin != -1) {
			//if (!parent.existChild(goal_path, disk->dataBlocks, child_goal) && child_goal == -1) {
			if (!disk->i_node_s[temp_inode_num].existChild(origin_path, disk->dataBlocks, child_goal) && child_goal == -1) {
				Mk(temp_inode_num,origin_path, false);
//				parent.existChild(origin_path, disk->dataBlocks, child_goal);
				disk->i_node_s[temp_inode_num].existChild(origin_path, disk->dataBlocks, child_goal);
			}

			I_NODE origin = disk->i_node_s[child_origin];
			disk->i_node_s[child_goal].addText(origin.getTxt(disk->dataBlocks), disk->dataBlocks, disk->blockBitMap);
			return true;

		}

	}
	else {
		cout << "文件不存在或不是文件" << endl;
		return false;
	}

	return true;
}

bool Cmd::Open(string cmd) {
	//TODO 打开文本函数


	if (cwd_inode_num == -1) {
		for (size_t i = 0; i < 4; i++) {
			if (cmd._Equal(disk->rootDirectory.direcoryEntries[i].fileName) && disk->rootDirectory.direcoryEntries[i].flag == 0) {
				if (disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].isReadOnly == 0)//可编辑
				{
					cout << endl;
					disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].show(disk->dataBlocks);
					cout << endl << "是否编辑？(Y/N)" << endl;
					if (getchar() == 'Y') {

						string content;
						cout << "请输入新的内容:" << endl;
						cin >> content;
						char* temp = new char[content.length() + 1];
						strcpy(temp, content.c_str());
						temp[content.length()] = '\0';
						content = string(temp);
						getchar();
						disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].addText(content, disk->dataBlocks, disk->blockBitMap);
						return true;
					}
					getchar();
				}
				else {
					disk->i_node_s[disk->rootDirectory.direcoryEntries[i].i_node_number].show(disk->dataBlocks);
				}
			}
		}
		cout << endl;
		return true;
	}


	I_NODE parent = disk->i_node_s[cwd_inode_num];
	int child_inode;
	if (!parent.existChild(cmd, disk->dataBlocks, child_inode)) {
		if (child_inode != -1) {
			if (disk->i_node_s[child_inode].isReadOnly == 0)//可编辑
			{
				cout << endl;
				disk->i_node_s[child_inode].show(disk->dataBlocks);
				cout << endl << "是否编辑？(Y/N)" << endl;
				if (getchar() == 'Y') {
					getchar();
					string content;
					cout << endl << "请输入新的内容:" << endl;
					cin >> content;
					char* temp = new char[content.length() + 1];
					strcpy(temp, content.c_str());
					temp[content.length()] = '\0';
					content = string(temp);
					getchar();
					disk->i_node_s[child_inode].addText(content, disk->dataBlocks, disk->blockBitMap);
					return true;
				}
				getchar();
			}
			else {
				disk->i_node_s[child_inode].show(disk->dataBlocks);
			}
		}
	}
	cout << endl;
	return true;
}

bool Cmd::Attrib(string path, string operation) {
	if (path.length() == 0 || operation.length() == 0) {
		cout << "参数不全" << endl;
		return false;
	}
	char operation_char[2] = {'h','r'};
	unsigned char operation_result[2] = {0,0};// h,r
	for (size_t j = 0; j < 2; j++) {
		int i = operation.find_first_of(operation_char[j]);
		if (i != -1)//存在h/r
		{
			if (i - 1 >= 0)//判断+/-
				switch (operation[i - 1]) {
				case '+':
					operation_result[j] = 1;
					break;
				case '-':
					operation_result[j] = 0;
					break;
				default:
					operation_result[j] = 0;

				}
			else {
				cout << "参数非法" << endl;
				return false;
			}
		}
	}


	if (cwd_inode_num == -1)//根目录
	{

		for (size_t i = 0; i < 4; i++) {
			DirectoryEntry direcoryEntry = disk->rootDirectory.direcoryEntries[i];
			if (path._Equal(direcoryEntry.fileName)) {//同名
				disk->i_node_s[direcoryEntry.i_node_number].isReadOnly = operation_result[1];
				disk->i_node_s[direcoryEntry.i_node_number].isHide = operation_result[0];
				return true;
			}
		}
		cout << path << "不存在" << endl;
		return false;
	}

	int childINode;

	I_NODE parentINode = disk->i_node_s[cwd_inode_num];
	parentINode.existChild(path, disk->dataBlocks, childINode);
	if (childINode != -1) {//不等于-1则说明存在
		disk->i_node_s[childINode].isReadOnly = operation_result[1];
		disk->i_node_s[childINode].isHide = operation_result[0];
		return true;
	}
	cout << path << "不存在" << endl;
	return false;
}

bool Cmd::ViewINodeMap() const {
	//共计512
	I_NodeBitmap bitmap = disk->i_nodeBitMap;
	for (size_t i = 0; i < 32; i++) {
		for (size_t j = 0; j < 16; j++) {
			cout << bitmap.i_node_bitmap[i * 16 + j] << "  ";
		}
		cout << endl;
	}
	cout << endl;
	return true;
}

bool Cmd::ViewBlockMap() {
	//共计1024
	BlockBitmap bitmap = disk->blockBitMap;
	for (size_t i = 0; i < 32; i++) {
		if (i == 16)
			cout << endl;
		for (size_t j = 0; j < 32; j++) {
			if (j == 16)
				cout << "\t" << endl;
			cout << bitmap.blocks[i * 32 + j] << " ";
		}
		cout << endl;
	}
	cout << endl;
	return true;

}


void usage() {
	cout << "Format\t\t\t初始化磁盘，划定结构" << endl <<
		"Mkfile [file path]\t创建文件" << endl <<
		"Mkdir [dir path]\t创建目录" << endl <<
		"Cd [dir path]\t\t改变当前目录" << endl <<
		"Delfile [file path]\t删除文件（注意只读属性）" << endl <<
		"Deldir [dir path]\t删除目录（注意只读属性）" << endl <<
		"Dir\t\t\t列文件目录" << endl <<
		"Copy [origin file path] [goal file path]\t\t\t复制文件到某一路经" << endl <<
		"Open [file path]\t打开并编辑文件（注意只读属性）" << endl <<
		"Attrib [+r|-r|+h|-h]  [file path]\t\t更改文件属性，加只读，减只读，加隐藏，减隐藏" << endl <<
		"Viewinodemap\t\t显示当前inode位示图状况" << endl <<
		"Viewblockmap\t\t显示当前block位示图状况" << endl;
}


bool DirectoryEntry::init(char fileName_[11], int flag_, int i_node_number_) {
	strcpy_s(this->fileName, fileName_);
	this->flag = flag_;
	this->i_node_number = i_node_number_;
	return true;
}

bool RootDirectory::getAnVoidDirecoryEntry(int& j) {
	for (int i = 0; i < 4; i++) {
		if (direcoryEntries[i].fileName[0] == '\0') {
			j = i;
			return true;
		}
	}
	return false;
}

bool I_NodeBitmap::getAnINodeNum(int& i) {
	i = 0;
	// ReSharper disable once CppPossiblyErroneousEmptyStatements
	while (i < 512 && i_node_bitmap[i++] != false);
	i -= 1;//抵消掉最后一次i++

	//如果存在，则对应位是false，则返回=！false->true
	//如果不存在，则i最后等于1023，其对应位为true，则返回=！true -> false

	return !i_node_bitmap[i];
}

bool BlockBitmap::getABlockNum(int& i) {
	i = 0;
	// ReSharper disable once CppPossiblyErroneousEmptyStatements
	while (i < 1024 && blocks[i++] != false);
	i -= 1;
	return !blocks[i];
}

void I_NODE::init() {
	time_t t = time(nullptr);

	char temp[5];
	strftime(temp, sizeof(temp), "%H", localtime(&t));
	hour = atoi(temp);

	strftime(temp, sizeof(temp), "%M", localtime(&t));
	minutes = atoi(temp);

	isHide = 0;
	isReadOnly = 0;
}

bool I_NODE::isFull(DataBlock dataBlocks[]) {


	for (size_t i = 0; i < 2; i++) {
		if (directAddress[i] != -1) {
			if (dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[0].flag == 1 || dataBlocks[directAddress[0]].directoryBlock.direcoryEntry[0].flag == 0)
				for (int j = 0; j < 4; j++) {
					if (dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[j].fileName[0] <= 0)
						return false;
				}
		}
		else { return false; }
	}


	//一级索引块为空——没有full
	if (firstClassIndexAddress != -1) {
		for (int i = 0; i < 16; i++) {
			int j = dataBlocks[firstClassIndexAddress].indexBlock.indexs[i];
			if (j != -1) {
				DataBlock datablock = dataBlocks[j];//拿到二级数据块
				if (datablock.directoryBlock.direcoryEntry[0].flag == 1 || datablock.directoryBlock.direcoryEntry[0].flag == 0) {
					for (DirectoryEntry temp: datablock.directoryBlock.direcoryEntry) {

						if (temp.fileName[0] <= 0) {
							return false;
						}
					}
				}
				else {
				}//不是目录文件块
			}
			else { return false; }//数据块空，没有满
		}

	}
	else { return false; }//索引块空，则没有满

	return true;
}

bool I_NODE::addChild(int childINodeNum, DataBlock dataBlocks[], BlockBitmap& blockBitMap, string path, bool isDir) {
	//TODO 在这里添加关于增加子节点的代码
	for (int i = 0; i < 2; i++) {
		if (directAddress[i] == -1) {
			//直接地址为空——申请数据块
			if (blockBitMap.getABlockNum(directAddress[i])) {
				blockBitMap.blocks[directAddress[i]] = true;
				DirectoryEntry childDirectory;
				childDirectory.init(path._Myptr(), isDir, childINodeNum);
				dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[0] = childDirectory;
				return true;
			}
			//没有空余数据块作为一级数据块

			return false;
		}
		//直接地址不为空，需要判断是否还存在空的目录项
		if (dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[0].flag == 1 || dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[0].flag == 0) {
			//先判断该数据块是否是目录文件数据块

			//如果不是，则不能作为目录文件数据块

			int z;
			bool flag;
			for (z = 0 , flag = false; z < 4; z++) {

				if (dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[z].i_node_number < 0) {
					flag = true;
					break;//存在空的目录项
				}
			}
			if (flag) {
				DirectoryEntry childDirectory;
				childDirectory.init(path._Myptr(), isDir, childINodeNum);
				dataBlocks[directAddress[i]].directoryBlock.direcoryEntry[z] = childDirectory;
				return true;
			}
		}

	}

	if (firstClassIndexAddress == -1)//直接索引满，一级索引为空，要先申请一个索引块
	{
		int i;
		if (blockBitMap.getABlockNum(i)) {//找到一个空数据块来作为索引块
			blockBitMap.blocks[i] = true;//更改对应状态位的值（false->true）
			for (size_t m = 0; m < BLOCK_SIZE / 4; m++) {
				dataBlocks[i].indexBlock.indexs[m] = -1;
			}

			int j;
			if (blockBitMap.getABlockNum(j))//找到一个空数据块作为目录文件数据块
			{
				dataBlocks[i].indexBlock.indexs[0] = j;//将目录文件数据块与索引块连接起来
				blockBitMap.blocks[j] = true;
				DirectoryEntry childDirectory;
				childDirectory.init(path._Myptr(), isDir, childINodeNum);
				dataBlocks[j].directoryBlock.direcoryEntry[0] = childDirectory;
				firstClassIndexAddress = i;//更新一级索引块位置
			}
			else//没有空余数据块作为目录文件数据块
			{
				blockBitMap.blocks[i] = false;
				return false;
			}
		}
		else//没有空余数据块作为索引块
		{
			return false;
		}
	}
	else if (firstClassIndexAddress != -1)//一级索引块不为空
	{
		//首先要找到一个非空的二级数据块
		for (size_t i = 0; i < BLOCK_SIZE / 4; i++) {
			int j;
			if ((j = dataBlocks[firstClassIndexAddress].indexBlock.indexs[i]) > -1) {
				//然后再看有没有空的目录项
				if (dataBlocks[j].directoryBlock.direcoryEntry[0].flag == 1 || dataBlocks[j].directoryBlock.direcoryEntry[0].flag == 0) {
					//先判断是否是目录数据块
					int x;
					bool flag = false;
					for (x = 1; x < 4; x++) {
						if (dataBlocks[j].directoryBlock.direcoryEntry[x].fileName[0] <= 0) {
							flag = true;
							break;
						}
					}

					if (flag) {
						DirectoryEntry childDirectory;
						childDirectory.init(path._Myptr(), isDir, childINodeNum);
						dataBlocks[j].directoryBlock.direcoryEntry[x] = childDirectory;
						return true;
					}
				}


			}
			else//二级数据块为空，则需要申请一个数据块
			{
				int y;
				if (blockBitMap.getABlockNum(y)) {
					blockBitMap.blocks[y] = true;
					DirectoryEntry childDirectory;
					childDirectory.init(path._Myptr(), isDir, childINodeNum);
					dataBlocks[y].directoryBlock.direcoryEntry[0] = childDirectory;

					//应该在一级索引块【i】的位置上放置新的数据块
					dataBlocks[firstClassIndexAddress].indexBlock.indexs[i] = y;
					return true;
				}

				return false;
			}
		}
	}
	return true;
}

bool I_NODE::existChild(string child, DataBlock dataBlocks[], int& inodeNum, DirectoryEntry** direcoryEntry_) {

	inodeNum = -1;
	for (size_t j = 0; j < 2; j++) {
		if (directAddress[j] != -1) {
			DataBlock datablock = dataBlocks[directAddress[j]];

			for (int i = 0; i < 4; i++) {
				if (datablock.directoryBlock.direcoryEntry[i].flag > -1) {//判断是否是目录文件块
					DirectoryEntry directEntry = datablock.directoryBlock.direcoryEntry[i];
					if (child._Equal(directEntry.fileName)) {
						inodeNum = directEntry.i_node_number;
						*direcoryEntry_ = &dataBlocks[directAddress[j]].directoryBlock.direcoryEntry[i];
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
		for (size_t i = 0; i < 16; i++) {
			int j;
			if ((j = datablock.indexBlock.indexs[i]) > -1) {//拿到非空二级数据块

				for (int k = 0; k < 4; k++) {
					if (dataBlocks[j].directoryBlock.direcoryEntry[k].flag > -1) {//判断是否是目录文件数据块
						DirectoryEntry directEntry = dataBlocks[j].directoryBlock.direcoryEntry[k];
						if (child._Equal(directEntry.fileName)) {
							inodeNum = directEntry.i_node_number;

							*direcoryEntry_ = &dataBlocks[j].directoryBlock.direcoryEntry[k];

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
	return false;
}

bool I_NODE::clear(DataBlock datablocks[], BlockBitmap& blockBitMap) {
	//清空文本
	int index;
	for (size_t j = 0; j < 2; j++) {
		if ((index = directAddress[j]) != -1) {

			for (size_t i = 0; i < 64; i++) {
				datablocks[index].txtBlock.txt[i] = '\0';
			}
			blockBitMap.blocks[index] = false;//数据块bitmap置为false
		}
		else {
			return true;
		}
	}

	if ((index = firstClassIndexAddress) > -1) {
		for (int j = 0; j < 16; j++) {
			if (datablocks[index].indexBlock.indexs[j] > -1) {
				DataBlock subDatablock = datablocks[datablocks[index].indexBlock.indexs[j]];//拿到二级数据块
				for (size_t i = 0; i < 64; i++) {
					subDatablock.txtBlock.txt[i] = '\0';
				}
				blockBitMap.blocks[datablocks[index].indexBlock.indexs[j]] = false;
			}
			else {
				blockBitMap.blocks[index] = false;
				return true;
			}

		}
		blockBitMap.blocks[index] = false;
	}
	return true;
}

void I_NODE::show(DataBlock datablocks[]) {


	for (int i = 0; i < 2; i++) {
		if (directAddress[i] != -1) {
			DataBlock datablock = datablocks[directAddress[i]];
			int c = 0;
			while (c < 64 && datablock.txtBlock.txt[c] > 0) {
				cout << datablock.txtBlock.txt[c++];
			}
			if (datablock.txtBlock.txt[c] == '\0') {
				return;
			}
		}
	}
	if (firstClassIndexAddress != -1) {
		DataBlock datablock = datablocks[firstClassIndexAddress];
		for (size_t i = 0; i < 16; i++) {
			int j;
			if ((j = datablock.indexBlock.indexs[i]) != -1) {
				int c = 0;
				while (c < 64 && datablocks[j].txtBlock.txt[c] > 0) {
					cout << datablocks[j].txtBlock.txt[c++];
				}
				if (datablocks[j].txtBlock.txt[c] == '\0') {
					return;
				}
			}

		}
	}


}

void I_NODE::addText(const string& cs, DataBlock datablock[], BlockBitmap& bitmap) {
	string temp(cs);
	int hhh = 0;


	for (int i = 0; i < 2; i++) {
		int a;
		if ((a = directAddress[i]) != -1) {
			int b;
			for (b = 0; b < 64 && b + hhh * 64 < temp.length(); b++) {
				char c = temp.at(b + hhh * 64);
				if (c != '\0')
					datablock[a].txtBlock.txt[b] = c;
				else {
					datablock[a].txtBlock.txt[b] = c;
					return;
				}
			}
			hhh++;
			if (b < 63) {
				datablock[a].txtBlock.txt[b] = '\0';
				return;
			}

		}
		else {

			bitmap.getABlockNum(directAddress[i]);
			bitmap.blocks[directAddress[i]] = true;
			int j = directAddress[i];
			int b;
			for (b = 0; b < 64 && b + hhh * 64 < temp.length(); b++) {
				char c = temp.at(b + hhh * 64);
				if (c != '\0')
					datablock[j].txtBlock.txt[b] = c;
				else {
					datablock[j].txtBlock.txt[b] = c;
					return;
				}
			}
			hhh++;
			if (b < 63) {
				datablock[j].txtBlock.txt[b] = '\0';
				return;
			}
		}
	}

	while (true) {
		if (firstClassIndexAddress >= 0) {
			for (size_t i = 0; i < 16; i++) {
				int a;
				if ((a = datablock[firstClassIndexAddress].indexBlock.indexs[i]) > -1) {
					int b;
					for (b = 0; b < 64 && b + hhh * 64 < temp.length(); b++) {
						char c = temp.at(b + hhh * 64);
						if (c != '\0')
							datablock[a].txtBlock.txt[b] = c;
						else {
							datablock[a].txtBlock.txt[b] = c;
							return;
						}
					}
					hhh++;
					if (b < 63) {
						datablock[a].txtBlock.txt[b] = '\0';
						return;
					}

				}
				else {
					int j;
					bitmap.getABlockNum(j);
					bitmap.blocks[j] = true;
					datablock[firstClassIndexAddress].indexBlock.indexs[i] = j;
					int b;
					for (b = 0; b < 64 && b + hhh * 64 < temp.length(); b++) {
						char c = temp.at(b + hhh * 64);
						if (c != '\0')
							datablock[j].txtBlock.txt[b] = c;
						else {
							datablock[j].txtBlock.txt[b] = c;
							return;
						}
					}
					hhh++;
					if (b < 63) {
						datablock[j].txtBlock.txt[b] = '\0';
						return;
					}
				}

			}

		}
		else {
			bitmap.getABlockNum(firstClassIndexAddress);
			bitmap.blocks[firstClassIndexAddress] = true;

		}
	}

}

string I_NODE::getTxt(DataBlock datablocks[]) {
	string temp;
	for (int i = 0; i < 2; i++) {
		if (directAddress[i] != -1) {
			DataBlock datablock = datablocks[directAddress[i]];
			int c = 0;
			while (c < 64 && datablock.txtBlock.txt[c] > 0) {
				temp += datablock.txtBlock.txt[c++];

			}
			if (datablock.txtBlock.txt[c] == '\0') {
				return temp;
			}
		}
	}
	if (firstClassIndexAddress != -1) {
		DataBlock datablock = datablocks[firstClassIndexAddress];
		for (size_t i = 0; i < 16; i++) {
			int j;
			if ((j = datablock.indexBlock.indexs[i]) != -1) {
				int c = 0;
				while (c < 64 && datablocks[j].txtBlock.txt[c] > 0) {
					temp += datablocks[j].txtBlock.txt[c++];
				}
				if (datablocks[j].txtBlock.txt[c] == '\0') {
					return temp;
				}
			}

		}
	}

	return temp;
}

bool InodeLinkManager::getParent(int& cwd_inode) {
	if (tail->val == -1) {
		cout << "根目录，不可退回上级" << endl;
		return false;
	}
	InodeLink* temp = head;
	while (temp->next != tail) {
		temp = temp->next;
	}
	int parentInodeNum = temp->val;
	tail = temp;
	tail->next = nullptr;
	cwd_inode = parentInodeNum;
	return true;
}

int InodeLinkManager::addInode(int inode_number) {
	tail->next = new InodeLink(inode_number);
	tail = tail->next;
	return 0;
}
