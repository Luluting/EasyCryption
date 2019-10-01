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

	//�������
	cout << "Please set up your password:";
	cin >> password;
	//������
	srand(time(0));
	salt = rand();
	psalt = (TCHAR*)&salt;
	//����+��ƴ�ӽ��й�ϣ
	strcat_s(password,MAX, psalt);
	EVP_MD_CTX* evpCtx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(evpCtx, EVP_sha512(), NULL);
	unsigned int length = lstrlen(password);
	EVP_DigestUpdate(evpCtx, password, length);
	unsigned char key_hash[SHA512_DIGEST_LENGTH] = { 0 };//hash���
	EVP_DigestFinal_ex(evpCtx, key_hash, &length);
	fp = fopen(FileName, "rb+");
	//��ȡ�ļ���С
	DWORD fileSize = GetSizeOfFile(fp);
	//�����ڴ�
	char* fileBuffer = (char*)malloc(fileSize + 1000);
	//���ļ�
	ReadFileIntoMem(FileName, &fileBuffer);
	for (int i = 0; i < sizeof(DWORD); i++) {
		*(fileBuffer-4+i) = *(psalt + i);
	}
	//ִ�м���
	for (int i = 0; i < fileSize; i++) {
		fileBuffer[i] ^= key_hash[i % SHA512_DIGEST_LENGTH];
	}
	//����
	strcat(FileName, "_Encrypt");
	saveTODisk(FileName, fileBuffer - 4, fileSize + 4);
	//�ָ�ջ
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
	//��ȡ����
	cout << "Please set up your password:";
	cin >> password;
	//���ļ���ȡ��
	DWORD fileSize = GetSizeOfFile(fp);
	char* fileBuffer = (char*)malloc(fileSize + 1000);
	ReadFileIntoMem(FileName, &fileBuffer);
	salt = *((DWORD*)fileBuffer);
	psalt = (TCHAR*)&salt;
	//����+�ν��й�ϣ����Կ
	strcat(password, psalt);
	EVP_MD_CTX* evpCtx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(evpCtx, EVP_sha512(), NULL);
	unsigned int length = lstrlen(password);
	EVP_DigestUpdate(evpCtx, password, length);
	unsigned char key_hash[SHA512_DIGEST_LENGTH] = { 0 };//hash���
	EVP_DigestFinal_ex(evpCtx, key_hash, &length);
	//����
	for (int i = 0; i < fileSize - sizeof(DWORD); i++) {
		*(fileBuffer + sizeof(DWORD) + i) ^= key_hash[i % 64];
	}
	//����
	strcat(FileName, "_Decrypt");
	saveTODisk(FileName, fileBuffer + 4, fileSize - 4);

	return TRUE;
}

//���ܺ���
//��ȡ�ļ���С
DWORD GetSizeOfFile(FILE* p) {
	fseek(p, 0, SEEK_END);
	DWORD length = ftell(p);
	fseek(p, 0, SEEK_SET);
	return length;
}

//��ȡ�ļ����ڴ棬������ FileBuffer
//����˵��
//path:�ļ�·��
//fileBuffer: ��ȡ�����ڴ��ַ
//����ֵ˵��
//�ɹ��򷵻�fileBuffer�Ĵ�С ʧ���򷵻�0
DWORD ReadFileIntoMem(char* path, char** fileBuffer) {

	//���ļ�
	FILE* p = fopen(path, "rb");

	if (!p) {
		printf("Open failed");
		fclose(p);
		return 0;
	}
	//��ȡ�ļ���С
	DWORD fileSize = GetSizeOfFile(p);

	//�����ڴ�
	char* room = (char*)malloc(fileSize + 1000);
	if (!room) {
		printf("Malloc failed");
		free(room);
		fclose(p);
		return 0;
	}
	memset(room, 0, fileSize + 1000);

	//�������ڴ�
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
	//�����ڴ���
	//printf("Address:%x\n\n", room);
	*fileBuffer = room;

	//�ռ��ͷ�
	fclose(p);
	return fileSize;

}

//�ڴ�ռ����
//����˵��:
//path: Ŀ����λ��
//buffer: Դ���ݿռ�
//fileSize: ��������ֽ���
//����ֵ˵��:
//�ɹ�����0�� ʧ�ܷ���1 
DWORD saveTODisk(char* path, char* buffer, DWORD fileSize) {
	//printf("Message From saveToDisk:\n");
	//��ȡ�ļ�
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