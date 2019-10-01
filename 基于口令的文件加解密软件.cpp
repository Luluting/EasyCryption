
/************************************************************************
课题:基于口令的文件加解密软件

流程设计:选择文件 -> 输入密码 -> 选择模式 -> 完成指定功能

加密部分:
	输入口令 生成盐
	口令 + 盐 -> 密钥
	密钥 加密文件
	保存盐

解密部分:
	输入口令 取得盐
	口令 + 盐 -> 密钥
	密钥 解密文件
*************************************************************************/

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <commdlg.h>
#include "OpenFileModule.h"

using namespace std;

int main()
{
	while (1) {
		int res;
		OpenFileModule target;
		

		//欢迎界面
		system("CLS");
		cout << R"( _           _       _   _             _      ____________ _____ )" << endl;
		cout << R"(| |         | |     | | (_)           ( )     | ___ \ ___ \  ___|)" << endl;
		cout << R"(| |    _   _| |_   _| |_ _ _ __   __ _|/ ___  | |_/ / |_/ / |__  )" << endl;
		cout << R"(| |   | | | | | | | | __| | '_ \ / _` | / __| |  __/| ___ \  __| )" << endl;
		cout << R"(| |___| |_| | | |_| | |_| | | | | (_| | \__ \ | |   | |_/ / |___ )" << endl;
		cout << R"(\_____/\__,_|_|\__,_|\__|_|_| |_|\__, | |___/ \_|   \____/\____/ )" << endl;
		cout << R"(                                  __/ |                          )" << endl;
		cout << R"(                                 |___/                           )" << endl;
		
		cout << "\n1.Encrypt 2.Decrypt 0.Exit" << endl;
		cout << "Select the mode:";
		cin >> res;
		switch (res) {
		case 1:
			//打开文件
			target = OpenFileModule();
			if (!target.ShowDialog()) {
				cout << "Please select a file!" << endl;
				return -1;
			}
			//PBE
			if (target.PBE_Encrypt(target.FileName)) {
				cout << "Encryption Succuss!" << endl;
				system("pause");
			}
			break;
		case 2:
			//打开文件
			target = OpenFileModule();
			if (!target.ShowDialog()) {
				cout << "Please select a file!" << endl;
				return -1;
			}
			//PBD
			if (target.PBE_Decrypt(target.FileName)) {
				cout << "Decryption Succuss!" << endl;
				system("pause");
			}
			break;
		default:
			cout << "Goodbye~" << endl;
			return 0;
		}
	}
}




