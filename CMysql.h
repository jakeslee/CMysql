#ifndef CMYSQL_H
#define CMYSQL_H
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <iostream>
#include <winsock2.h>
#include "mysql.h"

//Constant define
#define ES_NONE				0x00000000
#define ES_NOCONNECT 		0x00000001
#define ES_BASENOTEXIST		0x00000002
#define ES_COMMAND			0x00000004
#define ES_NOTABLE			0x00000008
#define ES_ARGUMENT			0x00000010
//Class define

class CMysql {
	public:
		typedef std::map<int,std::string> RowType;
		typedef std::map<std::string,std::string> RowCondition;
		typedef std::vector< std::map<int,std::string> > RowsType;
		typedef std::vector< std::map<int,std::string> >::iterator Rows_iterator;
		typedef std::vector< std::map<int,std::string> >::const_iterator Rows_const_iterator;
		typedef std::map<int,std::string>::iterator Row_iterator;
		typedef std::map<int,std::string>::const_iterator Row_const_iterator;
		typedef std::pair<std::string,std::string> Col_Pair;

		CMysql(void):HostName("localhost"),UserName("root"),PassWord(""),DataBaseName("test"),TableName("test"),nCurrentCol(0){
			hMysql = mysql_init(NULL);
			bConnected = false;
		}
		CMysql(const std::string& strHostName,const std::string& strUserName,const std::string& strPassword,
				const std::string& strDataBaseName = "test",const std::string& strTableName = "test");
		~CMysql(void);
		
		bool Connect(void);
		bool DisConnect(bool bIsClose = false);
		int GetCurTblColNames(bool bUpdate = false);
		unsigned int GetErr(void) const {std::cout<<mysql_error(hMysql); return uErrState;}
		unsigned int SelectDataBase(const std::string& strBaseName);
		unsigned int ChangeTable(const std::string& strTableName);
		unsigned int SetCharset(const std::string& strCharset = "GBK");
		unsigned int InitTable(void);

		friend std::ostream& operator<<(std::ostream& out,CMysql& cms);
		friend std::istream& operator>>(std::istream& in,CMysql& cms);
		friend CMysql& operator<<(CMysql& lvalue,std::string& rvalue);
		friend CMysql& operator<<(CMysql& lvalue,int& rvalue);
		friend CMysql& operator<<(CMysql& lvalue,double& rvalue);
		friend CMysql& operator>>(CMysql& lvalue,std::string& rvalue);
		friend CMysql& operator>>(CMysql& lvalue,int& rvalue);
		friend CMysql& operator>>(CMysql& lvalue,double& rvalue);

		static std::string  MakeNull(void);
		static std::string& ToString(std::string& strForConversion);
		static std::string& ToString(int nForConversion);
		static std::string& AddSlashes(std::string& strForSlashes);
		static std::string& StripSlashes(std::string& strForSlashes);
		static std::string& StringReplace(std::string& strSource,const std::string& forReplace,const std::string& asReplace);
		static int GetNumCol(RowsType& rtRow);
		static int GetNumRows(RowsType& rtRows);
		
		RowsType& GetRow(const RowCondition& rcRowCondition,bool bIsDESC = false);
		RowsType& GetRow(const int nFirst,const int nLast,bool bIsDESC = false);
		RowsType& GetRow(bool bIsDESC = false);

		unsigned int AddRow(const RowType& rtNewRow);
		unsigned int AddRow(const RowsType& rstNewRows);

		unsigned int EditRow(const std::string& strRowCondition,const std::string& strColumn,const std::string& strNewContent);
		unsigned int EditRow(const std::string& strRowCondition,const Col_Pair& cpPair);

		unsigned int DeleteRow(const std::string& strRowCondition);
	protected:
		bool bConnected;
		std::vector<std::string>::size_type nCurrentCol;
		unsigned int GetRes(bool bIsDESC = false);
		unsigned int MakeRes(void);
	private:
		std::string HostName,UserName,PassWord,DataBaseName,TableName;
		MYSQL * hMysql;
		MYSQL_RES * hMysql_res;
		MYSQL_ROW hMysql_row;
		unsigned int uErrState;
		RowsType svecRow;
		std::vector<std::string> vecColNames;

		unsigned int Query(const std::string strQuery);
};

#endif 
