#include "OpenFileModule.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <malloc.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h> 
#include <openssl/crypto.h>
#include <openssl/rand.h>

#define MAX 100
using namespace std;

DWORD GetSizeOfFile(FILE* p);
DWORD ReadFileIntoMem(char* path, char** fileBuffer);
DWORD saveTODisk(char* path, char* buffer, DWORD fileSize);

OpenFileModule::OpenFileModule(void)
{
	this->DefaultExtension = 0;
	this->FileName = new TCHAR[MAX_PATH];
	this->Filter = 0;
	this->FilterIndex = 0;
	this->Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	this->InitialDir = 0;
	this->Owner = 0;
	this->Title = 0;
}

bool OpenFileModule::ShowDialog()
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->Owner;
	ofn.lpstrDefExt = this->DefaultExtension;
	ofn.lpstrFile = this->FileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = this->Filter;
	ofn.nFilterIndex = this->FilterIndex;
	ofn.lpstrInitialDir = this->InitialDir;
	ofn.lpstrTitle = this->Title;
	ofn.Flags = this->Flags;

	GetOpenFileName(&ofn);

	if (_tcslen(this->FileName) == 0) return false;

	return true;
}


//Password Based Encryption
bool OpenFileModule::PBE_Encrypt(TCHAR* FileName) {
	TCHAR password[MAX] = { 0 };
	DWORD salt;
	TCHAR *mykey;
	TCHAR* psalt;
	FILE* fp;

	//获得密码
	cout << "Please set up your password:";
	cin >> password;
	//生成盐
	srand(time(0));
	salt = rand();
	psalt = (TCHAR*)&salt;
	//口令+盐拼接进行哈希
	strcat_s(password,MAX, psalt);
	EVP_MD_CTX* evpCtx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(evpCtx, EVP_sha512(), NULL);
	unsigned int length = lstrlen(password);
	EVP_DigestUpdate(evpCtx, password, length);
	unsigned char key_hash[SHA512_DIGEST_LENGTH] = { 0 };//hash结果
	EVP_DigestFinal_ex(evpCtx, key_hash, &length);
	fp = fopen(FileName, "rb+");
	//获取文件大小
	DWORD fileSize = GetSizeOfFile(fp);
	//申请内存
	char* fileBuffer = (char*)malloc(fileSize + 1000);
	//读文件
	ReadFileIntoMem(FileName, &fileBuffer);
	for (int i = 0; i < sizeof(DWORD); i++) {
		*(fileBuffer-4+i) = *(psalt + i);
	}
	//执行加密
	for (int i = 0; i < fileSize; i++) {
		fileBuffer[i] ^= key_hash[i % SHA512_DIGEST_LENGTH];
	}
	//存盘
	strcat(FileName, "_Encrypt");
	saveTODisk(FileName, fileBuffer - 4, fileSize + 4);
	//恢复栈
	for (int i = 0; i < sizeof(DWORD); i++) {
		*(fileBuffer - sizeof(DWORD) + i) = 0xfd;
	}
	return TRUE;
}
//Password Based Decryption
bool OpenFileModule::PBE_Decrypt(TCHAR* FileName) {
	TCHAR password[MAX] = { 0 };
	DWORD salt;
	TCHAR* mykey;
	TCHAR* psalt;
	FILE* fp = fopen(FileName, "rb+");
	//获取口令
	cout << "Please set up your password:";
	cin >> password;
	//读文件获取盐
	DWORD fileSize = GetSizeOfFile(fp);
	char* fileBuffer = (char*)malloc(fileSize + 1000);
	ReadFileIntoMem(FileName, &fileBuffer);
	salt = *((DWORD*)fileBuffer);
	psalt = (TCHAR*)&salt;
	//口令+盐进行哈希求密钥
	strcat(password, psalt);
	EVP_MD_CTX* evpCtx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(evpCtx, EVP_sha512(), NULL);
	unsigned int length = lstrlen(password);
	EVP_DigestUpdate(evpCtx, password, length);
	unsigned char key_hash[SHA512_DIGEST_LENGTH] = { 0 };//hash结果
	EVP_DigestFinal_ex(evpCtx, key_hash, &length);
	//解密
	for (int i = 0; i < fileSize - sizeof(DWORD); i++) {
		*(fileBuffer + sizeof(DWORD) + i) ^= key_hash[i % 64];
	}
	//存盘
	strcat(FileName, "_Decrypt");
	saveTODisk(FileName, fileBuffer + 4, fileSize - 4);

	return TRUE;
}

//功能函数
//获取文件大小
DWORD GetSizeOfFile(FILE* p) {
	fseek(p, 0, SEEK_END);
	DWORD length = ftell(p);
	fseek(p, 0, SEEK_SET);
	return length;
}

//读取文件到内存，即创建 FileBuffer
//参数说明
//path:文件路径
//fileBuffer: 读取到的内存地址
//返回值说明
//成功则返回fileBuffer的大小 失败则返回0
DWORD ReadFileIntoMem(char* path, char** fileBuffer) {

	//打开文件
	FILE* p = fopen(path, "rb");

	if (!p) {
		printf("Open failed");
		fclose(p);
		return 0;
	}
	//获取文件大小
	DWORD fileSize = GetSizeOfFile(p);

	//申请内存
	char* room = (char*)malloc(fileSize + 1000);
	if (!room) {
		printf("Malloc failed");
		free(room);
		fclose(p);
		return 0;
	}
	memset(room, 0, fileSize + 1000);

	//拷贝到内存
	size_t readnum = fread(room, fileSize, 1, p);
	if (!readnum) {
		printf("fread failed");
		free(room);
		fclose(p);
		return 0;
	}
	else {
		//printf("Successfully created FileBuffer\nMem spaces occupied:%d\n", readnum * (int)fileSize);
	}
	//返回内存编号
	//printf("Address:%x\n\n", room);
	*fileBuffer = room;

	//空间释放
	fclose(p);
	return fileSize;

}

//内存空间存盘
//参数说明:
//path: 目标存放位置
//buffer: 源数据空间
//fileSize: 存放数据字节数
//返回值说明:
//成功返回0， 失败返回1 
DWORD saveTODisk(char* path, char* buffer, DWORD fileSize) {
	//printf("Message From saveToDisk:\n");
	//读取文件
	FILE* p = fopen(path, "wb+");
	if (!p) {
		printf("file open failed, please try again!\n");
		fclose(p);
		return 1;
	}
	//printf("fileSize:%#x\n", fileSize);

	int writeNum = fwrite(buffer, 1, fileSize, p);
	//printf("WriteNum:%#x\n", writeNum);
	if (!writeNum) {
		printf("fwrite failed!");
		return 1;
	}
	fclose(p);
	return 0;
}