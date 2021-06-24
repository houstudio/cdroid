if [ -f 3rdtest.tar.gz ];then
   rm 3rdtest.tar.gz
fi
if [  -d porting ];then
   rm porting -rf
fi
cp ../porting ./ -R

tar -czvf 3rdtest.tar.gz * --exclude=out --exclude=3rdtest.tar.gz
