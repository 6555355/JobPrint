#ifndef _EP6PIPE_H_

#define _EP6PIPE_H_

/*
������䱣���ݲ��ǰ��մ�С�����˳���ͣ���ô���ܵ�����Զ�ò�����ȷ�����ݡ�
������ݴ���Ĺ����У���ô����һ������
*/

//#include <iostream>
#include <fstream>
#include <vector>
#include <list>

using namespace std;

using std::vector;
using std::list;

typedef struct ep6_pipe_subcmd{
	int Index;
	int Len;
	int dirty;
	vector<unsigned char> Buf;
	ep6_pipe_subcmd(){
		Index = 0;
		Len = 0;
		dirty = 0;
	}
	ep6_pipe_subcmd(int index){
		Index = index;
		Len = 0;
		dirty = 0;
		//Buf.resize(len);
	}
}Ep6PipeSubCmd;

typedef struct ep6_pipe_type{
	int Cmd;
	list<Ep6PipeSubCmd> SubCmd;
	ep6_pipe_type(){
		Cmd = 0;
	}
	ep6_pipe_type(int cmd){
		Cmd = cmd;
	}
}Ep6PipeCmdType;

class Ep6Pipe{
	
public:
	Ep6Pipe();
	~Ep6Pipe();
	int InsertData(Ep6PipeSubCmd& cmd, unsigned char * data, int len, int offset, int curlen);
	int GetData(int cmd, int index, unsigned char * buf);
	int GetDataLen(int cmd, int index);

//private:
	Ep6PipeSubCmd& Find(int cmd, int index);
private:
	ofstream fp;
	vector<Ep6PipeCmdType> Ep6PipeCmd;
};

#endif



