#transform the data files provided by Scott into files importable into
#the MySQL data model.
import sys
import csv

#Transforms pat-to-cpc CSV files into CSV for mysql import.
#example:
#   t_pat2cpc('pat2cpc-sample-results.csv',
#             open('t.pat2cpc.csv','w'))
#Similarly for each "t_" function.
def t_pat2cpc(infile='pat2cpc-sample.csv',outfile=sys.stdout):
    oldout = sys.stdout
    sys.stdout = outfile
    def filterseq(l):
        a = 5
        i = 0
        while i < l-5:
            if (a-4)%5 != 0:
                yield a
            a += 1
            i += 1

    data = csv.reader(open(infile))
    #fields = data.next() #header

    for row in data:
        imax = len(row) - 5 - 2
        trow = row[0] + ','
        for i,val in enumerate([row[x] for x in filterseq(len(row))]):
            if (i+1)%4 == 0: trow += '/'
            trow += val
            if (i+1)%4 == 0: trow += '|'
        print(trow.strip(',').replace(' ','').strip('|'))

    sys.stdout = oldout
    print('FINISHED!')

def t_records(infile='records.csv',outrec=sys.stdout,outcase=sys.stdout):
    def printrec(r):
        if r:
            outrec.write('%s,%s %s\n' % (r['record.no'],
                                       r['name.first'],
                                       r['name.last']))
    def printcase(c):
        if c:
            c = { k:v.replace(',','') for k,v in c.iteritems() }
            c = { k:v.replace(' ','') for k,v in c.iteritems() }
            outcase.write('%s,%s,%s,%s,%s,%s,%s\n' % (c['case.no'],
                                                      c['record.no'],
                                                      c['court'],
                                                      c['plaintiff'],
                                                      c['defendant'],
                                                      c['cpc'],
                                                      c['patent']))
    data = csv.reader(open(infile))
    record = { }
    case = { }
    icase = 0
    irec = 0
    for i,row in enumerate(data):
        row = [filter(None,row)]
        for k,v in row:
            if k == 'record.no':
                irec += 1
                if irec > 1:
                    printcase(case)
                    case.clear()
                    printrec(record)
                    record.clear()
                    icase = 0
                record[k] = v
            if k == 'case.no':
                icase += 1
                if icase > 1:
                    printcase(case)
                    case.clear()
                case['record.no'] = record['record.no']
            if k not in ['name.last','name.first']:
                if k in case: case[k] += '|' + v
                else: case[k] = v
            else:
                record[k] = v
    printrec(record)
    printcase(case)
