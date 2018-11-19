#!/usr/bin/env python
import datetime
import cgi, os, time,sys

form = cgi.FieldStorage()
val = form.getvalue('val')

if val:
    data_view=(int)(val)
else:
    data_view=300

def file_len(fname):
    with open(fname) as f:
        for i, l in enumerate(f):
            pass
    return i + 1

dataFile = '/tmp/data.csv'

print """
<html>
  <head>
    <script type="text/javascript" src="https://www.google.com/jsapi"></script>
    <script type="text/javascript">
      google.load("visualization", "1", {packages:["corechart"]});
      google.setOnLoadCallback(drawChart);
      function drawChart() {
        var data = google.visualization.arrayToDataTable([
          ['heure', 'niveau']"""

nbdata=file_len(dataFile)

if data_view > nbdata:
    data_view = nbdata
if data_view < 1:
    data_view = 1
    
with open(dataFile) as data:
    for i in range(0,nbdata - data_view):
        line = data.readline()
    line = data.readline()
    while line:
        line_data = line.split(",")
        timestamp = (int)(line_data[0])
        data_date = datetime.datetime.fromtimestamp(timestamp)

        if line_data[1][0].isdigit():
            print "         ,['%s', %d]" % (data_date.strftime('%H:%M:%S'),(int)(line_data[1]))

        line = data.readline()

print """\
        ]);
        var options = {
          title: 'lumiere',
          hAxis: {title: 'heure',  titleTextStyle: {color: '#333'}},
          vAxis: {minValue: 0, maxValue: 1000}
        };
        var chart = new google.visualization.AreaChart(document.getElementById('chart_div'));
        chart.draw(data, options);
      }
    </script>
    <!-- <META HTTP-EQUIV="refresh" CONTENT="10"> -->
  </head>
  <body>
    <div id="chart_div" style="width: 600; height: 300px;"></div>
<form method="POST" action="main.py">
  <input name="val" cols="20"></input>
  <input type="submit" value="Entrer">
"""
print "Nb: %s" % (nbdata)
print """
</form>
  </body>
</html>
"""
