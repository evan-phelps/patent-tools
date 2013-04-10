from bs4 import BeautifulSoup
from urllib.request import urlopen


nums =  [line.strip() for line in open('scrape-uspto.input','r')]
urltmpl = 'http://www.uspto.gov/web/patents/classification/cpc/html/us%stocpc.html'

outf = open('uspto.out','w',1)

for i,n in enumerate(nums):
    url = urlopen(urltmpl % n.zfill(3))

    htmldoc = url.read()

    soup = BeautifulSoup(htmldoc)

    groups = []
    titles = soup.find_all('th')
    for t in titles:
        if 'USPC Subclass' == t.text:
            table = t.parent.parent
            for row in table.find_all('tr'):
                cells = row.find_all('td')
                data = []
                for c in cells: data.append(c.text.strip(' more...'))
                outf.write(','.join(data)+'\n')
    print(n)
