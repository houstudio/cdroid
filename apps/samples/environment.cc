#include <porting/cdlog.h>
#include <core/environment.h>
using namespace cdroid;
int main(int argc,char*argv[]){
    LOG(INFO)<<" getRootDirectory="<<Environment::getRootDirectory();
    LOG(INFO)<<" getStorageDirectory="<<Environment::getStorageDirectory();
    LOG(INFO)<<" getOemDirectory="<<Environment::getOemDirectory();
    LOG(INFO)<<" getOdmDirectory="<<Environment::getOdmDirectory();
    LOG(INFO)<<" getVendorDirectory="<<Environment::getVendorDirectory();
    LOG(INFO)<<"getProductDirectory="<<Environment::getProductDirectory();
    LOG(INFO)<<"getDataDirectory="<<Environment::getDataDirectory();
    LOG(INFO)<<"getDownloadCacheDirectory="<<Environment::getDownloadCacheDirectory();
    LOG(INFO)<<"getExpandDirectory="<<Environment::getExpandDirectory();
    LOG(INFO)<<"getDataSystemDirectory="<<Environment::getDataSystemDirectory();
    LOG(INFO)<<"getDataMiscDeDirectory="<<Environment::getDataMiscDeDirectory(101);
    LOG(INFO)<<"getDataPreloadsFileCacheDirectory="<<Environment::getDataPreloadsFileCacheDirectory("ds");
    LOG(INFO)<<"getDataPreloadsMediaDirectory="<<Environment::getDataPreloadsMediaDirectory();
}
