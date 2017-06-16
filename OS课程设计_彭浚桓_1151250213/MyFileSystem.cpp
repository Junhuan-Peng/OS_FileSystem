// MyFileSystem.cpp : 定义控制台应用程序的入口点。
//


#include "MyFileSystem.h"//定义基本结构
#include <iostream>

void usage();
Cmd* Cmd::instance = nullptr;

int main(int args, char* argv[]){

	Disc disc;


	cout << "输入 help 获取帮助" << ",输入 Exit 退出" << endl;
	string input_cmd;
	Cmd* cmd = Cmd::getInstance();
	while (true){
		cout << ">>";
		getline(cin, input_cmd);
		if (input_cmd._Equal("Exit") || input_cmd._Equal("exit"))
			break;
		if (input_cmd._Equal("help")){
			usage();
			continue;
		}

		auto flag = cmd->parse(input_cmd);
		if (!flag){
			cout << "Error" << endl;
		}
	}
	return 0;
}

bool Cmd::parse(string cmd){
	string* strings = split(cmd);
	if (strings[0]._Equal("Format")){
		Format();
	} else if (strings[0]._Equal("MKfile")){
		MkFile(strings[1]);
	} else if (strings[0]._Equal("MKdir")){
		MkDir(strings[1]);
	} else if (strings[0]._Equal("Cd")){
		Cd(strings[1]);
	} else if (strings[0]._Equal("Delfile")){
		DelFile(strings[1]);
	} else if (strings[0]._Equal("Dir")){
		Dir();
	} else if (strings[0]._Equal("Copy")){
		Copy(strings[1],strings[2]);
	} else if (strings[0]._Equal("Open")){
		Open(strings[1]);
	} else if (strings[0]._Equal("Attrib")){
		Attrib(strings[1],strings[2]);
	} else if (strings[0]._Equal("Viewinodemap")){
		ViewINodeMap();
	} else if (strings[0]._Equal("Viewblockmap")){
		ViewBlockMap();
	} else{
		cout << strings[0] << " 找不到命令！请重新输入" << endl;
	}
	return true;
}

bool Cmd::Format(){
	cout << "正在初始化硬盘……请等待" << endl;
	
	return true;
}

bool Cmd::MkFile(string cmd){
	return true;
}

bool Cmd::MkDir(string cmd){
	return true;
}

bool Cmd::Cd(string cmd){
	return true;
}

bool Cmd::DelFile(string cmd){
	return true;
}

bool Cmd::DelDir(string cmd){
	return true;
}

bool Cmd::Dir(){
	return true;
}

bool Cmd::Copy(string orign_path, string goal_path){
	return true;
}

bool Cmd::Open(string cmd){
	return true;
}

bool Cmd::Attrib(string file_path, string operation){
	return true;
}

bool Cmd::ViewINodeMap(){
	return true;
}

bool Cmd::ViewBlockMap(){
	return true;
}


void usage(){
	cout << "Format\t\t\t初始化磁盘，划定结构" << endl <<
		"Mkfile\t\t\t创建文件" << endl <<
		"Mkdir\t\t\t创建目录" << endl <<
		"Cd\t\t\t改变当前目录" << endl <<
		"Delfile [file path]\t删除文件（注意只读属性）" << endl <<
		"Deldir [dir path]\t删除目录（注意只读属性）" << endl <<
		"Dir\t\t\t列文件目录 （列出名字和建立时间，注意隐藏属性）" << endl <<
		"Copy [origin file path] [goal file path]\t\t\t复制文件到某一路经" << endl <<
		"Open [file path]\t打开并编辑文件（注意只读属性）" << endl <<
		"Attrib [+r|-r] [+h|-h]  [file path]\t\t更改文件属性，加只读，减只读，加隐藏，减隐藏" << endl <<
		"Viewinodemap\t\t显示当前inode位示图状况" << endl <<
		"Viewblockmap\t\t显示当前block位示图状况" << endl;
}
