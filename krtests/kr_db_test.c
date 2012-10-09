#include "krutils/kr_utils.h"
#include "krdb/kr_db.h"
#include "krdb/kr_db_interface.h"
#include "dbs/dbs_basopr.h"
#include <time.h>

T_KRDB *gptKRDB = NULL;

static int MyPrintRecord(T_KRRecord *ptRecord, void *user_data);

int main(int argc,char *argv[])
{
    int i = 0;
    int iResult = 0;
    char caTmpBuff[200] = {0};
    T_KRTradFlow_1 stTradFlow1 = {0};
    
    T_KRRecord *ptRecord = NULL;
    
	time_t	lTime;
	struct tm	*tTmLocal;
	char	sLogTime[128];
	
	iResult = dbsDbConnect();
	if (iResult != 0)
	{
	    KR_LOG(KR_LOGERROR, "dbsDbConnect Failed!");
        return -1;
	}
	
	//Step 1:kr_db_startup
	gptKRDB = kr_db_startup("AFD KRDB");
	if (gptKRDB == NULL)
	{
	    KR_LOG(KR_LOGERROR, "kr_db_startup Failed!");
        return -1;
	}

    kr_db_load(gptKRDB);
    
    //Step 2:FraudDetect
    for (i=0; i<22; i++)
    {
        memset(&stTradFlow1, 0x00, sizeof(T_KRTradFlow_1));
        
        memset(caTmpBuff, 0x00, sizeof(caTmpBuff));
        sprintf(caTmpBuff, "%s%04d", "6223000000000000", i%4);
        memcpy(stTradFlow1.caOutCustNo, caTmpBuff, sizeof(stTradFlow1.caOutCustNo));
                
        memset(caTmpBuff, 0x00, sizeof(caTmpBuff));
        sprintf(caTmpBuff, "201205%02d", i%7);
        memcpy(stTradFlow1.caOutTransDate, caTmpBuff, sizeof(stTradFlow1.caOutTransDate));
        
        memset(caTmpBuff, 0x00, sizeof(caTmpBuff));
        sprintf(caTmpBuff, "0919%02d", i%60);
        memcpy(stTradFlow1.caOutTransTime, caTmpBuff, sizeof(stTradFlow1.caOutTransTime));

        memset(caTmpBuff, 0x00, sizeof(caTmpBuff));
        sprintf(caTmpBuff, "%015d", i);
        memcpy(stTradFlow1.caOutFlowNo, caTmpBuff, sizeof(stTradFlow1.caOutFlowNo));
        
        memset(caTmpBuff, 0x00, sizeof(caTmpBuff));
        sprintf(caTmpBuff, "%02d", i%2);
        memcpy(stTradFlow1.caOutTransType, caTmpBuff, sizeof(stTradFlow1.caOutTransType));
        
        stTradFlow1.dOutTransAmt = i*9.9;
        
        memset(caTmpBuff, 0x00, sizeof(caTmpBuff));
        sprintf(caTmpBuff, "TRANS_LOCATION:%02d", i);
        memcpy(stTradFlow1.caOutTransLoc, caTmpBuff, sizeof(stTradFlow1.caOutTransLoc));
        
        if (i%10000 == 0)
        {
        	/* get current time */
	        memset (sLogTime, 0x00, sizeof(sLogTime));
	        lTime = time (NULL);
	        tTmLocal = localtime (&lTime);
	        strftime (sLogTime, sizeof(sLogTime), "%Y-%m-%d %H:%M:%S", tTmLocal);
            printf("Inserting Record :[%d][%s]\n", i, sLogTime);
        }
        
        //Step 2.1:InsertIntoKRDB
        ptRecord = kr_db_insert(gptKRDB, 1, &stTradFlow1);
	    if (ptRecord == NULL)
	    {
	        KR_LOG(KR_LOGERROR, "kr_db_insert [%s] Failed!", stTradFlow1.caOutFlowNo);
            return -1;
	    }
	    
	    //sleep(1);
    }

/*
    FILE *fpDump;
    if ((fpDump = fopen("KRDB.dump", "w")) == NULL) 
    {
        BAS_LOG(BAS_LOGERROR, 0, errno, "Open DumpFile Error!");
        return -1;
    }
    KRDBDumpDBToFile(gptKRDB, fpDump);
    fclose(fpDump);
*/
    kr_db_dump_field_def(stdout, gptKRDB, 1);
    kr_db_dump(stdout, gptKRDB, 0);
    //kr_db_dump(stdout, gptKRDB, 1);
    /*    
    	printf("gptKRDB->ptCurrTable=[%p][%d][%s]\n", 
	        gptKRDB->ptCurrTable, gptKRDB->ptCurrTable->iRecordSize, 
	        gptKRDB->ptCurrTable->caMMapFile);
	kr_db_mmap_file_handle(gptKRDB->ptCurrTable->caMMapFile, MyPrintRecord, NULL);
*/
    
    //Step 3:kr_db_shutdown
    iResult = kr_db_shutdown(gptKRDB);
	if (iResult != 0)
	{
	    KR_LOG(KR_LOGERROR, "kr_db_shutdown Failed!");
        return -1;
	}
	
	dbsDbDisconnect();
	

	return 0;
}

static int MyPrintRecord(T_KRRecord *ptRecord, void *user_data)
{
    T_KRTable  *ptTable = (T_KRTable *)ptRecord->ptTable;
    FILE       *fp = stdout;
    void *pFieldVal = NULL;
    int i = 0;
    fprintf(fp, "    Record: FieldCnt[%d] \n", ptTable->iFieldCnt);
    for (i = 0; i<ptTable->iFieldCnt; i++)
    {
        pFieldVal = kr_get_field_value(ptRecord, i);
        switch(ptTable->ptFieldDef[i].type)
        {
            case KR_FIELDTYPE_INT:
                fprintf(fp, "      Field:id[%3d], name[%30s], value[%d]\n", \
                        ptTable->ptFieldDef[i].id, ptTable->ptFieldDef[i].name, \
                        *(int *)pFieldVal);
                break;
            case KR_FIELDTYPE_LONG:
                fprintf(fp, "      Field:id[%3d], name[%30s], value[%ld]\n", \
                        ptTable->ptFieldDef[i].id, ptTable->ptFieldDef[i].name, \
                        *(long *)pFieldVal);
                break;    
            case KR_FIELDTYPE_DOUBLE:
                fprintf(fp, "      Field:id[%3d], name[%30s], value[%f]\n", \
                        ptTable->ptFieldDef[i].id, ptTable->ptFieldDef[i].name, \
                        *(double *)pFieldVal);
                break;
            case KR_FIELDTYPE_STRING:
                fprintf(fp, "      Field:id[%3d], name[%30s], value[%s]\n", \
                        ptTable->ptFieldDef[i].id, ptTable->ptFieldDef[i].name, \
                        (char *)pFieldVal);
                break;
            case KR_FIELDTYPE_POINTER:
                fprintf(fp, "      Field:id[%3d], name[%30s], value[%p]\n", \
                        ptTable->ptFieldDef[i].id, ptTable->ptFieldDef[i].name, \
                        pFieldVal);
                break;
            default:
                break;           
        }
    }
    return 0;
}


