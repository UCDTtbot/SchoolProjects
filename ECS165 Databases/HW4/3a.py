import psycopg2
import os
import csv

def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

# connect to database ttbot with my user ttbot
conn = psycopg2.connect("dbname=postgres host = /home/" + os.environ['USER'] + "/postgres ")

#EXISTING SCHEMA:
#   Elec, Trans, Mkwh:
#    (
#        MSN varchar(10),
#        YYYYMM int,
#        Value real
#    );
#   Descrip:
#    (
#        MSN varchar(10),
#        Column_Order varchar(3),
#        Description varchar(100),
#        Unit varchar(100)
#    );
#Names: EIA_CO2_Trans, EIA_CO2_Elec, EIA_Mkwh, Descrip

daymap = {}
daymap[200803] = 31.0
daymap[200804] = 30.0
daymap[200805] = 31.0
daymap[200806] = 30.0
daymap[200807] = 31.0
daymap[200808] = 31.0
daymap[200809] = 30.0
daymap[200810] = 31.0
daymap[200811] = 30.0
daymap[200812] = 31.0
daymap[200901] = 31.0
daymap[200902] = 28.0
daymap[200903] = 31.0
daymap[200904] = 30.0



#Open a cursor to perform databse operations
cur = conn.cursor()
for o in range(5, 101, 5):
	cur.execute("""
		SELECT * FROM (
	    	SELECT HOUSEID, PERSONID, TDAYDATE, SUM(TRPMILES) AS total 
	    	FROM dayv2pub
	    	WHERE TRPMILES >= 0
	    	GROUP BY HOUSEID, PERSONID, TDAYDATE
	    ) AS q
	    WHERE q.total < """ + str(o)
	    )
	out = cur.fetchall()
	#out = cur.fetchmany(50)
	#for i in out:
	#    print i
	numer = 0.0
	for i in out:
		numer = numer + daymap[i[2]]
	cur.execute("""
	   	SELECT HOUSEID, PERSONID, TDAYDATE, SUM(TRPMILES) AS total 
	   	FROM dayv2pub
	   	WHERE TRPMILES >= 0
	   	GROUP BY HOUSEID, PERSONID, TDAYDATE 
	    """)
	out = cur.fetchall()
	denom = 0.0
	for i in out:
		denom = denom + daymap[i[2]]

	ans = (numer/denom) * 100
	print "Percent who traveled less than " + str(o) + " miles is: " + "{0:.2f}%".format(ans)
 
#commits the changes to the database permanately
#conn.commit()

#cur.execute("DROP TABLE EIA_CO2_Trans")
#close the communication to the database
cur.close()
conn.close()






