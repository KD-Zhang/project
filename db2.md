DB2��������
1.�������д���
�� #db2cmd
2.�򿪿�������
�� # db2cmd db2cc
3.������༭��
��db2cmd db2ce
********************************************************�������ݿ�����*****************************************************
4.�������ݿ�ʵ��
�� #db2start
5.ֹͣ���ݿ�ʵ��
�� #db2stop
�� ����㲻��ֹͣ���ݿ����ڼ�������ӣ�������db2stopǰִ��db2 force application all�Ϳ����� /db2stop force
6.�������ݿ�
�� #db2 create db [dbname]
7.���ӵ����ݿ�
�� #db2 connect to [dbname] user[username] using [password]
8.�Ͽ����ݿ�����
�� #db2 connect reset
9.�г��������ݿ�
��#db2 list db directory
10.�г����м�������ݿ�
�� #db2 list active databases
11.�г��������ݿ�����
�� #db2 get db cfg
12.ɾ�����ݿ�
�� #db2 drop database [dbname]
��ִ�д˲���ҪС�ģ�
�������ɾ�����Ͽ��������ݿ����ӻ�������db2
********************************************************�������ݱ�����*****************************************************
13.�г������û���
�� #db2 list tables
14���г�����ϵͳ��
��#db2 list tables for system
15���г����б�
�� #db2 list tables for all
16.�г�ϵͳ��
�� #db2 list tables for system
17���г��û���
�� #db2 list tables for user
18.�г��ض��û���
�� #db2 list tables for schema[user]
19.����һ�������ݿ���ĳ����(t2)�ṹ��ͬ���±�(t1)
�� #db2 create table t1 like t2
20.��һ����t1�����ݵ��뵽��һ����t2
#db2 "insert into t1 select * from t2"
21.��ѯ��
�� #db2 "select * from tablename where ..."
22.��ʾ���ṹ
�� #db2 describe table tablename
23.�޸���
�� #db2 alter table [tablename]alter column [columname] set data type varchar(24)
********************************************************�ű��ļ���������********************************************************
24.ִ�нű��ļ�
�� #db2 -tvf scripts.sql

#db2 -tvf scripts.sql > script.log
25����������
* �鿴�������
��#db2 ? db2start
* �鿴��������Ϣ
#db2 ? 22001
* memo: ��ϸ������ʹ��"db2 ? <command>"���в鿴����
********************************************************************************************************************************************************************
26���������ݿ�
#db2 backup db <db name>
��ע��ִ����������֮ǰ��Ҫ�Ͽ����ݿ�����
27�����߱������ݿ�
#db2 -v "BACKUP DATABASE <database name> ONLINE TO <path> WITH2 BUFFERS BUFFER 1024 INCLUDE LOGS WITHOUT PROMPTING"
28���ָ����ݿ�
#db2 restore db <source db name>
29�����߻ָ����ݿ�
#db2 "RESTORE DB <database name> TO <db path> LOGTARGET<logpath> WITHOUT PROMPTING"
#db2 "ROLLFORWARD DB <database name> TO END OF LOGS AND STOP"...
30�����������ļ�
#db2move <db name> export
[-sn <ģʽ���ƣ�һ��Ϊdb2admin>]
[-tn <���������֮���ö��ŷָ�>]
31�����������ļ�
#db2move <db name> import
32����ȡdb2���ݿ�������û�����Ϣ
#db2 get dbm cfg
33��.��ȡdb2ĳ�����ݿ����ݿ�������û�����Ϣ
#db2 get db cfg for <db name>
���ߣ�������ĳ�����ݿ��Ժ�ִ��db2 get db cfg
34������db2��־�ռ�Ĵ�С
��ע����������Ϊ�˷�ֹdb2���ݿ����ʹ��Ӳ�̿ռ���裬�����ڿ������Լ������ϵ�db2������Ƿ��������������Ҫ�޸ġ�
#db2 UPDATE DB CFG FOR <db name> USING logretain OFF logprimary 3logsecond 2 logfilsiz 25600;
���ҳ��С��4KB�������������3��100M����־�ļ���ռ��300MBӲ�̿ռ䡣25600*4KB=102400KB��
35��������ʱ���ռ�
#DB2 CREATE USER TEMPORARY TABLESPACE STMASPACE PAGESIZE 32 K MANAGED BYDATABASE USING (FILE 'D:\DB2_TAB\STMASPACE.F1' 10000)
EXTENTSIZE 256
36����ȡ���ݿ�������Ŀ�������
#db2 �Cv get snapshot for dbm
37����ʾ���г̺�
#db2 list applications show detail
********************************************************************************************************************************************************************
һ���������ݣ�
1.��Ĭ�Ϸָ�������,Ĭ��Ϊ��,����
db2 "import from btpoper.txt of del insert into btpoper"
2.��ָ���ָ�����|������
db2 "import from btpoper.txt of del modified by coldel| insert intobtpoper"
����ж�����ݣ�
1.ж��һ������ȫ������
db2 "export to btpoper.txt of del select * from btpoper"
db2 "export to btpoper.txt of del modified by coldel| select * frombtpoper"
2.������ж��һ����������
db2 "export to btpoper.txt of del select * from btpoper wherebrhid='907020000'"
db2 "export to cmmcode.txt of del select * from cmmcode wherecodtp='01'"
db2 "export to cmmcode.txt of del modified by coldel| select * fromcmmcode where codtp='01'"
������ѯ���ݽṹ�����ݣ�
db2 "select * from btpoper"
db2 "select * from btpoper where brhid='907020000' and oprid='0001'"
db2 "select oprid,oprnm,brhid,passwd from btpoper"
�ġ�ɾ���������ݣ�
db2 "delete from btpoper"
db2 "delete from btpoper where brhid='907020000' orbrhid='907010000'"
�塢�޸ı������ݣ�
db2 "update svmmst set prtlines=0 where brhid='907010000' and jobtp='02'"
db2 "update svmmst set prtlines=0 where jobtp='02' or jobtp='03'"
�����������ݿ�
db2 connect to btpdbs
�ߡ�������ݿ�����
db2 connect reset �Ͽ����ݿ�����
db2 terminate �Ͽ����ݿ�����
db2 force applications all �Ͽ��������ݿ�����
�ˡ��������ݿ�
1.db2 backup db btpdbs
2.db2move btpdbs export
db2look -d btpdbs -e -x [-a] -o crttbl.sql
�š��ָ����ݿ�
1.db2 restore db btpdbs withoutrolling forward
2.db2 -tvf crtdb.sql
crtdb.sql�ļ����ݣ�create db btpdbs on /db2catalog
db2 -stvf crttbl.sql
db2move btpdbs import
ʮ��DB2�������
db2 ?
db2 ? restroe
db2 ? sqlcode (����db2 ? sql0803) ע��code����Ϊ4λ��������4λ��ǰ�油0
ʮһ��bind�����Ӧ�ó��������ݿ���һ����,ÿ�λָ����ݿ�󣬽��鶼Ҫ��һ��bind
(1) db2 bind br8200.bnd
(2) /btp/bin/bndall /btp/bnd
/btp/bin/bndall /btp/tran/bnd
ʮ�����鿴���ݿ������
db2 get dbm cfg
db2 get db cfg for btpdbs
ʮ�����޸����ݿ������
db2 update db cfg for btpdbs using LOGBUFSZ 20
db2 update db cfg for btpdbs using LOGFILSIZ 5120
�����Ӧִ����������ʹ����Ч��
db2 stop
db2 start
���䣺
db2 set schema btp �޸ĵ�ǰģʽΪ"btp"
db2 list tablespaces show detail �鿴��ǰ���ݿ���ռ����״��
db2 list tablespace containers for 2 show detail �鿴tablespace id=2ʹ����������Ŀ¼
db2 list application
db2 list db directory �г��������ݿ�
db2 list active databases �г����л�����ݿ�
db2 list tables for all �г���ǰ���ݿ������еı�
db2 list tables for schema btp �г���ǰ���ݿ���schemaΪbtp�ı�
db2 list tablespaces show detail ��ʾ���ݿ�ռ�ʹ�����
db2 list packages for all
db2 "import from tab76.ixf of ixf commitcount 5000 insert intoachact"
db2 "create table achact_t like achact"
db2 "rename table achact_t to achact"
db2 "insert into achact_t select * from achact where txndt>=(selectlstpgdt from
acmact where actno=achact.actno)"
db2 get snapshot for dynaimic sql on jining
ɾ��һ��ʵ����
# cd /usr/lpp/db2_07_01/instance
# ./db2idrop InstName
�г�����DB2ʵ����
# cd /usr/lpp/db2_07_01/bin
# ./db2ilist
Ϊ���ݿ⽨����Ŀ
$ db2 catalog db btpdbs on /db2catalog
ȡ���ѱ�Ŀ�����ݿ�btpdbs
$ db2 uncatalog db btpdbs
�鿴�汾
# db2level
��ʾ��ǰ���ݿ����ʵ��
$ db2 get instance
����ʵ��ϵͳ����ʱ�Ƿ��Զ�������
$ db2iauto -on �Զ�����
$ db2iauto -off ���Զ�����
���ݿ��Ż����
reorg��runstats
�����ݿ⾭��һ��ʱ��ʹ�ã����ݿռ����Խ��Խ�Ӵ�һЩdelete��
�������Դ�������ݿ��У�ռ�����ݿռ䣬Ӱ��ϵͳ���ܡ������Ҫ����
����reorg��runstats��������delete�����ݣ��Ż����ݽṹ��
db2 reorg table ����
db2 runstats on table ���� with distribution and indexes all
��ΪҪ�Ż��ı��Ƚ϶࣬������/btp/binĿ¼���ṩ��һ��sh����runsall��
���ڵ���ҵ�����������runsall�������ݿ�����Ż�
��DB2�Ŀ��������У��ᴩ�����������̻��к���Ҫ��һ���ֹ����������ݿ��ά��������ά��һ���Ӵ���Ϣϵͳ��˵�Ƿǳ���Ҫ�ģ���һ�ݼ��׵�ά���ֲᣬ�Ա���ʱ֮�裻
�����ռ����Ĳ���ά������������ǵ�ά������ʦ����Ŀ������
********************************************************************************************************************************************************************
38������db2��־�ռ�Ĵ�С
��ע����������Ϊ�˷�ֹdb2���ݿ����ʹ��Ӳ�̿ռ���裬�����ڿ������Լ������ϵ�db2������Ƿ��������������Ҫ�޸ġ�
# db2 UPDATE DB CFG FOR <db name> USING logretain OFF logprimary 3logsecond 2 logfilsiz 25600;
���ҳ��С��4KB�������������3��100M����־�ļ���ռ��300MBӲ�̿ռ䡣25600*4KB=102400KB��
39��������ʱ���ռ�
#DB2 CREATE USER TEMPORARY TABLESPACE STMASPACE PAGESIZE 32 K MANAGED BYDATABASE USING (FILE 'D:\DB2_TAB\STMASPACE.F1' 10000) EXTENTSIZE 256
40���������ռ�
rem ��������ؿռ� 8K
#db2 connect to gather
#db2 CREATE BUFFERPOOL STMABMP IMMEDIATE SIZE 25000 PAGESIZE 8K
rem �������ռ䣺STMA
rem ����ȷ��·����ȷ
rem D:\DB2Container\Stma
#db2 drop tablespace stma
#db2 CREATE REGULAR TABLESPACE STMA PAGESIZE 8 K MANAGED BY SYSTEM USING('D:\DB2Container\Stma' ) EXTENTSIZE 8 OVERHEAD 10.5 PREFETCHSIZE 8TRANSFERRATE 0.14 BUFFERPOOL STMABMP DROPPED TABLE RECOVERY OFF
#db2 connect reset
41�����ݹҵ����ݻָ���ǰ��״̬
#db2 ROLLFORWARD DATABASE TESTDB TO END OF LOGS AND COMPLETE NORETRIEVE
42�����ݱ��ռ�
#BACKUP DATABASE YNDC TABLESPACE ( USERSPACE1 ) TO "D:\temp" WITH 2BUFFERS BUFFER 1024 PARALLELISM 1 WITHOUT PROMPTING
43������db2�������ݿ�
#db2 create tools catalog systools create new database toolsdb
44����ν�������/��������
��������һ���������������α���֮�����ӵ����ݲ��֣�
����(delta)���ϴα����������������������ݡ��������ݻ��߲������ݣ������α���֮�����ӵ����ݲ��֣�
45���������б���ͳ����Ϣ
#db2 -v connect to DB_NAME
#db2 -v "select tbname, nleaf, nlevels, stats_timefromsysibm.sysindexes"
#db2 -v reorgchkupdate statistics on table all
#db2 -v "select tbname, nleaf, nlevels, stats_timefromsysibm.sysindexes"
#db2 -v terminate
46����һ�ű�����ͳ����Ϣ
#db2 -v runstatson table TAB_NAMEand indexes all
47���鿴�Ƿ�����ݿ�ִ����RUNSTATS
#db2 -v "select tbname, nleaf, nlevels,stats_timefromsysibm.sysindexes"
48�����Ļ���صĴ�С
������У���syscat.bufferpools��npages��-1ʱ�������ݿ�����ò���bufferpage���ƻ���صĴ�С��
��npages��ֵ����Ϊ-1�����
#db2 -v connect to DB_NAME
#db2 -v select * from syscat.bufferpools
#db2 -v alter bufferpoolIBMDEFAULTBP size -1
#db2 -v connect reset
#db2 -v terminate
�������ݿ����ò���BufferPages���������£�
#db2 -v update db cfgfor dbnameusing BUFFPAGE bigger_value
#db2 -v terminate
49�������ݿ���������б�
#db2 -v get monitor switches
50����ĳ�����ݿ��������
#db2 -v update monitor switches using bufferpoolon
51����ȡ���ݿ����
#db2 -v get snapshot for all databases > snap.out
#db2 -v get snapshot for dbm>> snap.out
#db2 -v get snapshot for all bufferpools>> snap.out
#db2 -v terminate
52���������ݿ����
#db2 -v reset monitor all
53�����㻺���������
��������»������������95%���ϣ����㹫ʽ���£�
(1 -((buffer pool data physical reads + buffer pool index physical reads)
/(buffer pool data logical reads + pool index logical reads))) *100%
****************************************************************************************���ݿ�ʵ��****************************************************************************************
54������db2ʵ��
#db2icrt <ʵ������>
55��ɾ��db2ʵ��
#db2idrop <ʵ������>
56�����õ�ǰdb2ʵ��
#set db2intance=db2
57����ʾdb2ӵ�е�ʵ��
#db2ilist
58���ָ����������������ݿ������
#DB2 RESTORE DATABASE YNDC INCREMENTAL AUTOMATIC FROM D:\backup\autobak\db2TAKEN AT 20060314232015
59�������������ݿ�
��unixƽ̨��ʹ�ã�
#sqllib/bin/db2sampl <path>
��windows,os/2ƽ̨��ʹ�ã�db2sampl e,e�ǿ�ѡ������ָ�����������ݿ��������
60�������������ݿ�Ϊ���ã�Ĭ���������ݿⲻ���ã�
#db2 update dbm cfg using federated yes
61���г����ݿ������еı�
#db2 list tables
62������Ǩ�Ʒ���1
export�ű�ʾ��
#db2 connect to testdb user test password test
#db2 "export to aa1.ixf of ixf select * from table1"
#db2 "export to aa2.ixf of ixf select * from table2"
#db2 connect reset
import�ű�ʾ��
#db2 connect to testdb user test password test
#db2 "load from aa1.ixf of ixf replace into table1 COPY NO withoutprompting "
#db2 "load from aa2.ixf of ixf replace into table2 COPY NO withoutprompting "
#db2 connect reset

63 load������������

#db2 "load from aa1.txt of txt for del insert into table1" ����TXT�ļ�

#db2 "load from aa2.ixf of ixf insert into table2 nonrecoverable" ����¼��־

#db2 "load from aa3.ixf of ixf modified by identityoverride insert into table3 nonrecoverable" ������

65����DB2�ļ�

1.ִ�н���dbmove.dll�ļ�(�޸Ľ����ռ��·��)

2.db2move dbname load

3.db2 "alter tablespace tbs7(���ռ�) extend (all 20g)" ��չ���ռ�