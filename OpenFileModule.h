#pragma once

#include <Windows.h>
#include <Commdlg.h>
#include <tchar.h>

//打开文件对话框
class OpenFileModule
{
public:
	OpenFileModule(void);

	TCHAR* DefaultExtension;
	TCHAR* FileName;
	TCHAR* Filter;
	int FilterIndex;
	int Flags;
	TCHAR* InitialDir;
	HWND Owner;
	TCHAR* Title;

	//文件选择
	bool ShowDialog();

	//Encryption
	bool PBE_Encrypt(TCHAR* FileName);
	//Decryption
	bool PBE_Decrypt(TCHAR* FileName);
};

