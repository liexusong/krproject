DELETE from KR_TBL_RULE;

insert into KR_TBL_RULE (RULE_ID, RULE_NAME, RULE_DESC, RULE_STRING, RULE_TYPE, RULE_WEIGHT, RISK_ADVC_THRESH, RISK_NOTIF_THRESH, RISK_ALERT_THRESH, RISK_WARN_THRESH, THRESH_IS_CONFIG, RULE_STATUS, REC_CRET_DTTM, LST_UPD_DTTM, LST_UPD_USER_ID)
values (4, 'rule_4', '���Թ���', '(S_1>''091913'')&&(S_1<''091930'')&&(D_1>30)&&(C_3 >=''20120501'');', '1', 50, 0, 0, 0, 0, '0000', '1', '20120523141650   ', '20120523141650   ', 'Tiger');

commit;