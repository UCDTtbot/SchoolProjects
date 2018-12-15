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
cur.execute("""
    SELECT HOUSEID, TDAYDATE, VEHID, SUM(TRPMILES) as trip
    FROM dayv2pub
    WHERE TRPMILES >= 0 AND VEHID >= 1 
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
    SELECT COUNT(HOUSEID) FROM hhv2pub 
    """)
totHouse = cur.fetchone()
totH = totHouse[0]

total = 0
for k in daymap.keys():
    cur.execute("""
        SELECT value FROM EIA_CO2_Trans
        WHERE YYYYMM = """ + str(k) + """ AND Column_Order = 12
        """)
    data = cur.fetchone()
    total += data[0]
totalMet = total * 1000000

gasTot = 0
for h in houses.keys():
    #print houses[h]
    gasTot += houses[h]
#print gasTot / totH

co2_normal = ((gasTot * (117538000 / totH) * 0.008887)) 

for eMile in range(20, 61, 20):
    MKWHmap = {}
    for e in epa:
        MKWHmap[e[1]] = e[0] * 0.090634441

    elecUse = 0
    galUse = 0
    for c in out:
        if c[3] > eMile:
            mpgMiles = c[3] - eMile
            galUse += (mpgMiles / mpgmap[c[2]]) * daymap[c[1]]
            elecUse += (eMile / MKWHmap[c[2]]) * daymap[c[1]]
        else: 
            elecUse += (c[3] / MKWHmap[c[2]]) * daymap[c[1]]

    totalCO2 = 0
    for k in daymap.keys():
        cur.execute("""
            SELECT value FROM EIA_CO2_Elec
            WHERE YYYYMM = """ + str(k) + """ AND Column_Order = 9
            """)
        co2 = cur.fetchone()
        totalCO2 += co2[0]
    
    totalMKWH = 0
    for k in daymap.keys():
        cur.execute("""
            SELECT value FROM EIA_Mkwh
            WHERE YYYYMM = """ + str(k) + """ AND Column_Order = 13
            """)
        mkwh = cur.fetchone()
        totalMKWH += mkwh[0]

    co2Phour = totalCO2 / totalMKWH

    elecCO2 = elecUse * (117538000 / totH) * co2Phour
    galCO2 = galUse * (117538000 / totH) * 0.008887

    delta = co2_normal - (elecCO2 + galCO2)
    print delta
#PETROLEUM

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






