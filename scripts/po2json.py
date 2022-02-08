#/usr/bin/env python
#coding=utf8
#created by zhhou
import polib
import requests
import random
import json
from hashlib import md5
import time
import xlrd  #excel read
import xlwt  #excel write
import sys, getopt

appid = '20210610000859637' #Your APPID
secretKey = '7smZUeByLt4BJd5HqBMR' #Your SECURITY KEY
fromLang='en'
url='http://api.fanyi.baidu.com/api/trans/vip/translate'
headers = {'Content-Type': 'application/x-www-form-urlencoded'}
payload = {'appid': appid, 'from': 'en'}

def make_md5(s, encoding='utf-8'):
    return md5(s.encode(encoding)).hexdigest()

def potranslate(pofile,tolan,sheet):
    dict={}
    payload['to']=tolan
    salt = random.randint(32768, 65536)
    payload['salt']=salt
    r= requests.post(url,params=payload,headers=headers)
    po = polib.pofile(pofile) #'/home/houzh/Miniwin/out-x86/src/apps/ntvplus/ntvplus.po')
    i=1
    sheet.protect = True
    sheet.password='NgLHello321;'
    editable = xlwt.easyxf("protection: cell_locked false;")
    read_only = xlwt.easyxf("")
    sheet.write(0,0,'string id',read_only)
    sheet.write(0,1,'string context',read_only)
    for entry in po:
        #print entry.msgid, entry.msgstr
        q=entry.msgid
        payload['q']=q
        sign = make_md5(appid + q + str(salt) + secretKey)
        payload['sign']=sign
        r=requests.post(url,params=payload,headers=headers)
        result = r.json()
        rstrain= result['trans_result']
        sheet.write(i,0,q,read_only)
        if len(rstrain)>0 :
           dict[q]=rstrain[0]['dst']
           sheet.write(i,1,dict[q],editable)
           print(q+' ==> '+dict[q])
        i=i+1
        #print dict
        time.sleep(1)  #base version can call 1 per second(1QPS),advance version 10 QPS. change che sleep time to match diffrent version
    js=json.dumps(dict)
    return dict

#for language code,pls refto: https://api.fanyi.baidu.com/product/113
tolans=['pt'] #,'th','bul','fin','fra','ru','zh']

def po2json(pofile,tolans):
   xls = xlwt.Workbook();
   for lan in tolans :
       sheet = xls.add_sheet(lan,cell_overwrite_ok=True)
       sheet.col(0).width=256*50
       sheet.col(1).width=256*80
       print("************Translate to "+lan+"**************")
       dict=potranslate(pofile,lan,sheet)
       fp=open('strings-'+lan+'.json','w')
       data=json.dumps(dict,indent=4) #,encoding="utf-8")
       fp.write(data)
       fp.close()
   xls.save(sys.argv[1]+'.xls')
   del xls

def xls2json(fname): ##convert xml.sheet to json string resource
   xls=xlrd.open_workbook(fname)
   for sheet in xls.sheets():
       print(sheet.name)
       strdict={}
       for  i in range(sheet.nrows):
           strid=sheet.cell_value(i,0).encode('utf-8')
           strtxt=sheet.cell_value(i,1).encode('utf-8')
           print strid+'-->'+strtxt
           strdict[strid]=strtxt
       js=json.dumps(strdict,indent=4)
       fjs=open('strings-'+sheet.name+'.json','w')
       fjs.write(js)
       fjs.close()
   del xls

if __name__== "__main__":
   fname=sys.argv[1]
   if fname.endswith('po') :
      po2json(fname,tolans)
   else:
      xls2json(fname)
