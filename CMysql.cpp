#include "CMysql.h"
#ifndef CMYSQL_CPP
#define CMYSQL_CPP 1
CMysql::CMysql(const std::string& strHostName,const std::string& strUserName,const std::string& strPassword,
				const std::string& strDataBaseName,const std::string& strTableName):
		HostName(strHostName),UserName(strUserName),PassWord(strPassword),DataBaseName(strDataBaseName),
		TableName(strTableName),nCurrentCol(0),strPrimarykey("id"){
	hMysql = mysql_init(NULL);
	bConnected = false;
}
CMysql::~CMysql(void){
	DisConnect(true);
}
bool CMysql::Connect(void){
	if(hMysql != NULL && mysql_real_connect(hMysql,HostName.c_str(),UserName.c_str(),PassWord.c_str(),DataBaseName.c_str(),3306,NULL,CLIENT_INTERACTIVE)){
		bConnected = true;
		SetCharset();
		SetTimeZone();
		return true;
	}else{
		bConnected = false;
		return false;
	}
}
bool CMysql::DisConnect(bool bIsClose){
	if(bConnected){
		if(bIsClose){
			bConnected = false;	
			mysql_close(hMysql);
		}
		uErrState = ES_ARGUMENT;
		return bIsClose;
	}else{
		uErrState = ES_NOCONNECT;
		return false;
	}
	
}
unsigned int CMysql::SelectDataBase(const std::string& strBaseName){
	if(bConnected){
		DataBaseName = strBaseName;
		if(!mysql_select_db(hMysql,DataBaseName.c_str())){
			Query("set names 'GBK'");
			return ES_BASENOTEXIST;
		}else{
			return ES_NONE;
		}
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::ChangeTable(const std::string& strTableName){
	if(bConnected){
		if(strTableName.size()){
			TableName = strTableName;
			InitTable();
			return ES_NONE;
		}else
			return ES_ARGUMENT;
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::SetCharset(const std::string& strCharset){
	if(bConnected){
		return Query("set names '" + strCharset +"'");
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::SetTimeZone(const std::string& strTime_zone){
	if(bConnected){
		return Query("set time_zone='" + strTime_zone +"'");
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::SetInteractive_timeout(const std::string& strTime_out){
	if(bConnected){
		return Query("set interactive_timeout=" + strTime_out +"");
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::IsConnected(void){
	if(bConnected){
		if(!mysql_ping(hMysql)){
			return ES_NONE;
		}else{
			return ES_NOCONNECT;
		}
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::InitTable(void){
	GetCurTblColNames(true);
	return uErrState;
}
unsigned int CMysql::Query(const std::string strQuery){
	if(bConnected){
 		if(mysql_real_query(hMysql,strQuery.c_str(),strQuery.size())){
		 	return ES_COMMAND;
 		}else{
		 	return ES_NONE;
		} 
	}else{
		return ES_NOCONNECT;
	}
}
int CMysql::GetCurTblColNames(bool bUpdate){
	RowsType rstTmp = svecRow;
	std::ostringstream oss;
	if(bConnected){
		if(bUpdate){
			oss<<"SELECT `COLUMN_NAME`  FROM `information_schema`.`COLUMNS` WHERE `TABLE_NAME` LIKE '"<<TableName<<"'";
			if(Query(oss.str()) == ES_NONE){
				MakeRes();
				vecColNames.clear();
				for (Rows_iterator rsiter = svecRow.begin(); rsiter != svecRow.end(); ++rsiter){
					vecColNames.push_back((*rsiter)[0]);
				}
				svecRow = rstTmp;
				uErrState = ES_NONE;
				return vecColNames.size();
			}
			uErrState = ES_COMMAND;			
		}else{
			uErrState = ES_NONE;
			return vecColNames.size();
		}
	}else{
		uErrState = ES_NOCONNECT;
	}
	return 0;
}
std::string  CMysql::MakeNull(void){
	return "NULL";
}
std::string& CMysql::AddSlashes(std::string& strForSlashes){
	return StringReplace(strForSlashes,"\"","\\\"");
}
std::string& CMysql::StripSlashes(std::string& strForSlashes){
	return StringReplace(strForSlashes,"\\\"","\"");
}
std::string& CMysql::StringReplace(std::string& strSource,const std::string& forReplace,const std::string& asReplace){
	for(std::string::size_type pos=0;(pos = strSource.find(forReplace,pos))
		!= std::string::npos;){
		strSource.replace(pos,forReplace.size(),asReplace);
		pos += asReplace.size();
	}
	return strSource;
}
int CMysql::GetNumCol(RowsType& rtRow){
	return rtRow.at(0).size();
}
int CMysql::GetNumRows(RowsType& rtRows){
	return rtRows.size();
}
CMysql::RowsType& CMysql::GetRow(const RowCondition& rcRowCondition,bool bIsDESC){
	std::ostringstream oss;
	std::string tmp;
	if(bConnected){
		if(TableName.size()){
			RowCondition::const_iterator rcCiter = rcRowCondition.begin();
			oss<<"select * from `"<<TableName<<"` where ";//<<rcCiter->first<<"="<<rcCiter->second;
			int nCount = 0;
			for (RowConstCondition rcciter = rcRowCondition.begin();rcciter != rcRowCondition.end();++rcciter,++nCount)
			{
				oss<<rcciter->first<<"="<<rcciter->second<<((nCount != rcRowCondition.size()-1)?" AND ":"");
			}
			if(bIsDESC)
				oss<<" order by "<<strPrimarykey<<" DESC";
			if(Query(oss.str()) == ES_NONE){
				MakeRes();
				uErrState = ES_NONE;
				return svecRow;		
			}
			uErrState = ES_COMMAND;
		}else{
			uErrState = ES_NOTABLE;
		}
	}else{
		uErrState = ES_NOCONNECT;
	}
	return svecRow;
}
CMysql::RowsType& CMysql::GetRow(int nFirst,int nLast,bool bIsDESC){

	if((uErrState = GetRes(bIsDESC)) == ES_NONE){
 		RowsType * RowsTmp = new RowsType;
		if(nFirst < 0)
			nFirst = 0;
		if(nLast > static_cast<int>(svecRow.size()))
			nLast =	svecRow.size();
		for(;nFirst != nLast;++nFirst)
			RowsTmp->push_back(svecRow.at(nFirst));
		return *RowsTmp;
	}else{
		return svecRow;
	}
}
CMysql::RowsType& CMysql::GetRow(bool bIsDESC){
	uErrState = GetRes(bIsDESC);
	return svecRow;
}
unsigned int CMysql::GetRes(bool bIsDESC){
	std::ostringstream oss;
	if(bConnected){
		if(TableName.size()){
			oss<<"select * from `"<<TableName<<"` where 1";
			if(bIsDESC && strGroupBy.empty())
				oss<<" order by "<<strPrimarykey<<" DESC";
			if(!strGroupBy.empty()){
				oss<<" group by "<<strGroupBy;
				strGroupBy.clear();
			}
			if(Query(oss.str()) == ES_NONE){
				return MakeRes();
			}else{
				return ES_COMMAND;
			}
		}else{
			return ES_NOTABLE;
		}
	}else{
		return ES_NOCONNECT;
	}	
}
unsigned int CMysql::MakeRes(void){
	RowType mtemp;
	svecRow.clear();	//Clear svecRow vector
	hMysql_res = mysql_store_result(hMysql);
	while(hMysql_row = mysql_fetch_row(hMysql_res)){
		for(unsigned int nCol = 0;nCol < mysql_num_fields(hMysql_res);++nCol){
			mtemp.insert(std::make_pair(nCol,(hMysql_row[nCol]==NULL?"":hMysql_row[nCol])));
		}
		svecRow.push_back(mtemp);
		mtemp.clear();	//Clear mtemp vector
	}
	return ES_NONE;
}
unsigned int CMysql::MakeGroupby(const std::string& strGroupByCol,bool bIsDESC){
	if(bConnected){
		strGroupBy = strGroupBy = ((bIsDESC)?strGroupByCol+" DESC":strGroupByCol);;
		return ES_NONE;
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::MakePrimarykey(std::string& strPrimary){
	if(bConnected){
		transform(strPrimary.begin(),strPrimary.end(),strPrimary.begin(),toupper);
		strPrimarykey = strPrimary;
		return ES_NONE;
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::AddRow(const RowType& rtNewRow){
	std::ostringstream oss;
	int nCount;
	if(bConnected){
		if(TableName.size()){
			oss<<"insert into `"<<TableName<<"` values(";
			nCount = 0 ;
			for(Row_const_iterator riter = rtNewRow.begin();riter != rtNewRow.end();++riter,++nCount)
				oss<<riter->second<<(((nCount+1)!=rtNewRow.size())?",":")");
			return Query(oss.str());
		}else{
			return ES_NOTABLE;
		}
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::AddRow(const RowsType& rstNewRows){
	std::ostringstream oss;
	int nCount;
	if(bConnected){
		if(TableName.size()){
			uErrState = ES_NONE;
			for(Rows_const_iterator rsiter = rstNewRows.begin();rsiter != rstNewRows.end();++rsiter){
				oss.str("");
				oss<<"insert into `"<<TableName<<"` values(";
				nCount =0;
				for(Row_const_iterator riter = rsiter->begin();riter != rsiter->end();++riter,++nCount)
					oss<<riter->second<<(((nCount+1)!=rsiter->size())?",":")");
				uErrState |= Query(oss.str());
			}
			return uErrState;
		}else{
			return ES_NOTABLE;
		}
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::EditRow(const std::string& strRowCondition,const std::string& strColumn,const std::string& strNewContent){
	std::ostringstream oss;
	if(bConnected){
		if(TableName.size()){
			oss<<"update `"<<TableName<<"` set "<<strColumn<<"="<<ToString(strNewContent)<<" where "<<strRowCondition;
			return Query(oss.str());
		}else{
			return ES_NOTABLE;
		}
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::EditRow(const std::string& strRowCondition,const Col_Pair& cpPair){
	std::ostringstream oss;
	if(bConnected){
		if(TableName.size()){
			oss<<"update `"<<TableName<<"` set "<<cpPair.first<<"="<<ToString(cpPair.second)<<" where "<<strRowCondition;
			return Query(oss.str());
		}else{
			return ES_NOTABLE;
		}
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::EditRow(const Col_Pair& pairRowCondition,const RowType& rtRow){
	std::ostringstream oss;
	if(bConnected){
		if(TableName.size()){
			oss<<"update `"<<TableName<<"` set ";
			int nCount = 0;
			for(Row_const_iterator riter = rtRow.begin();riter != rtRow.end();++riter,++nCount){
				std::string strTmp1 = vecColNames[nCount],strTmp2 = pairRowCondition.first;
				transform(strTmp1.begin(),strTmp1.end(),strTmp1.begin(),toupper);
				transform(strTmp2.begin(),strTmp2.end(),strTmp2.begin(),toupper);
				if (strTmp1 != strTmp2)
				{
					oss<<vecColNames[nCount]<<"="<<ToString(riter->second)<<((nCount == vecColNames.size()-1)?" ":",");
				}
			}
			oss<<"where "<<pairRowCondition.first<<"="<<ToString(pairRowCondition.second);
			return Query(oss.str());
		}else{
			return ES_NOTABLE;
		}
	}else{
		return ES_NOCONNECT;
	}
}
unsigned int CMysql::DeleteRow(const std::string& strRowCondition){
	std::ostringstream oss;
	if(bConnected){
		if(TableName.size()){
			oss<<"delete `"<<TableName<<"` where "<<strRowCondition;
			return Query(oss.str());
		}else{
			return ES_NOTABLE;
		}
	}else{
		return ES_NOCONNECT;
	}
}

//---------Friend members------------
std::ostream& operator<<(std::ostream& out,CMysql& cms){
	cms.GetRes();
	for(CMysql::Rows_const_iterator iter = cms.svecRow.begin();iter != cms.svecRow.end();++iter){
		int nCount =0;
		for(CMysql::Row_const_iterator riter = iter->begin();riter != iter->end();++riter,nCount++){
			out<<riter->second<<(((nCount+1)!=iter->size())?"\t":"\n"); 
		}
	}
	return out;
}
std::istream& operator>>(std::istream& in,CMysql& cms){
	std::string strTmp;
	in>>strTmp;
	cms.rtAddTmp.insert(std::make_pair(++cms.nCurrentCol,CMysql::ToString(strTmp)));	
	if(cms.nCurrentCol == cms.vecColNames.size()){
		std::cout<<cms.nCurrentCol<<cms.rtAddTmp.size();
		cms.AddRow(cms.rtAddTmp);
		cms.GetRes();
		cms.nCurrentCol = 0;
		cms.rtAddTmp.clear();
	}
	return in;
}
#endif
