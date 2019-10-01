#pragma once

#include <Windows.h>
#include <Commdlg.h>
#include <tchar.h>

//���ļ��Ի���
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

	//�ļ�ѡ��
	bool ShowDialog();

	//Encryption
	bool PBE_Encrypt(TCHAR* FileName);
	//Decryption
	bool PBE_Decrypt(TCHAR* FileName);
};

