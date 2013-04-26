patent-tools
============

Scrape, score, and expose patent-witness data. These tools (1) extract
Cooperative Patent Classification codes, section/scheme/class/group,
(2) use the encoded information to build relationship weights between
different groups, (3) provide patent-to-CPC and witness-to-CPC
information, and (4) expose the information via a database-driven
search engine that finds the best expert witness for a particular
patent number.  Ultimately, these tools will feed into a larger system
of practical tools that allow easier access to various resources
useful for patent litigation.

I. scrape-uspto
II. score-uspto
III. search-engine
IV. data transformation/migration notes

scrape-uspto
============

Extracts USPTO website data.

* scrape-uspto-requirements.txt -- job description per oDesk posting.
* scrape-uspto-requirements-att.xls -- attachment to job description.
* scrape-uspto.py -- one-shot utility to scrape uspto web site for patent data
  according to scrape-uspto-requirements.txt.  outfile = uspto.out.
  * requires module BeautifulSoup to be installed (e.g., sudo pip install
    BeautifulSoup).
  * no error checking; dead links throw error without continuing to next link.
* scrape-uspto.input -- required plain-text input file for scrape-uspto.py; one
  number per line.
* execute: python scrape-uspto.py (Python3 was used)
* scrape-uspto-ouput.zip -- scraped data (as of 2013/03/31).

score-uspto
============

Scores each group within a single section according to its subject relevance to
each other group.

* score-uspto.cpp -- scoring algorithm.  requires C++11 due to use of
  unsorted_map. if C++11 is not supported, the unsorted_map could be converted
  to a traditional map or the external library TR1 could be used.
  * compile:
    > g++ -std=c++11 score-uspto.cpp -o score
* score-uspto-requirements.txt -- job description per oDesk posting.
* score-uspto-requirements.pdf -- attachment to job description. originally
  called "Workflow."
* score-uspto-tests -- directory contains test cases in the form of test.input
  and test.expected. test.results are the last results.
  * execute and make sure there are no differences
    > score test.input test.results > test.log
    > diff test.results test.expected
* score-uspto-input -- scraped data already divided into sections.
  * format: id,cpc,level. no header row.
  * cpc: Xsssc[*]/gg[*], where X = section, sss = scheme, c[*] = class, gg[*] =
    group
  * execute and monitor example:
    > score score-uspto-input/A.input A.csv > A.log &
    > tail -f A.log
* input, results, and logs (as of 2013/04/07) at
  https://dl.dropbox.com/u/10329615/uspto-scores.zip
* score-uspto-logs -- all logs from 2013/04/07 run

search-engine
============

(in-progress) Patent-witness matching database and web application. Project includes data transformation and migration information along with a deployable web2py application.

* rfp folder -- documents provided in RFP, including requirements document, sample data, file layout descriptions
* rfp-response.(pdf/odt) -- snapshot of response to RFP in LibreOffice and PDF formats; proposed content is always superseded by more recent design documents and implementation comments
* erd.(dia/png), sequence.(dia/png) -- editable dia files; png files used by rfp-response.
* t.*.py -- python scripts for transforming data into format importable into application's data model
* t.cpc.cpp -- C++11 program for transforming cpc flat-file data into application's data model; adapted from score-uspto.cpp, since it already calculated a node path for each CPC; node path offers alternative to storing all scores (i.e., recalculate score from relative node positions)

DATA TRANSFORMATION/MIGRATION NOTES (2013/04/14)
================================================
$ := bash prompt
>>> := python prompt

1. concatenate records into one file:
$ cat rfp/record\ \(*\).csv > rfp/records.csv

2. load python transformation functions in pyton 2.7:
$ python2 -i t.data.py

3. invoke corresponding functions:
>>> t_pat2cpc('rfp/pat2cpc-sample-result.csv',open('t.pat.csv','w'))
>>> t_records('rfp/records.csv',open('t.record.csv','w'),open('t.pat_case.csv','w'))

4. compile t.cpc.cpp with C++11 compiler:
$ g++ --std=c++11 -o t.cpc t.cpc.cpp

5. invoke t.cpc with input and output files:
$ t.cpc rfp/cpc\ table\ -\ sample\ -\ \(group\ D\).csv t.cpc.D.csv

6. upload files to server
7. import into MySQL

$ mysql -uevanphelps -hmysql.server -p

mysql> LOAD DATA LOCAL INFILE 't.record.csv' INTO TABLE record COLUMNS TERMINATED BY ',' LINES TERMINATED BY '\n';

mysql> LOAD DATA LOCAL INFILE 't.pat_case.csv' INTO TABLE pat_case COLUMNS TERMiNATED BY ',' LINES TERMINATED BY '\n' (case_num,rec_num,court,plaintiff,defendent,cpcs,pats);

mysql> LOAD DATA LOCAL INFILE 't.cpc.D.csv' INTO TABLE cpc COLUMNS TERMINATED BY ',' LINES TERMINATED BY '\n' (id,cpc,sect,schem,clas,grp,lvl,n0,n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12);

mysql> LOAD DATA LOCAL INFILE 't.pat.csv' INTO TABLE pat COLUMNS TERMINATED BY ',' LINES TERMINATED BY '\n' (pat_num,cpcs);

