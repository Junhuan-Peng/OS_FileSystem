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
	} else if (strings[0]._Equal("test"))
	{
		for (int i = 0; i < 70; i++){
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
		I_NODE *parentINode = &(disc->i_node_s[cwd_inode]);
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
	if (cwd_inode == -1)//根目录
	{

		for (size_t i = 0; i < 4; i++) {
			DirecoryEntry direcoryEntry = disc->rootDirectory.direcoryEntries[i];
			if (path._Equal(direcoryEntry.fileName)) {//同名
				if (direcoryEntry.flag == 1) {//目录
					cwd_inode = direcoryEntry.i_node_number;
					cwd = cwd  + path + "/";
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
			cwd_inode = childINode;
			cwd = cwd  + path+"/";
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
