#include "kr_sdi.h"
#include "kr_sdi_func.h"

int kr_sdi_aggr_func(T_KRSDI *krsdi, T_KRContext *calc_param)
{
    int iResult = -1;
    
    int iAbsLoc = -1;
    int iRelLoc = -1;
    
    T_KRListNode *node = calc_param->ptRecList->tail;
    while(node)
    {
        calc_param->ptRecord = (T_KRRecord *)kr_list_value(node);
        
        iAbsLoc++; /*绝对位置加一*/
                
        /*数据项绝对统计定位校验*/
        if (krsdi->ptShmSDIDef->caLocationProperty[0] == KR_LOC_ABSOLUTE) {     
            if (krsdi->ptShmSDIDef->lStatisticsLocation != iAbsLoc) {
                KR_LOG(KR_LOGDEBUG, "AbsLoc:[%d] [%d] Not Match", \
                        krsdi->ptShmSDIDef->lStatisticsLocation, iAbsLoc);
                node = node->prev;continue;
            }
        }
        
        /*统计数据源校验*/
        if (((T_KRTable *)calc_param->ptRecord->ptTable)->iTableId != \
            krsdi->ptShmSDIDef->lStatisticsDatasrc) {
            KR_LOG(KR_LOGDEBUG, "current table[%d] doesn't match[%ld]!", \
                   ((T_KRTable *)calc_param->ptRecord->ptTable)->iTableId, 
                   krsdi->ptShmSDIDef->lStatisticsDatasrc);
            node = node->prev;continue;
        }
        
        /*过滤器校验*/
        iResult = kr_calc_eval(krsdi->ptSDICalc, calc_param);
        if (iResult != 0) {
            KR_LOG(KR_LOGERROR, "kr_calc_eval[%ld] failed!", krsdi->lSDIId);
	    	return -1;
        } else if (krsdi->ptSDICalc->result_type != KR_CALCTYPE_BOOLEAN) {
            KR_LOG(KR_LOGERROR, "result_type of sdi_calc must be boolean!");
	    	return -1;
        } else if (krsdi->ptSDICalc->result_ind != KR_VALUE_SETED ||
                   !krsdi->ptSDICalc->result_value.b) {
            KR_LOG(KR_LOGDEBUG, "SDICalc [%c] [%d] Not Match", \
                    krsdi->ptSDICalc->result_ind, 
                    krsdi->ptSDICalc->result_value.b);
            node = node->prev;continue;
        }
    
        iRelLoc++; /*相对位置加一*/
            
        /*数据项相对统计定位校验*/
        if (krsdi->ptShmSDIDef->caLocationProperty[0] == KR_LOC_RELATIVE) {     
            if (krsdi->ptShmSDIDef->lStatisticsLocation != iRelLoc) {
                KR_LOG(KR_LOGDEBUG, "RelLoc:[%d] [%d] Not Match", \
                        krsdi->ptShmSDIDef->lStatisticsLocation, iRelLoc);
                node = node->prev;continue;
            }
        }
            
        /*获取数据项值*/
        void *val = kr_get_field_value(calc_param->ptRecord, \
                          krsdi->ptShmSDIDef->lStatisticsField);
        switch(krsdi->eValueType)
        {
            case KR_FIELDTYPE_INT:
                krsdi->eValue.i = *(int *)val;
                break;
            case KR_FIELDTYPE_LONG:
                krsdi->eValue.l = *(long *)val;
                break;
            case KR_FIELDTYPE_DOUBLE:
                krsdi->eValue.d = *(double *)val;
                break;
            case KR_FIELDTYPE_STRING:
                strncpy(krsdi->eValue.s, (char *)val, sizeof(krsdi->eValue.s));
                break;
            default:
                KR_LOG(KR_LOGERROR, "Bad FieldType [%c]!", krsdi->eValueType);
                return -1;
        }
    
        /* This is what the difference between SDI and DDI:
         * SDI only set once, while DDI still need to traversal all the list
         */
        krsdi->eValueInd = KR_VALUE_SETED;
        break;
        
        node = node->prev;
    }
    
    return 0;
}
