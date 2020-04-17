#include "ep6pipe.h"

static Ep6Pipe Ep6CmdPipe;

Ep6Pipe::Ep6Pipe()
{
	fp = ofstream("ep6_pipe_log.txt", ofstream::out);

}

Ep6Pipe::~Ep6Pipe()
{
	//check

	fp.close();
}

Ep6PipeSubCmd& Ep6Pipe::Find(int cmd, int index)
{
	fp << "cmd= " << cmd << ",index= " << index << " ";

	vector<Ep6PipeCmdType>::iterator pipe_cmd;
	list<Ep6PipeSubCmd>::iterator pipe_index;

	for (pipe_cmd = Ep6PipeCmd.begin(); pipe_cmd != Ep6PipeCmd.end(); pipe_cmd++)
	{
		if (pipe_cmd->Cmd == cmd){
			break;
		}
	}
	if (pipe_cmd == Ep6PipeCmd.end())
	{
		Ep6PipeCmdType tmp(cmd);
		Ep6PipeCmd.push_back(tmp);
		for (pipe_cmd = Ep6PipeCmd.begin(); pipe_cmd != Ep6PipeCmd.end(); pipe_cmd++)
		{
			if (pipe_cmd->Cmd == cmd){
				break;
			}
		}
	}

	for (pipe_index = pipe_cmd->SubCmd.begin(); pipe_index != pipe_cmd->SubCmd.end(); pipe_index++)
	{
		if (pipe_index->Index == index){
			break;
		}
	}
	if (pipe_index == pipe_cmd->SubCmd.end())
	{
		Ep6PipeSubCmd subcmd(index);
		pipe_cmd->SubCmd.push_back(subcmd);
		for (pipe_index = pipe_cmd->SubCmd.begin(); pipe_index != pipe_cmd->SubCmd.end(); pipe_index++)
		{
			if (pipe_index->Index == index){
				break;
			}
		}
	}

	return *pipe_index;
}
int Ep6Pipe::InsertData(Ep6PipeSubCmd& cmd, unsigned char * data, int len, int offset, int curlen)
{
	//
	if (curlen != 0)
	{
		if (cmd.dirty){
			fp << "is dirty\n";
			return 1;
		}

		if (cmd.Len == 0){
			fp << "allocated ";
			cmd.Buf.resize(len);
			cmd.Len = len;
		}

		if (cmd.Buf.size() != len){
			fp << "length does not match \n";
			return 0;
		}

		if (offset >= len){
			fp << "invalued offset\n";
			return 0;
		}

		int buflen = min(len-offset,curlen);
		for (int i = 0; i < buflen; i++)
			cmd.Buf[i+offset] = data[i];

		if (offset+curlen >= len)
		{
			cmd.dirty = 1;
			return 1;
		}
	}
	else
	{
		if (cmd.dirty){
			fp << "is dirty\n";
			return 1;
		}

		if (cmd.Len == 0){
			fp << "allocated ";
			cmd.Buf.resize(len);
		}

		if (cmd.Buf.size() != len){
			fp << "length does not match \n";
			return 0;
		}

		if (offset >= len){
			fp << "invalued offset\n";
			return 0;
		}

		//copy
		int dirty = 0;
		if (len <= 504)
		{
			dirty = 1;
			for (int i = 0; i < len; i++)
			{
				cmd.Buf[i] = data[i];
			}

			fp << "copy data " << len << ", ";
		}
		else
		{
			if (offset + 504 >= len){
				dirty = 1;
			}
			int buf_len = (len - offset > 504) ? 504 : (len - offset);
			for (int i = 0; i < buf_len; i++)
				cmd.Buf[offset + i] = data[i];

			fp << "copy data " << buf_len << ", ";
		}

		if (dirty)
		{
			unsigned char sum = 0;
			for (int i = 0; i < len - 1; i++){
				sum += cmd.Buf[i];
			}
			if (sum == cmd.Buf[len - 1]){
				cmd.dirty = 1;
				fp << " finished\n";
				return 1;
			}
			else{
				fp << " invalid checksum = " << (int)sum << " cmd.Buf[len - 1]=" << (int)cmd.Buf[len - 1] << "\n";
			}
		}
	}
	
	return 0;
}

int Ep6Pipe::GetData(int cmd, int index, unsigned char * buf)
{
	fp << "get data, cmd= " << cmd << ",index= " << index << " ";
	for (vector<Ep6PipeCmdType>::iterator pipe_cmd = Ep6PipeCmd.begin(); pipe_cmd != Ep6PipeCmd.end(); pipe_cmd++)
	{
		if (pipe_cmd->Cmd == cmd)
		{
			for (list<Ep6PipeSubCmd>::iterator pipe_index = pipe_cmd->SubCmd.begin(); pipe_index != pipe_cmd->SubCmd.end(); pipe_index++)
			{
				if (pipe_index->Index == index)
				{
					if (pipe_index->dirty)
					{
						//copy
						for (size_t i = 0; i < pipe_index->Buf.size(); i++){
							buf[i] = pipe_index->Buf[i];
						}
						fp << "get successed\n";
						pipe_cmd->SubCmd.erase(pipe_index);
						return 1;
					}
					else{
						fp << "data is unfinished\n";
						return 0;
					}
				}
			}
		}
	}

	fp << "unknow cmd or index\n";
	return 2;
}

int Ep6Pipe::GetDataLen(int cmd, int index)
{
	fp << "get length, cmd= " << cmd << ",index= " << index << " ";
	for (vector<Ep6PipeCmdType>::iterator pipe_cmd = Ep6PipeCmd.begin(); pipe_cmd != Ep6PipeCmd.end(); pipe_cmd++)
	{
		if (pipe_cmd->Cmd == cmd)
		{
			for (list<Ep6PipeSubCmd>::iterator pipe_index = pipe_cmd->SubCmd.begin(); pipe_index != pipe_cmd->SubCmd.end(); pipe_index++)
			{
				if (pipe_index->Index == index)
				{
					fp << "length= " << pipe_index->Buf.size();
					return (int)pipe_index->Buf.size();
				}
			}
		}
	}

	fp << "unknow cmd or index\n";
	return 0;
}

int SaveData(int cmd, int index, unsigned char * buf, int len, int offset, int curlen)
{
	return Ep6CmdPipe.InsertData(Ep6CmdPipe.Find(cmd, index), buf, len, offset, curlen);
}

int GetData(int cmd, int index, unsigned char *buf, int &len)
{
	if (len == 0)
	{
		len = Ep6CmdPipe.GetDataLen(cmd, index);
		return 1;
	}
	else
	{
		return Ep6CmdPipe.GetData(cmd, index, buf);
	}

}