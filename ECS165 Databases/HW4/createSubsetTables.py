import psycopg2
import os
import csv

def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

BATCH_LENGTH = 1000
PATH = '/home/cjnitta/ecs165a/subset/'

TABLE_NAMES = ['DAYV2PUB', 'HHV2PUB', 'PERV2PUB', 'VEHV2PUB']

attrMap = {}
attrMap['ANNMILES'] = 'REAL'
attrMap['AWAYHOME'] = 'INTEGER'
attrMap['BEST_EDT'] = 'INTEGER'
attrMap['BEST_FLG'] = 'INTEGER'
attrMap['BEST_OUT'] = 'INTEGER'
attrMap['BESTMILE'] = 'REAL'
attrMap['BORNINUS'] = 'INTEGER'
attrMap['CARRODE'] = 'INTEGER'
attrMap['CDIVMSAR'] = 'INTEGER'
attrMap['CENSUS_D'] = 'INTEGER'
attrMap['CENSUS_R'] = 'INTEGER'
attrMap['CNTTDHH'] = 'INTEGER'
attrMap['CNTTDTR'] = 'INTEGER'
attrMap['CONDNIGH'] = 'INTEGER'
attrMap['CONDPUB'] = 'INTEGER'
attrMap['CONDRIDE'] = 'INTEGER'
attrMap['CONDRIVE'] = 'INTEGER'
attrMap['CONDSPEC'] = 'INTEGER'
attrMap['CONDTAX'] = 'INTEGER'
attrMap['CONDTRAV'] = 'INTEGER'
attrMap['DELIVER'] = 'INTEGER'
attrMap['DIARY'] = 'INTEGER'
attrMap['DISTTOSC'] = 'INTEGER'
attrMap['DISTTOWK'] = 'REAL'
attrMap['DRIVER'] = 'INTEGER'
attrMap['DROP_PRK'] = 'INTEGER'
attrMap['DRVR_FLG'] = 'INTEGER'
attrMap['DRVRCNT'] = 'INTEGER'
attrMap['DTACDT'] = 'INTEGER'
attrMap['DTCONJ'] = 'INTEGER'
attrMap['DTCOST'] = 'INTEGER'
attrMap['DTRAGE'] = 'INTEGER'
attrMap['DTRAN'] = 'INTEGER'
attrMap['DTWALK'] = 'INTEGER'
attrMap['DWELTIME'] = 'INTEGER'
attrMap['EDUC'] = 'INTEGER'
attrMap['EIADMPG'] = 'REAL'
attrMap['ENDTIME'] = 'INTEGER'
attrMap['EPATMPG'] = 'REAL'
attrMap['EPATMPGF'] = 'INTEGER'
attrMap['EVERDROV'] = 'INTEGER'
attrMap['FLAG100'] = 'INTEGER'
attrMap['FLEXTIME'] = 'INTEGER'
attrMap['FMSCSIZE'] = 'INTEGER'
attrMap['FRSTHM'] = 'INTEGER'
attrMap['FUELTYPE'] = 'INTEGER'
attrMap['FXDWKPL'] = 'INTEGER'
attrMap['GASPRICE'] = 'REAL'
attrMap['GCDWORK'] = 'REAL'
attrMap['GRADE'] = 'INTEGER'
attrMap['GSCOST'] = 'REAL'
attrMap['GSTOTCST'] = 'INTEGER'
attrMap['GSYRGAL'] = 'INTEGER'
attrMap['GT1JBLWK'] = 'INTEGER'
attrMap['HBHTNRNT'] = 'INTEGER'
attrMap['HBHUR'] = 'VARCHAR(2)'
attrMap['HBPPOPDN'] = 'INTEGER'
attrMap['HBRESDN'] = 'INTEGER'
attrMap['HH_CBSA'] = 'INTEGER'
attrMap['HH_HISP'] = 'INTEGER'
attrMap['HH_ONTD'] = 'INTEGER'
attrMap['HH_RACE'] = 'INTEGER'
attrMap['HHC_MSA'] = 'INTEGER'
attrMap['HHFAMINC'] = 'INTEGER'
attrMap['HHMEMDRV'] = 'INTEGER'
attrMap['HHRELATD'] = 'INTEGER'
attrMap['HHRESP'] = 'INTEGER'
attrMap['HHSIZE'] = 'INTEGER'
attrMap['HHSTATE'] = 'CHAR(2)'
attrMap['HHSTFIPS'] = 'INTEGER'
attrMap['HHVEHCNT'] = 'INTEGER'
attrMap['HOMEOWN'] = 'INTEGER'
attrMap['HOMETYPE'] = 'INTEGER'
attrMap['HOUSEID'] = 'INTEGER'
attrMap['HTEEMPDN'] = 'INTEGER'
attrMap['HTHTNRNT'] = 'INTEGER'
attrMap['HTPPOPDN'] = 'INTEGER'
attrMap['HTRESDN'] = 'INTEGER'
attrMap['HYBRID'] = 'INTEGER'
attrMap['INTSTATE'] = 'INTEGER'
attrMap['ISSUE'] = 'INTEGER'
attrMap['LIF_CYC'] = 'INTEGER'
attrMap['LSTTRDAY'] = 'INTEGER'
attrMap['MAKECODE'] = 'INTEGER'
attrMap['MCUSED'] = 'INTEGER'
attrMap['MEDCOND'] = 'INTEGER'
attrMap['MEDCOND6'] = 'INTEGER'
attrMap['MODLCODE'] = 'INTEGER'
attrMap['MOROFTEN'] = 'INTEGER'
attrMap['MSACAT'] = 'INTEGER'
attrMap['MSASIZE'] = 'INTEGER'
attrMap['NBIKETRP'] = 'INTEGER'
attrMap['NONHHCNT'] = 'INTEGER'
attrMap['NUMADLT'] = 'INTEGER'
attrMap['NUMONTRP'] = 'INTEGER'
attrMap['NWALKTRP'] = 'INTEGER'
attrMap['OCCAT'] = 'INTEGER'
attrMap['OD_READ'] = 'INTEGER'
attrMap['ONTD_P1'] = 'INTEGER'
attrMap['ONTD_P10'] = 'INTEGER'
attrMap['ONTD_P11'] = 'INTEGER'
attrMap['ONTD_P12'] = 'INTEGER'
attrMap['ONTD_P13'] = 'INTEGER'
attrMap['ONTD_P14'] = 'INTEGER'
attrMap['ONTD_P15'] = 'INTEGER'
attrMap['ONTD_P2'] = 'INTEGER'
attrMap['ONTD_P3'] = 'INTEGER'
attrMap['ONTD_P4'] = 'INTEGER'
attrMap['ONTD_P5'] = 'INTEGER'
attrMap['ONTD_P6'] = 'INTEGER'
attrMap['ONTD_P7'] = 'INTEGER'
attrMap['ONTD_P8'] = 'INTEGER'
attrMap['ONTD_P9'] = 'INTEGER'
attrMap['OUTCNTRY'] = 'INTEGER'
attrMap['OUTOFTWN'] = 'INTEGER'
attrMap['PAYPROF'] = 'INTEGER'
attrMap['PAYTOLL'] = 'INTEGER'
attrMap['PERSONID'] = 'INTEGER'
attrMap['PRMACT'] = 'INTEGER'
attrMap['PROXY'] = 'INTEGER'
attrMap['PSGR_FLG'] = 'INTEGER'
attrMap['PTUSED'] = 'INTEGER'
attrMap['PUBTRANS'] = 'INTEGER'
attrMap['PURCHASE'] = 'INTEGER'
attrMap['R_AGE'] = 'INTEGER'
attrMap['R_RELAT'] = 'INTEGER'
attrMap['R_SEX'] = 'INTEGER'
attrMap['RAIL'] = 'INTEGER'
attrMap['RESP_CNT'] = 'INTEGER'
attrMap['SAMEPLC'] = 'INTEGER'
attrMap['SCHCARE'] = 'INTEGER'
attrMap['SCHCRIM'] = 'INTEGER'
attrMap['SCHDIST'] = 'INTEGER'
attrMap['SCHSPD'] = 'INTEGER'
attrMap['SCHTRAF'] = 'INTEGER'
attrMap['SCHTRN1'] = 'INTEGER'
attrMap['SCHTRN2'] = 'INTEGER'
attrMap['SCHTYP'] = 'INTEGER'
attrMap['SCHWTHR'] = 'INTEGER'
attrMap['SCRESP'] = 'INTEGER'
attrMap['SELF_EMP'] = 'INTEGER'
attrMap['SFWGT'] = 'REAL'
attrMap['STRTTIME'] = 'INTEGER'
attrMap['TDAYDATE'] = 'INTEGER'
attrMap['TDCASEID'] = 'BIGINT'
attrMap['TDTRPNUM'] = 'INTEGER'
attrMap['TDWKND'] = 'INTEGER'
attrMap['TIMETOSC'] = 'INTEGER'
attrMap['TIMETOWK'] = 'INTEGER'
attrMap['TOSCSIZE'] = 'INTEGER'
attrMap['TRACC1'] = 'INTEGER'
attrMap['TRACC2'] = 'INTEGER'
attrMap['TRACC3'] = 'INTEGER'
attrMap['TRACC4'] = 'INTEGER'
attrMap['TRACC5'] = 'INTEGER'
attrMap['TRACCTM'] = 'INTEGER'
attrMap['TRAVDAY'] = 'INTEGER'
attrMap['TREGR1'] = 'INTEGER'
attrMap['TREGR2'] = 'INTEGER'
attrMap['TREGR3'] = 'INTEGER'
attrMap['TREGR4'] = 'INTEGER'
attrMap['TREGR5'] = 'INTEGER'
attrMap['TREGRTM'] = 'INTEGER'
attrMap['TRIPPURP'] = 'VARCHAR(8)'
attrMap['TRPACCMP'] = 'INTEGER'
attrMap['TRPHHACC'] = 'INTEGER'
attrMap['TRPHHVEH'] = 'INTEGER'
attrMap['TRPMILES'] = 'REAL'
attrMap['TRPTRANS'] = 'INTEGER'
attrMap['TRVL_MIN'] = 'INTEGER'
attrMap['TRVLCMIN'] = 'INTEGER'
attrMap['TRWAITTM'] = 'INTEGER'
attrMap['URBAN'] = 'INTEGER'
attrMap['URBANSIZE'] = 'INTEGER'
attrMap['URBRUR'] = 'INTEGER'
attrMap['USEINTST'] = 'INTEGER'
attrMap['USEPUBTR'] = 'INTEGER'
attrMap['VARSTRAT'] = 'INTEGER'
attrMap['VEHAGE'] = 'INTEGER'
attrMap['VEHCOMM'] = 'INTEGER'
attrMap['VEHID'] = 'INTEGER'
attrMap['VEHOWNMO'] = 'REAL'
attrMap['VEHTYPE'] = 'INTEGER'
attrMap['VEHYEAR'] = 'INTEGER'
attrMap['VMT_MILE'] = 'REAL'
attrMap['WEBUSE'] = 'INTEGER'
attrMap['WHODROVE'] = 'INTEGER'
attrMap['WHOMAIN'] = 'INTEGER'
attrMap['WHYFROM'] = 'INTEGER'
attrMap['WHYTO'] = 'INTEGER'
attrMap['WHYTRP1S'] = 'INTEGER'
attrMap['WHYTRP90'] = 'INTEGER'
attrMap['WKFMHMXX'] = 'INTEGER'
attrMap['WKFTPT'] = 'INTEGER'
attrMap['WKRMHM'] = 'INTEGER'
attrMap['WKSTFIPS'] = 'INTEGER'
attrMap['WORKER'] = 'INTEGER'
attrMap['WRKCOUNT'] = 'INTEGER'
attrMap['WRKTIME'] = 'VARCHAR(7)'
attrMap['WRKTRANS'] = 'INTEGER'
attrMap['WTHHFIN'] = 'REAL'
attrMap['WTPERFIN'] = 'REAL'
attrMap['WTTRDFIN'] = 'REAL'
attrMap['YEARMILE'] = 'INTEGER'
attrMap['YRMLCAP'] = 'INTEGER'
attrMap['YRTOUS'] = 'INTEGER'

def createTableQuery(table, row):
	query = "CREATE TABLE " + table + " ("
	count = 1
	for col in row:
		query += col + ' ' + attrMap[col] 
		if count < len(row):
			query += ','

		count += 1
	query += ');'

	return query

def createInsertionQuery(row):
	query = "("
	count = 1
	for col in row:
		if 'XX' in col:
			query += 'NULL'
		else:
			query += "'" + col + "'" 

		if count < len(row):
			query += ','

		count += 1
	query += '),'

	return query
#Open a cursor to perform databse operations
conn = psycopg2.connect("dbname=postgres host = /home/" + os.environ['USER'] + "/postgres ")
cur = conn.cursor()

cur.execute("""
	SELECT EXISTS (
		SELECT 1
		FROM information_schema.tables
		WHERE 	table_schema = 'public'
		AND		table_name = 'eia_co2_trans'
		);
	""")
out = cur.fetchone();
if out[0] == True:
	print "Dropping EIA_CO2_Trans"
	cur.execute("DROP TABLE EIA_CO2_Trans")

cur.execute("""
	SELECT EXISTS (
		SELECT 1
		FROM information_schema.tables
		WHERE 	table_schema = 'public'
		AND		table_name = 'eia_co2_elec'
		);
	""")
out = cur.fetchone();
if out[0] == True:
	print "Dropping EIA_CO2_Elec"
	cur.execute("DROP TABLE EIA_CO2_ELEC")

cur.execute("""
	SELECT EXISTS (
		SELECT 1
		FROM information_schema.tables
		WHERE 	table_schema = 'public'
		AND		table_name = 'eia_mkwh'
		);
	""")
out = cur.fetchone();
if out[0] == True:
	print "Dropping EIA_CO2_Mkwh"
	cur.execute("DROP TABLE EIA_MKWH")

cur.execute("""
	SELECT EXISTS (
		SELECT 1
		FROM information_schema.tables
		WHERE 	table_schema = 'public'
		AND		table_name = 'descrip'
		);
	""")
out = cur.fetchone();
if out[0] == True:
	print "Dropping Descrip"
	cur.execute("DROP TABLE Descrip")

['DAYV2PUB', 'HHV2PUB', 'PERV2PUB', 'VEHV2PUB']

cur.execute("""
	SELECT EXISTS (
		SELECT 1
		FROM information_schema.tables
		WHERE 	table_schema = 'public'
		AND		table_name = 'dayv2pub'
		);
	""")
out = cur.fetchone();
if out[0] == True:
	print "Dropping DAYV2PUB"
	cur.execute("DROP TABLE DAYV2PUB")

cur.execute("""
	SELECT EXISTS (
		SELECT 1
		FROM information_schema.tables
		WHERE 	table_schema = 'public'
		AND		table_name = 'hhv2pub'
		);
	""")
out = cur.fetchone();
if out[0] == True:
	print "Dropping HHV2PUB"
	cur.execute("DROP TABLE HHV2PUB")

cur.execute("""
	SELECT EXISTS (
		SELECT 1
		FROM information_schema.tables
		WHERE 	table_schema = 'public'
		AND		table_name = 'perv2pub'
		);
	""")
out = cur.fetchone();
if out[0] == True:
	print "Dropping PERV2PUB"
	cur.execute("DROP TABLE PERV2PUB")

cur.execute("""
	SELECT EXISTS (
		SELECT 1
		FROM information_schema.tables
		WHERE 	table_schema = 'public'
		AND		table_name = 'vehv2pub'
		);
	""")
out = cur.fetchone();
if out[0] == True:
	print "Dropping VEHV2PUB"
	cur.execute("DROP TABLE VEHV2PUB")

conn.commit()

#Execute a command: creates a new table - JUST SQL


for name in TABLE_NAMES:
	with open(PATH + name + '.CSV', 'r') as f:
		query = "INSERT INTO " + name + " VALUES "
		index = 0

		reader = csv.reader(f)
		for row in reader:
			if index == 0:
				cur.execute(createTableQuery(name, row))
			else:
				if index % BATCH_LENGTH == 1 and index > 1:
					query = query[0:-1] + ';'
					cur.execute(query)
					#print(str(index) + ' inserted into table ' + name)
					query = "INSERT INTO " + name + " VALUES "

				query += createInsertionQuery(row)

			index += 1

		if len(query) > 0:
			query = query[0:-1] + ';'
			cur.execute(query)

		f.close()






cur.execute("""
	CREATE TABLE EIA_CO2_Trans
	(
		MSN varchar(10),
		Column_Order int,
		YYYYMM int,
		Value real
	);
	"""
	)
print "Created EIA_CO2_Trans"

cur.execute("""
	CREATE TABLE EIA_CO2_ELEC	
	(
		MSN varchar(10),
		Column_Order int,
		YYYYMM int,
		Value real
	);
	"""
	)
print "Created EIA_CO2_Elec"

cur.execute("""
	CREATE TABLE EIA_MKWH
	(
		MSN varchar(10),
		Column_Order int,
		YYYYMM int,
		Value real
	);
	"""
	)
print "Created EIA_CO2_Mkwh"

cur.execute("""
	CREATE TABLE Descrip
	(
		MSN varchar(10),
		Description varchar(100),
		Unit varchar(100)
	);
	"""
	)
print "Created EIA_CO2_Descrip"

#Pass data into new table
#doing below lets psycopg do the data conversion
#cur.execute("INSERT INTO test (num,data) VALUES (%s,%s)", (100,"abc,dfg"))
# MSN YYYYMM VALUE COLUMN_ORDER DESCRIPTION UNIT
EIA_Trans = "INSERT INTO EIA_CO2_Trans (MSN, Column_Order, YYYYMM, Value) VALUES (%s, %s, %s, %s);"
EIA_Elec = "INSERT INTO EIA_CO2_ELEC (MSN, Column_Order, YYYYMM, Value) VALUES (%s, %s, %s, %s);"
EIA_Mkwh = "INSERT INTO EIA_MKWH (MSN, Column_Order, YYYYMM, Value) VALUES (%s, %s, %s, %s);"
Descrip ="INSERT INTO Descrip (MSN, Description, Unit) VALUES (%s, %s, %s);"

csvObj = csv.reader(open("/home/cjnitta/ecs165a/EIA_CO2_Transportation_2015.csv"), delimiter = ',')
i = 0
ord = 0
for row in csvObj:
	if i > 0:
		if ord != row[3]:
			ord = row[3]
			descripLine = [row[0], row[4], row[5]]
			cur.execute(Descrip, descripLine)
		if is_number(row[2]):
			eiaLine = [row[0], row[3], int(row[1]), float(row[2])]
		else:
			eiaLine = [row[0], row[3], int(row[1]), None]
		#cur.execute(insertData, csvLine)
		cur.execute(EIA_Trans, eiaLine)
	i += 1

csvObj = csv.reader(open("/home/cjnitta/ecs165a/EIA_CO2_Electricity_2015.csv"), delimiter = ',')
i = 0
ord = 0
for row in csvObj:
	if i > 0:
		if ord != row[3]:
			ord = row[3]
			descripLine = [row[0],  row[4], row[5]]
			cur.execute(Descrip, descripLine)
		if is_number(row[2]):
			eiaLine = [row[0], row[3], int(row[1]), float(row[2])]
		else:
			eiaLine = [row[0], row[3], int(row[1]), None]
		#cur.execute(insertData, csvLine)
		cur.execute(EIA_Elec, eiaLine)
	i += 1

csvObj = csv.reader(open("/home/cjnitta/ecs165a/EIA_MkWh_2015.csv"), delimiter = ',')
i = 0
ord = 0
for row in csvObj:
	if i > 0:
		if ord != row[3]:
			ord = row[3]
			descripLine = [row[0], row[4], row[5]]
			cur.execute(Descrip, descripLine)
		if is_number(row[2]):
			eiaLine = [row[0], row[3], int(row[1]), float(row[2])]
		else:
			eiaLine = [row[0], row[3], int(row[1]), None]
		#cur.execute(insertData, csvLine)
		cur.execute(EIA_Mkwh, eiaLine)
	i += 1


#Query the database
#commits the changes to the database permanately
conn.commit()

#cur.execute("DROP TABLE EIA_CO2_Trans")
#close the communication to the database
cur.close()
conn.close()






