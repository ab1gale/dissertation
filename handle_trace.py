import re
fp=open('trace','r')
for i in xrange(11):
	print fp.readline(),

aline=fp.readline()
callsites=[]
while aline!="":
#for j in xrange(20):
	reqarr=re.findall("bytes_req=(\d+?)\s",aline)
	reqnum=int(reqarr[0],10)
	if reqnum<=32 and reqnum>16:
		callsitearr=re.findall("call_site=([0-9a-f]+?)\s",aline)
		callsites.append(callsitearr[0])
	aline=fp.readline()
callsiteset=set(callsites)
print len(callsiteset)
addr2call={}
funcaddrs=[]
fp2=open("kallsyms",'r')
for line in fp2.readlines():
	linearr=line.split(' ')
	addr=int(linearr[0],16)
	addr2call[addr]=linearr[2]
	funcaddrs.append(addr)
funcaddrs.sort()
#print funcaddrs
#print callsiteset
for callsite in callsiteset:
	calladdr=int(callsite,16)
	for i in xrange(len(funcaddrs)):
		if calladdr<=funcaddrs[i+1] and calladdr>funcaddrs[i]:
			if funcaddrs[i]==0xffffffff9647ee20:
				print "call:"+hex(calladdr)
			break
	#print addr2call[funcaddrs[i]], addr2call[funcaddrs[i+1]]
	#print addr2call[funcaddrs[i]],hex(funcaddrs[i])
	print addr2call[funcaddrs[i]],
