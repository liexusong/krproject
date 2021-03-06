#include "kr_iface.h"

typedef struct _kr_iface_json_data_t
{
    int datasrc_id;
    cJSON *schema;
    char filename[1024];
    FILE *fp;
}T_KRIfaceJsonData;

/*json format*/
void *json_define_pre_func(T_DatasrcCur *ptDatasrcCur)
{
    T_KRIfaceJsonData *json_data = kr_calloc(sizeof(*json_data));
    /* Create Schema json */
    json_data->schema = cJSON_CreateObject();

    /* Open Schema File */
    json_data->datasrc_id = ptDatasrcCur->lOutDatasrcId;
    snprintf(json_data->filename, sizeof(json_data->filename), \
                "%s.json", ptDatasrcCur->caOutDatasrcName);
    json_data->fp = fopen(json_data->filename, "w");
    if (json_data->fp == NULL) {
        fprintf(stdout, "open output file: %s failed\n", json_data->filename);
        cJSON_Delete(json_data->schema);
        kr_free(json_data);
        return NULL;
    }
    return json_data;
}

void json_define_post_func(void *data, int flag)
{
    T_KRIfaceJsonData *json_data = (T_KRIfaceJsonData *)data;

    if (flag != 0) goto clean;

    /* Dump schema to file */
    char *schema_string = cJSON_Print(json_data->schema);
    fprintf(json_data->fp, "%s \n", schema_string);
    kr_free(schema_string);

clean:
    /* Delete Schema*/
    cJSON_Delete(json_data->schema);
    fclose(json_data->fp);
    kr_free(json_data);
}

int json_define_func(T_DatasrcFieldCur *ptDatasrcFieldCur, void *data)
{
    T_KRIfaceJsonData *json_data = (T_KRIfaceJsonData *)data;
    cJSON *schema = (cJSON *)json_data->schema;

    char *name = ptDatasrcFieldCur->caOutFieldName;
    switch(ptDatasrcFieldCur->caOutFieldType[0])
    {
        case KR_TYPE_INT:
            cJSON_AddNumberToObject(schema, name, 0);
            break;
        case KR_TYPE_LONG:
            cJSON_AddNumberToObject(schema, name, 10);
            break;
        case KR_TYPE_DOUBLE:
            cJSON_AddNumberToObject(schema, name, 0.0);
            break;
        case KR_TYPE_STRING:
            cJSON_AddStringToObject(schema, name, " ");
            break;
        default:
            break;
    }

    return 0;
}


/*json format source*/
void *json_source_pre_func(T_DatasrcCur *ptDatasrcCur)
{
    T_KRIfaceJsonData *json_data = kr_calloc(sizeof(*json_data));

    /* Open Source File */
    json_data->datasrc_id = ptDatasrcCur->lOutDatasrcId;
    snprintf(json_data->filename, sizeof(json_data->filename), \
                "%s_json.c", ptDatasrcCur->caOutDatasrcName);
    json_data->fp = fopen(json_data->filename, "w");
    if (json_data->fp == NULL) {
        fprintf(stdout, "open output file: %s failed\n", json_data->filename);
        kr_free(json_data);
        return NULL;
    }

    /*generate pre&post function*/
    FILE *fp = json_data->fp;
    fprintf(fp, "#include <stdio.h> \n");
    fprintf(fp, "#include <stdlib.h> \n");
    fprintf(fp, "#include <string.h> \n");
    fprintf(fp, "#include <time.h> \n");
    fprintf(fp, "#include <jansson.h> \n");
    fprintf(fp, "\n\n");
    /* map_pre_func */
    fprintf(fp, "void *json_map_pre_func_%d(void *msg)\n", json_data->datasrc_id);
    fprintf(fp, "{ \n");
    fprintf(fp, "    json_t  *root; \n");
    fprintf(fp, "    json_error_t  json_error; \n");
    fprintf(fp, "    root = json_loads(msg, 0, &json_error); \n");
    fprintf(fp, "    return root;\n");
    fprintf(fp, "} \n");
    fprintf(fp, "\n\n");
    /* map_func_post */
    fprintf(fp, "void json_map_post_func_%d(void *data)\n", json_data->datasrc_id);
    fprintf(fp, "{ \n");
    fprintf(fp, "    json_t  *root = (json_t *)data; \n");
    fprintf(fp, "    json_decref(root); \n");
    fprintf(fp, "} \n");
    fprintf(fp, "\n\n");
    /* map_func header */
    fprintf(fp, "void json_map_func_%d(void *fldval, int fldno, int fldlen, void *data)\n", json_data->datasrc_id);
    fprintf(fp, "{ \n");
    fprintf(fp, "    json_t *root = (json_t *)data; \n");
    fprintf(fp, "    memset(fldval, 0x00, fldlen); \n");
    fprintf(fp, "    switch(fldno) {\n");

    return json_data;
}

void json_source_post_func(void *data, int flag)
{
    T_KRIfaceJsonData *json_data = (T_KRIfaceJsonData *)data;

    /* map_func footer */
    FILE *fp = json_data->fp;
    fprintf(fp, "    default: \n");
    fprintf(fp, "        break; \n");
    fprintf(fp, "    } \n");
    fprintf(fp, "} \n");

clean:
    fclose(json_data->fp);
    kr_free(json_data);
}

int json_source_func(T_DatasrcFieldCur *ptDatasrcFieldCur, void *data)
{
    T_KRIfaceJsonData *json_data = (T_KRIfaceJsonData *)data;
    FILE *fp = json_data->fp;
    char *name = ptDatasrcFieldCur->caOutFieldName;
    printf("processing field %s \n", name);

    fprintf(fp, "    case %ld: \n", ptDatasrcFieldCur->lOutFieldId);
    fprintf(fp, "    { \n");
    fprintf(fp, "        json_t *field = json_object_get(root, \"%s\"); \n", name);
    switch(ptDatasrcFieldCur->caOutFieldType[0])
    {
        case KR_TYPE_INT:
            fprintf(fp, "        *(int *)fldval = (int )json_integer_value(field);\n");
            break;
        case KR_TYPE_LONG:
            fprintf(fp, "        *(long *)fldval = (long )json_integer_value(field);\n");
            break;
        case KR_TYPE_DOUBLE:
            fprintf(fp, "        *(double *)fldval = (double )json_real_value(field);\n");
            break;
        case KR_TYPE_STRING:
            fprintf(fp, "        memcpy(fldval, json_string_value(field), fldlen); \n");
            break;
        default:
            break;
    }
    fprintf(fp, "        break; \n");
    fprintf(fp, "    } \n");
    return 0;
}

