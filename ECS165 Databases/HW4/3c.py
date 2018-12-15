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
#for o in range(5, 10, 5):
for k in daymap.keys():
    cur.execute("""
        SELECT HOUSEID, TDAYDATE, VEHID, SUM(TRPMILES) as trip
        FROM dayv2pub
        WHERE TRPMILES >= 0 AND VEHID >= 1 AND TDAYDATE = """ + str(k) + """
        GROUP BY HOUSEID, TDAYDATE, VEHID
        """)
    out = cur.fetchall()

    cur.execute("""
        SELECT EPATMPG, VEHID FROM vehv2pub
    """)
    epa = cur.fetchall()

    mpgmap = {}
    for v in epa:
        mpgmap[v[1]] = v[0]

    houses = {}
    for o in out:
        if o[0] in houses:
            houses[o[0]] += (o[3] / mpgmap[o[2]]) * daymap[o[1]]
        else:
            houses[o[0]] = (o[3] / mpgmap[o[2]]) * daymap[o[1]]
        
    cur.execute("""
        SELECT COUNT(HOUSEID) FROM hhv2pub WHERE TDAYDATE = """ + str(k)
        )
    totHouse = cur.fetchone()
    totH = totHouse[0]

    cur.execute("""
        SELECT value FROM EIA_CO2_Trans
        WHERE YYYYMM = """ + str(k) + """ AND Column_Order = 12
        """)
    total = cur.fetchone();
    totalMet = total[0] * 1000000

    gasTot = 0
    for h in houses.keys():
        #print houses[h]
        gasTot += houses[h]
    #print gasTot / totH

    ans = ((gasTot * (117538000 / totH) * 0.008887) / totalMet) * 100
    #print str(gasTot * 117538000 / totH * 0.008887)
    #print totalMet
    print "For Year/Month " + str(k) + ": " + "{0:.2f}%".format(ans)


#out = cur.fetchmany(50)
#for i in out:
#    print i
#numer = 0.0
#for i in out:
#   numer = numer + daymap[i[2]]
#cur.execute("""
#       SELECT HOUSEID, PERSONID, TDAYDATE, SUM(TRPMILES) AS total 
#       FROM dayv2pub
#       WHERE TRPMILES >= 0
#       GROUP BY HOUSEID, PERSONID, TDAYDATE 
#    """)
#out = cur.fetchall()
#denom = 0.0
#for i in out:
#   denom = denom + daymap[i[2]]

#   ans = (numer/denom) * 100
#   print "Percent who traveled less than " + str(o) + " miles is: " + "{0:.2f}%".format(ans)
 
#commits the changes to the database permanately
#conn.commit()

#cur.execute("DROP TABLE EIA_CO2_Trans")
#close the communication to the database
cur.close()
conn.close()






