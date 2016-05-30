import datetime
import os
time_start= datetime.datetime(2016,5, 23, 15,28,00,0)

while (time_start-datetime.datetime.now()).total_seconds()>=0:
	pass
os.system("make sniff")
